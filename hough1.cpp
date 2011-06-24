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
#include "hough1.h"
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
#include "kht.h"



using namespace std;
REGISTER_PLUGIN_BASIC(OpticksTutorial, HOUGH1);


namespace
{
   template<typename T>
   void conversion3(T* pData, double number, int mean)
   {
	   if(number-mean>0)
	   {
		   *pData=number-mean;
	   }

	   else
		   *pData=0;
   }
};


HOUGH1::HOUGH1()
{
   setDescriptorId("{1C6F3DA9-E2E3-456D-A141-C094A327781F}");
   setName("hough1");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/HOUGH1");
   setAbortSupported(true);
}



bool HOUGH1::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "H detection of image");
   return true;
}

bool HOUGH1::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool HOUGH1::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("HOUGH1", "app5", "344DF28A-8FD0-40AA-9E12-CBCC2EE165DF");
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

   pProgress->updateProgress("ok:", 0, ERRORS);

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   FactoryResource<DataRequest> pRequest;
   //FactoryResource<DataRequest> pRequest2;

   pRequest->setInterleaveFormat(BSQ);
   //pRequest2->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   //DataAccessor pAcc2 = pCube->getDataAccessor(pRequest2.release());
   pProgress->updateProgress("what now", 0, ERRORS);

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
  
   int rowSize=pDesc->getRowCount();
   int colSize=pDesc->getColumnCount();
   double number=0.0;
   unsigned int total=0;
   int count=0;
   int prevCol=0;
   int prevRow=0;
   int nextCol=0;
   int nextRow=0;
   int zero=0;
   //pDestAcc->toPixel(0,0);

   for (int row = 0; row < rowSize; ++row)
   {

      for (int col = 0; col < colSize; ++col)
      {
		  prevCol=max(col-7,zero);
		  prevRow=max(row-7,zero);
		  nextCol=min(col+7,colSize-1);
		  nextRow=min(row+7,rowSize-1);

		  for(int row1=prevRow; row1 < nextRow+1; ++row1)
		  {						
				for (int col1=prevCol; col1 < nextCol+1; ++col1)
				{
					pAcc->toPixel(row1,col1);
					total+=pAcc->getColumnAsInteger();
					count+=1;
				}
		  }

		  int mean=total/count;

		  pAcc->toPixel(row,col);
		  pDestAcc->toPixel(row,col);
		  switchOnEncoding(pDesc->getDataType(), conversion3, pDestAcc->getColumn(), pAcc->getColumnAsInteger(), mean);

		  	total=0;
			count=0;
				
	  }
      //pAcc->nextRow();

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