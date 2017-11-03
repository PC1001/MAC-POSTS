 #ifndef PMC_H
#define PMC_H

#include "path.h"
// #include "dta.h"

#include <unordered_map>

class MNM_PMC_Table{
public:
	MNM_PMC_Table(Path_Table *path_table);
	~MNM_PMC_Table();
	Path_Table *m_path_table;
    std::unordered_map<TInt,std::unordered_map<TInt,std::unordered_map<TInt,TFlt*>>> *m_upper_pmc_table; 
    std::unordered_map<TInt,std::unordered_map<TInt,std::unordered_map<TInt,TFlt*>>> *m_lower_pmc_table; 
    // <O id, <D id, <path id, pmc values>>>
    int update_lower_PMC();
    int update_upper_PMC();

};



#endif