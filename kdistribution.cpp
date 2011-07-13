/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


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


#include <QtCore/QEvent>
#include <QtCore/QFileInfo>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include "GcpList.h"
#include "LayerList.h"
#include "ModelServices.h"
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
#include "kdistribution.h"
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

#define M_PI 3.14159265358979323846264338327







REGISTER_PLUGIN_BASIC(OpticksTutorial, KDISTRIBUTION);


   string sconverter1(int i)
   {
	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
   }
   

   double gamma(double x)
{
    int i,k,m;
    double ga,gr,r,z;
	    static double g[] = {
        1.0,
        0.5772156649015329,
		-0.6558780715202538,
		-0.420026350340952e-1,
        0.1665386113822915,
		-0.421977345555443e-1,
		-0.9621971527877e-2,
        0.7218943246663e-2,
		-0.11651675918591e-2,
		-0.2152416741149e-3,
        0.1280502823882e-3,
		-0.201348547807e-4,
		-0.12504934821e-5,
        0.1133027232e-5,
		-0.2056338417e-6,
        0.6116095e-8,
        0.50020075e-8,
		-0.11812746e-8,
        0.1043427e-9,
        0.77823e-11,
		-0.36968e-11,
        0.51e-12,
		-0.206e-13,
		-0.54e-14,
        0.14e-14};
	    if (x > 171.0) return 1e308;    // This value is an overflow flag.
    if (x == (int)x) {
        if (x > 0.0) {
            ga = 1.0;               // use factorial
            for (i=2;i<x;i++) {
				ga *= i;
            }
		}
		else
            ga = 1e308;
	}
	else {
        if (fabs(x) > 1.0) {
            z = fabs(x);
            m = (int)z;
            r = 1.0;
            for (k=1;k<=m;k++) {
                r *= (z-k);
            }
            z -= m;
        }
        else
            z = x;
        gr = g[24];
        for (k=23;k>=0;k--) {
            gr = gr*z+g[k];
        }
        ga = 1.0/(gr*z);
        if (fabs(x) > 1.0) {
            ga *= r;
            if (x < 0.0) {
                ga = -M_PI/(x*ga*sin(M_PI*x));
            }
        }
    }
    return ga;
}

   
	  namespace
{
   template<typename T>
   void conversion1(T* pData, double number)
   {
	   *pData=number;
   }
};

   void updateStatistics3(double pData, double& total, double& total_sum, int& count)
   {
      total += pData;
	  total_sum += pData * pData;
	  count+=1;
   }




KDISTRIBUTION::KDISTRIBUTION()
{
   setDescriptorId("{6EB93DA1-50C0-47FE-8467-A2E3F286C56D}");
   setName("kdistribution");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/kdistribution");
   setAbortSupported(true);
}



bool KDISTRIBUTION::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "CFAR k-distribution detection of image");
   return true;
}

bool KDISTRIBUTION::getOutputSpecification(PlugInArgList* &pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool KDISTRIBUTION::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
	
   StepResource pStep("KDISTRIBUTION", "app10", "F298D57C-D816-42F0-AE27-43DAA02C0544");
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
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pCube->getDataDescriptor());


  
   int tester_count = 0;
   int eastCol = 0;
   int northRow = 0;
   int westCol = 0;
   int southRow = 0;
   double zstatistic = 0;
   double total = 0.0;
   double total_sum = 0.0;
   double mean = 0.0;
   double std = 0.0;
   double a=0;
   int rowSize=pDesc->getRowCount();
   int colSize=pDesc->getColumnCount();
   int prevCol = 0;
   int prevRow = 0;
   int nextCol = 0;
   int nextRow = 0;
   double long PFA = 0.0;
   int DEPTH1 = 10;
   int DEPTH2 = 10;
   int DEPTH3 = 1;
   int DEPTH4 = 1;
   int count=0;
   int zero=0;
   double long threshold = 100000.0;


   double look_table1[24][6];

   for(int i=0; i<24; i++)
   {
	   for(int j=0; j<3; j++)
	   {
			   look_table1[i][j]=0.0;
			   	   
	   }
   }
      


   QStringList Names("0.0000001");
   QString value = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input a PFA value", "Input a PFA value (0.0000001 or 0.00000001)", Names);
   
   std::string strAoi = value.toStdString();
   std::istringstream stm;
   stm.str(strAoi);
   //stm >> PFA;
   PFA=::atof(strAoi.c_str());

   

   if (PFA==0.0000001)
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
   
   
   if (PFA==0.00000001)
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
   //stm1 >> DEPTH1;
   DEPTH1=::atof(strAoi1.c_str());

   QStringList Names2("10");
   QString value2 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the window height", "Input the size of the window height in terms of the number of pixels (eg. 10)", Names2);
   
   std::string strAoi2 = value2.toStdString();
   std::istringstream stm2;
   stm2.str(strAoi2);
   //stm2 >> DEPTH2;
   DEPTH2=::atof(strAoi2.c_str());

   QStringList Names3("1");
   QString value3 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the gaurd width", "Input the size of the guard width in terms of the number of pixels (eg. 1)", Names3);
   
   std::string strAoi3 = value3.toStdString();
   std::istringstream stm3;
   stm3.str(strAoi3);
   //stm3 >> DEPTH3;
   DEPTH3=::atof(strAoi3.c_str());

   QStringList Names4("1");
   QString value4 = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Input the size of the guard height", "Input the size of the guard height in terms of the number of pixels (eg. 1)", Names4);
   
   std::string strAoi4 = value4.toStdString();
   std::istringstream stm4;
   stm4.str(strAoi4);
   stm4 >> DEPTH4;
   DEPTH4=::atof(strAoi4.c_str());

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
		  //p[col]=pAcc2->getColumnAsInteger();
		  
		  westCol=max(col-DEPTH1,zero);
		  northRow=max(row-DEPTH2,zero);
		  eastCol=min(colSize-1,col+DEPTH1);
		  southRow=min(rowSize-1,row+DEPTH2);
		  prevCol=max(col-DEPTH3,zero);
		  prevRow=max(row-DEPTH4,zero);
		  nextCol=min(col+DEPTH3,colSize-1);
		  nextRow=min(row+DEPTH4,rowSize-1);

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
					 updateStatistics3(pAcc2->getColumnAsDouble(), total, total_sum, count);
					}


					pAcc2->nextColumn();

				}

				pAcc2->nextRow();
			}

			mean = total / count;
			std = sqrt(total_sum / count - mean * mean);
			int ELVI = (mean/std)*(mean/std);
			int v = (ELVI+1)/((ELVI*mean/(std*std))-1);

			pAcc2->toPixel(row,col);
			pDestAcc->toPixel(row,col);
			zstatistic = (pAcc2->getColumnAsDouble()-mean)/std;

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
					threshold=look_table1[i][2];
				}
			}
					
			

			if(zstatistic>threshold)
			{

				switchOnEncoding(pDesc->getDataType(), conversion1, pDestAcc->getColumn(), 1000.0);
			}

			else
			{
				switchOnEncoding(pDesc->getDataType(), conversion1, pDestAcc->getColumn(), 0.0);
			}

			total = 0.0;
			total_sum=0.0;
            threshold=100000.0;
            mean = 0.0;
            std = 0.0;
			count=0;



			pAcc->nextColumn();
	  }

      pAcc->nextRow();

   }


      // Create a GCP layer
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

         // Reset the coordinates to be in active numbers relative to the chip
         const vector<DimensionDescriptor>& chipRows = pDescriptor->getRows();
         const vector<DimensionDescriptor>& chipColumns = pDescriptor->getColumns();
		 /*
         VERIFYNRV(chipRows.front().isActiveNumberValid() == true);
         VERIFYNRV(chipRows.back().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.front().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.back().isActiveNumberValid() == true);
		 */
         unsigned int chipStartRow = chipRows.front().getActiveNumber();
         unsigned int chipEndRow = chipRows.back().getActiveNumber();
         unsigned int chipStartCol = chipColumns.front().getActiveNumber();
         unsigned int chipEndCol = chipColumns.back().getActiveNumber();
         ulPoint.mPixel = LocationType(chipStartCol, chipStartRow);
         urPoint.mPixel = LocationType(chipEndCol, chipStartRow);
         llPoint.mPixel = LocationType(chipStartCol, chipEndRow);
         lrPoint.mPixel = LocationType(chipEndCol, chipEndRow);
         centerPoint.mPixel = LocationType((chipStartCol + chipEndCol) / 2, (chipStartRow + chipEndRow) / 2);

         // Create the GCP list
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

		 }
		 
	  
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

   pOutArgList->setPlugInArgValue("Result", pResultCube.release());

   pStep->finalize();
   return true;
   
}