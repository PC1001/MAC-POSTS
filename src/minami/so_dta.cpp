#include "dta.h"

// method of MNM_Dta class that used for SO-DTA

int MNM_Dta::update_pmc_lower(){
	return 0;
}

int MNM_Dta::update_pmc_upper(){
	return 0;
}

TFlt total_TT(){
	return 0.0;
}




namespace MNM{
	int demand_OD_update(MNM_OD_Factory *m_od_factory,
		MNM_Routing_Predetermined *m_routing){
		MNM_Origin *_origin;
		MNM_DMOND *_origin_node;
		MNM_Destination *_destination;
		TInt _node_ID, _next_link_ID;
		MNM_Dlink *_next_link;
		MNM_Veh *_veh;
		MNM_Path *_route_path;
		TInt _origin_node_id;
		TInt _destination_node_id;
		MNM_Pre_Routing * _pre_routing = m_routing -> m_pre_routing;
		for(auto _origin_it = m_od_factory -> m_origin_map.begin();
			_origin_it != m_od_factory -> m_origin_map.end();_origin_it++){
			_origin = _origin_it -> second;
			_origin_node_id  = _origin -> m_origin_node -> m_node_ID;
			_origin -> m_demand.erase(_origin -> m_demand.begin(),_origin -> m_demand.end());

		}
		return 0;
	}
}