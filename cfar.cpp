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
#include <sstream>
#include <string>
#include <math.h>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>
using namespace std;







double long pi=3.14159265;



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

   double long threshold_calculator(double long PFA)
   {
	   double long g = 1-2*PFA;
	   double long f = sqrt(2*pi)*0.5*(g+((pi/12)*g*g*g)+(((7*pi*pi)/480)*g*g*g*g*g));
	   return f;
   }

   string sconverter(int i)
   {
	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
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

bool CFAR::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("Result", NULL);
   return true;
}

bool CFAR::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("CFAR", "app5", "14530294-1F98-486A-9336-B7AC7DC768FB");
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
   FactoryResource<DataRequest> pRequest2;

   pRequest->setInterleaveFormat(BSQ);
   pRequest2->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   DataAccessor pAcc2 = pCube->getDataAccessor(pRequest2.release());

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
   double long PFA = 0.0;
   unsigned int DEPTH = 2;
   int count=0;
   unsigned int zero=0;

   QStringList Names("0.000001");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a PFA", "Input a PFA as a DOUBLE", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   stm >> PFA;
   

   double long threshold = threshold_calculator(PFA);

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

		  westCol=max(col-DEPTH,zero);
		  northRow=max(row-DEPTH,zero);
		  eastCol=min(colSize-1,col+DEPTH);
		  southRow=min(rowSize-1,row+DEPTH);
		  prevCol=max(col-1,zero);
		  prevRow=max(row-1,zero);
		  nextCol=min(col+1,colSize-1);
		  nextRow=min(row+1,rowSize-1);

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
			zstatistic = (pAcc2->getColumnAsDouble()-mean)/std;
				std::string s;
				std::stringstream out;
				out << zstatistic;
				s = out.str();
	
			pProgress->updateProgress(s,threshold, ERRORS);
			if(zstatistic>threshold)
			{
				switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc->getColumn(), 1000.0);
			}

			else
			{
				switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc->getColumn(), 0.0);
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