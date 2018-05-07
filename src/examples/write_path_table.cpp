#include "dta.h"
#include "workzone.h"
#include "io.h"
#include "path.h"

int main(int argc, char *argv[])
{
  std::string path_name = "../../data/input_files_SR41";
  std::string path_table = "../../path_generator/path_table1.txt";
  // ----------
  // std::cout <<"Here a" << std::endl;
  MNM_Dta *test_dta = new MNM_Dta(path_name);
  // std::cout <<"why" <<std::endl;
  // std::cout <<"Here a" << std::endl;
  test_dta -> m_path_file = path_table;
  std::cout <<"Here a" << std::endl;
  test_dta -> build_from_files();

  std::cout <<"Here a" << std::endl;
    test_dta -> hook_up_node_and_link();
  
  
  std::cout <<"Here" << std::endl;
  test_dta -> pre_loading();
  std::cout <<"start building" << std::endl;
  Path_Table *_path_table = MNM::build_pathset(test_dta ->m_graph, test_dta -> m_od_factory, test_dta -> m_link_factory);
  std::cout <<"END" << std::endl;
  MNM::save_path_table(_path_table, test_dta -> m_od_factory);
  std::cout <<"finished" << std::endl;

}