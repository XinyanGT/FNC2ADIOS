// #include <algorithm>
#include <numeric>
#include <map>
#include <vector>

#include "rose.h"
#include "RoseHelper.h"
#include "Common.h"
#include "Prepare.h"
#include "Group.h"

using namespace std;
using namespace RoseHelper;
using namespace SageBuilder;
using namespace SageInterface;



/**********************
 * main
 **********************/
int
main(int argc, char *argv[])
{


	// Build AST 
	SgProject *project = frontend(argc, argv);
	ROSE_ASSERT(project != NULL);

	// In Fortran ? 
	if (!is_Fortran_language()) {
		cout << "NOT in Fortran. Quit." << endl;
		exit(1);
	} else 
		cout << "In Fortran. " << endl;


	// Absolute path file names of source files 
	SgStringList srcNameList;
//	srcNameList = project->get_sourceFileNameList();
	srcNameList = project->getAbsolutePathFileNames();
	cout << "Absolute source files:" << endl;
	ostream_iterator<string> coutStrItr(cout, "\n");
	copy(srcNameList.begin(), srcNameList.end(), coutStrItr);
//	for(SgStringList::iterator itr = srcNameList.begin();
//			itr != srcNameList.end(); ++itr)
//			cout << *itr << endl;

	
	// Initialize Function names to indices(FUNC) map 
	map<string, FUNC> funcNameIndMap;	
	InitFuncNameIndMap(funcNameIndMap);

	// Query to get all function call expressions
	Rose_STL_Container<SgNode*>callList =
		NodeQuery::querySubTree(project, V_SgFunctionCallExp);

//	Rose_STL_Container<SgNode*>funcDeclList =
//		NodeQuery::querySubTree(project, V_SgFunctionDeclaration);

	// Print
//	for (Rose_STL_Container<SgNode*>::iterator itr = callList.begin();
//			itr != callList.end(); ++itr) 
//			PrintFuncCallInfo(*itr);
//
//	for (Rose_STL_Container<SgNode*>::iterator itr = funcDeclList.begin();
//			itr != funcDeclList.end(); ++itr) 
//			PrintFuncDeclInfo(*itr, srcNameList);



	vector<SgFunctionCallExp*> ncCall;
	vector< vector<SgFunctionCallExp*> > callGroupVec;

	FilterCall(callList, ncCall, funcNameIndMap);
	GroupNcCall(ncCall, callGroupVec);

	vector<Group*> groupPtrVec(callGroupVec.size());
	cout << "total group num: " << groupPtrVec.size() << endl;

	for (vector< vector<SgFunctionCallExp*> >::size_type i = 0;
			i < callGroupVec.size(); ++i) {
		cout << "group " << i << endl;
		groupPtrVec[i] = new Group(callGroupVec[i], funcNameIndMap, i);
//		for (vector<SgFunctionCallExp*>::size_type j = 0; 
//				j < callGroupVec[i].size(); ++j) {
//			cout << "\t" << GetCallName(callGroupVec[i][j]) << endl;
//		}
	}

//	InsertMPI(project);

	// Round one
	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Processing nf90_create..." << endl;
	groupPtrVec[0]->Process_nf90_create();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Processing nf90_def_dim..." << endl;
	groupPtrVec[0]->Process_nf90_def_dim();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Extracting nf90_def_var..." << endl;
	groupPtrVec[0]->Extract_nf90_def_var();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Extracting nf90_put_var..." << endl;
	groupPtrVec[0]->Extract_nf90_put_var();


	// Round two
	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Processing nf90_def_var..." << endl;
	groupPtrVec[0]->Process_nf90_def_var();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Processing nf90_enddef..." << endl;
	groupPtrVec[0]->Process_nf90_enddef();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Before the first put..." << endl;
	groupPtrVec[0]->BeforeTheFirstPut();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Process nf90_put_var..." << endl;
	groupPtrVec[0]->Process_nf90_put_var();

	cout << setw(80) << setfill('*')<< '*' << endl;
	cout << "Process nf90_close..." << endl;
	groupPtrVec[0]->Process_nf90_close();


	// Test, post processing and unparse
	AstTests::runAllTests(project);
	AstPostProcessing(project);
	project->unparse();


	return 0;
}


