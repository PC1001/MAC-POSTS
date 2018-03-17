#include "io.h"

#include <fstream>
#include <iostream>

//**************SO-DTA

Path_Table *MNM_IO::load_path_table_ksp(std::string file_name, PNEGraph graph){
  printf("Loading Path Table!\n");

  std::ifstream _path_table_file;
  _path_table_file.open(file_name, std::ios::in);
  Path_Table *_path_table = new Path_Table();

  /* read file */
  std::string _line;
  std::vector<std::string> _words;
  std::vector<std::string> _subwords;
  TInt _origin_node_ID, _dest_node_ID, _node_ID;
  std::unordered_map<TInt, MNM_Pathset*> *_new_map;
  MNM_Pathset *_pathset;
  MNM_Path *_path;
  TInt _from_ID, _to_ID, _link_ID;
  if (_path_table_file.is_open()){
    // for (int i=0; i<Num_Path; ++i){
  while(std::getline(_path_table_file,_line)){
    _words = split(_line, '#');


    /* Get OD node id*/
    _subwords = split(_words[0],',');
    _origin_node_ID = TInt(std::stoi(_subwords[0]));
    _dest_node_ID = TInt(std::stoi(_subwords[1]));


    /* Add path*/
    if (_path_table -> find(_origin_node_ID) == _path_table -> end()){
       _new_map = new std::unordered_map<TInt, MNM_Pathset*>();
        _path_table -> insert(std::pair<TInt, std::unordered_map<TInt, MNM_Pathset*>*>(_origin_node_ID, _new_map));
      }
      // if (_path_table -> find(_origin_node_ID) -> second -> find(_dest_node_ID) == _path_table -> find(_origin_node_ID) -> second -> end()){
        _pathset = new MNM_Pathset();
        _path_table -> find(_origin_node_ID) -> second -> insert(std::pair<TInt, MNM_Pathset*>(_dest_node_ID, _pathset));
      // }
      // printf("Loading Path Table2!\n");
        for (int _pid = 1;_pid<_words.size();_pid++){
          _path = new MNM_Path();
          _subwords = split(_words[_pid],',');
          for(std::string _s_node_ID:_subwords){
            _node_ID = TInt(std::stoi(_s_node_ID));
            _path->m_node_vec.push_back(_node_ID);

          }
          for (size_t i = 0; i < _path -> m_node_vec.size() - 1; ++i){
            _from_ID = _path -> m_node_vec[i];
            _to_ID = _path -> m_node_vec[i+1];
            // printf("Loading Path Table2.5!\n");
            // std::cout<<_from_ID << "," << _to_ID <<std::endl;
            _link_ID = graph -> GetEI(_from_ID, _to_ID).GetId();
            // printf("Loading Path Table3!\n");
            _path -> m_link_vec.push_back(_link_ID);
          }
          _path_table -> find(_origin_node_ID) -> second -> find(_dest_node_ID) -> second -> m_path_vec.push_back(_path);
        }
        _path_table_file.close();

    }
  }

  else{
    printf("Can't open path table file!\n");
    exit(-1);
  }
   printf("Finish Loading Path Table!\n");
  return _path_table;

}