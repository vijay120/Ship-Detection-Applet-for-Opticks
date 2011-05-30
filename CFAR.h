#ifndef CFAR_H
#define CFAR_H

#include "ExecutableShell.h"

class CFAR : public ExecutableShell
{
public:
   CFAR();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif