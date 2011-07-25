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
#include "RADON.h"
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


double long pi1=3.14159265;



REGISTER_PLUGIN_BASIC(OpticksTutorial, RADON);

namespace
{
   template<typename T>
   void conversion8(T* pData, double number, int rc, int cool, int cool2)
   {
	   *pData=(number)*255.0/cool;
   }
};

   string sconverter8(int i)
   {
	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
   }


RADON::RADON()
{
   setDescriptorId("{830CA203-8A3C-4017-8529-25665221CC35}");
   setName("RADON");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/RADON");
   setAbortSupported(true);
}



bool RADON::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "RADON TRANFORMATION of image");
   return true;
}

bool RADON::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool RADON::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("RADON", "app7", "33B24EAE-6CDE-48D2-B3D5-D52E3CAA4358");
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

   pProgress->updateProgress("step1", 0, ERRORS);

    int rowSize=pDesc->getRowCount();
    int colSize=pDesc->getColumnCount();

      		std::string s3;
			std::stringstream out3;
			out3 << rowSize;
			s3 = out3.str();
			pProgress->updateProgress(s3, 0, ERRORS);

			std::string s4;
			std::stringstream out4;
			out4 << colSize;
			s4 = out4.str();
			pProgress->updateProgress(s4, 0, ERRORS);



   int n = floor(rowSize/2+0.5);
   int m = floor(colSize/2+0.5);

   int rowsize_sq=rowSize*rowSize;
   int colsize_sq=colSize*colSize;

   int rhomax = ceil(sqrt((rowsize_sq*1.0)+(colsize_sq*1.0)));
   int rc = floor(rhomax/2+0.5);
   const int mt = 180;


   double costheta= 0.0;
   double sintheta=0.0;
   double a = 0.0;
   double b=0.0;
   int rho=0;
   int ymax=0;
   int ymin=0;
   int xmax=0;
   int xmin=0;
   double x=0.0;
   double y=0.0;
   int xfloor;
   int yfloor;
   double xup=0.0;
   double yup=0.0;
   double xlow=0.0;
   double ylow=0.0;

   pProgress->updateProgress("step2", 0, ERRORS);

   		   	std::string s1;
			std::stringstream out1;
			out1 << rhomax;
			s1 = out1.str();
			pProgress->updateProgress(s1, 0, ERRORS);

   double **res=0;
   res=new double *[rhomax+1];

   for(int i=0; i<rhomax+1; i++)
   {
	   res[i]=new double[mt];
   }

   pProgress->updateProgress("step3", 0, ERRORS);

   for(int k=0; k<rhomax+1; k++)
   {
	   for(int l=0; l<180; l++)
		   res[k][l]=0.0;
   }

   

/*
   vector<vector<double>> res;

   for(int i=1; i<mt+1; i++)
   {
	   vector<double> row;
	   res.push_back(row);
   }*/

   for(int t=1; t<46; t++)
   {
	   costheta=cos(1.0*t*pi1/180.0);
	   sintheta=sin(1.0*t*pi1/180.0);
	   a=-costheta/sintheta;
	   //pProgress->updateProgress("step4", 0, ERRORS);
	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=1.0*rho/sintheta;
		   int f=floor((-a*m)+b+0.5);
		   int g=floor(a*m+b+0.5);
		   ymax=min(f,(n-1));
		   ymin=max(g,-n);

		   //pProgress->updateProgress("step5", 0, ERRORS);
		   for(int y=ymin; y<ymax+1; y++)
		   {
			   x=1.0*(y-b)/a;
			   xfloor=floor(x);
			   xup=x-xfloor;
			   xlow=1-xup;
			   x=xfloor;
			   int l=x;
			   x=max(static_cast<int>(x),-m);
			   int q=x;
			   x=min(static_cast<int>(x),m-2);
			   //pProgress->updateProgress("step6", 0, ERRORS);
			   pAcc->toPixel(y+n,x+m);
			   double pixel1=pAcc->getColumnAsDouble();
			   pAcc->toPixel(y+n,x+m+1);
			   //pProgress->updateProgress("step7", 0, ERRORS);
			   double pixel2=pAcc->getColumnAsDouble();
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xlow*pixel1;
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xup*pixel2;

		   }
	   }
   }

   pProgress->updateProgress("step7", 0, ERRORS);
   

   for(int t=46; t<91; t++)
   {
	   costheta=cos(1.0*t*pi1/180.0);
	   sintheta=sin(1.0*t*pi1/180.0);
	   a=-costheta/sintheta;

	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=rho/sintheta;
		   int f=floor(1.0*(-n-b)/a+0.5);
		   int g=floor(1.0*(n-b)/a+0.5);
		   xmax=min(f,m-1);
		   xmin=max(g,-m);
		   

		   for(int x=xmin; x<xmax+1; x++)
		   {
			   y=a*x+b;
			   yfloor=floor(y);
			   yup=y-yfloor;
			   ylow=1-yup;
			   y=yfloor;
			   int l=y;
			   y=max(static_cast<int>(y),-n);
			   int q=y;
			   y=min(static_cast<int>(y),n-2);
			   pAcc->toPixel(y+n,x+m);
			   double pixel1=pAcc->getColumnAsDouble();
			   pAcc->toPixel(y+n+1,x+m);
			   double pixel2=pAcc->getColumnAsDouble();
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xlow*pixel1;
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }

   pProgress->updateProgress("step8", 0, ERRORS);

   for(int t=91; t<136; t++)
   {
	   costheta=cos(1.0*t*pi1/180.0);
	   sintheta=sin(1.0*t*pi1/180.0);
	   a=-costheta/sintheta;
	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=rho/sintheta;
		   int f=floor((n-b)/a+0.5);
		   int g=floor((-n-b)/a+0.5);
		   xmax=min(f,m-1);
		   xmin=max(g,-m);
		   for(int x=xmin; x<xmax+1; x++)
		   {
			   y=a*x+b;
			   yfloor=floor(y);
			   yup=y-yfloor;
			   ylow=1-yup;
			   y=yfloor;
			   int l=y;
			   y=max(static_cast<int>(y),-n);
			   int q=y;
			   y=min(static_cast<int>(y),n-2);
			   pAcc->toPixel(y+n,x+m);
			   double pixel1=pAcc->getColumnAsDouble();
			   pAcc->toPixel(y+n+1,x+m);
			   double pixel2=pAcc->getColumnAsDouble();
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xlow*pixel1;
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }

   pProgress->updateProgress("step9", 0, ERRORS);

   for(int t=136; t<180; t++)
   {
	   costheta=cos(t*pi1/180.0);
	   sintheta=sin(t*pi1/180.0);
	   a=-costheta/sintheta;
	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=rho/sintheta;
			//pProgress->updateProgress(s, 0, ERRORS);
		   int f=floor(1.0*(-a*m)+b+0.5);
		   int g=floor(a*m+b+0.5);
		   ymax=min(g,n-1);
		   ymin=max(f,-n);
		   for(int y=ymin; y<ymax+1; y++)
		   {
			   x=1.0*(y-b)/a;
			   xfloor=floor(x);
			   xup=x-xfloor;
			   xlow=1-xup;
			   x=xfloor;
			   int l=x;
			   x=max(static_cast<int>(x),-m);
			   int q=x;
			   x=min(static_cast<int>(x),m-2);
			   pAcc->toPixel(y+n,x+m);
			   double pixel1=pAcc->getColumnAsDouble();
			   pAcc->toPixel(y+n,x+m+1);
			   double pixel2=pAcc->getColumnAsDouble();
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xlow*pixel1;
			   res[rhomax-r][mt-t-1]=res[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }

   pProgress->updateProgress("step10", 0, ERRORS);
   
   int r=0;
   int rhooffset=floor((rhomax-colSize)/2+0.5);
   for(int x=1; x<colSize+1; x++)
   {
	   r=x+rhooffset;
	   r=rhomax-r+1;
	   for(int y=1; y<rowSize+1; y++)
	   {
		   //pProgress->updateProgress("step11", 0, ERRORS);
		   pAcc->toPixel(y-1,x-1);
		   double pixel=pAcc->getColumnAsDouble();
		   //pProgress->updateProgress("step12", 0, ERRORS);
		   	//std::string s;
			//std::stringstream out;
			//out << r;
			//s = out.str();
			//pProgress->updateProgress(s, 0, ERRORS);
		   res[r-1][179]=res[r-1][179]+pixel;
	   }
   }
   
   pProgress->updateProgress("step11", 0, ERRORS);
   //int rhoaxis=rhomax+1-rc;

   int rhoaxis=rhomax+1;

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
   "RADON result", rhoaxis, 180, pDesc->getDataType()));

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

   int cool = -10000;
   int cool2 = 10000;
   int rollcol=0;
   int colcol=0;

   for(int row=0; row<rhoaxis; row++)
   {
	   for(int col=0; col<180; col++)
	   {
		   pDestAcc->toPixel(row,col);
		   //switchOnEncoding(pDesc->getDataType(), conversion8, pDestAcc->getColumn(), res[row][col], rc);
		   if (res[row][col]>cool)
		   {
			   cool=res[row][col];
		   }

		   if(res[row][col]<cool2)
		   {
			   cool2=res[row][col];
			   colcol=col;
			   rollcol=row;
		   }
	   }
   }

      for(int row=0; row<rhoaxis; row++)
   {
	   for(int col=0; col<180; col++)
	   {
		   pDestAcc->toPixel(row,col);
		   switchOnEncoding(pDesc->getDataType(), conversion8, pDestAcc->getColumn(), res[row][col], rc, cool,cool2);
		   if (res[row][col]>cool)
		   {

			   cool=res[row][col];
		   }
	   }
   }


   for(int i=0; i<rhomax+1; i++)
	   delete[] res[i];
   delete[] res;




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