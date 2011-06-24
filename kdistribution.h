#ifndef KDISTRIBUTION_H
#define KDISTRIBUTION_H

#include "ExecutableShell.h"

class KDISTRIBUTION : public ExecutableShell
{
public:
   KDISTRIBUTION();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif