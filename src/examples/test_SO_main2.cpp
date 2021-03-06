#include "dta.h"
#include "workzone.h"
#include "io.h"

int main(int argc, char *argv[])
{

  // std::string path_name = "../../data/input_files_test_SO_2link";

  // std::string path_name = "../../data/input_files_2link";
  // std::cout <<"why" <<std::endl;

  //3 routes case
  // std::string path_name = "../../data/input_files_test_SO_3link_LWR";
  // std::string path_name ="../../data/input_files_new_philly";
  // std::string path_table = "../../path_generator/result2.txt";
  // std::string path_table = "../../build/examples/path_table_new_philly";

  //------


  //12 link case
  std::string path_name = "../../data/input_files_13_links_debug";
  std::string path_table = "../../path_generator/path_table1.txt";


  // ----------

  MNM_Dta *test_dta = new MNM_Dta(path_name);

  // std::cout <<"why" <<std::endl;
  test_dta -> m_path_file = path_table;
  test_dta -> build_from_files();
  
  
  printf("Hooking......\n");
  TFlt lambda = TFlt(0.1);
  test_dta -> hook_up_node_and_link();
  // printf("Checking......\n");
  // test_dta -> is_ok();
  // MNM::save_path_table(((MNM_Routing_Predetermined )test_dta -> m_routing) -> m_path_table);
  int maxiter = 10;
  std::vector<TFlt> tc;
  // std::cout << test_dta -> m_routing -> m_pre_routing -> toString() << std::endl;



  // //******************************************
  // //Print out and test the property of links
  // MNM_Dlink *_link;
  // for (auto _link_it = test_dta -> m_link_factory -> m_link_map.begin(); _link_it 
  //     != test_dta ->m_link_factory -> m_link_map.end(); _link_it++){
  //   _link = _link_it->second;
  //   std::cout <<  "Link ID:" << _link -> m_link_ID << std::endl;
  //   _link -> print_info();
  // } 
  // return 1;
  // //******************************************




  for(int i=0;i<maxiter;i++){
    std::cout << "Iteration " << i << std::endl;
    TInt cur_int = 0;
    TInt ass_int = test_dta -> m_start_assign_interval;
    test_dta -> pre_loading();
    while(!test_dta ->finished_loading(cur_int)){
      test_dta->load_once(false,cur_int,ass_int);
      // for (auto _link_it = test_dta -> m_link_factory -> m_link_map.begin(); _link_it !=  test_dta -> m_link_factory -> m_link_map.end(); _link_it++){
      //     std::cout<<"Time"<<cur_int<<" if congested" << _link_it ->second -> is_congested() << std::endl;
      // }

      if (cur_int % test_dta -> m_assign_freq == 0 || cur_int==0){
        ass_int++;
      }
      cur_int++;
      // std::cout << "Loaded time " << cur_int <<std::endl;





    }
    printf("Hooking......xxx\n");
    TFlt thistc = test_dta -> total_TT();
    test_dta -> link_update_dissipateTime();
    test_dta -> route_update_MSA(lambda,true);
    // test_dta -> route_update_PHA(lambda,true);
    
    // MNM_IO::dump_cumulative_curve("../../test_results", test_dta->m_link_factory);
    //*************test print the two vectors for the links of SO-DTA
    // MNM_Dlink *_link;
    // for (auto _link_it =  test_dta -> m_link_factory -> m_link_map.begin(); 
    //   _link_it != test_dta ->m_link_factory -> m_link_map.end(); _link_it++){
    //   _link = _link_it -> second;
    //   std::cout << "Path " << _link_it->first << std::endl;
    //   for(auto _indi = _link -> indicator_congestion -> begin();
    //      _indi != _link -> indicator_congestion -> end(); _indi++){
    //     std::cout << *_indi << ",";
    //   }
    //   std::cout << std::endl;
    // }

    test_dta -> reinstall_cumulative_curve();

    
    
    std::cout<<"Total cost "<< thistc<<std::endl;


    tc.push_back(thistc);

  }

  // test_dta -> loading(true);
  // MNM_Routing_Predetermined *testpre = static_cast<MNM_Routing_Predetermined*>(test_dta -> m_routing);
  // std::cout << testpre -> m_pre_routing -> test_function() << std::endl;
  // MNM_IO::dump_cumulative_curve("./",test_dta -> m_link_factory);
  for(int i=0;i<tc.size();i++){
    std::cout << tc[i] << ",";
  }
  std::cout << std::endl;


  delete test_dta;

  return 0;
}
