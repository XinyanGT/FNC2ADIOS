#include "Prepare.h"

/*******************************
 * Prepare functions
 *******************************/

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;


/****************************************
 * Decide whether the func name has nc 
 * functions' prefix "nf90_" or not
 ****************************************/
inline bool 
HasNcPrefix(const string &funcName)
{
	if ( (funcName.length() > 5) && 
			(funcName.substr(0, 5) == "nf90_") )
		return true;
	else 
		return false;
}


/************************************************
 * Fill in a function name to FUNC enum map 
 ***********************************************/
void 
InitFuncNameIndMap(map<string, FUNC> &nameIndMap)
{
	// Clear the map first
	nameIndMap.clear();

	// Fill in the map 
	nameIndMap.insert(make_pair("nf90_create", NF90_CREATE));
	nameIndMap.insert(make_pair("nf90_def_dim", NF90_DEF_DIM));
	nameIndMap.insert(make_pair("nf90_def_var", NF90_DEF_VAR));
	nameIndMap.insert(make_pair("nf90_enddef", NF90_ENDDEF));
	nameIndMap.insert(make_pair("nf90_put_var", NF90_PUT_VAR));
	nameIndMap.insert(make_pair("nf90_close", NF90_CLOSE));
}


	
/****************************************
 * Only leave supported NetCDF calls
 ****************************************/
void
FilterCall(vector<SgNode *> &callVec,
			vector<SgFunctionCallExp *> &ncCall, 
			const map<string, FUNC> &nameIndMap)
{

	SgFunctionCallExp *callExp;
	string funcName;
	map<string, FUNC>::const_iterator mapItr;

	// Clear nc call vector first 
	ncCall.clear();


	// Iterate through func call 
	for (vector<SgNode*>::iterator itr = callVec.begin();
			itr != callVec.end(); ++itr) {

		// Cast 
	 	callExp = isSgFunctionCallExp(*itr);
		ROSE_ASSERT(callExp != NULL);


		// Supported nc function? 
		funcName = GetCallName(callExp);
		if ( (mapItr = nameIndMap.find(funcName)) != nameIndMap.end()) {
			ncCall.push_back(callExp);
		} else {
			if (HasNcPrefix(funcName)) {
				cout << "ERROR: unsupport NetCDF function: " << 
					funcName << endl;
				exit(1);
			}
		}
	}
}

	

/************************************
 * Group nc functions related to one 
 * ncid together
 ************************************/
void 
GroupNcCall(vector<SgFunctionCallExp*> &callVec, 
			vector< vector<SgFunctionCallExp*> > &callGroupVec)
{
	map<SgInitializedName*, int> ncidMap;

	// Fill a map between declaration of ncid, which occurs in 
	// nc file calls, such as nc_create and nc_create_par 
	//  and index in callGroupVec 
	InitNcidMap(callVec, ncidMap);

	// Group NetCDF functions based on the map generated 
	// in the above step 
	GroupNcCallNow(callVec, callGroupVec, ncidMap);
}


/********************************************
 * Fill a mapping between SgInitializedName*
 * for ncid and index 
 ********************************************/
void 
InitNcidMap(const vector<SgFunctionCallExp*>&callVec, 
				map<SgInitializedName*, int> &ncidMap)
{
	SgFunctionCallExp *callExp;
	SgExpression *exp;
	SgInitializedName *varInitName;
	string funcName;

	int index = 0;;

	ncidMap.clear(); // clear the map first

	for (vector<SgFunctionCallExp*>::size_type i = 0;
		i != callVec.size(); ++i) {

		callExp = callVec[i];
		funcName = GetCallName(callExp);

		// Is it nf90_create call?
		if ( funcName == string("nf90_create") ) {
			exp= GetCallArgs(callExp)[2];

			// Update the map
			varInitName = ArgVarRef_InitName(exp);
			ncidMap.insert(make_pair(varInitName, index++));
		}
	}
}



/******************************************
 * Group NetCDF functions based on a ncid 
 * declaration to index map
 ******************************************/
void
GroupNcCallNow(vector<SgFunctionCallExp*> &callVec, 
			vector< vector<SgFunctionCallExp*> > &callGroupVec,
			map<SgInitializedName*, int>ncidMap)
{
	SgFunctionCallExp *callExp;
	SgInitializedName *varInitName;
	string funcName;


	// Clear and resize callGroupVec
	callGroupVec.clear();
	callGroupVec.resize(ncidMap.size());

	// Iterate through nc call Vec 
	for (vector<SgFunctionCallExp*>::size_type i = 0;
			i != callVec.size(); ++i) {

		callExp = callVec[i];
		funcName = GetCallName(callExp);
		if (funcName == string("nf90_create")) {
			varInitName = ArgVarRef_InitName(GetCallArgs(callExp)[2]); 
		} else {
			varInitName = ArgVarRef_InitName(GetCallArgs(callExp)[0]);
		}

		callGroupVec[ncidMap[varInitName]].push_back(callExp);
	}
}

