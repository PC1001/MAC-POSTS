#ifndef DTA_H
#define DTA_H

#include "Snap.h"
#include "dlink.h"
#include "od.h"
#include "dnode.h"
#include "factory.h"
#include "ults.h"
#include "io.h"
#include "statistics.h"
#include "routing.h"
#include "shortest_path.h"
#include "pre_routing.h"
#include "pmc_table.h"

#include <string>

class MNM_Dta
{
public:
  MNM_Dta(std::string file_folder);
  ~MNM_Dta();
  int initialize();
  int build_from_files();
  bool is_ok();
  int hook_up_node_and_link();
  int loading(bool verbose);
  int load_once(bool verbose, TInt load_int, TInt assign_int);
  int test();
// private:
  bool finished_loading(int cur_int);
  int set_statistics();
  int set_routing();
  int build_workzone();
  int check_origin_destination_connectivity();
  int pre_loading();
  TInt m_start_assign_interval;
  TInt m_total_assign_inter;
  TFlt m_unit_time;
  TFlt m_flow_scalar;
  TInt m_assign_freq;
  std::string m_file_folder;
  MNM_ConfReader *m_config;
  MNM_Veh_Factory *m_veh_factory;
  MNM_Node_Factory *m_node_factory;
  MNM_Link_Factory *m_link_factory;
  MNM_OD_Factory *m_od_factory;
  PNEGraph m_graph;
  MNM_Statistics *m_statistics;
  MNM_Routing *m_routing;
  MNM_Workzone *m_workzone;
  /********** for SO DTA ***********/
  MNM_PMC_Table *m_pmc_table;
  int update_pmc_lower();
  int update_pmc_upper();
  TFlt compute_pmc_upper(TInt t, MNM_Path* path);
  TFlt compute_pmc_lower(TInt t, MNM_Path* path);
  int route_update_MSA(TFlt lambda); // no subgradient
  int route_update_subgradient();
  int link_update_dissipateTime();
  // int link_update_iscongested();
  int reinstall_cumulative_curve();

  TFlt total_TT();
};               


namespace MNM
{
  int print_vehicle_statistics(MNM_Veh_Factory *veh_factory);
  int print_vehicle_info(MNM_Veh_Factory *veh_factory);
  bool has_running_vehicle(MNM_Veh_Factory *veh_factory);
  int demand_OD_update(MNM_OD_Factory *m_od_factory,MNM_Routing_Predetermined *m_routing);

}

#endif