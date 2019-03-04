#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <assert.h>
#include <vector>
#include <algorithm>
//#include <bits/stdc++.h> 

#include <config.h>
#include <Var.h>
#include <Clause.h>
#include <Conflict.h>
#include <util.cpp>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::size_t; 

// Util functions for host
void read_clause_file(string filename, int *c1, int *c2, int *c3,  int *max_size, 
  const int num_var, const int num_clauses); 

int vacate_learned(int** learned_clauses, int learned_cls_len[NUM_LEARN_1], 
    int learned_cls_freq[NUM_LEARN_1], int learned_end, int freq);

void find_decvar(vector<int> &buf_dec_lit, vector<int> &buf_ded_lit, Variable vars[NUM_VARS]);

int deduct(int* clause, int cls_size, Variable vars[NUM_VARS]);

int main() {
  //initialize timer
  auto start = std::chrono::high_resolution_clock::now();
  std::ofstream wf;
  wf.open("time.txt");
  int *c1 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *c2 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *c3 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *max_size = (int *)malloc(sizeof(int) * 1); 

  // Prepare data
  //std::string test_file="test"+to_string(test_idx);
  //for (int i = 1; i <= NUM_TEST; ++i) { 
  //for (int i = 1; i <= 5; ++i) { 
    int i = 5;
    auto ts1=std::chrono::high_resolution_clock::now(); 
    std::string first("./data/uf250/tests/uf250-0");
//    std::string first("./data/uuf250/tests/uuf250-0");
    std::string f_end(".cnf");
    std::string fileName=first+std::to_string(i)+f_end;
    read_clause_file(fileName, c1, c2, c3, max_size, NUM_VARS, NUM_ORG_CLAUSES);
    cout << "Max Number of cls for the same variable is " << max_size[0]<<endl;
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> readtime = ts2 -ts1; 
    wf<<"TestCase: " << fileName <<endl;  
    wf << "Time (Read file) : " << readtime.count() <<endl;

// Solver starts from here
/*************************** Variable Declaration ***************************/

  int dec_var[BUF_DEC_LVL]= {0}; // Variable idx at each decision lvl, we assume at most 100 decision level

  // Clauses
  //Clause local_clauses[NUM_ORG_CLAUSES]; 
  vector<Clause> all_clauses; 

  // Variable assignment information
  Variable vars[NUM_VARS]; 

  //Conflict information
  Conflict curr_conflict = new Conflict(); 

  int conf_learn_cls1;
  int conf_back_var1;

  //Other global variables
  int state = DECISION; 
  int prev_state = DECISION; 
  int new_var_idx = 1;
  int prop_var;
  int curr_lvl = -1; 
  int back_lvl;

  vector<int> buf_dec_lit;
  vector<int> buf_ded_lit;
  vector<int> buf_dec_lit_sort;

  //Temporay variables
  int par_lit1, par_lit2; //ANA
  int prev_assigned_value; //BACK_DED
  int idx; 
  bool tot_conflict; 
  int* new_cls; //ANA
  bool sat_tmp;
  int newval;

/*************************** Intializing  ******************************/


/*************************** Loading Clauses ***************************/
  for (int x = 0; x < NUM_ORG_CLAUSES; ++x) {
    all_clauses.push_back(new Clause(x, c1[x], c2[x], c3[x]));

    if (c1[x] > 0){
      vars[c1[x]].pos_cls.push_back(x);
    }else{
      vars[-c1[x]].neg_cls.push_back(x);
    }

    if (c2[x] > 0){
      vars[c2[x]].pos_cls.push_back(x);
    }else{
      vars[-c2[x]].neg_cls.push_back(x);
    }
    if (c2[x] > 0){
      vars[c2[x]].pos_cls.push_back(x);
    }else{
      vars[-c2[x]].neg_cls.push_back(x);
    }
  }

  free(c1);
  free(c2);
  free(c3);

  for (int x = 1; x < NUM_VARS; x++){
    printf("Var (%d) Pos cls : ", x);
    for (int y = 0; y < vars[x].pos_cls.size(); y++){
      printf("%d, ", vars[x].pos_cls.at(y));
    }
    printf("\n");

    printf("Var (%d) Neg cls : ", x);
    for (int y = 0; y < vars[x].neg_cls.size(); y++){
      printf("%d, ", vars[x].neg_cls.at(y));
    }
    printf("\n");
  }

  for (int x; x < NUM_ORG_CLAUSES; x++){
    all_clauses.at(x).to_string(); 
  }

/********************************* FSM **********************************/
  while (state != EXIT){
    switch(state){
      case DECISION: 
        prev_state = DECISION; 
        while (new_var_idx < NUM_VARS){
          if (vars[new_var_idx].value != U){
            //printf("Skip var %d(Value - %d)\n", new_var_idx, var_truth_table[new_var_idx]); 
            new_var_idx ++; 
          }else{
            break; 
          }
        } 
        
        if (new_var_idx == NUM_VARS){
          state = SOLVED; break; 
        }

        curr_lvl ++; 
        newval = vars[new_var_idx].pos_cls_nxtidx > vars[new_var_idx].neg_cls_nxtidx ? T : F; 
        vars[new_var_idx].assignment(newval, curr_lvl, 0, 0, 0, 1); 
        dec_var[curr_lvl] = new_var_idx;
//        printf("Decide Var(%d) - at lvl %d\n", new_var_idx, curr_lvl);
        state = PROP;
        break;

      case DEDUCTION:
        if (prev_state == PROP){
          prop_var = new_var_idx;
        }else if (prev_state == DEDUCTION){
          prop_var = abs(buf_ded_lit.back());
          buf_ded_lit.pop_back();
        }else if (prev_state == BACKTRACK_DED){
          printf("Error1\n"); 
        }
        prev_state = DEDUCTION;

        printf("Prop ded Var(%d): %d at lvl %d\n", prop_var, vars[prop_var].value, curr_lvl); 

        if (vars[prop_var].value == T || vars[prop_var].value == F){
          for(int x=0; x < vars[prop_var].neg_cls.size(); ++x) {
            int cls_idx = vars[prop_var].neg_cls.at(i);
            int ded_lit = all_clauses.at(cls_idx).deduct(vars); 
            if (ded_lit == -1){
              state = (vars[prop_var].dec_ded) ? BACKTRACK_DEC : ANALYSIS; 
              curr_conflict.set(vars[prop_var], vars.neg_cls.at(i)); 
              buf_ded_lit.clear();
              break;
            }else if (ded_lit != 0){
              int newval = ded_lit > 0 ? T : F;
              vars[abs(ded_lit)].assignment(newval, curr_lvl, vars.neg_cls.at(i), 0); 
              buf_ded_lit.push_back(abs(ded_lit));
            }
          }
        }else {
          for(int x=0; x < vars[prop_var].pos_cls.size(); ++x) {
            int cls_idx = vars[prop_var].pos_cls.at(i);
            int ded_lit = all_clauses.at(cls_idx).deduct(vars); 
            if (ded_lit == -1){
              state = (vars[prop_var].dec_ded) ? BACKTRACK_DEC : ANALYSIS; 
              curr_conflict.set(vars[prop_var], vars.pos_cls.at(i)); 
              buf_ded_lit.clear();
              break; 
            }else if (ded_lit != 0){
              int newval = ded_lit > 0 ? T : F;
              vars[abs(ded_lit)].assignment(newval, curr_lvl, vars.pos_cls.at(i), 0); 
              buf_ded_lit.push_back(abs(ded_lit));
            }
          }
        }
        

        for (int x = 0; x < vars[prop_var].learnt_clauses.size(); x++){
          int cls_idx = vars[prop_var].pos_cls.at(i);
          int ded_lit = all_clauses.at(cls_idx).deduct(vars); 
          if (ded_lit == -1){
            state = (vars[prop_var].dec_ded) ? BACKTRACK_DEC : ANALYSIS; 
            curr_conflict.set(vars[prop_var], vars.learnt_clauses.at(i));
            buf_ded_lit.clear();
            break; 
          }else if (ded_lit != 0){
            int newval = ded_lit > 0 ? T : F;
            vars[abs(ded_lit)].assignment(newval, curr_lvl, vars.learnt_clauses.at(i), 0); 
            buf_ded_lit.push_back(abs(ded_lit));
          }
        }
        
        if (buf_ded_lit.empty() && state != ANALYSIS && state != BACKTRACK_DEC){
          state = DECISION;
        }
        break; 

      case ANALYSIS:
        prev_state = ANALYSIS; 
        buf_dec_lit.clear(); 
        int* newcls; 
        curr_conflict.find_decvar(&buf_dec_lit, vars, newcls); 
        int new_id = learnt_clauses.size();
        learnt_clauses.push_back(curr_conflict.find_decvar(vars, new_id)); 
        learnt_clauses.back().print(); //Check it is added to vector
        state = BACKTRACK_DEC; 
        break; 

      case BACKTRACK_DEC: 

        state = FAILED; 
        break ;

      case SOLVED:
        printf("Solved\n");
        tot_conflict = 0;
        for (int x = 0; x < NUM_ORG_CLAUSES; x++){
          
        }

        for (int x = 1; x < NUM_VARS; x++){
          if (vars[x].value == U){
            tot_conflict = 1;
            printf("Not assign value to var(%d)\n", x);
            //break;
           }
         }

        if (tot_conflict){
          printf("Error!! Solution is not correct\n");
        }else{
          printf("Solution is correct\n");
        }

        state = EXIT;
        break; 

      case FAILED:
        printf("Failed to solve the problem. \n");
        state = EXIT; 
        break;
    }//end of sw
  }//end of while


// End here
    auto ts3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> readtime2 = ts3 -ts2; 
    wf << "Time(Kernel) : " << readtime2.count() << endl; 
//}//Comment this out for testing

  auto end=std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> total=end-start;
  cout<< "Time(total) : "<<total.count() <<endl;
  return 0;
}
