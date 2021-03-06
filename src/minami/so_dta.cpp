#include "dta.h"
#include <limits>
#include <set>
// method of MNM_Dta class that used for SO-DTA

TFlt MNM_Dta::compute_pmc_upper(TInt t, MNM_Path* path){
	//TODO 
	TFlt _pmc = TFlt(0.0);
	TInt _track_time = t;
	// std::cout<< "Computing upper pmc ..............." ;
	for (size_t _l_it = 0;_l_it < path->m_link_vec.size();_l_it++){
		// std::cout << _pmc << "," << _track_time << "," << m_link_factory -> get_link(path->m_link_vec[_l_it]) -> m_link_ID<< std::endl;
		// int ifcongest = m_link_factory -> get_link(path->m_link_vec[_l_it]) -> is_congested_after(_track_time);
		// _pmc += m_link_factory -> get_link(path->m_link_vec[_l_it]) -> get_link_fftt();
		// if (ifcongest!=-1){
		if (m_link_factory -> get_link(path->m_link_vec[_l_it]) -> m_ffs > 1000)continue;
		_pmc += m_link_factory -> get_link(path->m_link_vec[_l_it]) -> next_pmc_time_upper(_track_time,m_unit_time) -_track_time;
		// std::cout << _pmc  << "( " << m_link_factory -> get_link(path->m_link_vec[_l_it]) -> next_pmc_time_upper(_track_time,m_unit_time) << "," << _track_time << ")"  << ",";
		// }
		_track_time =  m_link_factory ->get_link(path->m_link_vec[_l_it]) ->next_pmc_time_upper(_track_time,m_unit_time);

	// std::cout<<  "This tt :" <<  m_link_factory -> get_link(path->m_link_vec[_l_it]) -> get_link_tt() 
	// 	 << ", real time tt: " <<  m_link_factory -> get_link(path->m_link_vec[_l_it]) -> get_link_fftt()
	//  	<< std::endl;
	}
	// std::cout << "..................................." << std::endl;
	// std::cout << _pmc<<std::endl;
	return _pmc;
}

TFlt MNM_Dta::compute_pmc_lower(TInt t, MNM_Path* path){
	//TODO
	TFlt _pmc = TFlt(0.0);
	TInt _track_time = t;
	// std::cout << "Computing UPMC:" ;
	for (size_t _l_it = 0;_l_it < path->m_link_vec.size();_l_it++){
		// int ifcongest = m_link_factory -> get_link(path->m_link_vec[_l_it]) -> is_congested_after(_track_time);
		// _pmc += m_link_factory -> get_link(path->m_link_vec[_l_it]) -> get_link_fftt();
		// if (ifcongest==1){
		if (m_link_factory -> get_link(path->m_link_vec[_l_it]) -> m_ffs > 1000)continue;
		_pmc += m_link_factory -> get_link(path->m_link_vec[_l_it]) -> next_pmc_time_lower(_track_time,m_unit_time) -_track_time;
		// std::cout << _pmc << ",";
		// }
		_track_time =  m_link_factory ->get_link(path->m_link_vec[_l_it]) ->next_pmc_time_lower(_track_time,m_unit_time);
	// std::cout<<  "This tt" <<  m_link_factory -> get_link(_path->m_link_vec[_l_it]) -> get_link_tt() 
	//  	<< std::endl;
	}
	// std::cout << _pmc<<std::endl;
	return _pmc;
}

int MNM_Dta::update_pmc_lower(){
	m_pmc_table -> m_upper_pmc_table -> clear();
	//useless
	

	return 0;
}



int MNM_Dta::route_update_MSA(TFlt lambda,bool verbose){
	//baseline MSA algorithm

	// TO DO
	// 1. what are the interval values (easy) done
	// 2. the upper_pmc function (hard, require first implement the is_congested() funciton of dlink)
	// 3. the reassign_routing function (easy but lengthy) done

	TInt _assign_inter = m_start_assign_interval;
	TInt _cur_int = 0;
	TInt _end_int = m_total_assign_inter; 
	TInt _oid;
	TInt _did;
	MNM_Pathset* _pset;
	MNM_Pre_Routing *pre_routing = m_routing-> m_pre_routing;
	Path_Table * _path_table = pre_routing->m_path_table;
	MNM_Path * _path;
	TFlt _thispmc;
	TInt _min_path_id;
	TFlt _min_PMC; 

	// for each O-D pair, for each time period
	// compute the PMC for each path in path set
	// and update the prerouting table
	// 
	// std::cout << "End time:" << _end_int << std::endl;
	// std::cout << "unit time:" << m_unit_time << std::endl;
	TInt _t;
	TInt _DeltaT = 7; // should be parametric later 
	// TO DO
	for (TInt _int = 0; _int < _end_int;_int++){
		_t = _int * m_assign_freq+_DeltaT;

		for (auto _ops = _path_table -> begin();_ops != _path_table ->end(); _ops++){
			_oid = _ops -> first;
			// std::cout <<_oid << "," << std::endl;
			MNM_DMOND* _othis = dynamic_cast<MNM_DMOND*>(m_node_factory->get_node(_oid));
			// std::cout << _othis->m_origin -> m_demand.size() << std::endl;
			// std::cout << m_node_factory->get_node(_oid)-> m_origin->m_demand.size() << std::endl;
			for (auto _dps = _ops -> second -> begin(); _dps != _ops -> second -> end(); _dps++){
				_did = _dps -> first;
				MNM_DMDND* _dthis = dynamic_cast<MNM_DMDND*>(m_node_factory->get_node(_did));
				if(_othis->m_origin -> m_demand.find(_dthis->m_dest)== _othis->m_origin -> m_demand.end()){
					continue;
				}
				// std::cout << _oid <<"," << _did << ":" << _xx <<std::endl;
				_pset = _dps -> second;
				if (_pset -> m_path_vec.size() ==0)
					continue;
				else{
					_min_path_id = 0;

					_min_PMC = TFlt(std::numeric_limits<float>::max());
					// std::cout << "O " << _oid <<",D " << _did <<": number of path " << _pset -> m_path_vec.size()  << std::endl;
					for(int _pit = 0;_pit < _pset -> m_path_vec.size() ; _pit ++){
						_path  = _pset -> m_path_vec[_pit];
						_thispmc = compute_pmc_upper(_t,_path);
						if ( _thispmc< _min_PMC){
							_min_path_id = _pit;
							_min_PMC = _thispmc;
						}
						// std::cout << "Path id:" << _pit << ", pmc:" <<_thispmc << ", at time " << _t << std::endl;
					}
					// std::cout<< " reassign from " << _oid << " to "<< _did << " at " << _min_path_id
					// 	 << " with pmc "<< _min_PMC << " at time "<< _t <<  std::endl; 
					pre_routing -> reassign_routing(_oid,_did,_min_path_id,_int,lambda);
				}
			}
		}
	}
	// std::cout << pre_routing -> toString() << std::endl;

}




int  MNM_Dta::route_update_PHA(TFlt lambda,bool verbose){

	TInt _assign_inter = m_start_assign_interval;
	TInt _cur_int = 0;
	TInt _end_int = m_total_assign_inter; 
	TInt _oid;
	TInt _did;
	MNM_Pathset* _pset;
	MNM_Pre_Routing *pre_routing = m_routing-> m_pre_routing;
	Path_Table * _path_table = pre_routing->m_path_table;
	MNM_Path * _path;
	TFlt _thisupmc;
	TInt _min_path_id;
	TFlt _min_uPMC; 
	TFlt _min_lPMC;
	TFlt _thislpmc;

	TInt _t;
	for (TInt _int = 0; _int < _end_int;_int++){
		_t = _int * m_assign_freq;

		for (auto _ops = _path_table -> begin();_ops != _path_table ->end(); _ops++){
			_oid = _ops -> first;
			// std::cout <<m_od_factory->get_origin(_oid)->m_demand.size() << std::endl;
			MNM_DMOND* _othis = dynamic_cast<MNM_DMOND*>(m_node_factory->get_node(_oid));
			for (auto _dps = _ops -> second -> begin(); _dps != _ops -> second -> end(); _dps++){

				_did = _dps -> first;
				_pset = _dps -> second;
				MNM_DMDND* _dthis = dynamic_cast<MNM_DMDND*>(m_node_factory->get_node(_did));
				if(_othis->m_origin -> m_demand.find(_dthis->m_dest)== _othis->m_origin -> m_demand.end()){
					continue;
				}
				if (_pset -> m_path_vec.size() ==0)
					continue;
				// else if (m_od_factory.get_origin(_oid)->m_demand[m_od_factory.get_demand(_did)].size()==0){
				// 	continue;
				// }
				else{
					_min_uPMC = TFlt(std::numeric_limits<double>::max());
					_min_lPMC = TFlt(std::numeric_limits<double>::max());
					std::set<TInt> _psmp = std::set<TInt>();

					for(int _pit = 0;_pit < _pset -> m_path_vec.size() ; _pit ++){
						_path  = _pset -> m_path_vec[_pit];
						_thisupmc = compute_pmc_upper(_t,_path);
						_thislpmc = compute_pmc_lower(_t,_path);
						// std::cout << "Path id:" << _pit << ", upmc:" <<_thisupmc << ",lpmc:" << _thislpmc << 
							// ", at time " << _t << std::endl;
						if (_thisupmc < _min_lPMC){
							_psmp.clear();
							_psmp.insert(TInt(_pit));
							_min_lPMC = _thislpmc;
							_min_uPMC = _thisupmc;
						}else if(_thislpmc > _min_uPMC){
							continue;
						}else{
							_psmp.insert(TInt(_pit));
							_min_lPMC = std::min(_min_lPMC,_thislpmc);
							_min_uPMC = std::max(_min_uPMC,_thisupmc);
						}	
					}
					// std::cout<< " reassign from " << _oid << " to "<< _did <<std::endl; 
					pre_routing -> reassign_routing_batch(_oid,_did,_psmp,_int,lambda);


					// _min_path_id = 0;
					// _min_PMC = TFlt(std::numeric_limits<float>::max());
				}
			}
		}
	}
	// std::cout << pre_routing -> toString() << std::endl;

	
	return 0;
}


int MNM_Dta::update_pmc_upper(){
	// //useless
	// m_pmc_table -> m_upper_pmc_table -> clear();
	// MNM_Pathset *_pset;
	// MNM_Path* _path;
	// TFlt _pmc = TFlt(0.0);
	// for (auto _o_it = m_od_factory -> m_origin_map.begin(); _o_it != m_od_factory -> m_origin_map.end(); _o_it++){
	// 	TInt _o_id = _o_it -> second -> m_origin_node -> m_node_ID;
	// 	std::pair<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>> _myo 
	// 		(_o_id,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>());
	// 	m_pmc_table -> m_upper_pmc_table->insert(_myo);
	// 	for (auto _d_it = m_od_factory -> m_destination_map.begin(); _d_it !=m_od_factory -> m_destination_map.end(); _d_it++){
	// 		TInt _d_id = _d_it -> second -> m_dest_node -> m_node_ID;
	// 		std::pair<TInt,std::unordered_map<TInt,std::vector<TFlt>>> _myd 
	// 			(_d_id,std::unordered_map<TInt,std::vector<TFlt>>());
	// 		m_pmc_table -> m_upper_pmc_table -> at(_o_id).insert(_myd);
	// 		_pset = m_pmc_table -> m_path_table -> at(_o_id)->at(_d_id);
	// 		for(size_t _p_it = 0;_p_it<_pset->m_path_vec.size();_p_it++){
	// 			_pmc = TFlt(0.0);
	// 			// std::pair<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>> _myo ;
	// 			std::pair<TInt,std::vector<TFlt>>  _myp (_p_it,std::vector<TFlt>());  
	// 			m_pmc_table -> m_upper_pmc_table->at(_o_id).at(_d_id).insert(_myp);
	// 			_path = _pset->m_path_vec.at(_p_it);
	// 			// std::cout << 
	// 			for (size_t _l_it = 0;_l_it < _path->m_link_vec.size();_l_it++){
	// 				_pmc += m_link_factory -> get_link(_path->m_link_vec[_l_it]) -> get_link_tt();
	// 				// std::cout<<  "This tt" <<  m_link_factory -> get_link(_path->m_link_vec[_l_it]) -> get_link_tt() 
	// 				//  	<< std::endl;

	// 			}

	// 			//TO-DO
				
	// 			m_pmc_table -> m_upper_pmc_table->at(_o_id).at(_d_id).at(_p_it).push_back(_pmc);
	// 		}
	// 	}
	// }
	return 0;
}

TFlt MNM_Dta::total_TT(){
	// only compute the total travel time, no schedule delay
	TFlt tc = TFlt(0.0);
	TFlt thisc = TFlt(0.0);
	MNM_Dlink *_link;
	MNM_Cumulative_Curve *_in_cc;
	MNM_Cumulative_Curve *_out_cc;
	TFlt _time;
	TFlt _count;

	for (auto _link_it =  m_link_factory -> m_link_map.begin(); _link_it != m_link_factory -> m_link_map.end(); _link_it++){
		thisc = TFlt(0.0);
		_link = _link_it -> second;
		if(_link -> m_N_in == NULL){
			std::cout<< "Warning: Cannot get Total COst due to lack of CC" <<std::endl;
			return 0.0;
		}else{
			_in_cc = _link -> m_N_in;
			_out_cc = _link -> m_N_out;
			if (_in_cc -> m_recorder.size() ==0)
				continue;
			else{
				TFlt _lastcount = TFlt(0.0);
				TFlt _lasttime = TFlt(0.0);
				for(size_t i=0;i<_in_cc->m_recorder.size();i++){
					_time = _in_cc -> m_recorder[i].first;
					_count = _in_cc -> m_recorder[i].second;
					thisc += (_count + _lastcount) * (_time - _lasttime) * 0.5;
					_lasttime = _time;
					_lastcount = _count;
				}
				_time = _out_cc -> m_recorder.back().first;
				_count = _out_cc -> m_recorder.back().second;
				thisc += (_count + _lastcount) * (_time - _lasttime) ;
				_lastcount = TFlt(0.0);
				_lasttime = TFlt(0.0);
				for(size_t i=0;i<_out_cc -> m_recorder.size();i++){
					_time = _out_cc -> m_recorder[i].first;
					_count = _out_cc -> m_recorder[i].second;
					thisc -= (_count + _lastcount) * (_time - _lasttime) * 0.5;
					_lasttime = _time;
					_lastcount = _count;
				}
			}
			tc += thisc;
		}

	}
	return tc;
}


int MNM_Dta::link_update_dissipateTime(){
	//TODO
	MNM_Dlink *_link;
	for (auto _link_it =  m_link_factory -> m_link_map.begin(); 
		_link_it != m_link_factory -> m_link_map.end(); _link_it++){
		_link = _link_it -> second;
		_link -> update_dissipate();
	}
	return 0;
}

int MNM_Dta::reinstall_cumulative_curve(){
	MNM_Dlink *_link;
	for (auto _link_it =  m_link_factory -> m_link_map.begin(); 
		_link_it != m_link_factory -> m_link_map.end(); _link_it++){
		_link = _link_it -> second;
		_link -> install_cumulative_curve();
	}
	return 0;
}

// int MNM_Dta::link_update_iscongested(){
// 	//TODO
// 	// should be done after each load_once() function being called
	


// 	return 0;
// }


namespace MNM{
	int demand_OD_update(MNM_OD_Factory *m_od_factory,
		MNM_Routing_Predetermined *m_routing){
		//useless


		// MNM_Origin *_origin;
		// MNM_DMOND *_origin_node;
		// MNM_Destination *_destination;
		// TInt _node_ID, _next_link_ID;
		// MNM_Dlink *_next_link;
		// MNM_Veh *_veh;
		// MNM_Path *_route_path;
		// TInt _origin_node_id;
		// TInt _destination_node_id;
		// MNM_Pre_Routing * _pre_routing = m_routing -> m_pre_routing;
		// for(auto _origin_it = m_od_factory -> m_origin_map.begin();
		// 	_origin_it != m_od_factory -> m_origin_map.end();_origin_it++){
		// 	_origin = _origin_it -> second;
		// 	_origin_node_id  = _origin -> m_origin_node -> m_node_ID;
		// 	_origin -> m_demand.erase(_origin -> m_demand.begin(),_origin -> m_demand.end());

		// }
		return 0;
	}
}