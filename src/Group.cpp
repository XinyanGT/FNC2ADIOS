/*******************************************
 * High level functions for Group
 *******************************************/

#include "Group.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;


/*************************************************************
 * Constructor
 * Input:
 *	vector<SgFunctionCallExp*> vec: netcdf calls that belong 
 *		to one group
 *	const map<string, FUNC> &nameIndMap: a map between netcdf
 *		function names and their indices
 *	int id: id for the group
 ************************************************************/

Group::Group(vector<SgFunctionCallExp*> vec, 
		const map<string, FUNC> &nameIndMap, int id) 
	: Name("DefaultGroup"), FileName("DefaultFile"), GroupID(id),
		CommExp(NULL), FuncNameIndMap(&nameIndMap)
{

	// Set var names of adios group id, adios file handler, 
	// and adios err
	char groupIDStr[30];
	snprintf(groupIDStr, 30, "%d", GroupID); 
	GroupIDVar = string("adios_group") + groupIDStr;
	FileVar = string("adios_file") + groupIDStr;
	ErrVar = string("adios_err") + groupIDStr;

	// Initialize CallVV
	CallVV.resize(FuncNameIndMap->size());
	cout << "Initializing CallVV..." << endl;
	InitCallVV(vec); 


	// Parallel I/O?
//	FillIsPara();
	IsPara = true;

}


/*************************************************
  * Process nf90_create function calls
  * There should only one nf90_create call
  * Output:
  *		string Name: group name
  *		string FileName: output file name
  *		int Cmode: nc create par mode
  **********************************************/
 void
 Group::Process_nf90_create()
 {
 	assert(CallVV[NF90_CREATE].size() == 1);
 	SgFunctionCallExp *callExp = CallVV[NF90_CREATE][0];

 	// File path 
 	SgExpression *pathExp = GetCallArgs(callExp)[0];
 	string path, pathVar;
 	path = ArgCharPtr(pathExp, pathVar);

 	// Is it given directly?
 	if (!path.empty()) {
 		cout << "\tFile name before: " << path << endl;
 		GetNames(path);
 		cout << "\tFile name after: " << FileName << endl;
 		cout << "\tGroup name: " << Name << endl;

 	} else {
 		cout << "\tFile name is in var: " << pathVar << endl;
 		cout << "\tCan NOT deal with it yet" << endl;
 		cout << "\tSet file name to default value:" << FileName << endl;
 	}


 	// File creation mode
 	// (TODO) update Cmode
 	SgExpression *cmodeExp = GetCallArgs(callExp)[1];
 	SgFunctionCallExp *IORexp = isSgFunctionCallExp(cmodeExp);
 	assert(IORexp != NULL);
 	SgFunctionRefExp *IExp, *JExp;
 	IExp = isSgFunctionRefExp(GetCallArgs(IORexp)[0]);
 	JExp = isSgFunctionRefExp(GetCallArgs(IORexp)[1]);
 	assert( (IExp != NULL) && (JExp != NULL) );
 	string IExpName = GetFuncRefName(IExp);
 	string JExpName = GetFuncRefName(JExp);
 	cout << "Call Name: " << GetCallName(IORexp) << endl;
 	cout << "Arg0 func name: " << IExpName << endl;
 	cout << "Arg1 func name: " << JExpName << endl;


 	// mpi comm 
 	assert(GetCallArgs(callExp).size() > 3);
 	SgActualArgumentExpression *commArgExp = 
	 	isSgActualArgumentExpression(GetCallArgs(callExp)[3]);
	assert(commArgExp != NULL);
	assert( strcasecmp(GetActArgName(commArgExp).c_str(), "comm") == 0 );
	CommExp = commArgExp->get_expression();
	cout << "comm class name: " << CommExp->class_name() << endl;


 	// Insert adios calls, some var decls, remove nc call
 	InsertAdInit();

 }



/*******************************************
 * Process all nf90_def_dim calls
 *******************************************/
void
Group::Process_nf90_def_dim()
{

	// Declare a adios varid variable for all later 
	// adios_define_var calls
	SgFunctionDeclaration *mainFunc = findMain(getProject());
	assert(mainFunc != NULL);	
	SgBasicBlock *body = mainFunc->get_definition()->get_body();
	pushScopeStack(body);	

	SgVariableDeclaration *adVarIDDecl = 
		buildVariableDeclaration("adios_varid",
			buildFortranKindType(buildIntType(), buildIntVal(8)));

	// SgVariableDeclaration *lastDecl = FindLastVarDecl(body);
	// assert(lastDecl != NULL);
	// insertStatementAfter(lastDecl, adVarIDDecl);
	insertStatementAfterLastDeclaration(adVarIDDecl, body);
	popScopeStack();


	// Iterate through nf90_def_dim call vector
	vector<SgFunctionCallExp*> &vec = CallVV[NF90_DEF_DIM];

	for (vector<SgFunctionCallExp*>::size_type i = 0; 
			i != vec.size(); ++i) {
		Process_nf90_def_dim_Worker(vec[i]);
	}

}

/*********************************************
 * Extract info from all nf90_def_var calls
 *********************************************/
void
Group::Extract_nf90_def_var()
{
	// Get vector
	vector<SgFunctionCallExp*> &vec = CallVV[NF90_DEF_VAR];

	// Iterate through it
	for (vector<SgFunctionCallExp*>::size_type i = 0; 
			i != vec.size(); ++i) {
		Extract_nf90_def_var_Worker(vec[i]);
	}
	
}


/*********************************************
 * Update VarMap: Update IsGlobal, and 
 * fill CountInit, OffsetInit, IterNum
 * if necessary
 * Also, update PutVarVec
 *******************************************/
void
Group::Extract_nf90_put_var()
{

	// Clear PutVarVec
	PutVarVec.clear();

	// Get call vector
	vector<SgFunctionCallExp*> &vec = CallVV[NF90_PUT_VAR];

	// Iterate through it
	for (vector<SgFunctionCallExp*>::size_type i = 0; 
			i != vec.size(); ++i) {
		Extract_nf90_put_var_Worker(vec[i]);
	}


}

/*******************************************
 * Process all nf90_def_var calls
 *******************************************/
void
Group::Process_nf90_def_var()
{
	// Get call vector
	vector<SgFunctionCallExp*> &vec = CallVV[NF90_DEF_VAR];

	// Iterate through it
	for (vector<SgFunctionCallExp*>::size_type i = 0; 
			i != vec.size(); ++i) {
		Process_nf90_def_var_Worker(vec[i]);
	}
	
}


/***************************************
 * Process the only nf90_enddef
 **************************************/
void 
Group::Process_nf90_enddef()
{
	// Simply remove it
	SgStatement *orginStmt;
	assert(CallVV[NF90_ENDDEF].size() == 1);
	orginStmt = getEnclosingStatement(CallVV[NF90_ENDDEF][0]);

	removeStatement(orginStmt);

}


/************************************************
 * Insert statements before the first put call
 ************************************************/
void
Group::BeforeTheFirstPut()
{
	// Scope
 	assert(CallVV[NF90_PUT_VAR].size() >= 1);
	SgFunctionCallExp *callExp = CallVV[NF90_PUT_VAR][0];
	SgStatement *firstPutStmt = getEnclosingStatement(callExp);

	SgScopeStatement *scope = getScope(firstPutStmt);
	SgStatement *prevStmt = FindLastVarDecl(scope);
 	pushScopeStack(scope);

 	// Build adios_open
 	SgExprStatement *adOpenCall= BuildAdOpen();
	// Insert 
	insertStatementBefore(firstPutStmt, adOpenCall);


	// Declare group size and total size vars
	char groupIDStr[30];
	snprintf(groupIDStr, 30, "%d", GroupID); 
	string adios_groupsize_str =
		string("adios_groupsize") + groupIDStr;
	string adios_totalsize_str = 
		string("adios_totalsize") + groupIDStr;

	// Declare adios_groupsizeXX
	SgVariableDeclaration *adios_groupsize_decl = 
		buildVariableDeclaration(adios_groupsize_str, 
			buildFortranKindType(buildIntType(), buildIntVal(8)));

	// Declare adios_totalsizeXX
	SgVariableDeclaration *adios_totalsize_decl = 
		buildVariableDeclaration(adios_totalsize_str, 
			buildFortranKindType(buildIntType(), buildIntVal(8)));

	// Insert 
	insertStatementAfter(prevStmt, adios_groupsize_decl);
	insertStatementAfter(adios_groupsize_decl, adios_totalsize_decl);


	// Build group size assignment statement
 	SgExprStatement *assignGroupSizeStmt = 
 		BuildGroupSizeAssign(adios_groupsize_str);
 	// Insert
	insertStatementBefore(firstPutStmt, assignGroupSizeStmt);

	// Build adios_group_size
	SgExprStatement *adGroupSizeStmt = 
		BuildAdGroupSize(adios_groupsize_str, adios_totalsize_str);
	// insert
	insertStatementAfter(assignGroupSizeStmt, adGroupSizeStmt);

	// Pop stack 
	popScopeStack();

 }


/******************************************
 * Process all nf90_put_var calls
 ******************************************/
void
Group::Process_nf90_put_var()
{
	// Iterate through PutVarSec
	for (vector<VarT*>::size_type i = 0; i < PutVarVec.size(); i++) {
		Process_nf90_put_var_Worker(i);
	} // End put var vec iteration

}



/************************************
 * Process nf90_close
 ************************************/
void
Group::Process_nf90_close()
{
	// Get the only function call 
	assert(CallVV[NF90_CLOSE].size() == 1);
 	SgFunctionCallExp *callExp = CallVV[NF90_CLOSE][0];

 	// Scope
	SgStatement *orginStmt;
	orginStmt = getEnclosingStatement(callExp);
	SgScopeStatement *scope = getScope(orginStmt);
	pushScopeStack(scope);

	// Build adios_close 
	SgStatement *adCloseCall = BuildAdClose();

	// Build MPI_Barrier 
	string MPIErrName = GetFuncArgName(scope, "MPI_Init", 0);
	assert(!MPIErrName.empty());

	SgStatement *mpiBarrierCall;
	if (CommExp != NULL) {
		mpiBarrierCall = BuildMPIBarrier(MPIErrName, CommExp);
	} else {
		mpiBarrierCall = BuildMPIBarrier(MPIErrName, "comm");
	}


	// Build adios_finalize 
	string MPIRankName = GetFuncArgName(scope, "MPI_Comm_rank", 1);
	assert(!MPIRankName.empty());
	
	SgStatement *adFinalizeCall = BuildAdFinalize(MPIRankName);


	//Insert, remove, pop
	insertStatementAfter(orginStmt, adCloseCall);
	insertStatementAfter(adCloseCall, mpiBarrierCall);
	insertStatementAfter(mpiBarrierCall, adFinalizeCall);

	removeStatement(orginStmt);
	popScopeStack();
}






