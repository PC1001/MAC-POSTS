#include "dta.h"

// method of MNM_Dta class that used for SO-DTA

int MNM_Dta::update_pmc_lower(){
	m_pmc_table -> m_upper_pmc_table -> clear();

	return 0;
}

int MNM_Dta::update_pmc_upper(){
	m_pmc_table -> m_upper_pmc_table -> clear();
	MNM_Pathset *_pset;
	MNM_Path* _path;
	TFlt _pmc = TFlt(0.0);
	for (auto _o_it = m_od_factory -> m_origin_map.begin(); _o_it != m_od_factory -> m_origin_map.end(); _o_it++){
		TInt _o_id = _o_it -> second -> m_origin_node -> m_node_ID;
		std::pair<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>> _myo 
			(_o_id,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>());

		m_pmc_table -> m_upper_pmc_table->insert(_myo);
		for (auto _d_it = m_od_factory -> m_destination_map.begin(); _d_it !=m_od_factory -> m_destination_map.end(); _d_it++){
			TInt _d_id = _d_it -> second -> m_dest_node -> m_node_ID;
			std::pair<TInt,std::unordered_map<TInt,std::vector<TFlt>>> _myd 
				(_d_id,std::unordered_map<TInt,std::vector<TFlt>>());
			m_pmc_table -> m_upper_pmc_table -> at(_o_id).insert(_myd);
			_pset = m_pmc_table -> m_path_table -> at(_o_id)->at(_d_id);
			for(size_t _p_it = 0;_p_it<_pset->m_path_vec.size();_p_it++){
				_pmc = TFlt(0.0);
				// std::pair<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>> _myo ;
				std::pair<TInt,std::vector<TFlt>>  _myp 
					(_p_it,std::vector<TFlt>());  
				m_pmc_table -> m_upper_pmc_table->at(_o_id).at(_d_id).insert(_myp);
				_path = _pset->m_path_vec.at(_p_it);
				for (size_t _l_it = 0;_l_it < _path->m_link_vec.size();_l_it++){
					_pmc += m_link_factory -> get_link(_path->m_link_vec[_l_it]) -> get_link_tt();

				}
				//TO-DO
				m_pmc_table -> m_upper_pmc_table->at(_o_id).at(_d_id).at(_p_it).push_back(_pmc);

			}
		}
	}
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




namespace MNM{
	int demand_OD_update(MNM_OD_Factory *m_od_factory,
		MNM_Routing_Predetermined *m_routing){
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