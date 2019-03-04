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
  char value;  // T, F, U (Undef), TF(assigned to T first), FT(assigned to F first)
  int dec_lvl; 
  Clause parent_cls; 
  vector<Clause> pos_cls; 
  vector<Clause> neg_cls; 
  //int pos_cls_len; 
  //int neg_cls_len; 

  bool dec_ded; //dec - 1, ded - 0
  int conf_cls; 

  vector<Clause> learnt_cls;

  // Default Constructor 
  Variable() 
  { 
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    //parent_lit[0] = 0;
    //parent_lit[1] = 0;
    dec_ded = 1; 
    conf_cls = -1;
  } 

  void reset(){
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    //parent_lit[0] = 0;
    //parent_lit[1] = 0;
    dec_ded = 1;
    conf_cls = -1; 
  }

  void assignment(int new_value, int dec_lvl_new, Clause parent_cls_new, int dec_ded_new){
    value = new_value; 
    dec_lvl = dec_lvl_new; 
    parent_cls = parent_cls_new;
    dec_ded = dec_ded_new;
  }
  
  void print(){
    printf("value %d, par_cls(%d), par1(%d), par2(%d) \n", value, parent_cls, parent_lit[0], parent_lit[1]);
  }

  void addLearntCls(Clause new_learnt_cls){
    learnt_cls.push_back(new_learnt_cls); 
  }



};
