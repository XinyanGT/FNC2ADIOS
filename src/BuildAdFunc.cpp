/*************************************************
 * Build various adios function calls for a Group
 **************************************************/

#include "Group.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;



/***************************************
 * Build adios_init_noxml
 * call adios_init_noxml(comm, ErrVar)
 ***************************************/
SgExprStatement *
Group::BuildAdInit()
{
	// Args

	// In case of serial output, no mpi calls in netcdf user code
	SgExpression *arg1 = (CommExp != NULL) ? 
		copyExpression(CommExp): buildVarRefExp("comm");

	SgExpression *arg2 = buildVarRefExp(ErrVar);


	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	
	// Build
	SgExprStatement *call = 
		buildFunctionCallStmt(SgName("adios_init_noxml"), 
			buildVoidType(), argList);
	return call;
}


/******************************************
 * Build adios_allocate_buffer
 * call adios_allocate_buffer(10, ErrVar)
 *****************************************/
SgExprStatement *
Group::BuildAdAllocBuf()
{
	// Args
	SgExpression *arg1, *arg2;
	arg1 = buildIntVal(10);
	arg2 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);

	// Build
	SgExprStatement *call = 
		buildFunctionCallStmt(SgName("adios_allocate_buffer"),
			buildVoidType(), argList);

	return call;
}


/**********************************************************
 * Build adios_declare_group
 * call adios_declare_group(GroupIDVar, Name, "", 1, ErrVar)
 **********************************************************/
SgExprStatement *
Group::BuildAdDeclGroup()
{
	// Args
	SgExpression *arg1 = buildVarRefExp(GroupIDVar);
	SgStringVal *arg2 = buildStringVal(Name);
	SgStringVal *arg3 = buildStringVal("");
	SgExpression *arg4 = buildIntVal(1);
	SgExpression *arg5 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);
	appendExpression(argList, arg5);


	// Build
	SgExprStatement *call = 
		buildFunctionCallStmt(SgName("adios_declare_group"),
			buildVoidType(), argList);

	return call;
}


/**************************************************************
 * Build adios_select_method 
 * call adios_select_method (GroupIDVar, "MPI", "", "", ErrVar) 
 ***************************************************************/
SgExprStatement *
Group::BuildAdSelMod()
{

	// Args
	SgVarRefExp *arg1 = buildVarRefExp(GroupIDVar);
	SgStringVal *arg2 = buildStringVal("MPI");
	SgStringVal *arg3 = buildStringVal("");
	SgStringVal *arg4 = buildStringVal("");
	SgExpression *arg5 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);
	appendExpression(argList, arg5);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_select_method"),
			buildVoidType(), argList);

	return call;
}
	


/****************************************************
 * Build adios_define_var
 * call adios_define_var (GroupIDVar, varName,"", 
 *			type, count, global, offset, varidName)
 ****************************************************/
SgExprStatement *
Group::BuildAdDefVar(const string &varName, int type,
			const string &varidName, const string &count, 
			const string &global, const string &offset)
{

	// Args
	SgExpression *arg1, *arg2, *arg3,
			*arg4, *arg5, *arg6, *arg7, *arg8;
	arg1 = buildVarRefExp(GroupIDVar);
	arg2 = buildStringVal(varName);
	arg3 = buildStringVal("");
//	arg4 = buildVarRefExp(typeName);
	arg4 = buildIntVal(type);
	arg5 = buildStringVal(count);
	arg6 = buildStringVal(global);
	arg7 = buildStringVal(offset);
	arg8 = buildVarRefExp(varidName);


	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);
	appendExpression(argList, arg5);
	appendExpression(argList, arg6);
	appendExpression(argList, arg7);
	appendExpression(argList, arg8);


	// Build
	SgExprStatement *call = 
		buildFunctionCallStmt(SgName("adios_define_var"),
			buildVoidType(), argList);

	return call;
}



/*****************************************************************
 * Build adios_open
 * call adios_open(FileVar, Name, FileName, "w", CommExp, ErrVar)
 *****************************************************************/
SgExprStatement *
Group::BuildAdOpen()
{

	// Args
	SgExpression *arg1 = buildVarRefExp(FileVar);
	SgExpression *arg2 = buildStringVal(Name);
	SgExpression *arg3 = buildStringVal(FileName);
	SgExpression *arg4 = buildStringVal("w");
	SgExpression *arg5 = (CommExp != NULL) ? 
		copyExpression(CommExp): buildVarRefExp("comm");
	SgExpression *arg6 = buildVarRefExp(ErrVar);

	// Args list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);
	appendExpression(argList, arg5);
	appendExpression(argList, arg6);

	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_open"),
			buildVoidType(), argList);

	return call;
}


/****************************************************
 * Build adios_group_size
 * call adios_group_size(FileVar, groupSizeVarName,
 *	totalSizeVarName, ErrVar)
 ****************************************************/
SgExprStatement *
Group::BuildAdGroupSize(const string &groupSizeVarName, 
						const string &totalSizeVarName)
{
	// Args
	SgExpression *arg1 = buildVarRefExp(FileVar);
	SgExpression *arg2 = buildVarRefExp(groupSizeVarName);
	SgExpression *arg3 = buildVarRefExp(totalSizeVarName);
	SgExpression *arg4 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_group_size"),
			buildVoidType(), argList);

	return call;

}


/***************************************************
 * Build adios_write
 * Input:
 *	int index: the index of a data array 
 *	bool isOffset: if a var indicating offset would 
 *		be written out
 ***************************************************/
SgExprStatement *
Group::BuildAdWrite(const string &varName, 
		const string &dataName, int index, bool isOffset)
{
	// Args
	SgExpression *arg1 = buildVarRefExp(FileVar);
	SgExpression *arg2 = buildStringVal(varName);
	SgExpression *arg3;

	// INT_MIN is the default value of index, indicating
	// no index needed. If index is needed...
	if (index != INT_MIN) {
		arg3 = buildPntrArrRefExp(
					buildVarRefExp(dataName),
					buildIntVal(index)
				);
		// Indices in netcdf fortran start from 1
		// But indices in adios fortran start from 0
		// So subtract 1 here
		if (isOffset) {
			arg3 = buildSubtractOp(arg3, buildIntVal(1));
		}

	} else {
		arg3 = buildVarRefExp(dataName);
	}

	SgExpression *arg4 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);
	appendExpression(argList, arg3);
	appendExpression(argList, arg4);


	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_write"),
			buildVoidType(), argList);

	return call;
}


/****************************
 * Build adios_close
 ****************************/
SgExprStatement *
Group::BuildAdClose()
{
	// Args 
	SgExpression *arg1 = buildVarRefExp(FileVar);
	SgExpression *arg2 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_close"),
			buildVoidType(), argList);

	return call;
}



/*********************************
 * Build adios_finalize
 *********************************/
SgExprStatement *
Group::BuildAdFinalize(const string &rankName)
{
	// Args 
	SgExpression *arg1 = buildVarRefExp(rankName);
	SgExpression *arg2 = buildVarRefExp(ErrVar);

	// Arg list
	SgExprListExp *argList = buildExprListExp();
	appendExpression(argList, arg1);
	appendExpression(argList, arg2);

	// Build
	SgExprStatement *call= 
		buildFunctionCallStmt(SgName("adios_finalize"),
			buildVoidType(), argList);

	return call;
}