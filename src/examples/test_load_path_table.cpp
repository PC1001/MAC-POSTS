#include "dta.h"
#include "workzone.h"
#include "io.h"
#include "path.h"

#include <string>
int main(int argc, char *argv[])
{
	std::string path_name = "../../data/input_files_test_SO_2link";
	std::string path_table = "../../path_generator/result.txt";
	MNM_Dta *test_dta = new MNM_Dta(path_name);
	test_dta -> build_from_files();
	test_dta -> hook_up_node_and_link();
	Path_Table *_path_table = MNM_IO::load_path_table_ksp(path_table,test_dta->m_graph);
	TInt _oid;
	TInt _did;
	MNM_Path * _path;
	MNM_Pathset* _pset;
	for (auto _ops = _path_table -> begin();_ops != _path_table ->end(); _ops++){
		_oid = _ops -> first;
		for (auto _dps = _ops -> second -> begin(); _dps != _ops -> second -> end(); _dps++){
			_did = _dps -> first;
			_pset = _dps -> second;
			if (_pset -> m_path_vec.size() ==0)
				continue;
			else{
				std::cout << "O " << _oid <<",D " << _did <<": number of path " << _pset -> m_path_vec.size()  << std::endl;
			}
		}
	}


	// delete test_dta;
	// delete _path_table;

}