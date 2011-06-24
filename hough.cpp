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
#include "hough.h"
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




REGISTER_PLUGIN_BASIC(OpticksTutorial, HOUGH);


namespace
{
   template<typename T>
   void conversion2(T* pData, double number)
   {
	   *pData=number;
   }
};



HOUGH::HOUGH()
{
   setDescriptorId("{C2720C19-DB6F-43CE-9B2C-1B2BB19BE387}");
   setName("HOUGH");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/HOUGH");
   setAbortSupported(true);
}



bool HOUGH::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>("Image1");
   pInArgList->addArg<RasterElement>("Image2");


   return true;
}

bool HOUGH::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool HOUGH::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("HOUGH", "app5", "ABD85C25-93CF-4249-AFC2-9DBAAFF2ACAB");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>("Image1");
   RasterElement* pCube1 = pInArgList->getPlugInArgValue<RasterElement>("Image2");
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

      if (pCube1 == NULL)
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
   RasterDataDescriptor* pDesc1 = static_cast<RasterDataDescriptor*>(pCube1->getDataDescriptor());
   VERIFY(pDesc != NULL);
   VERIFY(pDesc1 != NULL);
   FactoryResource<DataRequest> pRequest;
   FactoryResource<DataRequest> pRequest2;

   pRequest->setInterleaveFormat(BSQ);
   pRequest2->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   DataAccessor pAcc2 = pCube1->getDataAccessor(pRequest2.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
   "hough result", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));

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
   unsigned int total=0;
   int count = 0;

   for (int row = 0; row < rowSize; ++row)
   {
	   for (int col = 0; col < colSize; ++col)
	   {
		   pAcc2->toPixel(row,col);
		   total+=pAcc2->getColumnAsInteger();
		   count+=1;
	   }
   }

   int mean=total/count;

   pAcc2->toPixel(0,0);

   for (int row = 0; row < rowSize; ++row)
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

	        if (!pAcc2.isValid())
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
         pProgress->updateProgress("Calculating statistics", 0.5*row * 100 / pDesc->getRowCount(), NORMAL);
      }
	  		
      for (int col = 0; col < colSize; ++col)
      {
		  if(pAcc->getColumnAsInteger()!=0)
		  {
			  pDestAcc->toPixel(row,col);
			  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), mean);
		  }

		  else
		  {
			  pDestAcc->toPixel(row,col);
			  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), pAcc2->getColumnAsInteger());
		  }


			pAcc->nextColumn();
			pAcc2->nextColumn();
	  }
      pAcc->nextRow();
	  pAcc2->nextRow();

   }

/*
   pDestAcc->toPixel(0,0);
   pAcc->toPixel(0,0);
   pAcc2->toPixel(0,0);

   for (int row = 0; row < rowSize; ++row)
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

	        if (!pAcc2.isValid())
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
         pProgress->updateProgress("Calculating statistics", (100-(0.5* row * 100 / pDesc->getRowCount())), NORMAL);
      }
	  
	  		
      for (int col = 0; col < colSize; ++col)
      {
		  double number = ((pAcc2->getColumnAsInteger())/(pAcc->getColumnAsInteger()));
		  double angle = atan(number)*180/3.14159265;

		  if(angle<22.5 && angle>=-22.5)
		  {
			  angle=0;
		  }

		  if(angle>=22.5 && angle<67.5)
		  {
			  angle=45;
		  }

		  if(angle>=67.5 && angle<=90)
		  {
			  angle=90;
		  }

		  if(angle>=-90 && angle<-67.5)
		  {
			  angle=-90;
		  }

		  if(angle>=-67.5 && angle<-22.5)
		  {
			  angle=-45;
		  }

		  if(angle==90 || angle==-90)
		  {
			  pDestAcc->toPixel(row-1,col);
			  double a = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row+1,col);
			  double b = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row,col);
			  if(pDestAcc->getColumnAsDouble()>a && pDestAcc->getColumnAsDouble()>b)
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 1000);
			  }

			  else
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 0);
			  }
		  }

		  if(angle==0)
		  {
			  pDestAcc->toPixel(row,col+1);
			  double a = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row,col-1);
			  double b = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row,col);
			  if(pDestAcc->getColumnAsDouble()>a && pDestAcc->getColumnAsDouble()>b)
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 1000);
			  }

			  else
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 0);
			  }
		  }

		  	if(angle==45)
		  {
			  pDestAcc->toPixel(row-1,col+1);
			  double a = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row+1,col-1);
			  double b = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row,col);
			  if(pDestAcc->getColumnAsDouble()>a && pDestAcc->getColumnAsDouble()>b)
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 1000);
			  }

			  else
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 0);
			  }
		  }

				  if(angle==-45)
		  {
			  pDestAcc->toPixel(row-1,col-1);
			  double a = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row+1,col+1);
			  double b = pDestAcc->getColumnAsDouble();
			  pDestAcc->toPixel(row,col);
			  if(pDestAcc->getColumnAsDouble()>a && pDestAcc->getColumnAsDouble()>b)
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 1000);
			  }

			  else
			  {
				  switchOnEncoding(pDesc->getDataType(), conversion2, pDestAcc->getColumn(), 0);
			  }
		  }

			pDestAcc->nextColumn();
			pAcc->nextColumn();
			pAcc2->nextColumn();
			pProgress->updateProgress("FUCK YEAH!", 0, ERRORS);
	  }
      pDestAcc->nextRow();
	  pAcc->nextRow();
	  pAcc2->nextRow();
   }*/

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