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
#include "MORPH.h"
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
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "GcpList.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
using namespace std;



REGISTER_PLUGIN_BASIC(OpticksTutorial, MORPH);

namespace
{
   template<typename T>
   void conversion(T* pData, double number)
   {
	   *pData=number;
   }
};


MORPH::MORPH()
{
   setDescriptorId("{F3CBF6C1-E3E5-4908-9280-BE94008DD021}");
   setName("MORPH");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/MORPH");
   setAbortSupported(true);
}



bool MORPH::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>("Result");
   return true;
}

bool MORPH::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result1", NULL);
   return true;
}

bool MORPH::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("MORPH", "app8", "7F2FE71B-794C-4AC3-B3E7-0B44B3B4A9EE");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>("Result");
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
   "MORPH result", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));

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
   DataAccessor pDestAcc2 = pResultCube->getDataAccessor(pResultRequest.release());
  
   int upperLeftVal = 0;
   int upVal = 0;
   int upperRightVal = 0;
   int leftVal = 0;
   int rightVal = 0;
   int lowerLeftVal = 0;
   int downVal = 0;
   int lowerRightVal = 0;

   int rowSize=pDesc->getRowCount();
   int colSize=pDesc->getColumnCount();
   int prevCol = 0;
   int prevRow = 0;
   int nextCol = 0;
   int nextRow = 0;
   int count=0;
   int zero=0;
   int threshold = 0;
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pCube->getDataDescriptor());

   QStringList Names("3");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   //stm >> PFA;
   
   stm >> threshold;
   

   //double long threshold = threshold_calculator(PFA);

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

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", row * 100 / pDesc->getRowCount(), NORMAL);
      }
	  		
	  

      for (int col = 0; col < colSize; ++col)
      {
		  prevCol=max(col-1,zero);
		  prevRow=max(row-1,zero);
		  nextCol=min(col+1,colSize-1);
		  nextRow=min(row+1,rowSize-1);

	  pAcc2->toPixel(prevRow, prevCol);
      upperLeftVal = pAcc2->getColumnAsDouble();
	  	  if(upperLeftVal>0)
	  {
			  count+=1;
	  }

	  

      pAcc2->toPixel(prevRow, col);
      upVal = pAcc2->getColumnAsDouble();
	  if(upVal>0)
	  {
			  count+=1;
	  }


      pAcc2->toPixel(prevRow, nextCol);
      upperRightVal = pAcc2->getColumnAsDouble();
	  	  if(upperRightVal>0)
		  {
			  count+=1;
		  }

      pAcc2->toPixel(row, prevCol);
      leftVal = pAcc2->getColumnAsDouble();
	  	  if(leftVal>0)
		  {
			  count+=1;
		  }

      pAcc2->toPixel(row, nextCol);
      rightVal = pAcc2->getColumnAsDouble();
	  	  if(rightVal>0)
		  {
			  count+=1;
		  }

      pAcc2->toPixel(nextRow, prevCol);
      lowerLeftVal = pAcc2->getColumnAsDouble();
	  	  if(lowerLeftVal>0)
		  {
			  count+=1;
		  }

      pAcc2->toPixel(nextRow, col);
      downVal = pAcc2->getColumnAsDouble();
	  	  if(downVal>0)
		  {
			  count+=1;
		  }

      pAcc2->toPixel(nextRow, nextCol);
      lowerRightVal = pAcc2->getColumnAsDouble();
	  	  if(lowerRightVal>0)
		  {
			  count+=1;
		  }



		  std::string s;
		std::stringstream out;
		out << count;
		s = out.str();
		//pProgress->updateProgress(s, 0, ERRORS);



	  pDestAcc2->toPixel(row,col);

      if(count>threshold)
	  {
		  switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc2->getColumn(), 1000.0);
	  }

	  else
	  {
		  switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc2->getColumn(), 0.0);
	  }

	  count=0;

			pAcc->nextColumn();
	  }
      pAcc->nextRow();

   }


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
	  

	  	  // Create the GCP list
	  if (pCube->isGeoreferenced() == true)
	  {

      const vector<DimensionDescriptor>& rows = pDescriptor->getRows();
      const vector<DimensionDescriptor>& columns = pDescriptor->getColumns();
      if ((rows.empty() == false) && (columns.empty() == false))
      {
         // Get the geocoordinates at the chip corners
		  /*
         VERIFYNRV(rows.front().isActiveNumberValid() == true);
         VERIFYNRV(rows.back().isActiveNumberValid() == true);
         VERIFYNRV(columns.front().isActiveNumberValid() == true);
         VERIFYNRV(columns.back().isActiveNumberValid() == true);
		 */

         unsigned int startRow = rows.front().getActiveNumber();
         unsigned int endRow = rows.back().getActiveNumber();
         unsigned int startCol = columns.front().getActiveNumber();
         unsigned int endCol = columns.back().getActiveNumber();

         GcpPoint ulPoint;
         ulPoint.mPixel = LocationType(startCol, startRow);
         ulPoint.mCoordinate = pCube->convertPixelToGeocoord(ulPoint.mPixel);

         GcpPoint urPoint;
         urPoint.mPixel = LocationType(endCol, startRow);
         urPoint.mCoordinate = pCube->convertPixelToGeocoord(urPoint.mPixel);

         GcpPoint llPoint;
         llPoint.mPixel = LocationType(startCol, endRow);
         llPoint.mCoordinate = pCube->convertPixelToGeocoord(llPoint.mPixel);

         GcpPoint lrPoint;
         lrPoint.mPixel = LocationType(endCol, endRow);
         lrPoint.mCoordinate = pCube->convertPixelToGeocoord(lrPoint.mPixel);

         GcpPoint centerPoint;
         centerPoint.mPixel = LocationType((startCol + endCol) / 2, (startRow + endRow) / 2);
         centerPoint.mCoordinate = pCube->convertPixelToGeocoord(centerPoint.mPixel);

		 /*
         // Reset the coordinates to be in active numbers relative to the chip
         const vector<DimensionDescriptor>& chipRows = pDescriptor->getRows();
         const vector<DimensionDescriptor>& chipColumns = pDescriptor->getColumns();
		 
         VERIFYNRV(chipRows.front().isActiveNumberValid() == true);
         VERIFYNRV(chipRows.back().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.front().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.back().isActiveNumberValid() == true);
		 
         unsigned int chipStartRow = chipRows.front().getActiveNumber();
         unsigned int chipEndRow = chipRows.back().getActiveNumber();
         unsigned int chipStartCol = chipColumns.front().getActiveNumber();
         unsigned int chipEndCol = chipColumns.back().getActiveNumber();
         ulPoint.mPixel = LocationType(chipStartCol, chipStartRow);
         urPoint.mPixel = LocationType(chipEndCol, chipStartRow);
         llPoint.mPixel = LocationType(chipStartCol, chipEndRow);
         lrPoint.mPixel = LocationType(chipEndCol, chipEndRow);
         centerPoint.mPixel = LocationType((chipStartCol + chipEndCol) / 2, (chipStartRow + chipEndRow) / 2);
		 */
         
         Service<ModelServices> pModel;

         GcpList* pGcpList = static_cast<GcpList*>(pModel->createElement("Corner Coordinates",
            TypeConverter::toString<GcpList>(), pResultCube.get()));
         if (pGcpList != NULL)
         {
            list<GcpPoint> gcps;
            gcps.push_back(ulPoint);
            gcps.push_back(urPoint);
            gcps.push_back(llPoint);
            gcps.push_back(lrPoint);
            gcps.push_back(centerPoint);

            pGcpList->addPoints(gcps);

			pView->createLayer(GCP_LAYER, pGcpList);
		 }
	  }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Morph is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Result1", pResultCube.release());

   pStep->finalize();
   return true;
   
}