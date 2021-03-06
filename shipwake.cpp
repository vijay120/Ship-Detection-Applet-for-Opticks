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
#include "shipwake.h"
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
#include "GcpList.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
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


double long pi5=3.14159265;



REGISTER_PLUGIN_BASIC(OpticksTutorial, SHIPWAKE);

namespace
{
   template<typename T>
   void conversion8(T* pData, double number, int rc, int cool)
   {
	   *pData=(number)*255.0/cool;
   }
};

namespace
{
   template<typename T>
   void conversion11(T* pData, double number)
   {
	   *pData=number;
   }
};


   void updateStatistics15(double pData, double& total, double& total_sum, int& count)
   {
      total += pData;
	  total_sum += pData * pData;
	  count+=1;
   }


SHIPWAKE::SHIPWAKE()
{
   setDescriptorId("{BC6750B9-0878-4BB3-9C85-55ADEF3123AB}");
   setName("SHIPWAKE");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/SHIPWAKE");
   setAbortSupported(true);
}



bool SHIPWAKE::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Integrated TRANFORMATION of image");
   return true;
}

bool SHIPWAKE::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("RResult", NULL);
   return true;
}

bool SHIPWAKE::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("WAKE", "app", "5234FF2D-3845-450E-8056-C625B845CEAD");
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



   ///////////////////////////////// CFAR SECTION  ///////////////////////////////////////////////
   int rowSize=pDesc->getRowCount();
   int colSize=pDesc->getColumnCount();
   unsigned int total5_1=0;
   int count5_1 = 0;

   double **res=0;
   res=new double *[rowSize];

   for(int i=0; i<rowSize; i++)
   {
	   res[i]=new double[colSize];
   }

   for(int k=0; k<rowSize; k++)
   {
	   for(int l=0; l<colSize; l++)
	   {
		   pAcc->toPixel(k,l);
		   double pixel=pAcc->getColumnAsDouble();
		   total5_1+=pixel;
		   count5_1+=1;
		   res[k][l]=pixel;
	   }
   }

   double mean5 = total5_1/count5_1;


   double **res1=0;
   res1=new double *[rowSize];

   for(int i=0; i<rowSize; i++)
   {
	   res1[i]=new double[colSize];
   }

   for(int k=0; k<rowSize; k++)
   {
	   for(int l=0; l<colSize; l++)
	   {
		   res1[k][l]=0.0;
	   }
   }
 

   int eastCol = 0;
   int northRow = 0;
   int westCol = 0;
   int southRow = 0;
   double zstatistic = 0;
   double total = 0.0;
   double total_sum = 0.0;
   double mean = 0.0;
   double std = 0.0;
   int prevCol = 0;
   int prevRow = 0;
   int nextCol = 0;
   int nextRow = 0;
   double long PFA = 0.0;
   int DEPTH = 10;
   int count=0;
   int zero=0;

   QStringList Names("5.0");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold (eg. 5.5)", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   double long threshold = 0;
   stm >> threshold;

   pAcc->toPixel(0,0);

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
         pProgress->updateProgress("Calculating statistics", 0.3*row * 100 / pDesc->getRowCount(), NORMAL);
      }
	  		
	  

      for (int col = 0; col < colSize; ++col)
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
			
			for(int row1=northRow; row1 < southRow+1; ++row1)
			{
				for (int col1=westCol; col1 < eastCol+1; ++col1)
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
						updateStatistics15(pAcc2->getColumnAsDouble(), total, total_sum, count);
					}

					pAcc2->nextColumn();

				}

				pAcc2->nextRow();
			}

			mean = total / count;
			std = sqrt(total_sum / count - mean * mean);
			pAcc2->toPixel(row,col);
			//pDestAcc->toPixel(row,col);
			zstatistic = (pAcc2->getColumnAsDouble()-mean)/std;
				std::string s;
				std::stringstream out;
				out << zstatistic;
				s = out.str();
	
			//pProgress->updateProgress(s,threshold, ERRORS);
			if(zstatistic>threshold)
			{
				res1[row][col]+=255.0;
			}

			else
			{
				res1[row][col]+=0.0;
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

   /////////////////////////////////////////////////  SHIP MASK SECTION   ///////////////////////////////////

   for (int row = 0; row < rowSize; ++row)
   {
      for (int col = 0; col < colSize; ++col)
      {
		  if(res1[row][col]!=0)
		  {
			  res[row][col]=mean5;
		  }

		  else
			  continue;
	  }
   }


   ///////////////////////////RADON SECTION//////////////////////////////////////

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

   double **res3=0;
   res3=new double *[rhomax+1];

   for(int i=0; i<rhomax+1; i++)
   {
	   res3[i]=new double[mt];
   }

   for(int k=0; k<rhomax+1; k++)
   {
	   for(int l=0; l<180; l++)
		   res3[k][l]=0.0;
   }


   double **res7=0;
   res7=new double *[rhomax+1];

   for(int i=0; i<rhomax+1; i++)
   {
	   res7[i]=new double[mt];
   }

   for(int k=0; k<rhomax+1; k++)
   {
	   for(int l=0; l<180; l++)
		   res7[k][l]=0.0;
   }

   for(int t=1; t<46; t++)
   {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 30+0.25*0.2*t*100 / 45, NORMAL);
      }

	   costheta=cos(1.0*t*pi5/180.0);
	   sintheta=sin(1.0*t*pi5/180.0);
	   a=-costheta/sintheta;
	   
	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=1.0*rho/sintheta;
		   int f=floor((-a*m)+b+0.5);
		   int g=floor(a*m+b+0.5);
		   ymax=min(f,(n-1));
		   ymin=max(g,-n);

		   
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
			   int o = x;
			   double pixel1=res[y+n][o+m];
			   double pixel2=res[y+n][o+m+1];
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xlow*pixel1;
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xup*pixel2;

		   }
	   }
   }
   for(int t=46; t<91; t++)
   {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 35+0.25*0.2*t*100 / 45, NORMAL);
      }
	   costheta=cos(1.0*t*pi5/180.0);
	   sintheta=sin(1.0*t*pi5/180.0);
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
			   int p = y;
			   double pixel1=res[p+n][x+m];
			   double pixel2=res[p+n+1][x+m];
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xlow*pixel1;
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }

   for(int t=91; t<136; t++)
   {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 40+0.25*0.2*t*100 / 45, NORMAL);
      }
	   costheta=cos(1.0*t*pi5/180.0);
	   sintheta=sin(1.0*t*pi5/180.0);
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
			   int f=y;
			   double pixel1=res[f+n][x+m];
			   double pixel2=res[f+n+1][x+m];
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xlow*pixel1;
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }

   for(int t=136; t<180; t++)
   {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 45+0.25*0.2*t*100 / 45, NORMAL);
      }
	   costheta=cos(t*pi5/180.0);
	   sintheta=sin(t*pi5/180.0);
	   a=-costheta/sintheta;
	   for(int r=1; r<rhomax+1; r++)
	   {
		   rho=r-rc;
		   b=rho/sintheta;
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
			   int w=x;
			   double pixel1=res[y+n][w+m];
			   double pixel2=res[y+n][w+m+1];
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xlow*pixel1;
			   res3[rhomax-r][mt-t-1]=res3[rhomax-r][mt-t-1]+xup*pixel2;
		   }
	   }
   }   
   int r=0;
   int rhooffset=floor((rhomax-colSize)/2+0.5);
   for(int x=1; x<colSize+1; x++)
   {
	   r=x+rhooffset;
	   r=rhomax-r+1;
	   for(int y=1; y<rowSize+1; y++)
	   {
		   double pixel=res[y-1][x-1];
		   res3[r-1][179]=res3[r-1][179]+pixel;
	   }
   }

   int rhoaxis=rhomax+1;
   int cool = -10000;
   int cool2 = 10000;
   int rollcol=0;
   int colcol=0;

   for(int row=0; row<rhoaxis; row++)
   {
	   for(int col=0; col<180; col++)
	   {
		   if (res3[row][col]>cool)
		   {
			   cool=res3[row][col];
		   }

		   if(res3[row][col]<cool2)
		   {
			   cool2=res3[row][col];
			   colcol=col;
			   rollcol=row;
		   }
	   }
   }

   for(int row=0; row<rhoaxis; row++)
   {
	   for(int col=0; col<180; col++)
	   {
		   res7[row][col]=(res3[row][col])*255.0/cool;
	   }
   }

   ///////////////////////// K DISTRIBUTION SECTION /////////////////////////////////////////

   double **res4=0;
   res4=new double *[rowSize];

   for(int i=0; i<rowSize; i++)
   {
	   res4[i]=new double[colSize];
   }

   for(int k=0; k<rowSize; k++)
   {
	   for(int l=0; l<colSize; l++)
	   {
		   res4[k][l]=0.0;
	   }
   }

   int eastCol_k = 0;
   int northRow_k = 0;
   int westCol_k = 0;
   int southRow_k = 0;
   double zstatistic_k = 0;
   double total_k = 0.0;
   double total_sum_k = 0.0;
   double mean_k = 0.0;
   double std_k = 0.0;
   int prevCol_k = 0;
   int prevRow_k = 0;
   int nextCol_k = 0;
   int nextRow_k = 0;
   double long PFA_k = 0.0;
   int DEPTH1 = 10;
   int DEPTH2 = 10;
   int DEPTH3 = 1;
   int DEPTH4 = 1;
   int count_k=0;
   double long threshold_k = 100000.0;

   double look_table1[24][6];

   for(int i=0; i<24; i++)
   {
	   for(int j=0; j<3; j++)
	   {
			   look_table1[i][j]=0.0;	   
	   }
   }
      
   QStringList Names_k("0.0000001");
   QString value_k = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a PFA value", "Input a PFA value (0.0000001 or 0.00000001)", Names_k);
   
   std::string strAoi_k = value_k.toStdString();
   std::istringstream stm_k;
   stm_k.str(strAoi_k);
   PFA_k=::atof(strAoi_k.c_str());

   if (PFA_k==0.0000001)
   {
   look_table1[0][0]=1.0;
   look_table1[0][1]=5.0;
   look_table1[0][2]=32.3372530103729330;
   look_table1[1][0]=1.0;
   look_table1[1][1]=10.0;
   look_table1[1][2]=25.0723580041031010;
   look_table1[2][0]=1.0;
   look_table1[2][1]=15.0;
   look_table1[2][2]=22.3991160013551250;
   look_table1[3][0]=1.0;
   look_table1[3][1]=20.0;
   look_table1[3][2]=20.9821949998985920;
   look_table1[4][1]=1.0;
   look_table1[4][2]=40.0;
   look_table1[5][3]=18.7055519975583020;
   look_table1[5][1]=1.0;
   look_table1[5][2]=90.0;
   look_table1[5][3]=18.7055519975583020;

   look_table1[6][0]=2.0;
   look_table1[6][1]=5.0;
   look_table1[6][2]=20.2619339991581950;
   look_table1[7][0]=2.0;
   look_table1[7][1]=10.0;
   look_table1[7][2]=15.4860609951617470;
   look_table1[8][0]=2.0;
   look_table1[8][1]=15.0;
   look_table1[8][2]=13.7276789964777210;
   look_table1[9][0]=2.0;
   look_table1[9][1]=20.0;
   look_table1[9][2]=12.7942589971762930;
   look_table1[10][0]=2.0;
   look_table1[10][1]=40.0;
   look_table1[10][2]=11.2895769983023970;
   look_table1[11][0]=2.0;
   look_table1[11][1]=90.0;
   look_table1[11][2]=10.3695259989909640;

   look_table1[12][0]=3.0;
   look_table1[12][1]=5.0;
   look_table1[12][2]=15.9102209948443050;
   look_table1[13][0]=3.0;
   look_table1[13][1]=10.0;
   look_table1[13][2]=12.0443629977375150;
   look_table1[14][0]=3.0;
   look_table1[14][1]=15.0;
   look_table1[14][2]=10.6203179988032710;
   look_table1[15][0]=3.0;
   look_table1[15][1]=20.0;
   look_table1[15][2]=9.8635499993696367;
   look_table1[16][0]=3.0;
   look_table1[16][1]=40.0;
   look_table1[16][2]=8.6407550002847771;
   look_table1[17][0]=3.0;
   look_table1[17][1]=90.0;
   look_table1[17][2]=7.8893780007488568;

   look_table1[18][0]=4.0;
   look_table1[18][1]=5.0;
   look_table1[18][2]=13.6166519965608130;
   look_table1[19][0]=4.0;
   look_table1[19][1]=10.0;
   look_table1[19][2]=10.2336029990926890;
   look_table1[20][0]=4.0;
   look_table1[20][1]=15.0;
   look_table1[20][2]=10.6203179988032710;
   look_table1[21][0]=4.0;
   look_table1[21][1]=20.0;
   look_table1[21][2]=8.9868610000257512;
   look_table1[22][0]=4.0;
   look_table1[22][1]=40.0;
   look_table1[22][2]=7.2502150006595159;
   look_table1[23][0]=4.0;
   look_table1[23][1]=90.0;
   look_table1[23][2]=6.5879140005669408;
   }
   
   
   if (PFA_k==0.00000001)
   {
   look_table1[0][0]=1.0;
   look_table1[0][1]=5.0;
   look_table1[0][2]=20.0000019988889410;
   look_table1[1][0]=1.0;
   look_table1[1][1]=10.0;
   look_table1[1][2]=20.0000019988889410;
   look_table1[2][0]=1.0;
   look_table1[2][1]=15.0;
   look_table1[2][2]=20.0000019988889410;
   look_table1[3][0]=1.0;
   look_table1[3][1]=20.0;
   look_table1[3][2]=20.0000019988889410;
   look_table1[4][1]=1.0;
   look_table1[4][2]=40.0;
   look_table1[5][3]=20.0000019988889410;
   look_table1[5][1]=1.0;
   look_table1[5][2]=90.0;
   look_table1[5][3]=20.0000019988889410;

   look_table1[6][0]=2.0;
   look_table1[6][1]=5.0;
   look_table1[6][2]=18.3243529971664460;
   look_table1[7][0]=2.0;
   look_table1[7][1]=10.0;
   look_table1[7][2]=18.3243529971664460;
   look_table1[8][0]=2.0;
   look_table1[8][1]=15.0;
   look_table1[8][2]=16.0869139948664570;
   look_table1[9][0]=2.0;
   look_table1[9][1]=20.0;
   look_table1[9][2]=14.8998299956004820;
   look_table1[10][0]=2.0;
   look_table1[10][1]=40.0;
   look_table1[10][2]=12.9846719970337880;
   look_table1[11][0]=2.0;
   look_table1[11][1]=90.0;
   look_table1[11][2]=11.8094659979133120;

   look_table1[12][0]=3.0;
   look_table1[12][1]=5.0;
   look_table1[12][2]=18.9816659978421360;
   look_table1[13][0]=3.0;
   look_table1[13][1]=10.0;
   look_table1[13][2]=14.1167729961865230;
   look_table1[14][0]=3.0;
   look_table1[14][1]=15.0;
   look_table1[14][2]=12.3304539975234050;
   look_table1[15][0]=3.0;
   look_table1[15][1]=20.0;
   look_table1[15][2]=11.3819769982332450;
   look_table1[16][0]=3.0;
   look_table1[16][1]=40.0;
   look_table1[16][2]=9.8488249993806569;
   look_table1[17][0]=3.0;
   look_table1[17][1]=90.0;
   look_table1[17][2]=8.9039850000877756;

   look_table1[18][0]=4.0;
   look_table1[18][1]=5.0;
   look_table1[18][2]=16.1272319949079020;
   look_table1[19][0]=4.0;
   look_table1[19][1]=10.0;
   look_table1[19][2]=11.9117899978367330;
   look_table1[20][0]=4.0;
   look_table1[20][1]=15.0;
   look_table1[20][2]=10.3636999989953240;
   look_table1[21][0]=4.0;
   look_table1[21][1]=20.0;
   look_table1[21][2]=9.5411879996108926;
   look_table1[22][0]=4.0;
   look_table1[22][1]=40.0;
   look_table1[22][2]=8.2095870006074634;
   look_table1[23][0]=4.0;
   look_table1[23][1]=90.0;
   look_table1[23][2]=7.3860650006785047;
   }
   

   QStringList Names1("10");
   QString value1 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the window width", "Input the size of the window width in terms of the number of pixels (eg. 10)", Names1);
   
   std::string strAoi1 = value1.toStdString();
   std::istringstream stm1;
   stm1.str(strAoi1);
   DEPTH1=::atof(strAoi1.c_str());

   QStringList Names2("10");
   QString value2 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the window height", "Input the size of the window height in terms of the number of pixels (eg. 10)", Names2);
   
   std::string strAoi2 = value2.toStdString();
   std::istringstream stm2;
   stm2.str(strAoi2);
   DEPTH2=::atof(strAoi2.c_str());

   QStringList Names3("1");
   QString value3 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the gaurd width", "Input the size of the guard width in terms of the number of pixels (eg. 1)", Names3);
   
   std::string strAoi3 = value3.toStdString();
   std::istringstream stm3;
   stm3.str(strAoi3);
   DEPTH3=::atof(strAoi3.c_str());

   QStringList Names4("1");
   QString value4 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the guard height", "Input the size of the guard height in terms of the number of pixels (eg. 1)", Names4);
   
   std::string strAoi4 = value4.toStdString();
   std::istringstream stm4;
   stm4.str(strAoi4);
   stm4 >> DEPTH4;
   DEPTH4=::atof(strAoi4.c_str());


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

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 50+0.1*row * 100 / pDesc->getRowCount(), NORMAL);
      }
	  		
	  

      for (int col = 0; col < colSize; ++col)
      {
		  westCol_k=max(col-DEPTH1,zero);
		  northRow_k=max(row-DEPTH2,zero);
		  eastCol_k=min(colSize-1,col+DEPTH1);
		  southRow_k=min(rowSize-1,row+DEPTH2);
		  prevCol_k=max(col-DEPTH3,zero);
		  prevRow_k=max(row-DEPTH4,zero);
		  nextCol_k=min(col+DEPTH3,colSize-1);
		  nextRow_k=min(row+DEPTH4,rowSize-1);

			pAcc2->toPixel(northRow,westCol);
			
			for(int row1=northRow; row1 < southRow+1; ++row1)
			{
								
				for (int col1=westCol; col1 < eastCol+1; ++col1)
				{

					if((row1>=prevRow && row1<=nextRow) && (col1>=prevCol && col1<=nextCol))
					{
						continue;
					}

					else
					{
						updateStatistics15(pAcc2->getColumnAsDouble(), total_k, total_sum_k, count_k);
					}


					pAcc2->nextColumn();

				}

				pAcc2->nextRow();
			}

			mean_k = total_k / count_k;
			std_k = sqrt(total_sum_k / count_k - mean_k * mean_k);
			int ELVI = (mean_k/std_k)*(mean_k/std_k);
			int v = (ELVI+1)/((ELVI*mean_k/(std_k*std_k))-1);

			pAcc2->toPixel(row,col);
			zstatistic_k = (pAcc2->getColumnAsDouble()-mean_k)/std_k;

				 if(v<=7 && v>=0)
				 { v=5;
				 }

				 if(v<=12 && v>7)
				 {
					 v=10;
				 }

				 if(v<=17 && v>12)
				 {
					 v=15;
				 }

				 if(v<=30 && v>17)
				 {
					 v=20;
				 }

				 if(v<=65 && v>30)
				 {
					 v=40;
				 }

				 if(v<=90 && v>65)
				 {
					 v=90;
				 }


			for(int i=0; i<24; i++)
			{
				if((look_table1[i][0]=ELVI) && (look_table1[i][1]==v))
				{
					threshold_k=look_table1[i][2];
				}
			}

			if(zstatistic_k>threshold_k)
			{
				res4[row][col]=255.0;
			}

			else
			{
				res4[row][col]=0.0;
			}

			total_k = 0.0;
			total_sum_k=0.0;
            threshold_k=100000.0;
            mean_k = 0.0;
            std_k = 0.0;
			count_k=0;

			pAcc->nextColumn();
	  }
      pAcc->nextRow();
   }

   //////////////////////////////// Morph section ////////////////////////////////////////

   double **res5=0;
   res5=new double *[rowSize];

   for(int i=0; i<rowSize; i++)
   {
	   res5[i]=new double[colSize];
   }

   for(int k=0; k<rowSize; k++)
   {
	   for(int l=0; l<colSize; l++)
	   {
		   res5[k][l]=0.0;
	   }
   }


   double **res6=0;
   res6=new double *[rowSize];

   for(int i=0; i<rowSize; i++)
   {
	   res6[i]=new double[colSize];
   }

   for(int k=0; k<rowSize; k++)
   {
	   for(int l=0; l<colSize; l++)
	   {
		   res6[k][l]=0.0;
	   }
   }

   int upperLeftVal_m = 0;
   int upVal_m = 0;
   int upperRightVal_m = 0;
   int leftVal_m = 0;
   int rightVal_m = 0;
   int lowerLeftVal_m = 0;
   int downVal_m = 0;
   int lowerRightVal_m = 0;

   int prevCol_m = 0;
   int prevRow_m = 0;
   int nextCol_m = 0;
   int nextRow_m = 0;
   int count_m=0;
   int threshold_m = 0;

   int totalx=0;
   int totaly=0;
   int count_t=0;

   QStringList Names_m("3");
   QString value_m = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold", Names_m);
   
   std::string strAoi_m = value_m.toStdString();
   std::istringstream stm_m;
   stm_m.str(strAoi_m);
   stm_m >> threshold_m;
   
   for (int row = 0; row < rowSize; ++row)
   {

      for (int col = 0; col < colSize; ++col)
	  {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 60+0.05*row*100 / rowSize, NORMAL);
      }
		  prevCol_m=max(col-1,zero);
		  prevRow_m=max(row-1,zero);
		  nextCol_m=min(col+1,colSize-1);
		  nextRow_m=min(row+1,rowSize-1);

      upperLeftVal_m = res4[prevRow_m][prevCol_m];
	  if(upperLeftVal_m>0)
	  {
			  count_m+=1;
	  }

      upVal_m = res4[prevRow_m][col];
	  if(upVal_m>0)
	  {
			  count_m+=1;
	  }

      upperRightVal_m = res4[prevRow_m][nextCol_m];
	  	  if(upperRightVal_m>0)
		  {
			  count_m+=1;
		  }

      leftVal_m = res4[row][prevCol_m];
	  	  if(leftVal_m>0)
		  {
			  count_m+=1;
		  }

      rightVal_m = res4[row][nextCol_m];
	  	  if(rightVal_m>0)
		  {
			  count_m+=1;
		  }

      lowerLeftVal_m = res4[nextRow_m][prevCol_m];
	  	  if(lowerLeftVal_m>0)
		  {
			  count_m+=1;
		  }

      downVal_m = res4[nextRow_m][col];
	  	  if(downVal_m>0)
		  {
			  count_m+=1;
		  }

      lowerRightVal_m = res4[nextRow_m][nextCol_m];

	  	  if(lowerRightVal_m>0)
		  {
			  count_m+=1;
		  }

      if(count_m>threshold_m)
	  {
		  res5[row][col]=255.0;
		  res6[row][col]=255.0;
		  totalx+=col;
		  totaly-=row;
	      ++count_t;
	  }

	  else
	  {
		  res5[row][col]=0.0;
		  res6[row][col]=0.0;
	  }

	  count_m=0;
	  }
   }

   ///////////////////////////////////////////////// Elliptical Section ////////////////////////////////////

   double xcm=totalx/count_t;
   double ycm=totaly/count_t;
   double ycmt = -1*ycm;


	int n1 = floor(rowSize/2+0.5);
	int m1 = floor(colSize/2+0.5);

	int x_pos=xcm-m1;
	int y_pos=n1+ycm;

	double totalxx=0;
	double totalyy=0;
	double totalxy=0;

   double **res9=0;
   res9=new double *[rhomax+1];

   for(int i=0; i<rhomax+1; i++)
   {
	   res9[i]=new double[180];
   }

   for(int k=0; k<rhomax+1; k++)
   {
	   for(int l=0; l<180; l++)
	   {
		   res9[k][l]=0.0;
	   }
   }


	double count_decimal = count_t*1.0;

   for ( int row = 0; row < rowSize; ++row)
   {
	  if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", 65+0.20*row*100 / rowSize, NORMAL);
      }

      for ( int col = 0; col < colSize; ++col)
      {
			 if(res5[row][col]!=0.0)
			 {
					totalxx+=(col-xcm)*(col-xcm)/count_decimal;
					totalyy+=(-row-ycm)*(-row-ycm)/count_decimal;
					totalxy-=(col-xcm)*(-row-ycm)/count_decimal;
			 }
	  }
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
	 

   double orientation = -((atan(eigenvector_2y/eigenvector_2x)*180/pi5)-90.0);

   
   GcpPoint lrPoint;
   lrPoint.mPixel = LocationType(static_cast<int>(xcm), static_cast<int>(ycmt));
   lrPoint.mCoordinate = pCube->convertPixelToGeocoord(lrPoint.mPixel);
   double x_geo = lrPoint.mCoordinate.mX;
   double y_geo = lrPoint.mCoordinate.mY;


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
   double dLat = (y_geo-y_geo_code)*pi5/180;
   double dLon = (x_geo-x_geo_code)*pi5/180;
   double lat1 = y_geo*pi5/180;
   double lat2 = y_geo_code*pi5/180;
   double a2 = sin(dLat/2)*sin(dLat/2)+sin(dLon/2)*sin(dLon/2)*cos(lat1)*cos(lat2);
   double c = 2*atan2(sqrt(a2), sqrt(1.0-a2));
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

   double dLat1 = (y1_scale-y2_scale)*pi5/180;
   double dLon1 = (x1_scale-x2_scale)*pi5/180;
   double lat11 = y1_scale*pi5/180;
   double lat22 = y2_scale*pi5/180;
   double a1 = sin(dLat1/2)*sin(dLat1/2)+sin(dLon1/2)*sin(dLon1/2)*cos(lat11)*cos(lat22);
   double c1 = 2*atan2(sqrt(a1), sqrt(1.0-a1));
   double scale = R*c1*1000;

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

   QStringList Names_e("25.0");
   QString value_e = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a threshold value", "Input a threshold (eg. 5.5)", Names_e);
   
   std::string strAoi_e = value_e.toStdString();
   std::istringstream stm_e;
   stm_e.str(strAoi_e);
   double long threshold27 = 0;
   stm_e >> threshold27;

	 double orientation1=0;
	 double orientation2=0;

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


	int rowSize_e= rhomax+1;
	int colSize_e = 180;

	int total5=0;
	int total_sq5=0;
	int count5=0;

	int totalx3=0;
	int totaly3=0;
	int count3=0;


	for(int col=0; col<colSize_e; col++)
	{
		for(int row=0; row<rowSize_e; row++)
		{
			if (pProgress != NULL)
			{
				pProgress->updateProgress("Calculating statistics", 85+0.15*col*100 / colSize_e, NORMAL);
			}

			if(col>orientation1-10 && col<orientation1+10)
			{
				for(int i=row-5; i<row+6; i++)
				{
					int j=std::max(0,i);
					int k =std::min(rowSize_e-1,j);
					double a = res7[k][col];
					total5+=a;
					total_sq5+=a*a;
					count5+=1;
				}

				double mean_1 = 1.0*total5/count5;
				double std = sqrt((1.0*total_sq5/count5)-(mean_1*mean_1));

				if(std>threshold27)
				{
					totalx3+=col;
					totaly3-=row;
					++count3;
					res9[row][col]=255.0;
				}


				else
				{
					res9[row][col]=0.0;
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
					int k =std::min(rowSize_e-1,j);
					double a = res7[k][col];
					total5+=a;
					total_sq5+=a*a;
					count5+=1;
				}

				double mean_2 = 1.0*total5/count5;
				double std = sqrt((1.0*total_sq5/count5)-(mean_2*mean_2));

				if(std>threshold27)
				{
					totalx3+=col;
					totaly3-=row;
					++count3;
					res9[row][col]=255.0;
				}


				else
				{
					res9[row][col]=0.0;
				}

				total5=0;
				total_sq5=0;
				count5=0;
			}

			else
			{
				res9[row][col]=0.0;
			}
		}
	}
	
	double xcm3=1.0*totalx3/count3;
	double ycm3=1.0*totaly3/count3;

	int n2 = floor(rowSize_e/2+0.5);
	int m2 = floor(colSize_e/2+0.5);

	int y_pos3=-1*(n2+ycm3);

	double y_pos_wake2 = y_pos3/sin(xcm3*pi5/180.0);

	double y_pos_wake1 = x_pos/tan(xcm3*pi5/180.0);

	double y_pos_wake = y_pos_wake2 - y_pos_wake1;

	double y_cm_ship = y_pos;

	double velocity3 = ((y_pos_wake-y_pos)/abs(cos(orientation*pi5/180.0)));

	double velocity11 = velocity3*scale*6.6*1.9438;

	double velocity1 = velocity11/(static_cast<double>(distance));


	      		std::string s29;
				std::stringstream out29;
				out29 << orientation;
				s29 = out29.str();

				std::string s14;
				std::stringstream out14;
				out14 << x_geo;
				s14 = out14.str();

				std::string s15;
				std::stringstream out15;
				out15 << y_geo;
				s15 = out15.str();


				std::string s16;
				std::stringstream out16;
				out16 << velocity1;
				s16 = out16.str();

	
	  
	  pProgress->updateProgress("Orientation of Ship: "+s29+" degrees from the horizontal",0, ERRORS);

	  pProgress->updateProgress("Latitude position of ship: "+s14+" degrees",0, ERRORS);

	  pProgress->updateProgress("Longitude position of ship: "+s15+" degrees",0, ERRORS);
	  
	  pProgress->updateProgress("Velocity of Ship: "+s16+ " knots",0, ERRORS);


   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
   "Wake result", rhomax+1, 180, pDesc->getDataType()));

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


   for(int row=0; row<rhomax+1; row++)
   {
	   for(int col=0; col<180; col++)
	   {
		   pDestAcc->toPixel(row,col);
		   switchOnEncoding(pDesc->getDataType(), conversion11, pDestAcc->getColumn(), res9[row][col]);
	   }
   }


   for(int i=0; i<rowSize; i++)
	   delete[] res[i];
   delete[] res;


   for(int i=0; i<rowSize; i++)
	   delete[] res1[i];
   delete[] res1;


   for(int i=0; i<rhomax+1; i++)
	   delete[] res3[i];
   delete[] res3;

      for(int i=0; i<rhomax+1; i++)
	   delete[] res9[i];
   delete[] res9;

   for(int i=0; i<rowSize; i++)
	   delete[] res4[i];
   delete[] res4;

   for(int i=0; i<rowSize; i++)
	   delete[] res5[i];
   delete[] res5;


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
      pProgress->updateProgress("RRESULT is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("RResult", pResultCube.release());

   pStep->finalize();
   return true;
   
}