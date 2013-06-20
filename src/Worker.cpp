/******************************************************
 * Worker functions for some higher level extract and 
 * process functions
 *****************************************************/

#include "Group.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;


/******************************************************
 * Process one nf90_def_dim function
 **************************************************/
void 
Group::Process_nf90_def_dim_Worker(SgFunctionCallExp *callExp)
{

	// Scope
	SgStatement *orginStmt = getEnclosingStatement(callExp);
	SgScopeStatement *scope = getScope(orginStmt);
	pushScopeStack(scope);

//	int idOffset = 0;			// in case of ARRAY_OTHER

	// Get arg expressions
	SgExpression *nameExp = GetCallArgs(callExp)[1];
	SgExpression *lenExp = GetCallArgs(callExp)[2];
	SgExpression *dimidExp = GetCallArgs(callExp)[3];


	// Name for this dimension 
	string name, nameVar, adName;
	name = ArgCharPtr(nameExp, nameVar);
	if (!name.empty()) {
		cout << "\tdim name: " << name << endl;
		adName = name;
	} else {
		cout << "\tdim name is in var: " << nameVar << endl;
		adName = nameVar;
	}
	adName = "adios_" + adName;


	// Variable for the dimension id 
	SgInitializedName *dimidInitName;
	IDTYPE idType;

	// The id var is a single variable?
	if (dimidExp->variantT() == V_SgVarRefExp) {
		dimidInitName = ArgVarRef_InitName(dimidExp);
		idType = SINGLE;
	} else {
		cout << "ERROR: unsupported dimidExp class name: "
				<< dimidExp->class_name() << endl;
		exit(1);
	}

	// string adVarIDName = adName + "_id";
	// SgVariableDeclaration *adVarIDDecl = 
	// 	buildVariableDeclaration(adVarIDName,
	// 		buildFortranKindType(buildIntType(), buildIntVal(8)));


	// Declare length variable
	SgVariableDeclaration *adVarDecl = 
		buildVariableDeclaration(adName, buildIntType());

	// Assign the length variable
	SgStatement *adVarAssign = 
		buildExprStatement(
			buildAssignOp(
				buildVarRefExp(adName),
				copyExpression(lenExp)
			)
		);

	// Insert
	SgVariableDeclaration *lastDecl = FindLastVarDecl(scope);
	assert(lastDecl != NULL);
	insertStatementAfter(lastDecl, adVarDecl);
	insertStatementAfter(orginStmt, adVarAssign);

	// Build adios_define_var call
	string adVarIDName = "adios_varid";
	SgExprStatement *adDefVarCall = BuildAdDefVar(adName, 2, adVarIDName);


	// Insert, remove calls and pop scope
	insertStatementAfter(adVarAssign, adDefVarCall);
	removeStatement(orginStmt);
	popScopeStack();
	

	// SINGLE, then append to DimMap 
	if (idType == SINGLE) {
		DimMap.insert(make_pair(dimidInitName, vector<string>(1, adName))); 
	} else {
		cout << "Invalid idType: " << idType << endl;
		exit(1);
	}

}


/*********************************************
 * Extract info from one nf90_def_var
 *********************************************/
void 
Group::Extract_nf90_def_var_Worker(SgFunctionCallExp *callExp)
{

	// Get arg expressions
	SgExpression *nameExp, *xtypeExp, *dimidsExp, *varidExp;
	nameExp = GetCallArgs(callExp)[1];
	xtypeExp = GetCallArgs(callExp)[2];
	dimidsExp = GetCallArgs(callExp)[3];
	varidExp = GetCallArgs(callExp)[4];

	// Variable name 
	string name, nameVar, adName;
	name = ArgCharPtr(nameExp, nameVar);
	if (!name.empty()) {
		cout << "\tvar name: " << name << endl;
		adName = name;
	} else {
		cout << "\tvar name is in var: " << nameVar << endl;
		adName = nameVar;
	}

	// xtype 
	int adType, unitSize;

	assert(xtypeExp->variantT() == V_SgVarRefExp);
	string xtype = GetVarName(xtypeExp);
	// nf90_int
	if (strcasecmp(xtype.c_str(), "nf90_int") == 0) {
		adType = 2;
		unitSize = 4;	
	// nf90_float
	} else if (strcasecmp(xtype.c_str(), "nf90_float") == 0) {
		adType = 5;
		unitSize = 4;
	// Invalid
	} else {
		cout << "ERROR: unsupported NetCDF xtype: " 
			<< xtype << " .Quit. " << endl;
		exit(1);
	}


	// Dimension ids
	vector<DimT*> dimTVec;
	vector<string> dimStrVec;

	SgInitializedName *dimidsInitName = 
		ArgVarRef_InitName(dimidsExp);
		
	SgVarRefExp *dimVarRef;
	map<SgInitializedName*, vector<string> >::iterator itr 
		= DimMap.find(dimidsInitName);
	assert(itr == DimMap.end());

	vector<SgExpression*> dimidVec =
		GetDimidVec(dimidsInitName, callExp)->get_expressions();

	// Construct dimTVec and dimStrVec
	// Iterate through the dimension id vector
	for (vector<SgExpression*>::size_type i = 0;
			i < dimidVec.size(); i++) {

		dimVarRef = isSgVarRefExp(dimidVec[i]);
		assert(dimVarRef != NULL);

		// Search DimMap
		itr = DimMap.find(GetVarRef_InitName(dimVarRef));
		assert(itr != DimMap.end());

		// Update dimStrVec
		dimStrVec.insert(dimStrVec.end(), 
			itr->second.begin(), itr->second.end());

	 	// Update dimTVec
		dimTVec.push_back(&(*itr));
	}


	// varid 
	SgInitializedName *varidInitName = 
		ArgVarRef_InitName(varidExp);

	// Append to VarMap
	VarMap.insert( make_pair(varidInitName, 
				VarSec(dimTVec, dimStrVec, adName, adType, unitSize)) ); 


}


/*********************************************
 * Extract info from one nf90_put_var call
 *******************************************/
void
Group::Extract_nf90_put_var_Worker(SgFunctionCallExp *callExp)
{


	// Get arg list
 	const vector<SgExpression*> &argVec = GetCallArgs(callExp);

 	// Assume arg num >=5
	assert(argVec.size() >= 5);

 	// varid	
	SgExpression *varidExp = argVec[1];
	SgInitializedName *varidInitName = ArgVarRef_InitName(varidExp);

	// Search VarMap
	map<SgInitializedName*, VarSec>::iterator itr 
		= VarMap.find(varidInitName);
	assert(itr != VarMap.end());

	// Dimension number
	vector<string>::size_type ndims = itr->second.DimStrVec.size();
	cout << "ndims: " << ndims << endl;

	// Assume putting a global variable
	itr->second.IsGlobal = true;


 	// start 
 	SgActualArgumentExpression *startArgExp = 
	 	isSgActualArgumentExpression(argVec[3]);
	assert(startArgExp != NULL);
	assert( strcasecmp(GetActArgName(startArgExp).c_str(), "start") == 0 );
	SgExpression *startExp = startArgExp->get_expression();
	cout << "start class name: " << startExp->class_name() << endl;


	// count
 	SgActualArgumentExpression *countArgExp = 
	 	isSgActualArgumentExpression(argVec[4]);
	assert(countArgExp != NULL);
	assert( strcasecmp(GetActArgName(countArgExp).c_str(), "count") == 0 );
	SgExpression *countExp = countArgExp->get_expression();
	cout << "count class name: " << countExp->class_name() << endl;


	// Set CountInit and OffsetInt 
	// First check their dimensions
	SgArrayType *startType = isSgArrayType(ArgVarRef_Type(startExp));
	SgArrayType *countType = isSgArrayType(ArgVarRef_Type(countExp));
	assert(startType != NULL);
	assert(countType != NULL);

	const vector<SgExpression*> &startDimInfo = 
		startType->get_dim_info()->get_expressions();
	const vector<SgExpression*> &countDimInfo = 
		countType->get_dim_info()->get_expressions();

	assert(startDimInfo.size() == 1);
	assert(countDimInfo.size() == 1);
	assert(startDimInfo[0]->variantT() == V_SgVarRefExp);
	assert(countDimInfo[0]->variantT() == V_SgVarRefExp);

	assert(GetVarRef_InitIntVal(static_cast<SgVarRefExp*>(startDimInfo[0]))
		== ndims);
	assert(GetVarRef_InitIntVal(static_cast<SgVarRefExp*>(countDimInfo[0]))
		== ndims);	

	itr->second.OffsetInit = ArgVarRef_InitName(startExp);
	itr->second.CountInit = ArgVarRef_InitName(countExp);


	// Set IterNum 
	itr->second.IterNum = 1;	
	// int low, high;
	// SgStatement *orginStmt;
	// SgScopeStatement *scopeStmt;

	// orginStmt = getEnclosingStatement(callExp);
	// // scopeStmt = getScope(orginStmt);
	// // cout << "class name of scope of put function is " << 
	// // 	scopeStmt->class_name() << endl;;
	// scopeStmt = findEnclosingLoop(orginStmt);
	// SgForStatement *forStmt = isSgForStatement(scopeStmt);
	// assert(forStmt != NULL);

	// ExtractForStmtBounds(forStmt, low, high);
	// itr->second.IterNum = high - low;
	// cout << "Iteration number is: " << itr->second.IterNum << endl;

	// Update PutVarVec 
	PutVarVec.push_back(&(*itr));

}


/**********************************************
 * Process one nf90_def_var function call
 **********************************************/
void 
Group::Process_nf90_def_var_Worker(SgFunctionCallExp *callExp)
{

	// Scope
	SgStatement *orginStmt = getEnclosingStatement(callExp);
	SgScopeStatement *scope = getScope(orginStmt);
	pushScopeStack(scope);

	// Get varid  
	SgExpression *varidExp;
	varidExp = GetCallArgs(callExp)[4];
	SgInitializedName *varidInitName = 
		ArgVarRef_InitName(varidExp);

	// Get DimStrVec according to the varid
	map<SgInitializedName*, VarSec>::iterator itr 
		= VarMap.find(varidInitName);
	assert(itr != VarMap.end());
	assert(itr->second.IsGlobal);
	const vector<string> &strVec = itr->second.DimStrVec;


	// adios_define_var calls for count, offset vars 

	// Count var declarations 
	SgStatement *prevStmt = orginStmt; 
	SgExprStatement *adDefVarCall;
	string adVarIDName = "adios_varid";
	assert(prevStmt!= NULL);

	for (vector<string>::size_type i = 0; 
			i < strVec.size(); ++i) {
		// Build
		adDefVarCall = BuildAdDefVar("c"+strVec[i], 2, adVarIDName);
		// Insert
		insertStatementAfter(prevStmt, adDefVarCall);
		prevStmt = adDefVarCall;

	}

	// Offset var declarations(Use a For Statement if necessary)
	SgForStatement *forStmt;
	SgBasicBlock *forBody;

	// If netcdf put function is in a For Statement, and iteration
	// number is more than 1
	if (itr->second.IterNum > 1) {
		forStmt = BuildCanonicalForStmt(0, itr->second.IterNum);
		forBody = isSgBasicBlock(forStmt->get_loop_body());
		assert(forBody != NULL);
		insertStatementAfter(prevStmt, forStmt);
		prevStmt = forStmt;
		pushScopeStack(forBody);

		for (vector<string>::size_type i = 0; 
				i < strVec.size(); ++i) {
			// Build
			adDefVarCall = BuildAdDefVar("o"+strVec[i], 2, adVarIDName);
			// Insert
			appendStatement(adDefVarCall);
		}			
		popScopeStack();

	} else {

		for (vector<string>::size_type i = 0; 
				i < strVec.size(); ++i) {
			// Build
			adDefVarCall = BuildAdDefVar("o"+strVec[i], 2, adVarIDName);
			// Insert
			insertStatementAfter(prevStmt, adDefVarCall);
			prevStmt = adDefVarCall;
		}
	}


 	// adios_define_var call for the data var
	adDefVarCall = 
		BuildAdDefVar(itr->second.Name, itr->second.Type, adVarIDName,
			MakeStr(strVec, "c"), MakeStr(strVec, ""),
			MakeStr(strVec, "o")
		);

	// Insert, remove and pop scope
	insertStatementAfter(prevStmt, adDefVarCall);
//	insertStatementAfter(orginStmt, adDefVarCall);
	removeStatement(orginStmt);
	popScopeStack();
	

}

/******************************************
 * Process one nf90_put_var call
 ******************************************/
void
Group::Process_nf90_put_var_Worker(int index)
{

	// Scope
	SgStatement *putStmt = getEnclosingStatement(CallVV[NF90_PUT_VAR][index]);
	SgScopeStatement *putScope = getScope(putStmt);
	pushScopeStack(putScope);
	SgStatement *prevStmt = putStmt;

	// Data
	SgExpression *dataExp = GetCallArgs(CallVV[NF90_PUT_VAR][index])[2];
	assert(dataExp->variantT() == V_SgVarRefExp);
	string dataName = GetVarName(dataExp);

	// Get VarSec pointer
	VarSec *vsp = &(PutVarVec[index]->second);

	// Dimension string vec
	const vector<string> &dimStrVec = vsp->DimStrVec;

	// Dimension number 
	int ndims = dimStrVec.size();

	// If a global variable is to be put
	string cName, oName;
	SgStatement *writeStmt;
	if (vsp->IsGlobal) {

		cName = vsp->CountInit->get_name().getString();
		oName = vsp->OffsetInit->get_name().getString();

		// Iterate through the dimension string vec
		for (vector<string>::size_type i = 0; i < ndims; i++) {

			// count
			writeStmt = BuildAdWrite("c"+dimStrVec[i], cName, i+1); 
			insertStatementAfter(prevStmt, writeStmt);
			prevStmt = writeStmt;

			// global
			writeStmt = BuildAdWrite(dimStrVec[i], dimStrVec[i]); 
			insertStatementAfter(prevStmt, writeStmt);
			prevStmt = writeStmt;	

			// offset
			writeStmt = BuildAdWrite("o"+dimStrVec[i], oName, i+1, true); 
			insertStatementAfter(prevStmt, writeStmt);
			prevStmt = writeStmt;
		}
	} else {

	// Local var
		// Only has local dimensions
		for (vector<string>::size_type i = 0; i < ndims; i++) {

			writeStmt = BuildAdWrite(dimStrVec[i], dimStrVec[i]);
			insertStatementAfter(prevStmt, writeStmt);
			prevStmt = writeStmt;
		}
	}

	// Data
	writeStmt = BuildAdWrite(vsp->Name, dataName);
	insertStatementAfter(prevStmt, writeStmt);
	prevStmt = writeStmt;

	// Remove the original put call
	removeStatement(putStmt);

	// Pop stack 
	popScopeStack();
}