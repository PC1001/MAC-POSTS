#include "dta.h"
#include "workzone.h"

int main(int argc, char *argv[])
{
  // MNM_IO::build_node_factory(test_dta -> m_file_folder, test_dta->m_config, test_dta->m_node_factory);
  // std::cout << test_dta -> m_node_factory -> m_node_map.size() << "\n";
  // MNM_IO::build_link_factory(test_dta -> m_file_folder, test_dta->m_config, test_dta->m_link_factory);
  // std::cout << test_dta -> m_link_factory -> m_link_map.size() << "\n";
  // MNM_IO::build_od_factory(test_dta -> m_file_folder, test_dta->m_config, test_dta->m_od_factory, test_dta->m_node_factory);
  // std::cout << test_dta -> m_od_factory -> m_origin_map.size() << "\n";
  // std::cout << test_dta -> m_od_factory -> m_destination_map.size() << "\n";
  // test_dta -> m_graph = MNM_IO::build_graph(test_dta -> m_file_folder,test_dta -> m_config);
  // test_dta -> m_graph  -> Dump();
  // MNM_Dta *test_dta = new MNM_Dta("../../data/input_files_new_philly");
  // MNM_Dta *test_dta = new MNM_Dta("../../data/input_files_SR41");
  // MNM_Dta *test_dta = new MNM_Dta("../../data/input_files_7link");
  // MNM_Dta *test_dta = new MNM_Dta("../../data/input_files_1link");
  // MNM_Dta *test_dta = new MNM_Dta("../../data/input_files_PGH");
  // if (argc != 2){
  //   printf("Usage: ./dta_response . (do not forget the second argument)\n");
  //   return -1;
  // }
  // printf("Current working directory is......\n");
  // std::cout << argv[1] << std::endl;

  // std::string path_name(argv[1]);

  std::string path_name = "../../data/pc_test_data";
  // std::string path_name = "../../data/input_files_2link";
  // std::cout <<"why" <<std::endl;
  MNM_Dta *test_dta = new MNM_Dta(path_name);
  // std::cout <<"why" <<std::endl;
  test_dta -> build_from_files();
  printf("Hooking......\n");
  test_dta -> hook_up_node_and_link();
  // printf("Checking......\n");
  // test_dta -> is_ok();
  // MNM::save_path_table(((MNM_Routing_Predetermined )test_dta -> m_routing) -> m_path_table);
  test_dta -> loading(true);


  delete test_dta;

  return 0;
}