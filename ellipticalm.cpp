/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
#include "RasterUtilities.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "ELLIPTICALM.h"
#include "TypeConverter.h"
#include <limits>
#include <vector>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>
#include <sstream>
#include "BitMaskIterator.h"
#include <math.h>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>




REGISTER_PLUGIN_BASIC(OpticksTutorial, ELLIPTICALM);

namespace
{
   template<typename T>
   void conversion(T* pData, double number)
   {
	   *pData=number;
   }
};


ELLIPTICALM::ELLIPTICALM()
{
   setDescriptorId("{D3DAC3A0-E034-43D0-9047-CE6E54F86A50}");
   setName("Ellipticalm");
   setDescription("Using AOIs.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/Ellipticalm");
   setAbortSupported(true);
}

ELLIPTICALM::~ELLIPTICALM()
{
}

bool ELLIPTICALM::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>("Result1");
   pInArgList->addArg<RasterElement>("RResult");
   return true;
}

bool ELLIPTICALM::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   pOutArgList->addArg<double>("Velocity of Ship", "Velocity of Ship");
   return true;
}

bool ELLIPTICALM::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Ellipticalm", "app", "DB8DE947-7021-46EF-A533-E8EAA7674272");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>("Result1");
   RasterElement* pCube1 = pInArgList->getPlugInArgValue<RasterElement>("RResult");
 

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
   RasterDataDescriptor* pDesc1 = static_cast<RasterDataDescriptor*>(pCube1->getDataDescriptor());
   VERIFY(pDesc != NULL);

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   FactoryResource<DataRequest> pRequest1;
   pRequest1->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc1 = pCube1->getDataAccessor(pRequest1.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube1->getName() +
      "DResult", pDesc1->getRowCount(), pDesc1->getColumnCount(), pDesc1->getDataType()));
   

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

   double total = 0.0;
    int count = 0;
    int totalx=0;
    int totaly=0;
   int numerator=0;
   int denominator=0;
   double m=0.0;
    double totalxy=0;
    int totalx_sq=0;
	int totaly_sq=0;
	double totalxx=0;
	double totalyy=0;


      for ( int row = 0; row < rowSize; ++row)
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
      if (!pSrcAcc.isValid())
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

      for ( int col = 0; col < colSize; ++col)
      {

			 if(pSrcAcc->getColumnAsInteger()!=0)
			 {
					totalx+=col;
					totaly-=row;
					++count;
			 }
			 
		 
		 pSrcAcc->nextColumn();
	  }
      pSrcAcc->nextRow();
	  }


	  double xcm=totalx/count;
	  double ycm=totaly/count;

	int n = floor(rowSize/2+0.5);
	int m1 = floor(colSize/2+0.5);

	int rowsize_sq=rowSize*rowSize;
	int colsize_sq=colSize*colSize;

	int rhomax = ceil(sqrt((rowsize_sq*1.0)+(colsize_sq*1.0)));
	int rc = floor(rhomax/2+0.5);

	int x_pos=xcm-m1;
	int y_pos=n+ycm;

	  pSrcAcc->toPixel(0, 0);

	 for ( int row = 0; row < rowSize; ++row)
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
      if (!pSrcAcc.isValid())
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

      for ( int col = 0; col < colSize; ++col)
      {
			 if(pSrcAcc->getColumnAsInteger()!=0)
			 {
					
					totalxx+=(col-xcm)*(col-xcm)/count;
					totalyy+=(-row-ycm)*(-row-ycm)/count;
					totalxy-=(col-xcm)*(-row-ycm)/count;

			 }
			 
		 pSrcAcc->nextColumn();
	  }
      pSrcAcc->nextRow();
	 }


	 double t=totalxx+totalyy;
	 double d = (totalxx*totalyy)-(totalxy*totalxy);
	 double l1 = (t/2)+sqrt((t*t/4)-d);
	 double l2 = (t/2)-sqrt((t*t/4)-d);

	 double eigenvector_1x=0.0;
	 double eigenvector_1y=0.0;

	 double eigenvector_2x=0.0;
	 double eigenvector_2y=0.0;

	 if(totalxy==0.0)
	 {
		 eigenvector_1x=1.0;
		 eigenvector_1y=0.0;

		 eigenvector_2x=0.0;
		 eigenvector_2y=1.0;
	 }

	 else
	 {
		 eigenvector_1x=l1-totalyy;
		 eigenvector_1y=totalxy;

		 eigenvector_2x=l2-totalyy;
		 eigenvector_2y=totalxy;
	 }
	 

	 double orientation = -((atan(eigenvector_2y/eigenvector_2x)*180/3.14159265)-90.0);

   QStringList Names("25.0");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold (eg. 5.5)", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   double long threshold5 = 0;
   stm >> threshold5;

	 int orientation1=0;
	 int orientation2=0;

	 if (orientation<90 && orientation>0)
	 {
		  orientation1 = orientation;
		  orientation2 = orientation+90;
	 }

	 if(orientation>=90 && orientation<180)
	 {
		  orientation1 = orientation-90;
		  orientation2 = orientation;
	 }


	int rowSize1= pDesc1->getRowCount();
	int colSize1 = pDesc1->getColumnCount();

	int total5=0;
	int total_sq5=0;
	int count5=0;

	int totalx3=0;
	int totaly3=0;
	int count3=0;


	for(int col=0; col<colSize1; col++)
	{
		for(int row=0; row<rowSize1; row++)
		{
			if(col>orientation1-10 && col<orientation1+10)
			{
				for(int i=row-5; i<row+6; i++)
				{
					int j=std::max(0,i);
					int k =std::min(rowSize1-1,j);
					pSrcAcc1->toPixel(k,col);
					int a = pSrcAcc1->getColumnAsDouble();
					total5+=a;
					total_sq5+=a*a;
					count5+=1;
				}

				double mean = total5/count5;
				double product = (total_sq5/count5)-(mean*mean);
				double std=sqrt(product);
				pDestAcc->toPixel(row,col);

				if(std>threshold5)
				{
					switchOnEncoding(pDesc1->getDataType(), conversion, pDestAcc->getColumn(), 1000);
					totalx3+=col;
					totaly3-=row;
					++count3;
				}


				else
				{
					switchOnEncoding(pDesc1->getDataType(), conversion, pDestAcc->getColumn(), 0);
				}

				total5=0;
				total_sq5=0;
				count5=0;
			}

			else if(col>orientation2-10 && col<orientation2+10)
			{

				for(int i=row-5; i<row+6; i++)
				{
					int j=std::max(0,i);
					int k =std::min(rowSize1-1,j);
					pSrcAcc1->toPixel(k,col);
					int a = pSrcAcc1->getColumnAsDouble();
					total5+=a;
					total_sq5+=a*a;
					count5+=1;
				}

				double mean = total5/count5;
				double product = (total_sq5/count5)-(mean*mean);
	

				double std=sqrt(product);

				pDestAcc->toPixel(row,col);


				if(std>threshold5)
				{
					switchOnEncoding(pDesc1->getDataType(), conversion, pDestAcc->getColumn(), 1000);
					totalx3+=col;
					totaly3-=row;
					++count3;
				}


				else
				{
					switchOnEncoding(pDesc1->getDataType(), conversion, pDestAcc->getColumn(), 0);
				}

				total5=0;
				total_sq5=0;
				count5=0;
			}

			else
			{
				pDestAcc->toPixel(row,col);
				switchOnEncoding(pDesc1->getDataType(), conversion, pDestAcc->getColumn(), 0);
			}
		}
	}

	
	  double xcm3=totalx3/count3;
	  double ycm3=totaly3/count3;

	int n2 = floor(rowSize1/2+0.5);
	int m2 = floor(colSize1/2+0.5);

	//int x_pos3=xcm3-m2;
	int y_pos3=(n2+ycm3);
	int x_coordinate = x_pos*abs(cos(xcm3));

	double y_pos_wake = (y_pos3)/(sin(xcm3));/*-(x_pos*abs(cos(xcm3)))*/

	double y_cm_ship = y_pos;

	double velocity = ((y_pos_wake-y_pos)/abs(cos(orientation)));

	double velocity1 = velocity*0.284;

				std::string s14;
				std::stringstream out14;
				out14 << y_pos_wake;
				s14 = out14.str();

				std::string s15;
				std::stringstream out15;
				out15 << y_cm_ship;
				s15 = out15.str();


				std::string s16;
				std::stringstream out16;
				out16 << velocity1;
				s16 = out16.str();

	  
	  pProgress->updateProgress("y_cm_ship: "+s15,0, ERRORS);

	  pProgress->updateProgress("y_cm_wake of Ship: "+s14,0, ERRORS);
	  
	  pProgress->updateProgress("Velocity of Ship: "+s16+ " m/s",0, ERRORS);


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
      std::string msg = "Velocity of Ship: " + StringUtilities::toDisplayString(velocity1);
	  pProgress->updateProgress(msg, 100, NORMAL);
   }

   pStep->addProperty("Velocity of Ship", velocity1);
   
   pOutArgList->setPlugInArgValue("Result", pResultCube.release());
   pOutArgList->setPlugInArgValue("Velocity of Ship", &velocity1);

   pStep->finalize();
   return true;


}