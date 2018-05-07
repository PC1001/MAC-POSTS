#include "dta.h"
#include "workzone.h"
#include "io.h"
#include "path.h"

#include <string>
int main(int argc, char *argv[])
{
	// std::string path_name = "../../data/input_files_new_philly";
	// std::string path_name = "../../data/input_files_12_links";
	// std::string path_name = "../../data/input_files_SR41";
	std::string path_name = "../../data/input_files_PGH";
	MNM_Dta *test_dta = new MNM_Dta(path_name);
	test_dta -> build_from_files();
	test_dta -> hook_up_node_and_link();
	Path_Table *_path_table = MNM::build_pathset(test_dta->m_graph, test_dta->m_od_factory, test_dta->m_link_factory);
	// std::cout << test_dta->is_ok() << std::endl;
	MNM::save_path_table(_path_table, test_dta -> m_od_factory);
	delete test_dta;
}
