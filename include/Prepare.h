#ifndef PREPARE_H 
#define PREPARE_H

#include "Common.h"

inline bool HasNcPrefix(const std::string &funcName);

void InitFuncNameIndMap(std::map<std::string, FUNC> &nameIndMap);

void FilterCall(Rose_STL_Container<SgNode *>&callList,
			std::vector<SgFunctionCallExp *> &ncCall, 
			const std::map<std::string, FUNC> &nameIndMap);


void GroupNcCall(std::vector<SgFunctionCallExp*> &callVec,
			std::vector< std::vector<SgFunctionCallExp*> > &callGroupVec);


void InitNcidMap(const std::vector<SgFunctionCallExp*> &callVec,
				std::map<SgInitializedName*, int> &ncidMap);

void GroupNcCallNow(std::vector<SgFunctionCallExp*> &callVec,
			std::vector< std::vector<SgFunctionCallExp*> > &callGroupVec,
			std::map<SgInitializedName*, int>ncidMap);


#endif