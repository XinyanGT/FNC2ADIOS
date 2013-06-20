#ifndef ROSEHELPER_H
#define ROSEHELPER_H

#include "rose.h"
#include <vector>
#include <string>

/**************************************************
 * Helper inline functions for rose
 * (TODO) Clean function names 
 **************************************************/

namespace RoseHelper
{



/**********************************
 * Return function call 
 * argument vector 
 ***********************************/
inline const std::vector<SgExpression*> &
GetCallArgs(const SgFunctionCallExp *callExp)
{
	return callExp->get_args()->get_expressions();
}

/**********************************
 * Return function call file name
 ***********************************/
inline std::string
GetCallFileName(const SgFunctionCallExp *callExp)
{
	return callExp->get_file_info()->get_filenameString();
}


/****************************************
 * Return function call file line number 
 ***************************************/
inline int 
GetCallFileLine(const SgFunctionCallExp *callExp)
{
	return callExp->get_file_info()->get_line();
}


/***************************
 * Return function name
 ***************************/
inline std::string
GetCallName(const SgFunctionCallExp *callExp) 
{
	ROSE_ASSERT(callExp->getAssociatedFunctionSymbol() != NULL);
	return callExp->getAssociatedFunctionSymbol()->
		get_name().getString();
}

/**********************************
 * Return the actual argument name
 *********************************/
inline std::string 
GetActArgName(const SgActualArgumentExpression *exp)
{				
	return exp->get_argument_name().getString();
}

/*********************************
 * Return the function name from 
 * a function ref exp
 ********************************/
inline std::string 
GetFuncRefName(const SgFunctionRefExp *exp)
{				
	return exp->get_symbol()->get_name().getString();
}


/******************************
 * Return variable name
 *****************************/
inline std::string 
GetVarName(const SgExpression *exp)
{				
	return (static_cast<const SgVarRefExp*>(exp))->
		get_symbol()->get_name().getString();
}

/*********************************
 * Return the initialized name of a 
 * var ref exp
 *********************************/
inline SgInitializedName*
GetVarRef_InitName(const SgVarRefExp *exp)
{				
	return exp->get_symbol()->get_declaration();
}


/*********************************
 * Return the initializer of a 
 * var ref exp
 *********************************/
inline SgInitializer*
GetVarRef_Initializer(const SgVarRefExp *exp)
{				
	return GetVarRef_InitName(exp)->get_initializer();
}



/***********************************
 * Return the initial int value of a
 * var ref exp
 *********************************/
inline int
GetVarRef_InitIntVal(const SgVarRefExp *exp)
{
	SgAssignInitializer *init = 
		isSgAssignInitializer(GetVarRef_Initializer(exp));
	assert(init != NULL);
	SgIntVal *intVal = isSgIntVal(init->get_operand());
	assert(intVal!= NULL);

	return intVal->get_value();
}


/*******************************************
 * Extract info from char ptr argument
 * input:
 *		SgExpression *exp: IR node ptr
 * output:
 *		string &name: name of the arg var,
 *			leave unchanged if we can
 *			get the value
 * return:
 *		string: value of the arg,
 *			return empty string if we 
 *			don't get the value
 ******************************************/
inline std::string
ArgCharPtr(const SgExpression *exp, std::string &name)
{

	if (exp->variantT() == V_SgCastExp)
		exp = (static_cast<const SgCastExp*>(exp))->get_operand();

	if (exp->variantT() == V_SgStringVal) {
		return (static_cast<const SgStringVal*>(exp))->get_value();
	} else {
		assert(exp->variantT() == V_SgVarRefExp);
		name = GetVarName(exp);
		return std::string();
	}
}


/******************************************
 * Extract info from size_t argument
 * input:
 *		SgExpression *exp: IR node ptr
 * output:
 *		string &name: name of the arg var,
 *			leave unchanged if we can get 
 *			the value
 * return:
 *		size_t: value of the arg, return
 *			0 if we don't get the value
 ******************************************/
inline size_t 
ArgSize_t(const SgExpression *exp, std::string &name)
{
	if (exp->variantT() == V_SgCastExp)
		exp = (static_cast<const SgCastExp*>(exp))->get_operand();


	if (exp->variantT() == V_SgIntVal) {
		return (static_cast<const SgIntVal*>(exp))->get_value();

	} else if (exp->variantT() == V_SgUnsignedLongVal) {
		return (static_cast<const SgUnsignedLongVal*>(exp))->get_value();

	} else {
		assert(exp->variantT() == V_SgVarRefExp);
		name = GetVarName(exp);
		return 0;
	}

}


/******************************************
 * Extract info from int argument 
 * return:
 *		string: name of the arg var 
 *******************************************/
inline std::string
ArgInt(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgVarRefExp);
	return GetVarName(exp);
}


/******************************************
 * Extract info from int argument 
 * return:
 *		int: the value of the arg
 *******************************************/
inline int 
ArgInt_Val(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgIntVal);
	return (static_cast<const SgIntVal*>(exp))->get_value();
}


/***************************************************
 * Extract info from int argument 
 * return:
 *		SgVariableSymbol*: var symbol of the arg var 
 ***************************************************/
inline SgVariableSymbol *
ArgInt_VarSym(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<const SgVarRefExp*>(exp))->get_symbol();
}


	
/***************************************************
 * Extract info from argument 
 * return:
 *		SgInitializedName*: declaration of the arg var 
 ***************************************************/
inline SgInitializedName*
ArgVarRef_InitName(const SgExpression *exp)
{
	if (exp->variantT() == V_SgCastExp)
		exp = (static_cast<const SgCastExp*>(exp))->get_operand();

	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<const SgVarRefExp*>(exp))->
			get_symbol()->get_declaration();
}



/*******************************************
 * Extract info from int ptr argument 
 * return:
 *		string: name of the int ptr arg var 
 ******************************************/
inline std::string 
ArgIntPtr(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgAddressOfOp);
	exp = (static_cast<const SgAddressOfOp*>(exp))->get_operand();
	assert(exp->variantT() == V_SgVarRefExp);
	return GetVarName(exp);

}


/********************************************
 * Extract info from int ptr argument 
 * return:
 *		SgVariableSymbol*: var symbol of 
 *			the var, whose addr is the argument 
 ********************************************/
inline SgVariableSymbol *
ArgIntPtr_VarSym(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgAddressOfOp);
	exp = (static_cast<const SgAddressOfOp*>(exp))->get_operand();
	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<const SgVarRefExp*>(exp))->get_symbol();

}


/********************************************
 * Extract info from int ptr argument 
 * return:
 *		SgInitializedName*: declaration of 
 *			the var, whose addr is the argument 
 ********************************************/
inline SgInitializedName*
ArgIntPtr_InitName(const SgExpression *exp)
{
	if (exp->variantT() == V_SgCastExp)
	exp = (static_cast<const SgCastExp*>(exp))->get_operand();

	if (exp->variantT() == V_SgAddressOfOp) 
		exp = (static_cast<const SgAddressOfOp*>(exp))->get_operand();

	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<const SgVarRefExp*>(exp))->get_symbol()->get_declaration();

}


/********************************************
 * Extract info from const int ptr argument 
 * return:
 *		SgInitializedName*: declaration of 
 *			the var, whose addr is the argument 
 ********************************************/
inline SgInitializedName*
ArgConstIntPtr_InitName(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgAddressOfOp);
	exp = (static_cast<const SgAddressOfOp*>(exp))->get_operand();
	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<const SgVarRefExp*>(exp))->get_symbol()->get_declaration();

}


/******************************************
 * Extract info from enum argument(int)
 * return:
 *		int: the int value for the enum
 ******************************************/
inline int
ArgEnum(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgIntVal);
	return (static_cast<const SgIntVal*>(exp))->get_value();
}
	

/******************************************
 * Extract info from argument
 * return:
 *		string: the name of the macro
 ******************************************/
inline std::string
ArgCastMacro(const SgExpression *exp)
{
	assert(exp->variantT() == V_SgCastExp);
	exp = (static_cast<const SgCastExp*>(exp))->get_operand();	
	assert(exp->variantT() == V_SgEnumVal);
	return (static_cast<const SgEnumVal*>(exp))->get_name().getString();
}


/******************************************
 * Extract info from argument
 * return:
 *		int: the result of the bit or op
 ******************************************/
inline int
ArgBitOrOp(const SgExpression *exp)
{
	const SgIntVal *l, *r;
	const SgBitOrOp *op;

	op = isSgBitOrOp(exp);
	assert(op != NULL);
	l = isSgIntVal(op->get_lhs_operand());
	r = isSgIntVal(op->get_rhs_operand());
	assert( (l != NULL) && (r != NULL) );

	return (l->get_value()) | (r->get_value());
}


/******************************************
 * Get the declaration of the left value
 * of an add operation
 ******************************************/
inline SgInitializedName *
ArgIntPtr_AddOp_Left(SgExpression *exp)
{
	SgExpression *l;
	assert(exp->variantT() == V_SgAddOp);
	l = (static_cast<SgAddOp*>(exp))->get_lhs_operand();
	return ArgVarRef_InitName(l);
}


/*****************************************
 * Get the int value of the right value
 * of an add operation
 *****************************************/
inline int 
ArgIntPtr_AddOp_Right(SgExpression *exp)
{
	SgExpression *r;
	assert(exp->variantT() == V_SgAddOp);
	r = (static_cast<SgAddOp*>(exp))->get_rhs_operand();
	return ArgInt_Val(r);
}

/***********************************
 * Extract the value of an int
 ***********************************/
inline int
ArgCastInt(SgExpression *exp)
{
	assert(exp->variantT() == V_SgCastExp);
	exp = (static_cast<const SgCastExp*>(exp))->get_operand();
	return ArgInt_Val(exp);
}


/**********************************
 * Extract the type of a var
 *********************************/
inline SgType *
ArgVarRef_Type(SgExpression *exp)
{
	if (exp->variantT() == V_SgCastExp)
		exp = (static_cast<const SgCastExp*>(exp))->get_operand();
	assert(exp->variantT() == V_SgVarRefExp);
	return (static_cast<SgVarRefExp*>(exp))->get_type();
}



}	/***** End of namespace *****/



#endif
