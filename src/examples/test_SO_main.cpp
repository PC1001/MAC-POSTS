#include "dta.h"
#include "workzone.h"
#include "io.h"

int main(int argc, char *argv[])
{

  std::string path_name = "../../data/input_files_test_SO";
  // std::string path_name = "../../data/input_files_2link";
  // std::cout <<"why" <<std::endl;
  MNM_Dta *test_dta = new MNM_Dta(path_name);
  // std::cout <<"why" <<std::endl;
  test_dta -> build_from_files();
  printf("Hooking......\n");
  TFlt lambda = TFlt(0.1);
  test_dta -> hook_up_node_and_link();
  // printf("Checking......\n");
  // test_dta -> is_ok();
  // MNM::save_path_table(((MNM_Routing_Predetermined )test_dta -> m_routing) -> m_path_table);
  int maxiter = 100;
  std::vector<TFlt> tc;
   
  for(int i=0;i<maxiter;i++){
    TInt cur_int = 0;
    TInt ass_int = test_dta -> m_start_assign_interval;
    test_dta -> pre_loading();
    while(!test_dta ->finished_loading(cur_int)){
      test_dta->load_once(true,cur_int,ass_int);
      if (cur_int % test_dta -> m_assign_freq == 0 || cur_int==0){
        ass_int++;
      }
      cur_int++;

      //TO DO ----------------
      //record if each link is congested


    }
     printf("Hooking......xxx\n");
    test_dta -> route_update_MSA(lambda);
    // what is required before updating PMCs'? 
    // test_dta -> update_lower_PMC();
    // test_dta -> update_upper_PMC();


    tc.push_back(test_dta -> total_TT());

  }

  // test_dta -> loading(true);
  // MNM_Routing_Predetermined *testpre = static_cast<MNM_Routing_Predetermined*>(test_dta -> m_routing);
  // std::cout << testpre -> m_pre_routing -> test_function() << std::endl;
  // MNM_IO::dump_cumulative_curve("./",test_dta -> m_link_factory);


  delete test_dta;

  return 0;
}