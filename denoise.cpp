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

REGISTER_PLUGIN_BASIC(OpticksTutorial, DENOISE);

namespace
{
   template<typename T>
   void edgeDetection(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize)
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
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal1 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*2/159.0;

	  pSrcAcc->toPixel(prevRow1, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal2 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(prevRow1, col);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal3 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*5/159.0;

	  pSrcAcc->toPixel(prevRow1, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal4 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(prevRow1, nextCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal5 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*2/159.0;

	  pSrcAcc->toPixel(prevRow, nextCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal6 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(prevRow, prevCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal7 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(row, nextCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal8 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*5/159.0;

	  pSrcAcc->toPixel(row, prevCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal9 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*5/159.0;

	  pSrcAcc->toPixel(nextRow, prevCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal10 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(nextRow, nextCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal11 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(nextRow1, nextCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal12 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*2/159.0;

	  pSrcAcc->toPixel(nextRow1, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal13 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(nextRow1, col);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal14 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*5/159.0;

	  pSrcAcc->toPixel(nextRow1, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal15 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*4/159.0;

	  pSrcAcc->toPixel(nextRow1, prevCol1);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal16 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*2/159.0;

	  pSrcAcc->toPixel(row, col);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal17 = *reinterpret_cast<T*>(pSrcAcc->getColumn())*15/159.0;

      
      pSrcAcc->toPixel(prevRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*9/159.0;

      pSrcAcc->toPixel(prevRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      T upVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*12/159.0;

      pSrcAcc->toPixel(prevRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T upperRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*9/159.0;

      pSrcAcc->toPixel(row, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T leftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*12/159.0;

      pSrcAcc->toPixel(row, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T rightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*12/159.0;

      pSrcAcc->toPixel(nextRow, prevCol);
      VERIFYNRV(pSrcAcc.isValid());
      T lowerLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*9/159.0;

      pSrcAcc->toPixel(nextRow, col);
      VERIFYNRV(pSrcAcc.isValid());
      T downVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*12/159.0;

      pSrcAcc->toPixel(nextRow, nextCol);
      VERIFYNRV(pSrcAcc.isValid());
      T lowerRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn())*9/159.0;

 /*
      unsigned int gx = 9 * upperLeftVal + 12 * leftVal + 9 * lowerLeftVal + 9 * upperRightVal + 12 *
         rightVal + 9 * lowerRightVal + 12* downVal + 12*upVal;

	  unsigned int gy = 2*upperLeftVal1 + 4*upperLeftVal2 + 5*upperLeftVal3 + 4*upperLeftVal4 + 2* upperLeftVal5 + 4*upperLeftVal6 + 4*upperLeftVal7 + 5*upperLeftVal8 + 5*upperLeftVal9 + 4*upperLeftVal10 + 4*upperLeftVal11
		  + 2*upperLeftVal12 + 4*upperLeftVal13 + 5*upperLeftVal14 + 4*upperLeftVal15 + 2*upperLeftVal16 + 15*upperLeftVal17;
		  */

	  double g = upperLeftVal + leftVal + lowerLeftVal + upperRightVal + rightVal + lowerRightVal + downVal + upVal + upperLeftVal1 + upperLeftVal2 + upperLeftVal3 + upperLeftVal4 + upperLeftVal5 + upperLeftVal6 + upperLeftVal7 + upperLeftVal8 + upperLeftVal9 + upperLeftVal10 + upperLeftVal11 + upperLeftVal12 + upperLeftVal13 + upperLeftVal14 + upperLeftVal15 + upperLeftVal16 + upperLeftVal17;

	   //double divisor=(gx+gy)/159.0;


      *pData = static_cast<T>(g);
   }
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
   pProgress->updateProgress("here??", 0, ERRORS);
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
   if (pDesc->getDataType() == INT4SCOMPLEX || pDesc->getDataType() == FLT8COMPLEX)
   {
      std::string msg = "Edge detection cannot be performed on complex types.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
      "DResult", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));
   pProgress->updateProgress("here?????", 0, ERRORS);
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
         switchOnEncoding(pDesc->getDataType(), edgeDetection, pDestAcc->getColumn(), pSrcAcc, row, col,
            pDesc->getRowCount(), pDesc->getColumnCount());
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
