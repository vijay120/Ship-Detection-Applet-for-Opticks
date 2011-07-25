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
#include "radondetection.h"
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


double long pi9=3.14159265;

REGISTER_PLUGIN_BASIC(OpticksTutorial, RADONDETECTION);

namespace
{
   template<typename T>
   void conversion12(T* pData, double number)
   {
	   *pData=number;
   }
};

   double edgeDetection4(DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, double threshold)
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

	  int total = row1col1 + row1col2 + row1col3 + row1col4 + row1col5 + row2col1 + row2col2 + row2col3 + row2col4
		         + row2col5 + row3col1 + row3col2 + row3col4 + row3col5 + row4col1 + row4col2 + row4col3
				 + row4col4 + row4col5 + row5col1 + row5col2 + row5col3 + row5col4 + row5col5;

	  double mean = total/24.0;

	  int total_sum = row1col1*row1col1 + row1col2*row1col2 + row1col3*row1col3 + row1col4*row1col4 + row1col5*row1col5 + row2col1*row2col1 + row2col2*row2col2 + row2col3*row2col3 + row2col4*row2col4
		         + row2col5*row2col5 + row3col1*row3col1 + row3col2*row3col2 + row3col4*row3col4 + row3col5*row3col5 + row4col1*row4col1 + row4col2*row4col2 + row4col3*row4col3
				 + row4col4*row4col4 + row4col5*row4col5 + row5col1*row5col1 + row5col2*row5col2 + row5col3*row5col3 + row5col4*row5col4 + row5col5*row5col5;

	  double std = sqrt(total_sum / 24.0 - mean * mean);

	  double zstatistic = (row3col3-mean)/std;

	  double value=0;

	  if(row3col3<row1col1 && row3col3<row1col2 && row3col3<row1col3 && row3col3<row1col4 && row3col3<row1col5 &&
		  row3col3<row2col1 && row3col3<row2col2 && row3col3<row2col3  && row3col3<row2col4 && row3col3<row2col5 &&
		  row3col3<row3col1 && row3col3<row3col2 && row3col3<row3col4 && row3col3<row3col5 && row3col3<row4col1 &&
		  row3col3<row4col2 && row3col3<row4col3 && row3col3<row4col4 && row3col3<row4col5 && row3col3<row5col1 &&
		  row3col3<row5col2 && row3col3<row5col2 && row3col3<row5col3 && row3col3<row5col4 && row3col3<row5col5 && zstatistic<threshold)
	  {
		   value=1000.0;
	  }

	  else
		   value=0.0;


		return value;

      
   };


RADONDETECTION::RADONDETECTION()
{
   setDescriptorId("{8023F318-AAFF-4159-87BF-1D49EF8BACAB}");
   setName("RADONDETECTION");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/RADONDETECTION");
   setAbortSupported(true);
}



bool RADONDETECTION::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "RADONDETECTION detection of image");
   return true;
}

bool RADONDETECTION::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Image1", NULL);
   return true;
}

bool RADONDETECTION::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("CFAR", "app5", "750E77E9-7E7E-4FDA-9B00-489B7E05FA43");
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
   //DataAccessor pAcc2 = pCube->getDataAccessor(pRequest2.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
   "Image1", pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType()));

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

   
   QStringList Names("5.0");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold (eg. 5.5)", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   //stm >> PFA;
   double  threshold = 0;
   stm >> threshold;
   

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

   	int total5=0;
	int total_sq5=0;
	int count5=0;

   	for(int col=0; col<colSize; col++)
	{
		for(int row=0; row<rowSize; row++)
		{

				for(int i=row-5; i<row+6; i++)
				{
					int j=std::max(0,i);
					int k =std::min(rowSize-1,j);
					pAcc->toPixel(k,col);
					int a = pAcc->getColumnAsDouble();
					total5+=a;
					total_sq5+=a*a;
					count5+=1;
				}

				double mean = total5/count5;
				double product = (total_sq5/count5)-(mean*mean);
				double std=sqrt(product);

				pDestAcc->toPixel(row,col);


				if(std>threshold)
				{
					switchOnEncoding(pDesc->getDataType(), conversion12, pDestAcc->getColumn(), 1000);
				}


				else
				{
					switchOnEncoding(pDesc->getDataType(), conversion12, pDestAcc->getColumn(), 0);
				}

				total5=0;
				total_sq5=0;
				count5=0;
			}

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

   pOutArgList->setPlugInArgValue("Image1", pResultCube.release());

   pStep->finalize();
   return true;
   
}