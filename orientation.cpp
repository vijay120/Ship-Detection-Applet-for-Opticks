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
#include "ORIENTATION.h"
#include "TypeConverter.h"
#include <limits>
#include <vector>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>
#include <sstream>
#include "BitMaskIterator.h"






REGISTER_PLUGIN_BASIC(OpticksTutorial, ORIENTATION);



   void updateNumerator(int col, int row, int meanx, int numerator)
   {
	   numerator+=(col-meanx)*row;
   }

   void updateDenominator(int row, int meany, int denominator)
   {
	   denominator+=(row-meany)*(row-meany);
   }



ORIENTATION::ORIENTATION()
{
   setDescriptorId("{BEB0B654-710E-41D1-B3FC-A4459C6EBCA1}");
   setName("Orientation");
   setDescription("Using AOIs.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/Orientation");
   setAbortSupported(true);
}

ORIENTATION::~ORIENTATION()
{
}

bool ORIENTATION::getInputSpecification(PlugInArgList*& pInArgList)
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

bool ORIENTATION::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("m", "gradient");
   return true;
}

bool ORIENTATION::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Orientation", "app", "6988470F-DEAA-4AD4-A1F5-6E0E138BE5CE");
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
   double total = 0.0;
    int count = 0;
    int totalx=0;
    int totaly=0;
   int numerator=0;
   int denominator=0;
   double m=0.0;
    int totalxy=0;
    int totalx_sq=0;
      

   
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
					totalxy+=(-1)*col*row;
					totalx+=col;
					totaly-=row;
					++count;
					totalx_sq+=col*col;

			 }
			 
		 }
		 pAcc->nextColumn();
	  }
      pAcc->nextRow();
   }

	  m = (totalxy-((totalx*totaly)/count))/((totalx_sq/1.0)-((totalx*totalx)/count));


   if (pProgress != NULL)
   {
      std::string msg = "Orientation: " + StringUtilities::toDisplayString(m) + "\n";
      pProgress->updateProgress(msg, 100, NORMAL);
   }
   
   pStep->addProperty("Orientation", m);
   
   pOutArgList->setPlugInArgValue("gradient", &m);

   pStep->finalize();
   return true;
}
