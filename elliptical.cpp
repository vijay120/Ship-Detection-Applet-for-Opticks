/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "ELLIPTICAL.h"
#include "TypeConverter.h"
#include <limits>
#include <vector>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>
#include <sstream>
#include "BitMaskIterator.h"
#include <math.h>




REGISTER_PLUGIN_BASIC(OpticksTutorial, ELLIPTICAL);


ELLIPTICAL::ELLIPTICAL()
{
   setDescriptorId("{1046535B-7646-443A-9971-3D1C3C713D99}");
   setName("Elliptical");
   setDescription("Using AOIs.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/Elliptical");
   setAbortSupported(true);
}

ELLIPTICAL::~ELLIPTICAL()
{
}

bool ELLIPTICAL::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Generate statistics for this raster element");
   if (isBatch())
   {
      pInArgList->addArg<AoiElement>("AOI", NULL, "The AOI to calculate statistics over");
   }
   return true;
}

bool ELLIPTICAL::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("orientation", "orientation");
   pOutArgList->addArg<double>("semi major axis", "semi major axis");
   pOutArgList->addArg<double>("semi minor axis", "semi minor axis");
   pOutArgList->addArg<double>("effective area", "effective area");
   return true;
}

bool ELLIPTICAL::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Elliptical", "app", "E4B5D29C-9964-4615-8258-5BB9915768F7");
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

   AoiElement* pAoi = NULL;
   if (isBatch())
   {
      pAoi = pInArgList->getPlugInArgValue<AoiElement>("AOI");
   }
   else
   {
      Service<ModelServices> pModel;
      std::vector<DataElement*> pAois = pModel->getElements(pCube, TypeConverter::toString<AoiElement>());
      if (!pAois.empty())
      {
         QStringList aoiNames("<none>");
         for (std::vector<DataElement*>::iterator it = pAois.begin(); it != pAois.end(); ++it)
         {
            aoiNames << QString::fromStdString((*it)->getName());
         }
         QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Select an AOI", "Select an AOI for processing", aoiNames);
         // select AOI
         if (aoi != "<none>")
         {
            std::string strAoi = aoi.toStdString();
            for (std::vector<DataElement*>::iterator it = pAois.begin(); it != pAois.end(); ++it)
            {
               if ((*it)->getName() == strAoi)
               {
                  pAoi = static_cast<AoiElement*>(*it);
                  break;
               }
            }
            if (pAoi == NULL)
            {
               std::string msg = "Invalid AOI.";
               pStep->finalize(Message::Failure, msg);
               if (pProgress != NULL)
               {
                  pProgress->updateProgress(msg, 0, ERRORS);
               }

               return false;
            }
         }
      }
   } // end if


   const BitMask* pPoints = (pAoi == NULL) ? NULL : pAoi->getSelectedPoints();
   BitMaskIterator it(pPoints, pCube);
    int startRow = it.getBoundingBoxStartRow();
    int startColumn = it.getBoundingBoxStartColumn();
    int endRow = it.getBoundingBoxEndRow();
    int endColumn = it.getBoundingBoxEndColumn();

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(pDesc->getActiveRow(startRow), pDesc->getActiveRow(endRow));
   pRequest->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(endColumn));
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   FactoryResource<DataRequest> pRequest2;
   pRequest2->setRows(pDesc->getActiveRow(startRow), pDesc->getActiveRow(endRow));
   pRequest2->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(endColumn));
   pRequest2->setInterleaveFormat(BSQ);
   DataAccessor pAcc2 = pCube->getDataAccessor(pRequest2.release());
   double total = 0.0;
    int count = 0;
    int totalx=0;
    int totaly=0;
   int numerator=0;
   int denominator=0;
   double m=0.0;
    double totalxy=0;
    int totalx_sq=0;
	double totalxx=0;
	double totalyy=0;
      

   
      for ( int row = startRow; row <= endRow; ++row)
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
         pProgress->updateProgress("Calculating statistics", 0.5*row * 100 / pDesc->getRowCount(), NORMAL);
      }

      for ( int col = startColumn; col <= endColumn; ++col)
      {
         if (pPoints == NULL || pPoints->getPixel(col, row))
         {
			 if(pAcc->getColumnAsInteger()!=0)
			 {
					totalx+=col;
					totaly-=row;
					++count;
			 }
			 
		 }
		 pAcc->nextColumn();
	  }
      pAcc->nextRow();
	  }

	  double xcm=totalx/count;
	  double ycm=totaly/count;
	  /*
					std::string p;
					std::stringstream out4;
					out4 << xcm;
					p = out4.str();
					pProgress->updateProgress("xcm "+p,0, ERRORS);

					std::string p2;
					std::stringstream out5;
					out5 << ycm;
					p2 = out5.str();
					pProgress->updateProgress("ycm "+p2,0, ERRORS);
					*/



	  //pAcc->toPixel(startRow, startColumn);

	 for ( int row = startRow; row <= endRow; ++row)
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

      for ( int col = startColumn; col <= endColumn; ++col)
      {
         if (pPoints == NULL || pPoints->getPixel(col, row))
         {
			 if(pAcc2->getColumnAsInteger()!=0)
			 {
					
					totalxx+=(col-xcm)*(col-xcm)/count;
					totalyy+=(-row-ycm)*(-row-ycm)/count;
					totalxy-=(col-xcm)*(-row-ycm)/count;

					/*
					std::string s;
					std::stringstream out;
					out << totalxx;
					s = out.str();
					pProgress->updateProgress("totalxx"+s,0, ERRORS);

					std::string q;
					std::stringstream out1;
					out1 << totalyy;
					q = out1.str();
					pProgress->updateProgress("totalyy"+q,0, ERRORS);

					std::string f;
					std::stringstream out2;
					out2 << totalxy;
					f = out2.str();
					pProgress->updateProgress("totalxy"+f,0, ERRORS);
					*/
			 }
			 
		 }
		 pAcc2->nextColumn();
	  }
      pAcc2->nextRow();
	 }

	 double t=totalxx+totalyy;
	 double d = (totalxx*totalyy)-(totalxy*totalxy);
	 double l1 = (t/2)+sqrt((t*t/4)-d);
	 double l2 = (t/2)-sqrt((t*t/4)-d);
	 /*
	 	 			std::string e;
					std::stringstream out12;
					out12 << l1;
					e = out12.str();
					pProgress->updateProgress("l1 "+e,0, ERRORS);

	 	 			std::string e1;
					std::stringstream out13;
					out13 << l2;
					e1 = out13.str();
					pProgress->updateProgress("l2 "+e1,0, ERRORS);*/



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
	 
	 /*
	 double tester = eigenvector_2y/eigenvector_2x;
	 				std::string f1;
					std::stringstream out9;
					out9 << tester;
					f1 = out9.str();
					pProgress->updateProgress("tester "+f1,0, ERRORS);

	 				std::string v;
					std::stringstream out10;
					out10 << eigenvector_2y;
					v = out10.str();
					pProgress->updateProgress("eigenvector_2y "+v,0, ERRORS);


	 				std::string l;
					std::stringstream out11;
					out11 << eigenvector_2x;
					l = out11.str();
					pProgress->updateProgress("eigenvector_2x "+l,0, ERRORS);
					*/

	 double orientation = -((atan(eigenvector_2y/eigenvector_2x)*180/3.14159265)-90.0);
	 double semimajor = sqrt(l1)*2.0;
	 double semiminor = sqrt(l2)*2.0;
	 double area = 3.14159265*semimajor*semiminor;




   if (pProgress != NULL)
   {
      std::string msg = "Orientation: " + StringUtilities::toDisplayString(orientation) + " degrees from horizontal"+ "\n"
						+"Semi Major Axis: "+ StringUtilities::toDisplayString(semimajor)+ " pixels"+ "\n"
						+"Semi Minor Axis: "+StringUtilities::toDisplayString(semiminor)+ " pixels" + "\n"
						+"Effective area of ship: "+StringUtilities::toDisplayString(area)+" pixels^2";
      pProgress->updateProgress(msg, 100, NORMAL);
   }
   
   pStep->addProperty("Orientation", orientation);
   pStep->addProperty("Semi Major Axis", semimajor);
   pStep->addProperty("Semi Minor Axis", semiminor);
   pStep->addProperty("Effective area of ship", area);

   
   pOutArgList->setPlugInArgValue("Orientation", &orientation);
   pOutArgList->setPlugInArgValue("Semi Major Axis", &semimajor);
   pOutArgList->setPlugInArgValue("Semi Minor Axis", &semiminor);
   pOutArgList->setPlugInArgValue("Effective area of ship", &area);


   pStep->finalize();
   return true;
}