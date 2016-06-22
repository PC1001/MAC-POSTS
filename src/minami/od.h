#ifndef OD_H
#define OD_H

#include "Snap.h"
#include "factory.h"
#include "dnode.h"
#include "ults.h"

#include <map>

class MNM_Destination;
class MNM_Veh;
class MNM_Veh_Factory;
class MNM_DMOND;
class MNM_DMDND;

class MNM_Origin
{
public:
  MNM_Origin(TInt ID, TInt max_interval, TFlt flow_scalar, TInt frequency);
  ~MNM_Origin();
  TInt m_Origin_ID;
  int release(MNM_Veh_Factory* veh_factory, TInt current_interval);//TODO change to generalized version
  int release_one_interval(TInt current_interval, MNM_Veh_Factory* veh_factory, TInt assign_interval, TFlt adaptive_ratio);
  int add_dest_demand(MNM_Destination *dest, TFlt* demand);
  MNM_DMOND *m_origin_node;
// private:
  TInt m_frequency;
  TInt m_current_assign_interval;
  TInt m_max_assign_interval;
  TFlt m_flow_scalar;
  std::map<MNM_Destination*, TFlt*> m_demand;
};


class MNM_Destination
{
public:
  MNM_Destination(TInt ID);
  ~MNM_Destination();
  TInt m_Dest_ID;
  TFlt m_flow_scalar;
  MNM_DMDND *m_dest_node;
  int receive(TInt current_interval);
};

#endif
