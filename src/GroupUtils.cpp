/****************************************************
 * Utility functions for Group
 ****************************************************/

#include "Group.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;
 

/*******************************************
 * Construct a string for dimension which 
 * can be used in adios calls
 ******************************************/
string
Group::MakeStr(vector<string> strVec, string prefix)
{
	string str;
	for (vector<string>::size_type i = 0; i < strVec.size()-1; i++) { 
		str += prefix + strVec[i] + ",";
	}
	str += prefix + strVec[strVec.size()-1];

	return str;
}


/*****************************************************
 * Distribute calls of supported netcdf functions 
 * of one group to their respective vectors
 * (data member CallVV)
 * Input:
 *		vector <SgFunctionCallExp *vec:
 *			supported NetCDF functions calls vector
 * Output:
 *		vector< vector<SgFunctionCallExp*> > CallVV
 ****************************************************/
void 
Group::InitCallVV(vector<SgFunctionCallExp*> vec)
{
	map<string, FUNC>::const_iterator mapItr;
	string funcName;

	// Iterate through the netcdf call vector
	for (vector<SgFunctionCallExp*>::size_type i = 0;
			i != vec.size(); ++i) {

		funcName = GetCallName(vec[i]);
		// Find index
		assert( (mapItr = FuncNameIndMap->find(funcName)) 
				!= FuncNameIndMap->end() ); 
		CallVV[mapItr->second].push_back(vec[i]);
	}
}



/******************************************************
 * Get adios group name and output file name
 * based on netcdf output file name
 * Input:
 *		const string &path: netcdf output file name
 * Output:
 *		string Name: adios group name
 *		string FileName: adios output file name
 *********************************************************/
void
Group::GetNames(const string &path)
{
	size_t len, posSlash, posLast;

	FileName = path;
	len = FileName.length();
	posLast = len - 1;

	if ( (len > 3) && (FileName.substr(len-3) == ".nc") ) {
		FileName.replace(len-3, 3, ".bp");
		posLast = len - 4;
	}

	if ( (posSlash = FileName.rfind('/')) == string::npos )
		posSlash = 0;
	else
		posSlash++;

	Name = FileName.substr(posSlash, posLast-posSlash+1);
}




SgExprStatement *
Group::BuildGroupSizeAssign(const string &groupSizeVarName)
{

	/***** Build group size assign statement *****/
	int ndims;
	VarSec *vsp;
	SgStatement *prevDecl = FindLastVarDecl(topScopeStack());
	SgStatement *prevStmt = 
		getPreviousStatement(getEnclosingStatement(CallVV[NF90_PUT_VAR][0]));
	SgStatement *adVarDecl, *assignStmt;
	SgExpression *rhs = buildUnsignedLongLongIntVal(0);
	SgExpression *countCal = buildUnsignedLongLongIntVal(1);
	SgExpression *dataSize;
	string str, countName;

	vector<string> strVec;

	/***** Iterate through PutVarVec *****/
	for (vector<VarT*>::size_type i = 0; i != PutVarVec.size(); ++i) {
		vsp = &(PutVarVec[i]->second);
		const vector<string> dimStrVec = vsp->DimStrVec;
		ndims = dimStrVec.size();

		if (vsp->IsGlobal == true) {
			countName = vsp->CountInit->get_name().getString();
//			assert(CheckChange(countInit, ) == false;
			for (vector<string>::size_type i = 0; i < ndims; i++) {
				str = "c" + dimStrVec[i];
				// Insert count var declarations
				adVarDecl = 
					 buildVariableDeclaration(str, buildIntType());
				insertStatementAfter(prevDecl, adVarDecl);
				prevDecl = adVarDecl;
				// Assign count vars 
				assignStmt = 
					buildExprStatement(
						buildAssignOp(
							buildVarRefExp(str),
							buildPntrArrRefExp(
								buildVarRefExp(countName),
								buildIntVal(i+1)
							)
						)
					);
				insertStatementAfter(prevStmt, assignStmt);
				prevStmt = assignStmt;
				// Update count calculate expression
				countCal =
					buildMultiplyOp(
						countCal,
						buildVarRefExp(str)
					);
			}
		} else {
			for (vector<string>::size_type i = 0; i < ndims; i++) {
				countCal =
					buildMultiplyOp(
						countCal,
						buildVarRefExp(dimStrVec[i])
					);
			}
		}

	 	dataSize =	
			buildMultiplyOp(
				buildMultiplyOp(
					countCal,
					buildIntVal(vsp->UnitSize)
				),
				buildIntVal(vsp->IterNum)
			);	

		rhs = 
			buildAddOp(
				rhs,
				buildAddOp(
					buildAddOp(
						buildAddOp(
							buildMultiplyOp(
								buildIntVal(ndims),
								buildIntVal(4)
							),
							buildMultiplyOp(
								buildIntVal(ndims),
								buildIntVal(4)
							)
						),
						buildMultiplyOp(
							buildMultiplyOp(
								buildIntVal(ndims),
								buildIntVal(4)
							),
							buildIntVal(vsp->IterNum)
						)
					),
					dataSize
				)
			);
	}

	SgExprStatement *groupSizeAssignStmt = 
		buildExprStatement(
			buildAssignOp(
				buildVarRefExp(groupSizeVarName),
				rhs
			)
		);


	return groupSizeAssignStmt;

}

/***********************************
 * Insert adios initial code
 **********************************/
void
Group::InsertAdInit()
{
	
	// SgFunctionDeclaration *mainFunc = findMain(project);
	// assert(mainFunc != NULL);	
	// SgBasicBlock *body = mainFunc->get_definition()->get_body();
	// pushScopeStack(body);	

	// Scope
	SgStatement *orginStmt;
	orginStmt = getEnclosingStatement(CallVV[NF90_CREATE][0]);
	SgScopeStatement *scope = getScope(orginStmt);
	pushScopeStack(scope);



	// Declare adios variables
	// adios group id, adios file handler and adios err 

	// adios_groupXX
	SgVariableDeclaration *adios_group_VarDecl = 
		buildVariableDeclaration(GroupIDVar, 
			buildFortranKindType(buildIntType(), buildIntVal(8))
		);

	// adios_fileXX 
	SgVariableDeclaration *adios_file_VarDecl = 
		buildVariableDeclaration(FileVar, 
			buildFortranKindType(buildIntType(), buildIntVal(8))
		);			

	// adios_errXX 
	SgVariableDeclaration *adios_err_VarDecl = 
		buildVariableDeclaration(ErrVar, buildIntType());

	// Insert
	SgVariableDeclaration *lastDecl = FindLastVarDecl(scope);
	assert(lastDecl != NULL);
	insertStatementAfter(lastDecl, adios_group_VarDecl);
	insertStatementAfter(adios_group_VarDecl, adios_file_VarDecl);
	insertStatementAfter(adios_file_VarDecl, adios_err_VarDecl);



	// Build adios calls
//	SgTypedefDeclaration *decl = Get_MPI_Comm_Declaration();
	SgExprStatement *adInitCall	= BuildAdInit();
	SgExprStatement *adAllocBufCall = BuildAdAllocBuf();
	SgExprStatement *adDeclGroupCall = BuildAdDeclGroup();
	SgExprStatement *adSelModCall = BuildAdSelMod();


	// Insert 
	insertStatementAfter(orginStmt, adInitCall);
	insertStatementAfter(adInitCall, adAllocBufCall);
	insertStatementAfter(adAllocBufCall, adDeclGroupCall);
	insertStatementAfter(adDeclGroupCall, adSelModCall);


	// Remove and pop
	removeStatement(orginStmt);
	popScopeStack();
}



/************************************
 * Fill IsPara data member
 ************************************/
// void 
// Group::FillIsPara()
// {
// 	if (!CallVV[NC_CREATE].empty())
// 		IsPara = false;
// 	else if (!CallVV[NC_CREATE_PAR].empty())
// 		IsPara = true;
// 	else {
// 		cout << "ERROR: no &ncid call!" << endl;
// 		exit(1);
// 	}
// }
