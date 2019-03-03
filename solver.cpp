#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <assert.h>
#include <vector>
#include <algorithm>
//#include <bits/stdc++.h> 

#include <config.h>
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

void find_decvar(vector<int> &buf_dec_lit, vector<int> &buf_ded_lit, 
  int parent_lit[NUM_VARS][2], bool dec_ded[NUM_VARS]);


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

  // Default Constructor 
  construct() 
  { 
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    parent_lit = {0, 0};
    dec_ded = 1; 
  } 

  reset(){
    value = U; 
    dec_lvl = -1; 
    parent_cls = -1;
    parent_lit = {0, 0};
    dec_ded = 1;
  }

  assignment(value_new, dec_lvl_new, parent_cls_new, parent_lit1, parent_lit2, dec_ded_new){
    value = value_new; 
    dec_lvl = dec_lvl_new; 
    parent_cls = parent_cls_new;
    parent_lit[0] = parent_lit1;
    parent_lit[1] = parent_lit2;
    dec_ded = dec_ded_new;
  }


}; 


int main() {
  //initialize timer
  auto start = std::chrono::high_resolution_clock::now();
  std::ofstream wf;
  wf.open("time.txt");
  int *c1 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *c2 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *c3 = (int *)malloc(sizeof(int) * NUM_ORG_CLAUSES);
  int *max_size = (int *)malloc(sizeof(int) * 1); 
 
  int *result = (int *)malloc(sizeof(int));

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
  int local_clauses[NUM_ORG_CLAUSES][3];  
  vector<int> pos_cls_vec[NUM_VARS]; 
  vector<int> neg_cls_vec[NUM_VARS]; 
  //int ** pos_cls = new int*[NUM_VARS];
  //int ** neg_cls = new int*[NUM_VARS];
  //int pos_cls_nxtidx[NUM_VARS];
  //int neg_cls_nxtidx[NUM_VARS];
  int ** learned_clauses = new int*[NUM_LEARN_1];
  int learned_cls_len[NUM_LEARN_1]; 
  int learned_cls_freq[NUM_LEARN_1] = {0};
  int learned_end = -1; 
  int dec_var[BUF_DEC_LVL]= {0}; // Variable idx at each decision lvl, we assume at most 100 decision level

  // Variable assignment information
  Variable vars[NUM_VARS]; 

  //Conflict information
  int conf_var, conf_cls;
  int conf_parents_lit[2];
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

/*************************** Intializing  ******************************/


/*************************** Loading Clauses ***************************/
  for (int x = 0; x < NUM_ORG_CLAUSES; ++x) {
    local_clauses[x][0] = c1[x];
    local_clauses[x][1] = c2[x];
    local_clauses[x][2] = c3[x];

    if (c1[x] > 0){
      pos_cls_vec[c1[x]].push_back(x);
    }else{
      neg_cls_vec[-c1[x]].push_back(x); 
    }

    if (c2[x] > 0){
      pos_cls_vec[c2[x]].push_back(x);
    }else{
      neg_cls_vec[-c2[x]].push_back(x); 
    }

    if (c3[x] > 0){
      pos_cls_vec[c3[x]].push_back(x);
    }else{
      neg_cls_vec[-c3[x]].push_back(x); 
    }
  }

  free(c1);
  free(c2);
  free(c3);

  for (int x = 1; x < NUM_VARS; x++){
    vars[x].pos_cls[x] = new int[pos_cls_vec[x].size()];
    vars[x].neg_cls[x] = new int[neg_cls_vec[x].size()];
    vars[x].pos_cls_nxtidx= pos_cls_vec[x].size();
    vars[x].neg_cls_nxtidx = neg_cls_vec[x].size();
    
    idx =0; 
    //for (vector<int>::iterator it =pos_cls_tmp.begin(); it<pos_cls_vec.end(); it++){
    while (idx < vars[x].pos_cls_nxtidx){
      vars.pos_cls[idx] = pos_cls_vec[x].at(idx);
      idx++;
    }  

    idx = 0; 
    while (idx < vars[x].neg_cls_nxtidx){
      vars[x].neg_cls[idx] = neg_cls_vec[x].at(idx);
      idx++;
    }  
    pos_cls_vec[x].clear();
    neg_cls_vec[x].clear(); 
  }


  for (int x = 1; x < NUM_VARS; x++){
    printf("Var (%d) Pos cls(%d): ", x, vars[x].pos_cls_nxtidx);
    for (int y = 0; y < vars[x].pos_cls_nxtidx; y++){
      printf("%d, ", vars[x].pos_cls[y]);
    }
    printf("\n");

    printf("Var (%d) Neg cls(%d): ", x, vars[x].neg_cls_nxtidx);
    for (int y = 0; y < vars[x].neg_cls_nxtidx; y++){
      printf("%d, ", vars[x].neg_cls[y]);
    }
    printf("\n");
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
        int newval = vars[new_var_idx].pos_cls_nxtidx > vars[new_var_idx].neg_cls_nxtidx ? T : F; 
        assignment(newval, curr_lvl, 0, 0, 0, 1); 
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

        //printf("Prop ded Var(%d): %d at lvl %d\n", prop_var, var_truth_table[prop_var], curr_lvl);
        int l1, l2, var1, var2;
        bool unsat1, unsat2;  
        if (vars[prop_var].value == T || vars[prop_var].value == FT){
          for(int x=0; x < vars[prop_var].neg_cls_nxtidx; ++x) {
            int cls = vars[prop_var].neg_cls[x];
            l1 = (local_clauses[cls][0] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][0];
            l2 = (local_clauses[cls][2] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][2];
            var1 = vars[abs(l1)].value;
            var2 = vars[abs(l2)].value;
            unsat1 = (l1 > 0) ? (var1 == F || var1 ==TF) : (var1 == T || var1 ==FT);
            unsat2 = (l2 > 0) ? (var2 == F || var2 ==TF) : (var2 == T || var2 ==FT);
            if (unsat1 && unsat2){
              state = (vars[prop_var].dec_ded || learned_end >= NUM_LEARN) ? BACKTRACK_DEC : ANALYSIS; 
              conf_var = prop_var;
              conf_cls = cls;
              conf_parents_lit[0] = l1; 
              conf_parents_lit[1] = l2; 
              buf_ded_lit.clear();
              //printf("Found conflict - Var (%d) due to conf_cls %d\n", prop_var, conf_cls);
              break;
            }else if (unsat1 && (var2 == U)){
              //Change ded value here
              int newval = l2 > 0 ? T : F;
              vars[abs(l2)].assignment(newval, curr_lvl, cls, -prop_var, l1, 0); 
              buf_ded_lit.push_back(l2);
              //printf("Add ded var(%d) due to cls %d -par1 %d(val %d), par2 %d(val %d)\n", l2, cls,
		        //	 parent_lit[abs(l2)][0], var_truth_table[abs(parent_lit[abs(l2)][0])], 
                //         parent_lit[abs(l2)][1], var_truth_table[abs(parent_lit[abs(l2)][1])]);
            }else if (unsat2 && (var1 == U)){
              int newval = l1 > 0 ? T : F;
              vars[abs(l1)].assignment(newval, curr_lvl, cls, -prop_var, l2, 0); 
              buf_ded_lit.push_back(l1);
              //printf("Add ded var(%d) due to cls %d -par1 %d(val %d), par2 %d(val %d)\n", l1, cls,
		        //	 parent_lit[abs(l1)][0], var_truth_table[abs(parent_lit[abs(l1)][0])], 
                //         parent_lit[abs(l1)][1], var_truth_table[abs(parent_lit[abs(l1)][1])]);
            }
          }
        }else{
          for(int x=0; x < vars[prop_var].pos_cls_nxtidx; ++x) {
            int cls = vars[prop_var].pos_cls[x];
            l1 = (local_clauses[cls][0] == prop_var)? local_clauses[cls][1] : local_clauses[cls][0];
            l2 = (local_clauses[cls][2] == prop_var)? local_clauses[cls][1] : local_clauses[cls][2];
            var1 = vars[abs(l1)].value;
            var2 = vars[abs(l2)].value;
            unsat1 = (l1 > 0) ? (var1 == F || var1 ==TF) : (var1 == T || var1 ==FT);
            unsat2 = (l2 > 0) ? (var2 == F || var2 ==TF) : (var2 == T || var2 ==FT);
            if (unsat1 && unsat2){
              state = (vars[prop_var].dec_ded || learned_end >= NUM_LEARN) ? BACKTRACK_DEC : ANALYSIS; 
              conf_var = prop_var;
              conf_cls = cls;
              conf_parents_lit[0] = l1; 
              conf_parents_lit[1] = l2; 
              buf_ded_lit.clear();
              //printf("Found conflict - Var (%d) due to conf_cls %d\n", prop_var, conf_cls);
              break;
            }else if (unsat1 && (var2 == U)){
              //Change ded value here
              int newval = l2 > 0 ? T : F;
              vars[abs(l2)].assignment(newval, curr_lvl, cls, prop_var, l1, 0); 
              buf_ded_lit.push_back(l2);
              //printf("Add ded var(%d) due to cls %d -par1 %d(val %d), par2 %d(val %d)\n", l2, cls,
		        //	 parent_lit[abs(l2)][0], var_truth_table[abs(parent_lit[abs(l2)][0])], 
                //         parent_lit[abs(l2)][1], var_truth_table[abs(parent_lit[abs(l2)][1])]);
            }else if (unsat2 && (var1 == U)){
              int newval = l1 > 0 ? T : F;
              vars[abs(l1)].assignment(newval, curr_lvl, cls, prop_var, l2, 0); 
              buf_ded_lit.push_back(l1);
              //printf("Add ded var(%d) due to cls %d -par1 %d(val %d), par2 %d(val %d)\n", l1 , cls,
	         	//	 parent_lit[abs(l1)][0], var_truth_table[abs(parent_lit[abs(l1)][0])], 
                //         parent_lit[abs(l1)][1], var_truth_table[abs(parent_lit[abs(l1)][1])]);
            }
          }
        }
        
        if (buf_ded_lit.empty() && state != ANALYSIS && state != BACKTRACK_DEC){
          state = DECISION;
        }

        break; 

      case ANALYSIS:
        prev_state = ANALYSIS; 
        buf_dec_lit.clear(); 
        buf_ded_lit.clear(); 
        buf_dec_lit_sort.clear();
        par_lit1 = vars[conf_var].parent_lit[0];
        par_lit2 = vars[conf_var].parent_lit[1];
/*
        printf("Conflict Var(%d), conf cls(%d), parent cls (%d)\n", conf_var, conf_cls, parent_cls[conf_var]);
        printf("Conf Lit: %d(%d), %d(%d)\n", conf_parents_lit[0], vars[abs(conf_parents_lit[0])].dec_lvl, conf_parents_lit[1], dec_lvl[abs(conf_parents_lit[1])]);
        printf("Parent Lit: %d(%d), %d(%d)\n", par_lit1, vars[abs(par_lit1)].dec_lvl, par_lit2, vars[abs(par_lit2)].dec_lvl);
*/
        if (vars[abs(par_lit1)].dec_ded == 1){
          buf_dec_lit.push_back(par_lit1);
        }else{
          buf_ded_lit.push_back(par_lit1);
        }

        if (vars[abs(par_lit2)].dec_ded == 1){
          buf_dec_lit.push_back(par_lit2);
        }else{
          buf_ded_lit.push_back(par_lit2);
        }

        if (vars[abs(conf_parents_lit[0])].dec_ded == 1){
          buf_dec_lit.push_back(conf_parents_lit[0]);
        }else{
          buf_ded_lit.push_back(conf_parents_lit[0]);
        }

        if (vars[abs(conf_parents_lit[1])].dec_ded == 1){
          buf_dec_lit.push_back(conf_parents_lit[1]);
        }else{
          buf_ded_lit.push_back(conf_parents_lit[1]);
        }
/*
	printf("Inital : buf_dec_lit : ");
	for (int i = 0; i < buf_dec_lit.size(); i++){
          printf("%d, ", buf_dec_lit.at(i));
        }
        printf("\n");

	printf("Initial buf_ded_lit : ");
	for (int i = 0; i < buf_ded_lit.size(); i++){
          printf("%d, ", buf_ded_lit.at(i));
        }
        printf("\n");
*/
        find_decvar(buf_dec_lit, buf_ded_lit, vars);

	//For debug
	/*
	printf("Final buf_dec_lit : ");
	for (int i = 0; i < buf_dec_lit.size(); i++){
          printf("%d, ", buf_dec_lit.at(i));
        }
        printf("\n");
*/
        learned_end ++;
        learned_clauses[learned_end] = new int[buf_dec_lit.size()];
        new_cls = new int[buf_dec_lit.size()];
        learned_cls_len[learned_end] = buf_dec_lit.size();

        idx = 0;
        while (!buf_dec_lit.empty()){
          new_cls[idx] = buf_dec_lit.back(); 
          buf_dec_lit.pop_back();
          idx++; 
        } 
        for (int i = 0; i < learned_cls_len[learned_end]; i++){
          int max_lit = 0;
          int max_id = 0;
          for (int j= 0; j < learned_cls_len[learned_end]; j++){
            if (abs(max_lit) < abs(new_cls[j])){
              max_lit = new_cls[j];
              max_id = j;
            }
          }
          learned_clauses[learned_end][i] = max_lit;
          new_cls[max_id] = 0;
        }	

        
/*
        printf("Add learned clause (%d): ", learned_end);
	for (int i = 0; i < learned_cls_len[learned_end]; i++){
	  printf("%d, ", learned_clauses[learned_end][i]);
	}
	printf("\n"); 
*/
        state = BACKTRACK_DEC; 
        break; 

      case BACKTRACK_DEC: 
        //printf("State = BACKTRACK_DEC; ");
        if (prev_state == DEDUCTION){
          //printf("DED -> BACK: \n");
          back_lvl = curr_lvl; 
          while(vars[dec_var[back_lvl]].value == TF || vars[dec_var[back_lvl]].value == FT){
            back_lvl --; 
            if (back_lvl < 0){
              break; 
            }
          }
        }else if (prev_state == ANALYSIS){
          //printf("ANA -> BACK: \n");
          int foundvar = 0; 
          for (int i = 0; i < learned_cls_len[learned_end]; i++){
            int tmp = abs(learned_clauses[learned_end][i]);
            if(vars[tmp].value == T || vars[tmp].value == F){
              foundvar = tmp;
              break;
            } 
          }
          back_lvl = (foundvar == 0) ? -1: vars[foundvar].dec_lvl;
        }else if (prev_state == PROP){
          //printf("PROP -> BACK: \n");
          back_lvl = vars[conf_back_var1].dec_lvl;
        }

        if (back_lvl < 0){
          printf("Failed at lvl %d\n", back_lvl);
          state = FAILED; 
          break;
        }

if (back_lvl < 22){
        printf("Back to lvl %d - Var %d\n", back_lvl, dec_var[back_lvl]);
}

        prev_assigned_value = cars[dec_var[back_lvl]].value; 
        //Undo all variable assignment after back_lvl
        for (int i = 0; i < NUM_VARS; i ++){
          if (vars[i].dec_lvl >= back_lvl){
            vars[i].reset(); 
          }
        }

       for (int i = 0; i < BUF_DEC_LVL; i++){
          if (i > back_lvl){
            dec_var[i] = 0; 
            //dec_lst_lvl[i] = -1; 
          }
        }

        new_var_idx = dec_var[back_lvl];
        vars[new_var_idx].value = (prev_assigned_value == T) ? TF : FT;
//        printf("Reassign Var(%d) - %d\n", new_var_idx, var_truth_table[new_var_idx]);
        vars[new_var_idx].dec_lvl = back_lvl;
        vars[new_var_idx].dec_ded = 1;
        curr_lvl = back_lvl;

        state = PROP; 
        prev_state = BACKTRACK_DEC;
        break; 

      case PROP: 
        sat_tmp = 1;
        for (int i = 0; i <= learned_end; i++){
          sat_tmp = 0;
          for (int j = 0; j < learned_cls_len[i]; j ++){ 
            int lit_tmp = learned_clauses[i][j]; 
            int val_tmp = vars[abs(lit_tmp)].value; 
            sat_tmp |= ((lit_tmp > 0) && (val_tmp == T || val_tmp == FT)) || ((lit_tmp < 0) && (val_tmp == F || val_tmp == TF)) || (val_tmp == U);
            //printf("Var(%d) - val %d, ", lit_tmp, val_tmp);
          }
          if (!sat_tmp){ 
            conf_learn_cls1 = i; 
            break; 
          }
        }

        if (!sat_tmp){
          if (prev_state == DECISION){
            state = PROP; 
            vars[new_var_idx].value = (vars[new_var_idx].value == T) ? TF : FT;
            conf_back_var1 = new_var_idx;
          }else{
            buf_dec_lit.clear(); 
            buf_ded_lit.clear(); 
            for (int i = 0; i < learned_cls_len[conf_learn_cls1]; i++){
              par_lit1 = learned_clauses[conf_learn_cls1][i];
              if (vars[abs(par_lit1)].dec_ded){
                buf_dec_lit.push_back(par_lit1);
              }else{
                buf_ded_lit.push_back(par_lit1);
              }
            }

            find_decvar(buf_dec_lit, buf_ded_lit, vars);
	    //For debug
	    /*
	    printf("Final buf_dec_lit : ");
	    for (int i = 0; i < buf_dec_lit.size(); i++){
              printf("%d(val %d), ", buf_dec_lit.at(i), var_truth_table[abs(buf_dec_lit.at(i))]);
            }
            printf("\n");
*/
            conf_back_var1 = 0;
            for (int i = 0; i < buf_dec_lit.size(); i++){
              int var_tmp = abs(buf_dec_lit.at(i));
              conf_back_var1 = (vars[var_tmp].dec_lvl > vars[conf_back_var1].value) && (vars[var_tmp].value == T ||  vars[var_tmp].value == F) ? var_tmp : conf_back_var1;
            }
            if (conf_back_var1 == 0){
              state = FAILED;
              printf("Prop conflict due to learned cls1 %d(backvar %d (val %d))\n", conf_learn_cls1, conf_back_var1, vars[conf_back_var1].value);
            }else{
              state = BACKTRACK_DEC;
            }

            learned_cls_freq[conf_learn_cls1] ++;
            if (learned_end >= 3000){ 
              learned_end = vacate_learned(learned_clauses, learned_cls_len, learned_cls_freq, learned_end, 10); 
              printf("Vacate learned table, new learned size %d\n", learned_end);
/*
              for (int i = 0; i <= learned_end; i++){
                printf("Cls %d \n", i);
                for (int j = 0; j < learned_cls_len[i]; j++){
                  printf("%d, ", learned_clauses[i][j]);
                }
                printf("\n");
              }*/
            }
          }//End of if-else
          //printf("Prop conflict due to learned cls1 %d(backvar %d (val %d))\n", conf_learn_cls1, conf_back_var1, var_truth_table[conf_back_var1]);
        }//End of sat

        prev_state = PROP; 
        state = sat_tmp ? DEDUCTION : state;
        break;

      case SOLVED:
        printf("Solved\n");
        tot_conflict = 0;
        for (int x = 0; x < NUM_ORG_CLAUSES; x++){
          int l1_tmp = local_clauses[x][0];
          int l2_tmp = local_clauses[x][1];
          int l3_tmp = local_clauses[x][2];
          bool unsat1 = l1_tmp >0 ? (vars[l1_tmp].value == F || vars[l1_tmp].value == TF) : (vars[-l1_tmp].value == T || vars[-l1_tmp].value == FT);
          bool unsat2 = l2_tmp >0 ? (vars[l2_tmp].value == F || vars[l2_tmp].value == TF) : (vars[-l2_tmp].value == T || vars[-l2_tmp].value == FT);
          bool unsat3 = l3_tmp >0 ? (vars[l3_tmp].value == F || vars[l3_tmp].value == TF) : (vars[-l3_tmp].value == T || vars[-l3_tmp].value == FT);
          tot_conflict |= (unsat1 && unsat2 && unsat3);
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
  free(c1);
  free(c2);
  free(c3);
  return 0;
}
