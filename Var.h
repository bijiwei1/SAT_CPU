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
  int parent_cls; 
  int parent_lit[2]; 
  bool dec_ded; //dec - 1, ded - 0
  int* pos_cls;
  int* neg_cls;  
  int pos_cls_nxtidx; 
  int neg_cls_nxtidx; 
  int conf_cls; 

  // Default Constructor 
  Variable() 
  { 
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    parent_lit[0] = 0;
    parent_lit[1] = 0;
    dec_ded = 1; 
    conf_cls = -1;
  } 

  void reset(){
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    parent_lit[0] = 0;
    parent_lit[1] = 0;
    dec_ded = 1;
    conf_cls = -1; 
  }

  void assignment(int newval, int dec_lvl_new, int parent_cls_new, int parent_lit1, int parent_lit2, int dec_ded_new){
    value = newval; 
    dec_lvl = dec_lvl_new; 
    parent_cls = parent_cls_new;
    parent_lit[0] = parent_lit1;
    parent_lit[1] = parent_lit2;
    dec_ded = dec_ded_new;
  }
  
  void print(){
    printf("value %d, par_cls(%d), par1(%d), par2(%d) \n", value, parent_cls, parent_lit[0], parent_lit[1]);
  }

};
