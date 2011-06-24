#ifndef HOUGH1_H
#define HOUGH1_H

#include "ExecutableShell.h"

class HOUGH1 : public ExecutableShell
{
public:
   HOUGH1();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif