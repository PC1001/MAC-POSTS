#include "pmc_table.h"

MNM_PMC_Table::MNM_PMC_Table(Path_Table *path_table){
	m_path_table = path_table;
	m_upper_pmc_table = new std::unordered_map<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>>();
	m_lower_pmc_table =  new std::unordered_map<TInt,std::unordered_map<TInt,std::unordered_map<TInt,std::vector<TFlt>>>>();
}

MNM_PMC_Table::~MNM_PMC_Table(){
	
}