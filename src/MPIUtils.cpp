/*****************************************
 * Functions that deal with MPI code
 *****************************************/
 
#include "MPIUtils.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;

/*****************************************
 * Decide whether a node is the type 
 * define declaration for MPI_Comm.
 * If it is, push it back to a vector.
 ****************************************/
static vector<SgNode*>
Is_MPI_Comm_Declaration(SgNode *node)
{
	SgTypedefDeclaration *decl;
	vector<SgNode*> vec;

	decl = isSgTypedefDeclaration(node);
	if ( (decl != NULL) &&
//			(decl->get_file_info()->get_filenameString() == "mpi.h") &&
			(decl->get_name().getString() == "MPI_Comm") 
		)
		vec.push_back(decl);

	return vec;

}


/****************************************
 * Return the type define declaration
 * for MPI_Comm
 ***************************************/
SgTypedefDeclaration *
Get_MPI_Comm_Declaration(SgNode *node)
{
	static SgTypedefDeclaration *decl = NULL;

	if (decl == NULL) {
		assert(node != NULL);
		vector<SgNode*> vec = 
			NodeQuery::querySubTree(node, Is_MPI_Comm_Declaration);
		cout << "vector size: " << vec.size() << endl;
		assert(vec.size() == 1);
		decl = isSgTypedefDeclaration(vec[0]);
	}

	return decl;
}



/******************************************
 * Insert MPI related code 
 *****************************************/
void 
InsertMPI(SgProject *project)
{

	// Get the body of main function
	SgFunctionDeclaration *mainFunc = findMain(project);
	SgBasicBlock *body = mainFunc->get_definition()->get_body();


	// Use main body scope 
	pushScopeStack(body);


	// int rank
	SgVariableDeclaration *rankVarDecl = 
		buildVariableDeclaration("rank", buildIntType());


	// int size
	SgVariableDeclaration *sizeVarDecl = 
		buildVariableDeclaration("size", buildIntType());


	// MPI_Comm comm = MPI_WORLD_COMM
	SgTypedefDeclaration *commTypeDecl;
	commTypeDecl= Get_MPI_Comm_Declaration(project);
	SgType *MPICommType = SgTypedefType::createType(commTypeDecl);
	SgType *intType = buildIntType();

	SgExpression *initExp = 
			buildCastExp(buildIntVal(0x44000000), 
				MPICommType
			);
	SgVariableDeclaration *commVarDecl =
			buildVariableDeclaration("comm", 
				MPICommType,
				buildAssignInitializer(initExp)
			);


	// Insert var declarations for MPI
	prependStatement(rankVarDecl);
	insertStatementAfter(rankVarDecl, sizeVarDecl);
	insertStatementAfter(sizeVarDecl, commVarDecl);


	// MPI_Init(&argc, &argv) 
	SgAddressOfOp *arg11 = buildAddressOfOp(buildVarRefExp(SgName("argc")));
	SgAddressOfOp *arg12 = buildAddressOfOp(buildVarRefExp(SgName("argv")));
	SgExprListExp *argList1 = buildExprListExp();
	appendExpression(argList1, arg11);
	appendExpression(argList1, arg12);
	SgExprStatement *MPIInitCall = 
		buildFunctionCallStmt(SgName("MPI_Init"), intType, argList1);


	// MPI_Comm_rank(comm, &rank)
	SgVarRefExp *arg21 = buildVarRefExp(SgName("comm")); 
	SgAddressOfOp *arg22 = buildAddressOfOp(buildVarRefExp(SgName("rank")));
	SgExprListExp *argList2 = buildExprListExp();
	appendExpression(argList2, arg21);
	appendExpression(argList2, arg22);
	SgExprStatement *MPICommRankCall = 
		buildFunctionCallStmt(SgName("MPI_Comm_rank"), intType, argList2);

	// MPI_Comm_size(comm, &size)
	SgVarRefExp *arg31 = buildVarRefExp(SgName("comm")); 
	SgAddressOfOp *arg32 = buildAddressOfOp(buildVarRefExp(SgName("size")));
	SgExprListExp *argList3 = buildExprListExp();
	appendExpression(argList3, arg31);
	appendExpression(argList3, arg32);
	SgExprStatement *MPICommSizeCall = 
		buildFunctionCallStmt(SgName("MPI_Comm_size"), intType, argList3);

	// MPI_Finalize() 
	SgExprListExp *argList4 = buildExprListExp();
	SgExprStatement *MPIFinalizeCall = 
		buildFunctionCallStmt(SgName("MPI_Finalize"), intType, argList4);


	// Insert MPI calls 
 	insertStatementAfter(commVarDecl, MPIInitCall);
	insertStatementAfter(MPIInitCall, MPICommRankCall);
	insertStatementAfter(MPICommRankCall, MPICommSizeCall);
	instrumentEndOfFunction(mainFunc, MPIFinalizeCall);


	// Pop
	popScopeStack();
}


/***************************************************************
 * Output:
 *		SgExprStatement *: a function call statement that can 
 *			be inserted into source code
 ***************************************************************/
SgExprStatement *
BuildMPIBarrier(const string &errVarName, SgExpression *commExp)
{
	// Args
	SgExpression *arg1 = copyExpression(commExp);
	SgExpression *arg2 = buildVarRefExp(errVarName);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("MPI_Barrier"),
			buildVoidType(), argList);

	return call;

}


/****************************************************************
 * Output:
 *		SgExprStatement *: a function call statement that can 
 *			be inserted into source code
 ***************************************************************/
SgExprStatement *
BuildMPIBarrier(const string &errVarName, const string &commVarName)
{
	// Args
	SgExpression *arg1 = buildVarRefExp(commVarName);
	SgExpression *arg2 = buildVarRefExp(errVarName);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("MPI_Barrier"),
			buildVoidType(), argList);

	return call;

}
