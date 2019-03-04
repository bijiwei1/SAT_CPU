#include <iostream>
#include <string>
#include <chrono>
#include <vector>

#include <config.h>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::size_t; 

class Variable { 
public: 
  int id; 
  char value;  // T, F, U (Undef), TF(assigned to T first), FT(assigned to F first)
  int dec_lvl; 
  int parent_cls; 
  vector<int> pos_cls; 
  vector<int> neg_cls;
  vector<int> learnt_cls;
  bool dec_ded; //dec - 1, ded - 0
  int conf_cls; 

  // Default Constructor 
  Variable() 
  { 
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    dec_ded = 1; 
    conf_cls = -1;
  } 

  void reset(){
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1; 
    dec_ded = 1;
    conf_cls = -1; 
  }

  void assignment(int _value, int _dec_lvl, int _parent_cls, int _dec_ded){
    value = _value; 
    dec_lvl = _dec_lvl; 
    parent_cls = _parent_cls;
    dec_ded = _dec_ded;
  }
  

  void add_pos_cls(int _pos_cls){
    pos_cls.add(_pos_cls); 
  }

  void add_neg_cls(int _neg_cls){
    pos_cls.add(_neg_cls); 
  }

  void add_learnt_cls(int _learnt_cls){
    learnt_cls.push_back(_learnt_cls); 
  }

  int set_id(int _id){
    id = _id;
  }

  int get_id(){
    return id; 
  }

  int get_parent_cls(){
    return parent_cls; 
  }

  void print(){
    printf("Var(%d), lvl(%d), val(%d), par_cls(%d)\n", id, lvl, value, parent_cls);
  }

};
