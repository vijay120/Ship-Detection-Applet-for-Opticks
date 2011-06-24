#ifndef HOUGH_H
#define HOUGH_H

#include "ExecutableShell.h"

class HOUGH : public ExecutableShell
{
public:
   HOUGH();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif