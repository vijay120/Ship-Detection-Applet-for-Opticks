/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "switchOnEncoding.h"
#include "DENOISE.h"
#include <limits>
#include <string>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksTutorial, DENOISE);

namespace
{
   template<typename T>
   void conversion(T* pData, double number)
   {
	   *pData=number;
   }
};



   double edgeDetection6(DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize)
   {
      int prevCol = std::max(col - 1, 0);
      int prevRow = std::max(row - 1, 0);
      int nextCol = std::min(col + 1, colSize - 1);
      int nextRow = std::min(row + 1, rowSize - 1);

	  int prevCol1 = std::max(col-2,0);
	  int prevRow1= std::max(row-2,0);
	  int nextCol1= std::min(col+2,colSize-1);
	  int nextRow1= std::min(row+2,rowSize-1);

	  pSrcAcc->toPixel(prevRow1, prevCol1);
      int row1col1 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow1, prevCol);
      int row1col2 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow1, col);
      int row1col3 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow1, nextCol);
      int row1col4 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow1, nextCol1);
      int row1col5 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow, prevCol1);
      int row2col1 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow, prevCol);
      int row2col2 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow, col);
      int row2col3 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow, nextCol);
      int row2col4 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(prevRow, nextCol1);
      int row2col5 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(row, prevCol1);
      int row3col1 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(row, prevCol);
      int row3col2 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(row, col);
      int row3col3 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(row, nextCol);
      int row3col4 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(row, nextCol1);
      int row3col5 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(nextRow, prevCol1);
      int row4col1 = pSrcAcc->getColumnAsInteger();

	  pSrcAcc->toPixel(nextRow, prevCol);
      int row4col2 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow, col);
      int row4col3 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow, nextCol);
      int row4col4 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow, nextCol1);
      int row4col5 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow1, prevCol1);
      int row5col1 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow1, prevCol);
      int row5col2 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow1, col);
      int row5col3 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow1, nextCol);
      int row5col4 = pSrcAcc->getColumnAsInteger();

      pSrcAcc->toPixel(nextRow1, nextCol1);
      int row5col5 = pSrcAcc->getColumnAsInteger();
	 
	   int g = 0*row1col1 + (-1)*row1col2 + (-1)*row1col3 + (-1)*row1col4 + 0*row1col5 + (-1)*row2col1 + 2*row2col2 + (-4)*row2col3 + 2*row2col4
		         + (-1)*row2col5 + (-1)*row3col1 + (-4)*row3col2 + 13*row3col3 + (-4)*row3col4 + (-1)*row3col5 + (-1)*row4col1 + 2*row4col2 + (-4)*row4col3
				 + 2*row4col4+ (-1)*row4col5 + 0*row5col1 + (-1)*row5col2 + (-1)*row5col3 + (-1)*row5col4 + 0*row5col5;

	    double value = g/159.0;

		return value;
      
   };


DENOISE::DENOISE()
{
   setDescriptorId("{67B25DF5-725F-44FA-856F-F00B76696BB9}");
   setName("DENOISE");
   setVersion("Sample");
   setDescription("Calculate and return an edge detection raster element for first band "
      "of the provided raster element.");
   setCreator("Opticks Community");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Edge Detection");
   setMenuLocation("[Tutorial]/DENOISE");
   setAbortSupported(true);
}

DENOISE::~DENOISE()
{
}

bool DENOISE::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Denoise detection of image");
   return true;
}

bool DENOISE::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool DENOISE::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Tutorial 5", "app", "219F1882-A59F-4835-BE2A-E83C0C8111EB");
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
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
      "DResult", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));

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
   int rowSize= pDesc->getRowCount();
   int colSize = pDesc->getColumnCount();
   int zero=0;
   int prevCol = 0;
      int prevRow = 0;
      int nextCol = 0;
      int nextRow = 0;

	  int prevCol1 = 0;
	  int prevRow1= 0;
	  int nextCol1= 0;
	  int nextRow1= 0;

   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating result", row * 100 / pDesc->getRowCount(), NORMAL);
      }
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
      if (!pDestAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL) 
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         return false;
      }
      for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
      {
		  
		  double value=edgeDetection6(pSrcAcc, row, col, pDesc->getRowCount(), pDesc->getColumnCount());
          switchOnEncoding(pDesc->getDataType(), conversion, pDestAcc->getColumn(), value);
          pDestAcc->nextColumn();
		  
      }

      pDestAcc->nextRow();
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

      pProgress->updateProgress("DENOISE is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Result", pResultCube.release());

   pStep->finalize();
   return true;
}
