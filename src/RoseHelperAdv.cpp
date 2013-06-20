/******************************************
 * More advanced rose helper functions
 ******************************************/
#include "RoseHelperAdv.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;

/*************************************
 * Print info about function call 
 ************************************/
void
PrintFuncCallInfo(SgNode *node)
{
	SgFunctionCallExp *callExp;

// 	funcCallExp = dynamic_cast<SgFunctionCallExp*>(node);
 	callExp = isSgFunctionCallExp(node);
	ROSE_ASSERT(callExp != NULL);

//	const vector<SgExpression*> &argVec = GetCallArgs(callExp);

	cout << "Function call found in " << GetCallFileName(callExp) <<
			"(line:" << GetCallFileLine(callExp) << ")" << endl;
	cout << "\t" << GetCallName(callExp) << endl;
}



/******************************************
 * Print info about function declaration
 ******************************************/
void
PrintFuncDeclInfo(SgNode *node, const SgStringList &srcNameList)
{
	SgFunctionDeclaration *funcDecl;
// 	funcDecl = dynamic_cast<SgFunctionDeclaration*>(node);
 	funcDecl = isSgFunctionDeclaration(node);
	ROSE_ASSERT(funcDecl != NULL);
	string fileName = funcDecl->get_file_info()->get_filenameString();


//	if (funcDecl->get_file_info()->isCompilerGenerated() == true) {

	// Only function declarations in input files *****/
	if (find(srcNameList.begin(), srcNameList.end(), fileName)
			!= srcNameList.end()) {
		cout << "Function declaration found in " << 
			funcDecl->get_file_info()->get_filenameString() << 
			"(line:" << funcDecl->get_file_info()->get_line() << ")" << endl;
		cout << "\t" << funcDecl ->get_name().getString() << endl;
	}
}

/******************************************
 * Get the initial int value of a var,
 * which is initialized with declaration
 ******************************************/
static int
GetInitializerIntVal(SgInitializer *initptr)
{
	assert(initptr->variantT() == V_SgAssignInitializer);
	SgExpression *exp;
	exp = (static_cast<SgAssignInitializer*>(initptr))->get_operand();
	assert(exp->variantT() == V_SgIntVal);
	return (static_cast<SgIntVal*>(exp))->get_value();
}



/*******************************************
 * Get the int value of an enum constant
 * Return INT_MIN for failure
 ******************************************/
int
GetEnumIntVal(SgEnumDeclaration *decl, const string &name)
{
	vector<SgInitializedName*> vec = decl->get_enumerators();
	int val = 0;

	for (vector<SgInitializedName*>::size_type i = 0;
			i != vec.size(); ++i) {
		if (vec[i]->get_initializer() != NULL)
			val = GetInitializerIntVal(vec[i]->get_initializer());
		if (vec[i]->get_name() == name) 
			return val;
		val++;
	}
	return INT_MIN;
}


/*********************************************
 * Get the declaration for an enum type
 ********************************************/
SgEnumDeclaration *
GetEnumDecl(const string &name)
{
	static vector<SgNode*> vec;
	SgEnumDeclaration *decl;

	if (vec.empty()) {
		SgProject *project = getProject();
		vec = NodeQuery::querySubTree(project, V_SgEnumDeclaration);
	}

	for (vector<SgNode*>::size_type i = 0; 
			i < vec.size(); ++i) {
		decl = isSgEnumDeclaration(vec[i]);
		if (decl->get_name().getString() == name)
			return decl;
	}
	return NULL;
}


/*****************************************
 * Get the expression of an enum constant
 ******************************************/
SgExpression *
GetEnumExpr(const string &enumType, const string &enumConstant)
{
	SgEnumDeclaration *decl = GetEnumDecl(enumType);
	assert(decl != NULL);
	int val = GetEnumIntVal(decl, enumConstant);
	return BuildEnumVal(val, decl, enumConstant);
}
	
/******************************
 * Build a enum value
 *****************************/
SgEnumVal * 
BuildEnumVal(int value, SgEnumDeclaration* decl, SgName name)
{
  SgEnumVal* enumVal= new SgEnumVal(value,decl,name);
  ROSE_ASSERT(enumVal);
  SageInterface::setOneSourcePositionForTransformation(enumVal);
  return enumVal;
}


/*************************************
 * Build a common for statement
 *************************************/
SgForStatement *
BuildCanonicalForStmt(int low, int high, const string &indexName)
{
	if (indexName.empty())
		return BuildCanonicalForStmtDeclIn(low, high);
	else
		return BuildCanonicalForStmtDeclOut(low, high, indexName);
}


/**********************************************
 * Create a For Statement with a empty
 * basic block body
 * Note: the index "i" is declared in 
 *		the for init statement
 ***********************************************/
SgForStatement *
BuildCanonicalForStmtDeclIn(int low, int high)
{
	SgStatement *initStmt = 
		buildVariableDeclaration( "i", buildIntType(), 
			buildAssignInitializer(buildIntVal(low)), NULL);

	SgStatement *testStmt = 
		buildExprStatement(buildLessThanOp(
			buildVarRefExp("i", NULL),
			buildIntVal(high)
			)
		);

	SgExpression *incrementExp = 
		buildPlusPlusOp(buildVarRefExp("i", NULL),
			SgUnaryOp::postfix);

	SgStatement *loopStmt = buildBasicBlock();

	SgForStatement *forStmt = 
		buildForStatement(initStmt, testStmt, incrementExp, loopStmt);

	return forStmt;
}


/*****************************************
 * Create a For Statement with a empty
 * basic block body
 * Note: the index is declared outside 
 *		the For Statement
 *****************************************/
SgForStatement *
BuildCanonicalForStmtDeclOut(int low, int high, string indexName)
{

	SgStatement *initStmt = 
		buildAssignStatement(buildVarRefExp(indexName), buildIntVal(low));

	SgStatement *testStmt =
		buildExprStatement(buildLessThanOp(
			buildVarRefExp(indexName), buildIntVal(high) 
			)
		);

	SgExpression *incrementExp = 
		buildPlusPlusOp(buildVarRefExp(indexName));

	SgStatement *loopStmt = buildBasicBlock();

	SgForStatement *forStmt = 
		buildForStatement(initStmt, testStmt, incrementExp, loopStmt);

	return forStmt;
}



/*****************************************
 * Extract low bound and high bound from 
 * a For Statement
 *****************************************/
void
ExtractForStmtBounds(SgForStatement *forStmt, int &low, int &high)
{

	/***** Get index low bound *****/
	vector<SgStatement*> initStmtVec = forStmt->get_init_stmt();
	assert(initStmtVec.size() == 1);
	assert(initStmtVec[0]->variantT() == V_SgExprStatement);
	SgAssignOp *assOp= 
		isSgAssignOp( (static_cast<SgExprStatement*>(initStmtVec[0]))->
				get_expression());
	assert(assOp != NULL);

	SgCastExp *r;
	r = isSgCastExp(assOp->get_rhs_operand());
	assert(r != NULL);

	SgIntVal *intVal;
	intVal = isSgIntVal(r->get_operand());
	assert(intVal != NULL);
	low = intVal->get_value();
//	cout << "Low bound is " << low << endl;


	/***** Get index high bound *****/
	SgExprStatement *testStmt;
	testStmt = isSgExprStatement(forStmt->get_test());
	assert(testStmt != NULL);

	SgLessThanOp *lessOp;
	lessOp = isSgLessThanOp(testStmt->get_expression());
	assert(lessOp != NULL);

	r = isSgCastExp(lessOp->get_rhs_operand());
	assert(r != NULL);
	intVal = isSgIntVal(r->get_operand());
	assert(intVal != NULL);
	high = intVal->get_value();
//	cout << "High bound is " << high << endl;


	/***** Ensure that the index increments 1 per iter *****/
	SgPlusPlusOp *ppOP;
	ppOP = isSgPlusPlusOp(forStmt->get_increment());
	assert(ppOP != NULL);

}


/***********************************************
 * Return the last variable Declaration in 
 * a scope
 **********************************************/
SgVariableDeclaration *
FindLastVarDecl(SgScopeStatement *scope)
{
	vector<SgNode*> vec = 
		NodeQuery::querySubTree(scope, V_SgVariableDeclaration);
	if (vec.size() > 0)
		return static_cast<SgVariableDeclaration*>(vec[vec.size()-1]);

	return NULL;
}


/***************************************************
 * Get the dimension id vector that is assigned to 
 * a dimension array before a function call
 ***************************************************/
SgExprListExp *
GetDimidVec(SgInitializedName *initName, SgFunctionCallExp *callExp)
{
	SgExprStatement *expStmt;
	SgVarRefExp *lhs;
	SgAssignOp *op, *tmpOp;
	op = tmpOp = NULL;

	// Get a vector containing all statements in a scope
	SgStatement *callStmt = getEnclosingStatement(callExp);
	vector<SgStatement*> stmtVec = 
		getScope(callExp)->getStatementList();

	// Iterate through the statement vector
	// to find the assign operation
	vector<SgStatement*>::size_type i;
	for (i = 0; stmtVec[i] != callStmt; i++) {
		expStmt = isSgExprStatement(stmtVec[i]);
		if (expStmt != NULL) {
			tmpOp = isSgAssignOp(expStmt->get_expression());
			if (tmpOp != NULL) {
				if ( (lhs = isSgVarRefExp(tmpOp->get_lhs_operand())) != NULL) {
					if (GetVarRef_InitName(lhs) == initName) {
						op = tmpOp;
					}
				}
			}
		}
	}

	// Extract the dimension id array
	assert(op != NULL);
	SgAggregateInitializer *aggInit = isSgAggregateInitializer(op->get_rhs_operand());
	assert(aggInit != NULL);
	SgExprListExp *outerExpList = aggInit->get_initializers();
	assert(outerExpList->get_expressions().size() == 1);
	SgExprListExp *innerExpList = isSgExprListExp(outerExpList->get_expressions()[0]);
	assert(innerExpList != NULL);

	return innerExpList;

} 


/***********************************************************
 * Get the var name in a function call through querying a 
 * scope
 * Output:
 *		string: var name if successful. Otherwise, return 
 *			an empty string
 ************************************************************/ 
string
GetFuncArgName(SgScopeStatement *scope, const string &funcName, int index)
{
	SgVarRefExp *varRef;
	SgFunctionCallExp *callExp;
	string currName;

	// Query
	vector<SgNode*> callList =
		NodeQuery::querySubTree(scope, V_SgFunctionCallExp);

	// Iterate
	for (vector<SgNode*>::size_type i = 0; i < callList.size(); i++) {
		callExp = isSgFunctionCallExp(callList[i]);
		currName = GetCallName(callExp);
		if ( strcasecmp(currName.c_str(), funcName.c_str()) == 0 ) {
			// Invalid index
			if ( (index < 0) || (GetCallArgs(callExp).size() < index+1) )
				return string();
			varRef = isSgVarRefExp(GetCallArgs(callExp)[index]);
			return GetVarName(varRef);
		}
	}

	return string();
}
