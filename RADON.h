#ifndef RADON_H
#define RADON_H

#include "ExecutableShell.h"

class RADON : public ExecutableShell
{
public:
   RADON();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif