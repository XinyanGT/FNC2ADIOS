#ifndef GROUP_H
#define GROUP_H

#include <map>
#include <vector>
#include <cstdio>
#include <cstring>
#include <climits>

#include "rose.h"

#include "Common.h"
#include "RoseHelper.h"
#include "RoseHelperAdv.h"
#include "MPIUtils.h"

typedef std::pair<SgInitializedName*const, std::vector<std::string> > DimT;

class VarSec;
typedef std::pair<SgInitializedName*const, VarSec> VarT;


class Group
{
	
public:

	// Constructor
	Group(std::vector<SgFunctionCallExp*>, 
				const std::map<std::string, FUNC> &, int id);

	// Round one
	void Process_nf90_create();
	void Process_nf90_def_dim();
	void Extract_nf90_def_var();
	void Extract_nf90_put_var();

	// Round two
	void Process_nf90_def_var();
	void Process_nf90_enddef();
	void BeforeTheFirstPut();
	void Process_nf90_put_var();
	void Process_nf90_close();


private:
	// Type of dimension id arguments
	enum IDTYPE {
		SINGLE,
		ARRAY_FIRST,
		ARRAY_OTHER
	};

	std::string Name;				// Group name
	std::string FileName;			// Output file name
	std::string GroupIDVar;			// Group id variable name
	std::string FileVar;			// File handler variable name
	std::string ErrVar;				// adios err variable name
	int Cmode;						// netcdf file creation mode
	int GroupID;					// Id for this Group object
	SgExpression *CommExp;			// Expression for mpi comm argument
	bool IsPara;					// Parallel I/O?
	// Mapping netcdf function names to indices
	const std::map<std::string, FUNC> *FuncNameIndMap;
	// netcdf dimension info
	std::map<SgInitializedName*, std::vector<std::string> > DimMap;
	// netcdf variable info
	std::map<SgInitializedName*, VarSec> VarMap;
	// Calls that belong to this Group
	std::vector< std::vector<SgFunctionCallExp*> > CallVV;
	// Variable info array
	// Its indices are related to indices of put functions in CallVV
	std::vector<VarT*> PutVarVec;


	// Build adios functions
	SgExprStatement *BuildAdInit();
	SgExprStatement *BuildAdAllocBuf();
	SgExprStatement *BuildAdDeclGroup();
	SgExprStatement *BuildAdSelMod();
	SgExprStatement *BuildAdDefVar(const std::string &varName, int type, 
		const std::string &varidName, const std::string &count = std::string(), 
		const std::string &global = std::string(), const std::string &offset = std::string());
	SgExprStatement *BuildAdOpen();
	SgExprStatement *BuildAdGroupSize(const std::string &groupSizeVarName,
		const std::string &totalSizeVarName);
	SgExprStatement *BuildAdWrite(const std::string &varName, 
		const std::string &dataName, int index = INT_MIN, bool isOffset = false);
	SgExprStatement *BuildAdClose();
	SgExprStatement *BuildAdFinalize(const std::string &rankName);


	// Workers for extract and process functions 
	void Process_nf90_def_dim_Worker(SgFunctionCallExp *callExp);
	void Extract_nf90_def_var_Worker(SgFunctionCallExp *callExp);
	void Extract_nf90_put_var_Worker(SgFunctionCallExp *callExp);
	void Process_nf90_def_var_Worker(SgFunctionCallExp *callExp);
	void Process_nf90_put_var_Worker(int index);


	// Utilities
	std::string MakeStr(std::vector<std::string> strVec, std::string prefix);
	void InitCallVV(std::vector<SgFunctionCallExp*> vec);
	void InsertAdInit();
	// void FillIsPara();
	SgExprStatement *BuildGroupSizeAssign(const std::string &groupVarName);
	void GetNames(const std::string &path);

};
	

class VarSec
{
public:
	VarSec(std::vector<DimT*> vec, std::vector<std::string> strVec,
		std::string name, int type, int unitSize):
			DimTVec(vec), DimStrVec(strVec), CountInit(NULL), OffsetInit(NULL),
			Name(name), Type(type), UnitSize(unitSize),
			IterNum(0), IsGlobal(false)  {}

	// A vector of pointers to elements in DimMap
	std::vector<DimT*> DimTVec;				
	// Dimension 
	std::vector<std::string> DimStrVec;
	SgInitializedName *CountInit;
	SgInitializedName *OffsetInit;

	std::string Name;
	int Type;
	int UnitSize;
	int IterNum;
	bool IsGlobal;


};


#endif	
