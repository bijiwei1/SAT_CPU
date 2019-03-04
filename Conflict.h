#include <iostream>
#include <string>
#include <chrono>
#include <vector>

class Conflict{

public:
  Variable conf_var;
  Clause conf_cls;

  void set(Variable new_conf_var, Clause new_conf_cls){
    conf_var = new_conf_var;
    conf_cls = new_conf_cls; 
    }


  Clause find_decvar(Variable vars[NUM_VARS], int id, vector<int> &learnt_clauses){
    vector<int> buf_ded_lit, buf_dec_lit; 
    Clause parent = learnt_clauses.at(conf_var.parent_cls);
    assert(parent_cls >=0) ;
    
    // Add parents
    for (int i = 0; i < parent.len; i++){
      int par_lit = parent.lits[i]; 
      if (vars[abs(par_lit)].dec_ded){
        it = find(buf_dec_lit.begin(), buf_dec_lit.end(),par_lit);
        if (it == buf_dec_lit.end()){
          buf_dec_lit.push_back(par_lit);
        }
      }else{
        it = find(buf_ded_lit.begin(), buf_ded_lit.end(),par_lit);
        if (it == buf_ded_lit.end()){
          buf_ded_lit.push_back(par_lit);
        }
      }
    }

    // Add conflicts
    for (int i = 0; i < conf_cls.len; i++){
      int par_lit = conf_cls.lits[i]; 
      if (vars[abs(par_lit)].dec_ded){
        it = find(buf_dec_lit.begin(), buf_dec_lit.end(),par_lit);
        if (it == buf_dec_lit.end()){
          buf_dec_lit.push_back(par_lit);
        }
      }else{
        it = find(buf_ded_lit.begin(), buf_ded_lit.end(),par_lit);
        if (it == buf_ded_lit.end()){
          buf_ded_lit.push_back(par_lit);
        }
      }
    }

    while (!buf_ded_lit.empty()){
      int curr_ded_lit = buf_ded_lit.back(); 
      buf_ded_lit.pop_back();
      parent = vars[abs(curr_ded_lit)].parent_cls; 
      for (int i = 0; i < parent.len; i++){
        int par_lit = parent.lits[i];
        if (vars[abs(par_lit)].dec_ded){
          it = find(buf_dec_lit.begin(), buf_dec_lit.end(),par_lit);
          if (it == buf_dec_lit.end()){
            buf_dec_lit.push_back(par_lit);
          }
      	}else{
          it = find(buf_ded_lit.begin(), buf_ded_lit.end(),par_lit);
          if (it == buf_ded_lit.end()){
            buf_ded_lit.push_back(par_lit);
          }
        }
      }
    }

    Clause newcls = new Clause(id, buf_dec_lit.size());
    for (int i = 0; i < buf_dec_lit.size; i++){
      newcls.set(i, buf_dec_lit.at(i));
    } 
    return newcls; 
  }//End of func

}//End of class
