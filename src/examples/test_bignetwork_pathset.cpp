#include "dta.h"
#include "workzone.h"
#include "io.h"
#include "path.h"

#include <string>
int main(int argc, char *argv[])
{
	std::string path_name = "../../data/input_files_new_philly";
	MNM_Dta *test_dta = new MNM_Dta(path_name);
	test_dta -> build_from_files();
	test_dta -> hook_up_node_and_link();
	Path_Table *_path_table = MNM::build_pathset(test_dta->m_graph, test_dta->m_od_factory, test_dta->m_link_factory);
	delete test_dta;
}
