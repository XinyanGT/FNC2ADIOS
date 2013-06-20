#ifndef ROSEHELPERADV_H
#define ROSEHELPERADV_H

#include "Common.h"
#include "RoseHelper.h"

SgEnumDeclaration *GetEnumDecl(const std::string &name);

int GetEnumIntVal(SgEnumDeclaration *decl, const std::string &name);

SgExpression *
GetEnumExpr(const std::string &enumType, const std::string &enumConstant);

void PrintFuncCallInfo(SgNode *node);

void PrintFuncDeclInfo(SgNode *node, const SgStringList &srcNameList);

SgEnumVal* BuildEnumVal(int value, SgEnumDeclaration* decl, SgName name);

SgForStatement * BuildCanonicalForStmt(int start, int end,
			const std::string &indexName = std::string());

SgForStatement * BuildCanonicalForStmtDeclIn(int start, int end);

SgForStatement *
BuildCanonicalForStmtDeclOut(int start, int end, std::string indexName);

void ExtractForStmtBounds(SgForStatement *forStmt, int &low, int &high);

SgVariableDeclaration *FindLastVarDecl(SgScopeStatement *scope);

SgExprListExp * 
GetDimidVec(SgInitializedName *initName, SgFunctionCallExp *callExp);

std::string
GetFuncArgName(SgScopeStatement *scope, const std::string &funcName, int index);


#endif