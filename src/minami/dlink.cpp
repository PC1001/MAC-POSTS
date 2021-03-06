#include "dlink.h"
#include "limits.h"
#include <math.h>
#include <algorithm>
#include <limits>

MNM_Dlink::MNM_Dlink( TInt ID,
                      TInt number_of_lane,
                      TFlt length,
                      TFlt ffs )
{
  m_link_ID = ID;
  if (ffs < 0){
    printf("link speed less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);    
  }
  m_ffs = ffs;
  
  if (number_of_lane < 0){
    printf("m_number_of_lane less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);    
  }
  m_number_of_lane = number_of_lane;

  if (length < 0){
    printf("link length less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);    
  }
  m_length = length;

  m_finished_array = std::deque<MNM_Veh *>();
  m_incoming_array = std::deque<MNM_Veh *>();
  m_N_in = NULL;
  m_N_out = NULL;
  indicator_congestion = NULL;
  congestion_dissipate = NULL;
}

MNM_Dlink::~MNM_Dlink()
{
  m_finished_array.clear();
  m_incoming_array.clear();
  if (m_N_out != NULL) delete m_N_out;
  if (m_N_in != NULL) delete m_N_in;  
  if (indicator_congestion !=NULL) delete indicator_congestion;
  if (congestion_dissipate !=NULL) delete congestion_dissipate;
}

int MNM_Dlink::hook_up_node(MNM_Dnode *from, MNM_Dnode *to)
{
  m_from_node = from;
  m_to_node = to;
  return 0;
}

int MNM_Dlink::install_cumulative_curve()
{
  if (m_N_out != NULL) delete m_N_out;
  if (m_N_in != NULL) delete m_N_in;  
  if (indicator_congestion !=NULL) delete indicator_congestion;
  if (congestion_dissipate !=NULL) delete congestion_dissipate;


  m_N_out = new MNM_Cumulative_Curve();
  m_N_in = new MNM_Cumulative_Curve();
  m_N_in -> add_record(std::pair<TFlt, TFlt>(TFlt(0), TFlt(0)));
  m_N_out -> add_record(std::pair<TFlt, TFlt>(TFlt(0), TFlt(0)));
  // install cc now is equivalent to do SO-DTA
  // thus we also install the congestion indicatorinstall
  indicator_congestion = new std::vector<int>();
  congestion_dissipate = new std::vector<TInt>();
  return 0;
}




int MNM_Dlink::move_veh_queue(std::deque<MNM_Veh*> *from_queue,
                                  std::deque<MNM_Veh*> *to_queue, 
                                  TInt number_tomove)
{
  MNM_Veh* _veh;
  for (int i=0; i<number_tomove; ++i) {
    _veh = from_queue -> front();
    from_queue -> pop_front();
    to_queue -> push_back(_veh);
  }
  return 0;
}

MNM_Dlink_Ctm::MNM_Dlink_Ctm( TInt ID,
                              TFlt lane_hold_cap, 
                              TFlt lane_flow_cap, 
                              TInt number_of_lane,
                              TFlt length,
                              TFlt ffs,
                              TFlt unit_time,
                              TFlt flow_scalar)
  : MNM_Dlink::MNM_Dlink ( ID, number_of_lane, length, ffs )
{

  //SO-DTA
  m_congested = 0;
  //end --------
  if (lane_hold_cap < 0){
    printf("lane_hold_cap can't be less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);
  }
  if (lane_hold_cap > TFlt(300) / TFlt(1600)){
    // printf("lane_hold_cap is too large, we will replace it to 300 veh/mile, current link ID is %d\n", m_link_ID());
    lane_hold_cap = TFlt(300) / TFlt(1600);
  }  
  m_lane_hold_cap = lane_hold_cap;

  if (lane_flow_cap < 0){
    printf("lane_flow_cap can't be less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);
  }
  if (lane_flow_cap > TFlt(3500) / TFlt(3600)){
    // printf("lane_flow_cap is too large, we will replace it to 3500 veh/hour, current link ID is %d\n", m_link_ID());
    lane_flow_cap = TFlt(3500) / TFlt(3600);
  }  
  m_lane_flow_cap = lane_flow_cap;
  m_flow_scalar = flow_scalar;
  m_cell_array = std::vector<Ctm_Cell*>();
  TFlt _std_cell_length = m_ffs * unit_time;
  m_num_cells = TInt(floor(m_length/_std_cell_length)); 
  if(m_num_cells == 0) {
    m_num_cells = 1;
  }
  TFlt _lane_hold_cap_last_cell = MNM_Ults::max(((m_length - TFlt(m_num_cells - 1) * _std_cell_length) / _std_cell_length) * m_lane_hold_cap,  m_lane_hold_cap);
  TFlt _wave_speed =  m_lane_flow_cap / (m_lane_hold_cap - m_lane_flow_cap / m_ffs); //laneFlwCap/ffs is the critical density.
  m_wave_ratio = _wave_speed / m_ffs; // note that h >= 2c/v, where h is holding capacity, c is capcity, v is free flow speed. i.e., wvRatio should < 1.
  if (m_wave_ratio < 0){
    printf("Wave ratio won't less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);
  }
  m_last_wave_ratio = (m_lane_flow_cap / (_lane_hold_cap_last_cell - m_lane_flow_cap / m_ffs))/m_ffs;
  if (m_last_wave_ratio < 0){
    printf("Last cell Wave ratio won't less than zero, current link ID is %d\n", m_link_ID());
    exit(-1);
  }  
  init_cell_array(unit_time, _std_cell_length, _lane_hold_cap_last_cell);
}

MNM_Dlink_Ctm::~MNM_Dlink_Ctm()
{
  for (Ctm_Cell* _cell : m_cell_array){
    delete _cell;
  }
  m_cell_array.clear();
}

int MNM_Dlink_Ctm::init_cell_array( TFlt unit_time, TFlt std_cell_length, TFlt lane_hold_cap_last_cell )
{

  Ctm_Cell *aCell = NULL;
  for(int i = 0; i<m_num_cells - 1; ++i) {
    aCell = new Ctm_Cell(TFlt(m_number_of_lane) * std_cell_length * m_lane_hold_cap, 
                         TFlt(m_number_of_lane) * m_lane_flow_cap * unit_time,
                         m_flow_scalar,
                         m_wave_ratio);
    if(aCell == NULL) {
      // LOG(WARNING) << "Fail to init the cell.";
      exit(-1);
    };
    m_cell_array.push_back(aCell);
    aCell = NULL;
  }
  
  //since the last cell is a non-standard cell
  if(m_length > 0.0) {
    aCell = new Ctm_Cell(TFlt(m_number_of_lane) * std_cell_length * lane_hold_cap_last_cell, 
                         TFlt(m_number_of_lane) * m_lane_flow_cap * unit_time,
                         m_flow_scalar,
                         m_last_wave_ratio);
    if(aCell == NULL) {
      // LOG(WARNING) << "Fail to init the cell.";
      exit(-1);
    }
    m_cell_array.push_back(aCell);
  }

  //compress the cellArray to reduce space 
  m_cell_array.shrink_to_fit();

  return 0;
}

void MNM_Dlink_Ctm::print_info() {
  printf("Total number of cell: \t%d\t Standard wave ratio: \t%.4f\nLast cell wave ratio: \t%.4f\n", 
          int(m_num_cells), double(m_wave_ratio), double(m_last_wave_ratio));
  printf("Volume for each cell is:\n");
  for (int i = 0; i < m_num_cells; ++i)
  {
    printf("%d, ", int(m_cell_array[i] -> m_volume));
  }
  printf("\n");
}

int MNM_Dlink_Ctm::update_out_veh()
{
  TFlt _temp_out_flux, _supply, _demand,_lastdemand,_lastsupply;
  int _eqaulCap = 0;
  if(m_num_cells > 1) // if only one cell, no update is needed
  {
    for (int i = 0; i < m_num_cells - 1; ++i)
    {
      _demand = m_cell_array[i]->get_demand();
      _supply = m_cell_array[i+1]->get_supply();
      if(i!=0){
        if (_lastdemand > _supply){
          m_congested = 1;
        }else if(_lastdemand == _supply){
          _eqaulCap = 1;
        }
      }
      // if (_supply < m_cell_array[i+1]-> m_flow_cap){
      //   m_congested = 1;
      // }else if(_supply  == m_cell_array[i+1]-> m_flow_cap){
      //   _eqaulCap = 1;

      // }
      _lastdemand = _demand;
      _temp_out_flux = MNM_Ults::min(_demand, _supply) * m_flow_scalar;
      m_cell_array[i] -> m_out_veh= MNM_Ults::round(_temp_out_flux); 
    }
  }
  m_cell_array[m_num_cells - 1] -> m_out_veh = m_cell_array[m_num_cells - 1] -> m_veh_queue.size();
  if (m_congested==0 && _eqaulCap ==1){
    m_congested =-1;
  }
  return 0;
}

int MNM_Dlink_Ctm::evolve(TInt timestamp)
{
  // printf("update_out_veh\n");
  m_congested = 0;
  update_out_veh();
  TInt _num_veh_tomove;
  // printf("move previou cells\n");
  /* previous cells */
  if(m_num_cells > 1) {
    for (int i = 0; i < m_num_cells - 1; ++i) {
      _num_veh_tomove = m_cell_array[i] -> m_out_veh;
      move_veh_queue(&(m_cell_array[i] -> m_veh_queue),
                 &(m_cell_array[i+1] -> m_veh_queue),
                 _num_veh_tomove);
    }
  }
  /* last cell */
  // printf("move last cell \n");
  move_last_cell();

  /* update volume */
  // printf("update volume\n");
  if(m_num_cells > 1) {
    for (int i = 0; i < m_num_cells - 1; ++i) {
      m_cell_array[i] -> m_volume = m_cell_array[i] -> m_veh_queue.size();
    }
  }
  m_cell_array[m_num_cells - 1] -> m_volume = m_cell_array[m_num_cells - 1] -> m_veh_queue.size()
                                                + m_finished_array.size();
  return 0;
}


TFlt MNM_Dlink_Ctm::get_link_supply() {
  return m_cell_array[0] -> get_supply();
}

int MNM_Dlink_Ctm::clear_incoming_array() {
  // printf("link ID: %d, in comming: %d, supply : %d\n", (int )m_link_ID,(int)m_incoming_array.size(), (int) (get_link_supply() * m_flow_scalar) );
  if (get_link_supply() * m_flow_scalar < m_incoming_array.size()) {
    // LOG(WARNING) << "Wrong incoming array size";
    exit(-1);
  }
  move_veh_queue(&m_incoming_array, &(m_cell_array[0] -> m_veh_queue), m_incoming_array.size());

  m_cell_array[0] -> m_volume = m_cell_array[0] ->m_veh_queue.size();
  return 0;
}


int MNM_Dlink_Ctm::move_last_cell() {
  TInt _num_veh_tomove;
  _num_veh_tomove = m_cell_array[m_num_cells - 1] -> m_out_veh;
  MNM_Veh* _veh;
  std::map<TInt, std::deque<MNM_Veh*>*>::iterator _que_it;
  for (int i=0; i<_num_veh_tomove; ++i) {
    _veh = m_cell_array[m_num_cells - 1] -> m_veh_queue.front();
    m_cell_array[m_num_cells - 1] -> m_veh_queue.pop_front();
    if (_veh -> has_next_link()) {
      m_finished_array.push_back(_veh);
    }
    else {
      printf("Dlink_CTM::Some thing wrong!\n");
      exit(-1);
      // _veh -> get_destionation() -> receive_veh(_veh);
    }
  }
  return 0;
}

MNM_Dlink_Ctm::Ctm_Cell::Ctm_Cell(TFlt hold_cap, TFlt flow_cap, TFlt flow_scalar, TFlt wave_ratio)
{
  m_hold_cap = hold_cap;
  m_flow_cap = flow_cap;
  m_flow_scalar = flow_scalar;
  m_wave_ratio = wave_ratio;
  m_volume = TInt(0);
  m_veh_queue = std::deque<MNM_Veh*>();
}

MNM_Dlink_Ctm::Ctm_Cell::~Ctm_Cell()
{
  m_veh_queue.clear();
}


TFlt MNM_Dlink_Ctm::Ctm_Cell::get_demand()
{
  TFlt _real_volume = TFlt(m_volume) / m_flow_scalar;
  return std::min(_real_volume, m_flow_cap);
}

TFlt MNM_Dlink_Ctm::Ctm_Cell::get_supply()
{
  TFlt _real_volume = TFlt(m_volume) / m_flow_scalar;
  if (_real_volume >= m_hold_cap) 
  {
    // _real_volume = m_hold_cap;
    return TFlt(0.0);
  }
  if(m_wave_ratio <= 1.0) //this one is quite tricky, why not just _min(flwCap, hldCap - curDensity)*wvRatio? 
    return m_flow_cap > _real_volume ? m_flow_cap: TFlt((m_hold_cap - _real_volume) * m_wave_ratio);  
    //flowCap equals to critical density
  else 
    return std::min(m_flow_cap, TFlt(m_hold_cap - _real_volume));
}


TFlt MNM_Dlink_Ctm::get_link_flow()
{
  TInt _total_volume = 0;
  for (int i = 0; i < m_num_cells; ++i){
    _total_volume += m_cell_array[i] -> m_volume;
  }
  return TFlt(_total_volume) / m_flow_scalar;
}


TFlt MNM_Dlink_Ctm::get_link_tt()
{
  TFlt _cost, _spd;
  TFlt _rho  = get_link_flow()/m_number_of_lane/m_length;// get the density in veh/mile
  TFlt _rhoj = m_lane_hold_cap; //get the jam density
  TFlt _rhok = m_lane_flow_cap/m_ffs; //get the critical density
  //  if (abs(rho - rhok) <= 0.0001) cost = POS_INF_INT;
  if (_rho >= _rhoj) {
    _cost = MNM_Ults::max_link_cost(); //sean: i think we should use rhoj, not rhok
  } 
  else {
    if (_rho <= _rhok) {
      _spd = m_ffs;
    }
    else {
      _spd = MNM_Ults::max(0.001*m_ffs, m_lane_flow_cap *(_rhoj - _rho)/((_rhoj - _rhok)*_rho));
    }
    _cost = m_length/_spd;
  } 
  return _cost;
}

/**************************************************************************
                          Poing Queue
**************************************************************************/
MNM_Dlink_Pq::MNM_Dlink_Pq(   TInt ID,
                              TFlt lane_hold_cap, 
                              TFlt lane_flow_cap, 
                              TInt number_of_lane,
                              TFlt length,
                              TFlt ffs,
                              TFlt unit_time,
                              TFlt flow_scalar)
  : MNM_Dlink::MNM_Dlink ( ID, number_of_lane, length, ffs )
{
  m_lane_hold_cap = lane_hold_cap;
  m_lane_flow_cap = lane_flow_cap;
  m_flow_scalar = flow_scalar;
  m_hold_cap = m_lane_hold_cap * TFlt(number_of_lane) * m_length;
  m_max_stamp = MNM_Ults::round(m_length/(m_ffs * unit_time));
  m_veh_queue = std::unordered_map<MNM_Veh*, TInt>();
  m_volume = TInt(0);
  m_unit_time = unit_time;
}


MNM_Dlink_Pq::~MNM_Dlink_Pq()
{
  m_veh_queue.clear();
}

TFlt MNM_Dlink_Pq::get_link_supply()
{
  return m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time;
}

int MNM_Dlink_Pq::clear_incoming_array() {
  TInt _num_veh_tomove = std::min(TInt(m_incoming_array.size()), TInt(get_link_supply() * m_flow_scalar));
  MNM_Veh *_veh;
  for (int i=0; i < _num_veh_tomove; ++i) {
    _veh = m_incoming_array.front();
    m_incoming_array.pop_front();
    m_veh_queue.insert(std::pair<MNM_Veh*, TInt>(_veh, TInt(0)));
  }
  m_volume = TInt(m_finished_array.size() + m_veh_queue.size());
  // move_veh_queue(&m_incoming_array, , m_incoming_array.size());

  // m_cell_array[0] -> m_volume = m_cell_array[0] ->m_veh_queue.size();
  return 0;
}

void MNM_Dlink_Pq::print_info()
{
  printf("Link Dynamic model: Poing Queue\n");
  printf("Real volume in the link: %.4f\n", (float)(m_volume/m_flow_scalar));
  printf("Finished real volume in the link: %.2f\n", (float)(TFlt(m_finished_array.size())/m_flow_scalar));
}

int MNM_Dlink_Pq::evolve(TInt timestamp)
{
  std::unordered_map<MNM_Veh*, TInt>::iterator _que_it = m_veh_queue.begin();
  while (_que_it != m_veh_queue.end()) {
    if (_que_it -> second >= m_max_stamp) {
      m_finished_array.push_back(_que_it -> first);
      _que_it = m_veh_queue.erase(_que_it); //c++ 11
    }
    else {
      _que_it -> second += 1;
      _que_it ++;
    }
  }

  /* volume */
  // m_volume = TInt(m_finished_array.size() + m_veh_queue.size());

  return 0;
}

TFlt MNM_Dlink_Pq::get_link_flow()
{
  return TFlt(m_volume) / m_flow_scalar;
}



TFlt MNM_Dlink_Pq::get_link_tt()
{
  TFlt _cost, _spd;
  TFlt _rho  = get_link_flow()/m_number_of_lane/m_length;// get the density in veh/mile
  TFlt _rhoj = m_lane_hold_cap; //get the jam density
  TFlt _rhok = m_lane_flow_cap/m_ffs; //get the critical density
  //  if (abs(rho - rhok) <= 0.0001) cost = POS_INF_INT;
  if (_rho >= _rhoj) {
    _cost = MNM_Ults::max_link_cost(); //sean: i think we should use rhoj, not rhok
  } 
  else {
    if (_rho <= _rhok) {
      _spd = m_ffs;
    }
    else {
      _spd = MNM_Ults::max(DBL_EPSILON * m_ffs, m_lane_flow_cap *(_rhoj - _rho)/((_rhoj - _rhok)*_rho));
    }
    _cost = m_length/_spd;
  } 
  return _cost;
}



/**************************************************************************
                          Link Queue
**************************************************************************/
MNM_Dlink_Lq::MNM_Dlink_Lq(   TInt ID,
                              TFlt lane_hold_cap, 
                              TFlt lane_flow_cap, 
                              TInt number_of_lane,
                              TFlt length,
                              TFlt ffs,
                              TFlt unit_time,
                              TFlt flow_scalar)
  : MNM_Dlink::MNM_Dlink ( ID, number_of_lane, length, ffs )
{
  m_lane_hold_cap = lane_hold_cap;
  m_lane_flow_cap = lane_flow_cap;
  m_flow_scalar = flow_scalar;
  m_hold_cap = m_lane_hold_cap * TFlt(number_of_lane) * m_length;
  m_veh_queue = std::deque<MNM_Veh*>();
  m_volume = TInt(0);
  m_C = m_lane_flow_cap * TFlt(m_number_of_lane);
  m_k_c = m_lane_flow_cap / m_ffs * TFlt(m_number_of_lane);
  m_unit_time = unit_time;
}

MNM_Dlink_Lq::~MNM_Dlink_Lq()
{
  m_veh_queue.clear();
}

TFlt MNM_Dlink_Lq::get_link_supply()
{
  TFlt _cur_density = get_link_flow() / m_length;
  return  _cur_density > m_k_c ? 
          TFlt(get_flow_from_density(_cur_density) * m_unit_time)
          : TFlt(m_C * m_unit_time);
}

int MNM_Dlink_Lq::clear_incoming_array() {
  if (TInt(m_incoming_array.size()) > TInt(get_link_supply() * m_flow_scalar)){
    printf("Error in MNM_Dlink_Lq::clear_incoming_array()\n");
    exit(-1);
  }
  move_veh_queue(&m_incoming_array, &m_veh_queue, m_incoming_array.size());

  m_volume = TInt(m_finished_array.size() + m_veh_queue.size());
  return 0;
}

void MNM_Dlink_Lq::print_info()
{
  printf("Link Dynamic model: Link Queue\n");
  printf("Real volume in the link: %.4f\n", (float)(m_volume/m_flow_scalar));
  printf("Finished real volume in the link: %.2f\n", (float)(TFlt(m_finished_array.size())/m_flow_scalar));
}

int MNM_Dlink_Lq::evolve(TInt timestamp)
{
  // printf("1\n");
  TFlt _demand = get_demand();
  MNM_Veh *_veh;
  if (_demand < TFlt(m_finished_array.size()) / m_flow_scalar){
    // printf("2\n");
    TInt _veh_to_reduce = TInt(m_finished_array.size()) - TInt(_demand * m_flow_scalar);
    _veh_to_reduce = std::min(_veh_to_reduce, TInt(m_finished_array.size()));
    // printf("_veh_to_reduce %d, finished size is %d\n", _veh_to_reduce(), (int)m_finished_array.size());
    for (int i=0; i< _veh_to_reduce; ++i){
      _veh = m_finished_array.back();
      m_veh_queue.push_front(_veh);
      m_finished_array.pop_back();
    }
  }
  else {
    // printf("3\n");
    TInt _veh_to_move = MNM_Ults::round(_demand * m_flow_scalar) - TInt(m_finished_array.size());
    // printf("demand %f, Veh queue size %d, finished size %d, to move %d \n", (float) _demand(), (int) m_veh_queue.size(), (int)m_finished_array.size(), _veh_to_move());

    _veh_to_move = std::min(_veh_to_move, TInt(m_veh_queue.size()));

    for (int i=0; i< _veh_to_move; ++i){
      _veh = m_veh_queue.front();
      m_finished_array.push_back(_veh);
      m_veh_queue.pop_front();
    }
  }
  return 0;
}

TFlt MNM_Dlink_Lq::get_link_flow()
{
  return TFlt(m_volume) / m_flow_scalar;
}


TFlt MNM_Dlink_Lq::get_link_tt()
{
  TFlt _cost, _spd;
  TFlt _rho  = get_link_flow()/m_number_of_lane/m_length;// get the density in veh/mile
  TFlt _rhoj = m_lane_hold_cap; //get the jam density
  TFlt _rhok = m_lane_flow_cap/m_ffs; //get the critical density
  //  if (abs(rho - rhok) <= 0.0001) cost = POS_INF_INT;
  if (_rho >= _rhoj) {
    _cost = MNM_Ults::max_link_cost(); //sean: i think we should use rhoj, not rhok
  } 
  else {
    if (_rho <= _rhok) {
      _spd = m_ffs;
    }
    else {
      _spd = MNM_Ults::max(DBL_EPSILON * m_ffs, m_lane_flow_cap *(_rhoj - _rho)/((_rhoj - _rhok)*_rho));
    }
    _cost = m_length/_spd;
  } 
  return _cost;
}

TFlt MNM_Dlink_Lq::get_flow_from_density(TFlt density)
{
  TFlt _flow;
  if (density < m_k_c){
    _flow = m_ffs * density;
  }
  else {
    TFlt _w = m_lane_flow_cap / (m_lane_hold_cap - m_lane_flow_cap / m_ffs);
    _flow = _w * density;
  }
  return _flow; 
}


TFlt MNM_Dlink_Lq::get_demand()
{
  TFlt _density = get_link_flow() / m_length;
  TFlt _demand;
  if (_density < m_k_c){
    _demand = get_flow_from_density(get_link_flow()/m_length);
  }
  else {
    _demand = m_C;
  }
  return _demand * m_unit_time;
}


/**************************************************************************
                          Cumulative curve
**************************************************************************/

MNM_Cumulative_Curve::MNM_Cumulative_Curve()
{
  m_recorder =  std::deque<std::pair<TFlt, TFlt>>();
}


MNM_Cumulative_Curve::~MNM_Cumulative_Curve()
{
  m_recorder.clear();
}


bool pair_compare (std::pair<TFlt, TFlt> i,std::pair<TFlt, TFlt> j) 
{
  return (i.first<j.first); 
}

int MNM_Cumulative_Curve::arrange()
{
  std::sort(m_recorder.begin(), m_recorder.end(), pair_compare);
  return 0;
}

int MNM_Cumulative_Curve::add_record(std::pair<TFlt, TFlt> r)
{
  m_recorder.push_back(r);
  return 0;
}


int MNM_Cumulative_Curve::shrink(TInt number)
{
  if (TInt(m_recorder.size()) > number){
    arrange();
    while(TInt(m_recorder.size()) > number){
      m_recorder.pop_back();
    }
  }
  return 0;
}

int MNM_Cumulative_Curve::add_increment(std::pair<TFlt, TFlt> r)
{
  // if (m_recorder.size() == 0){
  //   add_record(r);
  //   return 0;
  // }
  // // std::pair <TFlt, TFlt> _best = *std::max_element(m_recorder.begin(), m_recorder.end(), pair_compare);
  // std::pair <TFlt, TFlt> _best = m_recorder[m_recorder.size() - 1];
  // r.second += _best.second;
  // // printf("New r is <%lf, %lf>\n", r.first(), r.second());
  // m_recorder.push_back(r);
  // return 0;

  if (m_recorder.size() == 0){
      add_record(r);
      return 0;
    }
  // std::pair <TFlt, TFlt> _best = *std::max_element(m_recorder.begin(), m_recorder.end(), pair_compare);
  std::pair <TFlt, TFlt> _best = m_recorder[m_recorder.size() - 1];
  if (r.first < _best.first){
    throw std::runtime_error("Error, MNM_Cumulative_Curve::add_increment, early time index");
  }
  if (r.first == _best.first){
    m_recorder[m_recorder.size() - 1].second += r.second;
  }
  else{
    r.second += _best.second;
    // printf("New r is <%lf, %lf>\n", r.first(), r.second());
    m_recorder.push_back(r); 
  }
  return 0;
}


TFlt MNM_Cumulative_Curve::get_result(TFlt time)
{
  // arrange();
  if (m_recorder.size() == 0){
    return TFlt(0);
  }
  if (m_recorder.size() == 1){
    return m_recorder[0].second;
  }
  if (m_recorder[0].first >= time){
    return m_recorder[0].second;
 }
  for (size_t i=1; i<m_recorder.size(); ++i){
    if (m_recorder[i].first >= time){
      return m_recorder[i-1].second 
          + (m_recorder[i].second - m_recorder[i-1].second)/(m_recorder[i].first - m_recorder[i-1].first)
            * (time - m_recorder[i-1].first);
    }
  }
  return m_recorder.back().second;
}


std::string MNM_Cumulative_Curve::to_string()
{
  std::string _output = "";
  arrange();
  for (size_t i=0; i<m_recorder.size() - 1; ++i){
    _output += std::to_string(m_recorder[i].first) + ":" + std::to_string(m_recorder[i].second) + ",";
  }
  _output += std::to_string(m_recorder[m_recorder.size() - 1].first) + ":" + std::to_string(m_recorder[m_recorder.size() - 1].second);
  return _output;
}

/**************************************************************************
                          Link Transmission model
**************************************************************************/
MNM_Dlink_Ltm::MNM_Dlink_Ltm(   TInt ID,
                              TFlt lane_hold_cap, 
                              TFlt lane_flow_cap, 
                              TInt number_of_lane,
                              TFlt length,
                              TFlt ffs,
                              TFlt unit_time,
                              TFlt flow_scalar)
  : MNM_Dlink::MNM_Dlink ( ID, number_of_lane, length, ffs )
{
  m_lane_hold_cap = lane_hold_cap;
  m_lane_flow_cap = lane_flow_cap;
  m_flow_scalar = flow_scalar;
  m_hold_cap = m_lane_hold_cap * TFlt(number_of_lane) * m_length;
  m_veh_queue = std::deque<MNM_Veh*>();
  m_volume = TInt(0);
  m_current_timestamp = TInt(0);
  m_unit_time = unit_time;
  m_w = m_lane_flow_cap / (m_lane_hold_cap - m_lane_flow_cap / m_ffs);
  m_N_in2 = MNM_Cumulative_Curve();
  m_N_out2 = MNM_Cumulative_Curve();
  m_previous_finished_flow = TFlt(0);
  m_record_size = TInt(MNM_Ults::max(m_length/m_w, m_length/m_ffs) / m_unit_time) + 1;
}

MNM_Dlink_Ltm::~MNM_Dlink_Ltm()
{
  m_veh_queue.clear();
}

TFlt MNM_Dlink_Ltm::get_link_flow()
{
  return TFlt(m_volume) / m_flow_scalar;
}


TFlt MNM_Dlink_Ltm::get_link_tt()
{
  TFlt _cost, _spd;
  TFlt _rho  = get_link_flow()/m_number_of_lane/m_length;// get the density in veh/mile
  TFlt _rhoj = m_lane_hold_cap; //get the jam density
  TFlt _rhok = m_lane_flow_cap/m_ffs; //get the critical density
  //  if (abs(rho - rhok) <= 0.0001) cost = POS_INF_INT;
  if (_rho >= _rhoj) {
    _cost = MNM_Ults::max_link_cost(); //sean: i think we should use rhoj, not rhok
  } 
  else {
    if (_rho <= _rhok) {
      _spd = m_ffs;
    }
    else {
      _spd = MNM_Ults::max(DBL_EPSILON * m_ffs, m_lane_flow_cap *(_rhoj - _rho)/((_rhoj - _rhok)*_rho));
    }
    _cost = m_length/_spd;
  } 
  return _cost;
}

void MNM_Dlink_Ltm::print_info()
{
  printf("Link Dynamic model: Link Transmission Model\n");
  printf("Real volume in the link: %.4f\n", (float)(m_volume/m_flow_scalar));
  printf("Finished real volume in the link: %.2f\n", (float)(TFlt(m_finished_array.size())/m_flow_scalar));
}



int MNM_Dlink_Ltm::clear_incoming_array() {
  // printf("MNM_Dlink_Ltm::clear_incoming_array\n");
  if (TInt(m_incoming_array.size()) > TInt(get_link_supply() * m_flow_scalar)){
    printf("Error in MNM_Dlink_Ltm::clear_incoming_array()\n");
    exit(-1);
  }
  m_N_in2.add_increment(std::pair<TFlt, TFlt>(TFlt(m_current_timestamp * m_unit_time), TFlt(m_incoming_array.size())/m_flow_scalar));
  move_veh_queue(&m_incoming_array, &m_veh_queue, m_incoming_array.size());

  m_volume = TInt(m_finished_array.size() + m_veh_queue.size());
  // printf("Finished clear\n");
  return 0;
}

TFlt MNM_Dlink_Ltm::get_link_supply()
{
  // printf("MNM_Dlink_Ltm::get_link_supply\n");
  TFlt _recv = m_N_out2.get_result(TFlt(m_current_timestamp * m_unit_time + m_unit_time) - m_length / m_w)
                   + m_hold_cap
                   - m_N_in2.get_result(TFlt(m_current_timestamp));
  TFlt _res = MNM_Ults::min(_recv, m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time);
  return MNM_Ults::max(_res, TFlt(0));
}

int MNM_Dlink_Ltm::evolve(TInt timestamp)
{
  // printf("MNM_Dlink_Ltm::evolve\n");
  m_N_in2.shrink(m_record_size);
  m_N_out2.shrink(m_record_size);
  // printf("target is %d, n in size %d, n out size %d\n",m_record_size(), m_N_in2.m_recorder.size(), m_N_out2.m_recorder.size());
  TFlt _demand = get_demand();
  MNM_Veh *_veh;
  if (_demand < TFlt(m_finished_array.size()) / m_flow_scalar){
    TInt _veh_to_reduce = TInt(m_finished_array.size()) - TInt(_demand * m_flow_scalar);
    _veh_to_reduce = std::min(_veh_to_reduce, TInt(m_finished_array.size()));
    for (int i=0; i < _veh_to_reduce; ++i){
      _veh = m_finished_array.back();
      if (_veh == NULL){
        printf("Error in MNM_Dlink_Ltm::evolve -> not enough in finish\n");
        exit(-1);
      }
      m_veh_queue.push_front(_veh);
      m_finished_array.pop_back();
    }
  }
  else {
    TInt _veh_to_move = MNM_Ults::round(_demand * m_flow_scalar) - TInt(m_finished_array.size());
    // printf("demand %f, Veh queue size %d, finished size %d, to move %d \n", (float) _demand(), (int) m_veh_queue.size(), (int)m_finished_array.size(), _veh_to_move());

    _veh_to_move = std::min(_veh_to_move, TInt(m_veh_queue.size()));

    for (int i=0; i< _veh_to_move; ++i){
      _veh = m_veh_queue.front();
      if (_veh == NULL){
        printf("Error in MNM_Dlink_Ltm::evolve -> not enough in veh_queue\n");
        exit(-1);
      }      
      m_finished_array.push_back(_veh);
      m_veh_queue.pop_front();
    }
  }

  m_current_timestamp += 1;
  m_previous_finished_flow = TFlt(m_finished_array.size()) / m_flow_scalar;
  return 0;
}

TFlt MNM_Dlink_Ltm::get_demand()
{
  // printf("MNM_Dlink_Ltm::get_demand\n");
  TFlt _real_finished_flow = m_previous_finished_flow - TFlt(m_finished_array.size()) / m_flow_scalar;
  m_N_out2.add_increment(std::pair<TFlt, TFlt>(TFlt(m_current_timestamp * m_unit_time), _real_finished_flow));
  TFlt _send = m_N_in2.get_result(TFlt(m_current_timestamp * m_unit_time + m_unit_time) - m_length / m_ffs)
               - m_N_out2.get_result(TFlt(m_current_timestamp * m_unit_time));
  return MNM_Ults::min(_send, m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time);
}








/**************************************************************************
                          Point Queue 2 (infinite capacity, outflow rate limited by the property of the link)
**************************************************************************/
MNM_Dlink_Pq2::MNM_Dlink_Pq2(   TInt ID,
                              TFlt lane_hold_cap, 
                              TFlt lane_flow_cap, 
                              TInt number_of_lane,
                              TFlt length,
                              TFlt ffs,
                              TFlt unit_time,
                              TFlt flow_scalar)
  : MNM_Dlink::MNM_Dlink ( ID, number_of_lane, length, ffs )
{
  m_lane_hold_cap = lane_hold_cap;
  m_lane_flow_cap = lane_flow_cap;
  m_flow_scalar = flow_scalar;
  m_hold_cap = m_lane_hold_cap * TFlt(number_of_lane) * m_length;
  m_max_stamp = MNM_Ults::round(m_length/(m_ffs * unit_time));
  m_veh_queue = std::unordered_map<MNM_Veh*, TInt>();
  m_volume = TInt(0);
  m_unit_time = unit_time;
  m_veh_dequeue = std::deque<MNM_Veh*>();
}


MNM_Dlink_Pq2::~MNM_Dlink_Pq2()
{
  m_veh_queue.clear();
}

TFlt MNM_Dlink_Pq2::get_link_supply()
{
  return TFlt(std::numeric_limits<float>::max());
}

int MNM_Dlink_Pq2::clear_incoming_array() {
  // TInt _num_veh_tomove = std::min(TInt(m_incoming_array.size()), TInt(get_link_supply() * m_flow_scalar));
  TInt _num_veh_tomove = TInt(m_incoming_array.size());
  MNM_Veh *_veh;
  for (int i=0; i < _num_veh_tomove; ++i) {
    _veh = m_incoming_array.front();
    m_incoming_array.pop_front();
    m_veh_queue.insert(std::pair<MNM_Veh*, TInt>(_veh, TInt(0)));
    m_veh_dequeue.push_front(_veh);
  }
  m_volume = TInt(m_finished_array.size() + m_veh_queue.size());
  // move_veh_queue(&m_incoming_array, , m_incoming_array.size());

  // m_cell_array[0] -> m_volume = m_cell_array[0] ->m_veh_queue.size();
  return 0;
}

void MNM_Dlink_Pq2::print_info()
{
  TFlt cap = m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time;
  std::cout << "Flow cap:" << m_lane_flow_cap << ", number of lanes:" << m_number_of_lane << ",unit time:"
    << m_unit_time << std::endl;
  printf("Link Dynamic model: Poing Queue\n");
  printf("Real volume in the link: %.4f\n", (float)(m_volume/m_flow_scalar));
  printf("Finished real volume in the link: %.2f\n", (float)(TFlt(m_finished_array.size())/m_flow_scalar));
  printf("The capacity is %.4f\n",cap);
  printf("Flow scalar rate is %.4f\n",m_flow_scalar);
}

int MNM_Dlink_Pq2::evolve(TInt timestamp)
{
  
  if (m_veh_dequeue.size()==0)
    return 0;
  MNM_Veh* _veh = m_veh_dequeue.back();
  TFlt _cap = m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time * m_flow_scalar;
  
  while (m_veh_dequeue.size()>0){
    if (m_veh_queue[_veh] >= m_max_stamp && m_finished_array.size() < _cap){
      m_finished_array.push_back(_veh);
      m_veh_queue.erase(_veh);
      m_veh_dequeue.pop_back();
      _veh = m_veh_dequeue.back();
    }else{
      break;
    }
  }
  for(auto _deq_it =m_veh_dequeue.begin();_deq_it!=m_veh_dequeue.end();_deq_it++){
    m_veh_queue[*_deq_it] +=1;
  }
  // std::cout << " finish evolve" << std::endl;
  return 1;


  // std::unordered_map<MNM_Veh*, TInt>::iterator _que_it = m_veh_queue.begin();
  // TFlt _cap = m_lane_flow_cap * TFlt(m_number_of_lane) * m_unit_time * m_flow_scalar;
  // while (_que_it != m_veh_queue.end()) {
  //   if (_que_it -> second >= m_max_stamp && m_finished_array.size() < _cap) {
  //     m_finished_array.push_back(_que_it -> first);
  //     _que_it = m_veh_queue.erase(_que_it); //c++ 11
  //   }
  //   else {
  //     _que_it -> second += 1;
  //     _que_it ++;
  //   }
  // }

  // /* volume */
  // // m_volume = TInt(m_finished_array.size() + m_veh_queue.size());

  // return 0;
}

TFlt MNM_Dlink_Pq2::get_link_flow()
{
  return TFlt(m_volume) / m_flow_scalar;
}



TFlt MNM_Dlink_Pq2::get_link_tt()
{
  TFlt _cost, _spd;
  TFlt _rho  = get_link_flow()/m_number_of_lane/m_length;// get the density in veh/mile
  TFlt _rhoj = m_lane_hold_cap; //get the jam density
  TFlt _rhok = m_lane_flow_cap/m_ffs; //get the critical density
  //  if (abs(rho - rhok) <= 0.0001) cost = POS_INF_INT;
  if (_rho >= _rhoj) {
    _cost = MNM_Ults::max_link_cost(); //sean: i think we should use rhoj, not rhok
  } 
  else {
    if (_rho <= _rhok) {
      _spd = m_ffs;
    }
    else {
      _spd = MNM_Ults::max(DBL_EPSILON * m_ffs, m_lane_flow_cap *(_rhoj - _rho)/((_rhoj - _rhok)*_rho));
    }
    _cost = m_length/_spd;
  } 
  return _cost;
}

/**************************************************************************
                      End Point Queue 2 (infinite capacity, )
**************************************************************************/


/**********
        extra method for SO DTA
*******************/

TFlt MNM_Dlink::get_link_fftt(){
  return m_length/m_ffs;
}



TFlt MNM_Cumulative_Curve::get_time(TFlt count){
  //need to use ceil, so that the travel time is integer times of unit time
  arrange();
  if (m_recorder.size() == 0 || m_recorder.back().second < count){
    std::cout<<"Warning: you are requesting the time of a count that exceeds the total counts of the link" << std::endl;
    return TFlt(0);
  }
  if (m_recorder.size() == 1 && m_recorder[0].second >= count){
    return MNM_Ults::divide(count,m_recorder[0].second) * m_recorder[0].first;
  }
 //  if (m_recorder[0].first >= time){
 //    return m_recorder[0].second;
 // }
  for (size_t i=1; i<m_recorder.size(); ++i){
    if (m_recorder[i].second >= count){
      return m_recorder[i-1].first  
          + (count - m_recorder[i-1].second)/(m_recorder[i].second - m_recorder[i-1].second)
            * (m_recorder[i].first - m_recorder[i-1].first);
    }
  }
  return m_recorder.back().first;
}



TFlt MNM_Dlink::get_link_real_tt(TFlt t){
  if (m_N_in == NULL || m_N_out == NULL){
    std::cout << "Warning :: Try to get travel time but the CC is not installed";
    return TFlt(0.0);
  }else{
    TFlt _arrival_count = m_N_in -> get_result(t);
    TFlt _leave_time = m_N_out -> get_time(_arrival_count);
    return TFlt(floor(_leave_time - t));
  }

}



TFlt MNM_Dlink::get_link_lower_PMC(TInt t){
  int _cgst = is_congested_after(t);
  
  if(_cgst==1){
    int _diss_time = (*congestion_dissipate)[t];
    return _diss_time - t + get_link_tt();
  }else{
    return get_link_tt();
  }
}

TFlt MNM_Dlink::get_link_upper_PMC(TInt t){
  int _cgst = is_congested_after(t);
  if (_cgst == -1){
    return get_link_tt();
  }else {
    int _diss_time = (*congestion_dissipate)[t];
    return _diss_time - t + get_link_tt();
  }

  
}


TFlt MNM_Dlink::get_link_inflow(TFlt t){
  // get the inflow rate at time t
  return TFlt(0.0);
}

TFlt MNM_Dlink::get_link_outflow(TFlt t){
  // get the outflow rate at time t
  return TFlt(0.0);
}

int MNM_Dlink_Pq::is_congested(){
  // check if the link is congested at current time
  // 0: inflow equal to capacity and no queued flow ()
  // 1: congested 
  // -1: not congested
  int result = 0;
  if (get_link_supply() < get_link_flow() ){
    result = -1;
  }else if(get_link_supply() > get_link_flow() )
    result = 1;
  return result;
}

int MNM_Dlink_Pq2::is_congested(){
  // std::unordered_map<MNM_Veh*, TInt>::iterator _que_it = m_veh_queue.begin();
  // if(m_veh_queue.size()!=0)
    // std::cout << m_veh_queue.end()->second << std::endl;
  // if (m_veh_queue.size() != m_veh_dequeue.size()){
  //   std::cout<<"Wrong with the sizes" <<std::endl;
  //   exit(1);
  // }
  if (m_veh_dequeue.size() > 0 && m_veh_queue[m_veh_dequeue.back()] > m_max_stamp){
    return 1;
  }else
    return 0;
  // if(m_veh_queue.size()!=0 && m_veh_queue.back().second > m_max_stamp)
  //   return 1;
  // else
  //   return 0;

  // if (m_finished_array.size()>0) return 1;
  // while(_que_it!= m_veh_queue.end()){
  //   if(_que_it->second > m_max_stamp){
  //     if (m_link_ID == TInt(2))
  //     std::cout<<_que_it->second<<std::endl;
  //     return 1;
  //   }else{
  //     if (m_link_ID == TInt(2))
  //     std::cout<<_que_it->second << ",";
  //     _que_it ++;
  //   }
  // }
  // if (m_link_ID == TInt(2))
  // std::cout<<std::endl;
  // return 0;
  // check if the link is congested at current time
  // 0: inflow equal to capacity and no queued flow ()
  // 1: congested 
  // -1: not congested
  // int result = 0;
  // if (get_link_supply() < get_link_flow() ){
  //   result = -1;
  // }else if(get_link_supply() > get_link_flow() )
  //   result = 1;
  // return result;

}

int MNM_Dlink_Ctm::is_congested(){
  // check if the link is congested at current time
  // return (m_cell_array[0] -> m_volume) < (m_cell_array[0] -> m_flow_cap) ;
  // if (m_finished_array.size()>0)return 1;
  // else return 0;
  return m_congested;
}

int MNM_Dlink_Lq::is_congested(){
  // check if the link is congested at current time, TODO
  return 1;
}

int MNM_Dlink_Ltm::is_congested(){
  // check if the link is congested at current time, TODO
  return 1;
}


int MNM_Dlink::update_dissipate(){
  //since the DNL will continue untill all vehicles arrives its destinations
  //we should expected that the the final element of indicator_congestion should be -1
  int lasti = 0;
  // std::cout << "Print the congestion indicator:" << std::endl;
  // for (int i=0;i<indicator_congestion->size();i++){
  //   std::cout << indicator_congestion->at(i) << ",";
  // }
  // std::cout << std::endl;
  congestion_dissipate->push_back(0);
  for (size_t i = 0; i < indicator_congestion -> size();i++){
    // std::cout<< (*indicator_congestion)[i] << ",";
    if((*indicator_congestion)[i] == -1 ){
      for (size_t j = lasti+1;j<=i;j++){
        // (*congestion_dissipate)[j] = i;
        congestion_dissipate -> push_back(i);
      }
      lasti = i;
    }else{
      continue;
    }

  }
  if (lasti != indicator_congestion -> size()-1){
    for (size_t j = lasti+1;j<indicator_congestion -> size();j++){
      // (*congestion_dissipate)[j] = indicator_congestion -> size()-1;
      congestion_dissipate -> push_back(indicator_congestion -> size()-1);
    }
  }
  // std::cout << std::endl;
  // std::cout << congestion_dissipate->size() << ":" << std::endl;
  // for (int i=0;i < congestion_dissipate->size();i++){
  //   std::cout << congestion_dissipate->at(i)<<",";
  // }
  // std::cout << std::endl;

  return 1;
}

int MNM_Dlink::is_congested_after(TInt t){

  // std::cout << t <<  ","<<  indicator_congestion->size()<< std::endl;
  // if (t > indicator_congestion->size()){
  //   std::cout <<"Abnormal here:" <<indicator_congestion->size() << "," << t  <<std::endl;
  //   std::cout << (*indicator_congestion)[t] << std::endl;
  // }
  if (t > indicator_congestion->size()){
    return -1;
  }
  return (*indicator_congestion)[t];
}


TInt MNM_Dlink::next_pmc_time_lower(TInt t,TFlt unit_time){
  // if (t > indicator_congestion->size()){
  //   std::cout <<"Abnormal:" <<indicator_congestion->size() << "," << t  <<std::endl;
  //   std::cout << (*indicator_congestion)[t] << std::endl;
  // }
  int _cgst = is_congested_after(t);
  TInt result;
  if(_cgst==1){
    int _diss_time = (*congestion_dissipate)[t];
    // std::cout << "used disstime" << _diss_time << std::endl;
    result =  TInt(_diss_time);
  }else{
    // std::cout << "used fftt" << TInt(std::floor(get_link_fftt()/unit_time+t)) << std::endl;
    result =  TInt(std::floor(get_link_fftt()/unit_time+t));
  }
  // if (result > indicator_congestion->size()){
  //   std::cout <<"Abnormal:" <<indicator_congestion->size() << "," << t  <<std::endl;
  // }
  return result;
}


TInt MNM_Dlink::next_pmc_time_upper(TInt t,TFlt unit_time){
  // std::cout << "Link ID:" << m_link_ID << std::endl;
  // std::cout<< "FFs:"<<m_ffs << std::endl;
  // std::cout <<  "Here" <<congestion_dissipate -> size() << std::endl;
  // std::cout << indicator_congestion -> size() << std::endl;
  // std::cout<< " At time:" << t ;
  TInt result;
  int _cgst = is_congested_after(t);
  // std::cout<< ", is congested?:" << _cgst  <<std::endl;
  if(_cgst!=-1){
    int _diss_time = (*congestion_dissipate)[t];
    // std::cout << "used disstime" << _diss_time << " at " << t << std::endl;
    result =  TInt(_diss_time);
  }else{
    // std::cout << "used fftt" << TInt(std::floor(get_link_fftt()/unit_time+t)) << std::endl;
    result =  TInt(std::floor(get_link_fftt()/unit_time+t));
  }
    // if (result > indicator_congestion->size()){
    // std::cout <<"Abnormal:" <<indicator_congestion->size() << "," << t  <<std::endl;
  // }

  return result;

}
