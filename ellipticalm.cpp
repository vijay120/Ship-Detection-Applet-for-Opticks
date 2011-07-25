/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <Point.h>
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
#include <math.h>
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


#define M_PI1 3.14159265358979323846264338327

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
   double ycmt = -1*ycm;

   GcpPoint lrPoint;
   lrPoint.mPixel = LocationType(static_cast<int>(xcm), static_cast<int>(ycmt));
   lrPoint.mCoordinate = pCube->convertPixelToGeocoord(lrPoint.mPixel);
   double x_geo = lrPoint.mCoordinate.mX;
   double y_geo = lrPoint.mCoordinate.mY;

		 	    std::string s19;
				std::stringstream out19;
				out19 << x_geo;
				s19 = out19.str();

		 	    std::string s20;
				std::stringstream out20;
				out20 << y_geo;
				s20 = out20.str();

	  
	  pProgress->updateProgress("x: "+s19,0, ERRORS);	  
	  pProgress->updateProgress("y: "+s20,0, ERRORS);

	  QStringList Names_x("0");
      QString value_x = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a longitude value", "Input a longitudal value", Names_x);
   
   std::string strAoi_x = value_x.toStdString();
   std::istringstream stm_x;
   stm_x.str(strAoi_x);
   long x_geo_code = 0;
   stm_x >> x_geo_code;

   	  QStringList Names_y("0");
      QString value_y = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a latitude value", "Input a latitude value", Names_y);
   
   std::string strAoi_y = value_y.toStdString();
   std::istringstream stm_y;
   stm_y.str(strAoi_y);
   long y_geo_code = 0;
   stm_y >> y_geo_code;

   int R = 6371;
   double dLat = (y_geo-y_geo_code)*M_PI1/180;
   double dLon = (x_geo-x_geo_code)*M_PI1/180;
   double lat1 = y_geo*M_PI1/180;
   double lat2 = y_geo_code*M_PI1/180;
   double a = sin(dLat/2)*sin(dLat/2)+sin(dLon/2)*sin(dLon/2)*cos(lat1)*cos(lat2);
   double c = 2*atan2(sqrt(a), sqrt(1.0-a));
   double distance = R*c;


   	     GcpPoint llPoint1;
         llPoint1.mPixel = LocationType(0,0);
         llPoint1.mCoordinate = pCube->convertPixelToGeocoord(llPoint1.mPixel);
		 double x1_scale = llPoint1.mCoordinate.mX;
		 double y1_scale = llPoint1.mCoordinate.mY;

		 GcpPoint llPoint2;
         llPoint2.mPixel = LocationType(1,0);
         llPoint2.mCoordinate = pCube->convertPixelToGeocoord(llPoint2.mPixel);
		 double x2_scale = llPoint2.mCoordinate.mX;
		 double y2_scale = llPoint2.mCoordinate.mY;

   double dLat1 = (y1_scale-y2_scale)*M_PI1/180;
   double dLon1 = (x1_scale-x2_scale)*M_PI1/180;
   double lat11 = y1_scale*M_PI1/180;
   double lat22 = y2_scale*M_PI1/180;
   double a1 = sin(dLat1/2)*sin(dLat1/2)+sin(dLon1/2)*sin(dLon1/2)*cos(lat11)*cos(lat22);
   double c1 = 2*atan2(sqrt(a1), sqrt(1.0-a1));
   double scale = R*c1*1000;

      		 	std::string s22;
				std::stringstream out22;
				out22 << scale;
				s22 = out22.str();

	  
	  pProgress->updateProgress("scale: "+s22,0, ERRORS);

   		 	    std::string s21;
				std::stringstream out21;
				out21 << distance;
				s21 = out21.str();

	  
	  pProgress->updateProgress("distance: "+s21,0, ERRORS);


   	  QStringList Names_a("0");
      QString value_a = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the satellite altitude", "Input a altitude value (km)", Names_a);
   
   std::string strAoi_a = value_a.toStdString();
   std::istringstream stm_a;
   stm_y.str(strAoi_a);
   long altitude = 0;
   stm_a >> altitude;

   double theta_heading = atan(distance/altitude);

      QStringList Names_v("0");
      QString value_v = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the satellite velocity", "Input a velocity value (km/s)", Names_v);
   
   std::string strAoi_v = value_v.toStdString();
   std::istringstream stm_v;
   stm_y.str(strAoi_v);
   double velocity = 0;
   stm_v >> velocity;

   double numerical = 7.609/distance;

         		std::string s25;
				std::stringstream out25;
				out25 << numerical;
				s25 = out25.str();

	  
	  pProgress->updateProgress("numerical: "+s25,0, ERRORS);

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

	int y_pos3=(n2+ycm3);
	int x_coordinate = x_pos*abs(cos(xcm3));

	double y_pos_wake = (y_pos3)/(sin(xcm3))-(cos(xcm3)/sin(xcm3))*x_pos;/*-(x_pos*abs(cos(xcm3)))*/

	double y_cm_ship = y_pos;

	double velocity3 = ((y_pos_wake-y_pos)/abs(cos(orientation)));

	double velocity11 = velocity3*scale*6.6*1.9438;

	double velocity1 = velocity11/(static_cast<double>(distance));

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
	  
	  pProgress->updateProgress("Velocity of Ship: "+s16+ " knots",0, ERRORS);


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