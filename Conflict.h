#include <iostream>
#include <string>
#include <chrono>
#include <vector>

class Conflict{

public:
  Variable *conf_var;
  Clause *conf_cls;

  void set(Variable &new_conf_var, Clause &new_conf_cls){
    conf_var = new_conf_var;
    conf_cls = new_conf_cls; 
    }


  

};//End of class
