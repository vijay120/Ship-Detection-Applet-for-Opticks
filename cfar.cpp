/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include "RasterUtilities.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "CFAR.h"
#include <limits>
#include "AoiElement.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "TypeConverter.h"
#include <limits>
#include <vector>
#include <algorithm>
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"



REGISTER_PLUGIN_BASIC(OpticksTutorial, CFAR);

namespace
{
   template<typename T>
   void conversion(T* pData, double number)
   {
	   *pData=number;
   }
};
   void updateStatistics2(double pData, double& total, double& total_sum, int& count)
   {
      total += pData;
	  total_sum += pData * pData;
	  count+=1;
   }


CFAR::CFAR()
{
   setDescriptorId("{3A013609-051D-4AE9-876E-6D3EA5DE0153}");
   setName("CFAR");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/CFAR");
   setAbortSupported(true);
}



bool CFAR::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "CFAR detection of image");
   return true;
}

bool CFAR::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("Result", NULL);
   return true;
}

bool CFAR::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("CFAR", "app", "27170298-10CE-4H6C-AD7A-97E8058C27FF");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (pCube == NULL)
   {
      std::string msg = "A raster cube must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   DataAccessor pAcc2 = pCube->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
   "CFAR result", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));
   if (pResultCube.get() == NULL)
   {
      std::string msg = "A raster cube could not be created.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   FactoryResource<DataRequest> pResultRequest;
   pResultRequest->setWritable(true);
   DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());
  
   int tester_count = 0;
   unsigned int eastCol = 0;
   unsigned int northRow = 0;
   unsigned int westCol = 0;
   unsigned int southRow = 0;
   double zstatistic = 0;
   double total = 0.0;
   double total_sum = 0.0;
   double mean = 0.0;
   double std = 0.0;
   double a=0;
   unsigned int rowSize=pDesc->getRowCount();
   unsigned int colSize=pDesc->getColumnCount();
   int prevCol = 0;
   int prevRow = 0;
   int nextCol = 0;
   int nextRow = 0;
   double threshold = 0.1;
   int DEPTH = 5;
   int count=0;

   for (unsigned int row = 0; row < rowSize; ++row)
   {
      if (isAborted())
      {
         std::string msg = getName() + " has been aborted.";
         pStep->finalize(Message::Abort, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ABORT);
         }

         return false;
      }
      if (!pAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }

         return false;
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", row * 100 / pDesc->getRowCount(), NORMAL);
      }

      for (unsigned int col = 0; col < colSize; ++col)
      {

			if(col-DEPTH>0)
			{
				westCol=col-DEPTH;
			}
			else
				westCol=0;

			if(row-DEPTH>0)
			{
				northRow=row-DEPTH;
			}
			else
				northRow=0;

			if(col+DEPTH>colSize-1)
			{
				eastCol=colSize-1;
			}
			else
				eastCol=col+DEPTH;

			if(row+DEPTH>rowSize-1)
			{
				southRow=rowSize-1;
			}
			else
				southRow=row+DEPTH;

			if(col-1>0)
			{
				prevCol=col-1;
			}
			else
				prevCol=0;

			if(row-1>0)
			{
				prevRow=row-1;
			}
			else
				prevRow=0;

			if(col+1>colSize-1)
			{
				nextCol=colSize-1;
			}
			else
				nextCol=col+1;

			if(row+1>rowSize-1)
			{
				nextRow=rowSize-1;
			}
			else
				nextRow=row+1;

			pAcc2->toPixel(northRow,westCol);
			
			for(unsigned int row1=northRow; row1 < southRow+1; ++row1)
			{
				for (unsigned int col1=westCol; col1 < eastCol+1; ++col1)
				{
					if(((col1==prevCol)&&(row1==prevRow)) ||
						((col1==col)&&(row1==prevRow)) ||
						((col1==nextCol)&&(row1==prevRow)) ||
						((col1==prevCol)&&(row1==row)) ||
						((col1==nextCol)&&(row1==row)) ||
						((col1==prevCol)&&(row1==nextRow)) ||
						((col1==col)&&(row1==nextRow)) ||
						((col1==nextCol)&&(row1==nextRow)) ||
						((col1==col)&&(row1==row)))
					{
						continue;
					}

					else
					{	   
					updateStatistics2(pAcc2->getColumnAsDouble(), total, total_sum, count);
					}

					pAcc2->nextColumn();

				}

				pAcc2->nextRow();
			}

			mean = total / count;
			std = sqrt(total_sum / count - mean * mean);
			pAcc2->toPixel(row,col);
			pDestAcc->toPixel(row,col);
			zstatistic = abs((pAcc2->getColumnAsDouble()-mean))/std;
			if(zstatistic>threshold)
			{
				switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc->getColumn(), 0.0);
			}

			else
			{
				switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc->getColumn(), 1000.0);
			}


			total = 0.0;
            total_sum = 0.0;
            mean = 0.0;
            std = 0.0;
			count=0;

			pAcc->nextColumn();
	  }
      pAcc->nextRow();

   }

      if (!isBatch())
   {
      Service<DesktopServices> pDesktop;

      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
         SPATIAL_DATA_WINDOW));

      SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
      if (pView == NULL)
      {
         std::string msg = "Unable to create view.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL) 
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         return false;
      }

      pView->setPrimaryRasterElement(pResultCube.get());
      pView->createLayer(RASTER, pResultCube.get());
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("CFAR is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("CFAR_result", pResultCube.release());

   pStep->finalize();
   return true;
}