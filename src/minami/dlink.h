#ifndef DLINK_H
#define DLINK_H

#include "vehicle.h"
#include "ults.h"

// #include "g3log/g3log.hpp"
// #include "g3log/logworker.hpp"

#include <vector>
#include <deque>
#include <unordered_map>

class MNM_Veh;
class MNM_Dnode;

/**************************************************************************
                          Cumulative curve
**************************************************************************/
class MNM_Cumulative_Curve
{
public:
  MNM_Cumulative_Curve();
  ~MNM_Cumulative_Curve();
  std::deque<std::pair<TFlt, TFlt>> m_recorder;
  int add_record(std::pair<TFlt, TFlt> r);
  int add_increment(std::pair<TFlt, TFlt> r);
  TFlt get_result(TFlt time);
  TFlt get_time(TFlt count);


  std::string to_string();
  int shrink(TInt number);
private:
  int arrange();
};


class MNM_Dlink
{
public:
  MNM_Dlink( TInt ID,
             TInt number_of_lane,
             TFlt length,
             TFlt ffs );
  virtual ~MNM_Dlink();
  int virtual evolve(TInt timestamp) {return 0;};
  TFlt virtual get_link_supply() {return TFlt(0);};
  int virtual clear_incoming_array() {return 0;};
  void virtual print_info() {};
  int hook_up_node(MNM_Dnode *from, MNM_Dnode *to);
  TFlt virtual get_link_flow() {return TFlt(0);};
  TFlt virtual get_link_tt() {return TFlt(0);};
  /*******
  For  SODTA
  TO DO
  ********/
  std::vector<int> *indicator_congestion;
  std::vector<TInt> *congestion_dissipate;

  TFlt get_link_real_tt(TFlt t);
  TFlt get_link_fftt();
  TFlt get_link_inflow(TFlt t);
  TFlt get_link_outflow(TFlt t);
  int virtual is_congested(){return 0;};
  TFlt get_link_lower_PMC(TInt t);
  TFlt get_link_upper_PMC(TInt t);
  int update_dissipate();
  int is_congested_after(TInt t);
  TInt next_pmc_time_lower(TInt t,TFlt unit_time);
  TInt next_pmc_time_upper(TInt t,TFlt unit_time);



  int install_cumulative_curve();
// protected:
  TInt m_link_ID;
  MNM_Dnode *m_from_node;
  MNM_Dnode *m_to_node;
  TInt m_number_of_lane;
  TFlt m_length;
  TFlt m_ffs;
  std::deque<MNM_Veh *> m_finished_array;
  std::deque<MNM_Veh *> m_incoming_array;
  MNM_Cumulative_Curve *m_N_in;
  MNM_Cumulative_Curve *m_N_out;

protected:
  int move_veh_queue(std::deque<MNM_Veh*> *from_queue, std::deque<MNM_Veh*> *to_queue, TInt number_tomove);
};


class MNM_Dlink_Ctm : public MNM_Dlink
{
public:
  MNM_Dlink_Ctm(TInt ID,
                TFlt lane_hold_cap, 
                TFlt lane_flow_cap, 
                TInt number_of_lane,
                TFlt length,
                TFlt ffs,
                TFlt unit_time,
                TFlt flow_scalar);
  ~MNM_Dlink_Ctm();
  int virtual evolve(TInt timestamp) override;
  TFlt virtual get_link_supply() override;
  int virtual clear_incoming_array() override;
  void virtual print_info() override;
  TFlt virtual get_link_flow() override;
  TFlt virtual get_link_tt() override;

  //SO-DTA
  int virtual is_congested() override;


// private:
  class Ctm_Cell;
  int init_cell_array(TFlt unit_time, TFlt std_cell_length, TFlt lane_hold_cap_last_cell);
  int update_out_veh();
  int move_last_cell();
  TInt m_num_cells;
  TFlt m_lane_hold_cap;
  TFlt m_lane_flow_cap;
  TFlt m_flow_scalar;
  TFlt m_wave_ratio;
  TFlt m_last_wave_ratio;
  std::vector<Ctm_Cell*> m_cell_array;

  int m_congested;
};


class MNM_Dlink_Ctm::Ctm_Cell
{
public:
  Ctm_Cell(TFlt hold_cap, TFlt flow_cap, TFlt flow_scalar, TFlt wave_ratio);
  ~Ctm_Cell();
  TFlt get_demand();
  TFlt get_supply();

  TInt m_volume;
  TFlt m_flow_scalar;
  TFlt m_hold_cap;
  TFlt m_flow_cap;
  TFlt m_wave_ratio;
  TInt m_out_veh;
  std::deque<MNM_Veh*>  m_veh_queue;
};

/**************************************************************************
                          Poing Queue
**************************************************************************/
class MNM_Dlink_Pq : public MNM_Dlink
{
public:
  MNM_Dlink_Pq(TInt ID,
               TFlt lane_hold_cap, 
               TFlt lane_flow_cap, 
               TInt number_of_lane,
               TFlt length,
               TFlt ffs,
               TFlt unit_time,
               TFlt flow_scalar);
  ~MNM_Dlink_Pq();
  int virtual evolve(TInt timestamp) override;
  TFlt virtual get_link_supply() override;
  int virtual clear_incoming_array() override;
  void virtual print_info() override;
  TFlt virtual get_link_flow() override;
  TFlt virtual get_link_tt() override;
// private:
  std::unordered_map<MNM_Veh*, TInt> m_veh_queue;
  TInt m_volume; //vehicle number, without the flow scalar
  TFlt m_lane_hold_cap;
  TFlt m_lane_flow_cap;
  TFlt m_flow_scalar;
  TFlt m_hold_cap;
  TInt m_max_stamp;
  TFlt m_unit_time;

  //SO-DTA
  int virtual is_congested() override;
};







/**************************************************************************
                          Link Queue
**************************************************************************/
class MNM_Dlink_Lq : public MNM_Dlink
{
public:
  MNM_Dlink_Lq(TInt ID,
               TFlt lane_hold_cap, 
               TFlt lane_flow_cap, 
               TInt number_of_lane,
               TFlt length,
               TFlt ffs,
               TFlt unit_time,
               TFlt flow_scalar);
  ~MNM_Dlink_Lq();
  int virtual evolve(TInt timestamp) override;
  TFlt virtual get_link_supply() override;
  int virtual clear_incoming_array() override;
  void virtual print_info() override;
  TFlt virtual get_link_flow() override;
  TFlt virtual get_link_tt() override;
// private:
  std::deque<MNM_Veh*> m_veh_queue;
  TInt m_volume; //vehicle number, without the flow scalar
  TFlt m_lane_hold_cap;
  TFlt m_lane_flow_cap;
  TFlt m_flow_scalar;
  TFlt m_hold_cap;
  TFlt m_C;  //maximum free flow capacity
  TFlt m_k_c; //maximum free flow density
  TFlt m_unit_time;

  TFlt get_flow_from_density(TFlt density);
  TFlt get_demand();

  //SO-DTA
  int virtual is_congested() override;
};



/**************************************************************************
                          Link Transmission model
**************************************************************************/

class MNM_Dlink_Ltm : public MNM_Dlink
{
public:
  MNM_Dlink_Ltm(TInt ID,
               TFlt lane_hold_cap, 
               TFlt lane_flow_cap, 
               TInt number_of_lane,
               TFlt length,
               TFlt ffs,
               TFlt unit_time,
               TFlt flow_scalar);
  ~MNM_Dlink_Ltm();
  int virtual evolve(TInt timestamp) override;
  TFlt virtual get_link_supply() override;
  int virtual clear_incoming_array() override;
  void virtual print_info() override;
  TFlt virtual get_link_flow() override;
  TFlt virtual get_link_tt() override;
// private:
  std::deque<MNM_Veh*> m_veh_queue;
  MNM_Cumulative_Curve m_N_in2;
  MNM_Cumulative_Curve m_N_out2;
  TInt m_volume; //vehicle number, without the flow scalar
  TFlt m_lane_hold_cap;
  TFlt m_lane_flow_cap;
  TFlt m_flow_scalar;
  TFlt m_hold_cap;
  TFlt m_unit_time;
  TInt m_current_timestamp; //used only in clear incoming array
  TFlt m_w;
  TFlt m_previous_finished_flow;
  TInt m_record_size;

  TFlt get_demand();

  //SO-DTA
  int virtual is_congested() override;
};



/**************************************************************************
                          Point Queue 2 (infinite capacity, )
**************************************************************************/

class MNM_Dlink_Pq2 : public MNM_Dlink
{
public:
  MNM_Dlink_Pq2(TInt ID,
               TFlt lane_hold_cap, 
               TFlt lane_flow_cap, 
               TInt number_of_lane,
               TFlt length,
               TFlt ffs,
               TFlt unit_time,
               TFlt flow_scalar);
  ~MNM_Dlink_Pq2();
  int virtual evolve(TInt timestamp) override;
  TFlt virtual get_link_supply() override;
  int virtual clear_incoming_array() override;
  void virtual print_info() override;
  TFlt virtual get_link_flow() override;
  TFlt virtual get_link_tt() override;
// private:
  std::unordered_map<MNM_Veh*, TInt> m_veh_queue;
  std::deque<MNM_Veh*> m_veh_dequeue;
  TInt m_volume; //vehicle number, without the flow scalar
  TFlt m_lane_hold_cap;
  TFlt m_lane_flow_cap;
  TFlt m_flow_scalar;
  TFlt m_hold_cap;
  TInt m_max_stamp;
  TFlt m_unit_time;

  //SO-DTA
  int virtual is_congested() override;
};


/**************
Helper function for 
**************/

#endif