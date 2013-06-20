#ifndef MPIUTILS_H
#define MPIUTILS_H

#include "Common.h"
#include "RoseHelper.h"
#include "RoseHelperAdv.h"


SgTypedefDeclaration *
Get_MPI_Comm_Declaration(SgNode *node = NULL);

void InsertMPI(SgProject *project);

SgExprStatement * 
BuildMPIBarrier(const std::string &errVarName, SgExpression *commExp);

SgExprStatement *
BuildMPIBarrier(const std::string &errVarName, const std::string &commVarName);



#endif
