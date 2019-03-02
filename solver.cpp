#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <assert.h>
#include <vector>
#include <algorithm>

#include <config.h>
#include <util.cpp>

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::size_t; 

// Util functions for host
void read_clause_file(string filename, int *c1, int *c2, int *c3,  int *max_size, 
  const int num_var, const int num_clauses); 

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
    //std::string first("./data/uuf250/tests/uuf250-0");
    std::string f_end(".cnf");
    std::string fileName=first+std::to_string(i)+f_end;
    read_clause_file(fileName, c1, c2, c3, max_size, NUM_VARS, NUM_ORG_CLAUSES);
    cout << "Max Number of cls for the same variable is " << max_size[0]<<endl;
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> readtime = ts2 -ts1; 
    wf<<"TestCase: " << fileName <<endl;  
    wf << "Time (Read file) : " << readtime.count() <<endl;

// Solver starts from here

  int local_clauses[NUM_ORG_CLAUSES][3];  
  vector<int> pos_cls_tmp[NUM_VARS]; 
  vector<int> neg_cls_tmp[NUM_VARS]; 
  int ** pos_cls = new int*[NUM_VARS];
  int ** neg_cls = new int*[NUM_VARS];
  int ** learned_clauses = new int*[NUM_LEARN];
  int learned_end = 0; 

  // Variable assignment information
  char var_truth_table[NUM_VARS] = {U}; // T, F, U (Undef), TF(assigned to T first), FT(assigned to F first)
  int dec_lvl[NUM_VARS] = {-1};
  int parent_cls[NUM_VARS] = {0}; 
  int parent_lit[NUM_VARS][2] = {0}; 
  bool dec_ded[NUM_VARS] = {1}; //dec - 1, ded - 0

  int dec_var[BUF_DEC_LVL]= {0}; // Variable idx at each decision lvl, we assume at most 100 decision level

  vector<int> buf_ded_lit;

  int conf_ded, conf_cls;
  int conf_parents_lit[2]

  //Other global variables
  int state = DECISION; 
  int prev_state = DECISION; 
  int new_var_idx = 1;
  int curr_lvl = -1; 

  vector<int> buf_dec_lit;
  vector<int> buf_ded_lit;
  vector<int> buf_dec_lit_sort;


/*************************** Loading Clauses ***************************/
  for (int x = 0; x < NUM_ORG_CLAUSES; ++x) {
    local_clauses[x][0] = c1[x];
    local_clauses[x][1] = c2[x];
    local_clauses[x][2] = c3[x];

    if (c1[x] > 0){
      pos_cls_tmp[c1[x]].push_back(x);
    }else{
      neg_cls_tmp[-c1[x]].push_back(x); 
    }
  }

  for (int x = 0; x < NUM_VARS; x++){
    pos_cls[x] = new int[pos_cls_tmp[x].size()];
    neg_cls[x] = new int[neg_cls_tmp[x].size()];
    pos_cls_tmp.clear();
    neg_cls_tmp.clear(); 
  }

/********************************* FSM **********************************/
  while (state != EXIT){
    switch(state){
      case DECISION: 
        prev_state = DECISION; 
        while (new_var_idx < NUM_VARS){
          if (var_truth_table[new_var_idx] != U){
            //printf("Skip var %d(Value - %d)\n", new_var_idx, var_truth_table[new_var_idx]); 
            new_var_idx ++; 
          }else{
            break; 
          }
        } 
        
        if (new_var_idx == NUM_VARS){
          state = SOLVED; break; 
        }

        bool conf_tmp_T = 0; 
        bool conf_tmp_F = 0; 
        var_truth_table[new_var_idx] = T;
        for (int i = 0; i <= learned_end; i++){
          for (int j = 0; j < sizeof(learned_clauses[i]); j ++){
            int lit_tmp = learned_clauses[i][j]; 
            int lit_var = abs(lit_tmp); 
            conf_tmp_T |= ((lit_tmp > 0) && (var_truth_table[lit_var] == T || var_truth_table[lit_var] == FT))
                        || ((lit_tmp < 0) && (var_truth_table[lit_var] == F || var_truth_table[lit_var] == TF))
                        || (var_truth_table[lit_var] == U);
            if (conf_tmp_T){ break; }
          }
        }

        var_truth_table[new_var_idx] = T;
        for (int i = 0; i <= learned_end; i++){
          for (int j = 0; j < sizeof(learned_clauses[i]); j ++){
            int lit_tmp = learned_clauses[i][j]; 
            int lit_var = abs(lit_tmp); 
            conf_tmp_F |= ((lit_tmp > 0) && (var_truth_table[lit_var] == T || var_truth_table[lit_var] == FT))
                        || ((lit_tmp < 0) && (var_truth_table[lit_var] == F || var_truth_table[lit_var] == TF))
                        || (var_truth_table[lit_var] == U);
            if (conf_tmp_F){ break; }
          }
        }

        if (conf_tmp_T && !conf_tmp_F){
          var_truth_table[new_var_idx] = FT; 
        }else if (!conf_tmp_T && conf_tmp_F){
          var_truth_table[new_var_idx] = TF; 
        }else if (conf_tmp_T && conf_tmp_F){
          var_truth_table = sizeof(pos_cls[new_var_idx]) > sizeof(neg_cls[new_var_idx]) ? T : F; 
        }else{
          state = BACKTRACK_DEC; 
          break; 
        }
           
        state = PROP;
        curr_lvl ++; 
        dec_lvl[new_var_idx] = curr_lvl; 
        dec_ded[new_var_idx] = 1; 
        dec_var[curr_lvl] = new_var_idx;
//      printf("Decide Var(%d) - at lvl %d\n", new_var_idx, curr_lvl);

        break;

      case PROP:
        if (prev_state == DECISION || prev_state == BACKTRACK_DEC){
          prop_var = new_var_idx;
        }else if (prev_state == DEDUCTION){
          prop_var = buf_ded.pop_back();
        }else if (prev_state == BACKTRACK_DED){
          prop_var = abs(learned_lit[2]);
        }

//       printf("Prop ded Var(%d): %d\n", prop_var, var_truth_table[prop_var]);
        int l1, l2, var1, var2;  
        bool unsat1, unsat2, unsat3; 
        int l_ded_tmp; 
        if (var_truth_table[prop_var] == T || var_truth_table[prop_var] == FT){
          for(size_t x=0; x < sizeof(neg_cls[prop_var]); ++x) {
            int cls = neg_cls[prop_var][x];
            l1 = (local_clauses[cls][0] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][0];
            l2 = (local_clauses[cls][2] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][2];
            var1 = var_truth_table[abs(l1)];
            var2 = var_truth_table[abs(l2)];
            unsat1 = (l1 > 0) ? (var1 == F || var1 ==TF) : (var1 == T || var1 ==FT);
            unsat2 = (l2 > 0) ? (var2 == F || var2 ==TF) : (var2 == T || var2 ==FT);
            if (unsat1 && unsat2){
              state = (dec_ded[prop_var]) ? BACKTRACK_DEC : ANALYSIS; 
              conf_var = prop_var;
              conf_cls = cls;
              conf_parents_lit[0] = l1; 
              conf_parents_lit[1] = l2; 
              break;
            }else if (unsat1 && (var2 == U)){
              //Change ded value here
              dec_lvl[abs(l2)] = curr_lvl;  
              parent_cls[abs(l2)] = cls; 
              parent_lit[abs(l2)][0] = -prop_var; //Need to be neg
              parent_lit[abs(l2)][1] = l1;
              var_truth_table[abs(l2)] = l2 > 0 ? T : F;
              buf_ded_lit.push_back(l2);
              //dec_ded[abs(l1)] = 1;
            }else if (unsat2 && (var1 == U)){
              dec_lvl[abs(l1)] = curr_lvl;  
              parent_cls[abs(l1)] = cls; 
              parent_lit[abs(l1)][0] = -prop_var; //Need to be neg
              parent_lit[abs(l1)][1] = l2;
              buf_ded_lit.push_back(l1);
              //dec_ded[abs(l1)] = 0;
            }
          }
        }else{
          for(size_t x=0; x < sizeof(pos_cls[prop_var].size()); ++x) {
            int cls = pos_cls[prop_var][x];
            l1 = (local_clauses[cls][0] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][0];
            l2 = (local_clauses[cls][2] == -prop_var)? local_clauses[cls][1] : local_clauses[cls][2];
            var1 = var_truth_table[abs(l1)];
            var2 = var_truth_table[abs(l2)];
            unsat1 = (l1 > 0) ? (var1 == F || var1 ==TF) : (var1 == T || var1 ==FT);
            unsat2 = (l2 > 0) ? (var2 == F || var2 ==TF) : (var2 == T || var2 ==FT);
            if (unsat1 && unsat2){
              state = (dec_ded[prop_var]) ? BACKTRACK_DEC : ANALYSIS; 
              conf_var = prop_var;
              conf_cls = cls;
              conf_parents_lit[0] = l1; 
              conf_parents_lit[1] = l2; 
              break;
            }else if (unsat1 && (var2 == U)){
              //Change ded value here
              dec_lvl[abs(l2)] = curr_lvl;  
              parent_cls[abs(l2)] = cls; 
              parent_lit[abs(l2)][0] = prop_var; 
              parent_lit[abs(l2)][1] = l1;
              var_truth_table[abs(l2)] = l2 > 0 ? T : F;
              buf_ded_lit.push_back(l2);
              //dec_ded[abs(l1)] = 1;
            }else if (unsat2 && (var1 == U)){
              dec_lvl[abs(l1)] = curr_lvl;  
              parent_cls[abs(l1)] = cls; 
              parent_lit[abs(l1)][0] = prop_var; 
              parent_lit[abs(l1)][1] = l2;
              buf_ded_lit.push_back(l1);
              //dec_ded[abs(l1)] = 0;
            }
          }
        }
        
        if (buf_ded.empty()){
          state = DECISION;
        }else if (state != BACKTRACK_DEC && state != ANALYSIS){
          state = PROP; 
        }
        break; 

      case ANALYSIS:
        prev_state = ANALYSIS; 
        buf_dec_lit.clear(); 
        buf_ded_lit.clear(); 
        buf_dec_lit_sort.clear();

        int par_lit1 = parent_lit[conf_var][0];
        int par_lit2 = parent_lit[conf_var][1];

        if (dec_ded[abs(par_lit1)] == 1){
          buf_dec_lit.push_back(par_lit1);
        }else{
          buf_ded_lit.push_back(par_lit1);
        }

        if (dec_ded[abs(par_lit2)] == 1){
          buf_dec_lit.push_back(par_lit2);
        }else{
          buf_ded_lit.push_back(par_lit2);
        }

        if (dec_ded[abs(conf_parents_lit[0])] == 1){
          buf_dec_lit.push_back(conf_parents_lit[0]);
        }else{
          buf_ded_lit.push_back(conf_parents_lit[0]);
        }

        if (dec_ded[abs(conf_parents_lit[0])] == 1){
          buf_dec_lit.push_back(conf_parents_lit[1]);
        }else{
          buf_ded_lit.push_back(conf_parents_lit[1]);
        }

        while (!buf_ded_lit.empty){
          int curr_ded_lit = buf_ded_lit.pop_back(); 
          par_lit1 = parent_lit[abs(curr_ded_lit)][0];
          par_lit2 = parent_lit[abs(curr_ded_lit)][1];
          if (dec_ded[abs(par_lit1)] == 1){
            buf_dec_lit.push_back(par_lit1);
          }else{
            buf_ded_lit.push_back(par_lit1);
          }

          if (dec_ded[abs(par_lit2)] == 1){
            buf_dec_lit.push_back(par_lit2);
          }else{
            buf_ded_lit.push_back(par_lit2);
          }
        }

         learned_end ++;
        learned_clauses[learned_end] = new int[sizeof(buf_dec_lit)];
        printf("Add learned clause (%d): ", learned_end);
        int i = 0;
        while (!buf_dec_lit.empty()){
          learned_clauses[learned_end][i] = buf_dec_lit.pop_back(); 
          printf("%d, \n", learned_clauses[learned_end][i]);
          i++; 
        } 
        printf("\n");
        std::sort(std::begin(learned_clauses[learned_end]), std::end(learned_clauses[learned_end]), std::greater<int>());
        

        state = BACKTRACK_DEC; 
        break; 

      case BACKTRACK_DEC: 
        //printf("State = BACKTRACK_DEC; ");
        if (prev_state == PROP){
          back_lvl = curr_lvl; 
          while(var_truth_table[dec_var[back_lvl]] == TF || var_truth_table[dec_var[back_lvl]] == FT){
            back_lvl --; 
            if (back_lvl < 0){
              break; 
            }
          }
        }else if (prev_state == ANALYSIS){
          int foundvar = 0; 
          for (int i = 0; i < sizeof(learned_clauses[learned_end]); i++){
            int tmp = abs(learned_clauses[learned_end][i]);
            if(var_truth_table[tmp] == T || var_truth_table[tmp] = F){
              foundvar = tmp;
              break;
            } 
          }
          back_lvl = (foundvar == 0) ? -1: dec_lvl[foundvar];
        }

        if (back_lvl < 0){
          printf("Failed at lvl %d\n", back_lvl);
          state = FAILED; 
          break;
        }

        printf("Back to lvl %d - Var %d\n", back_lvl, dec_var[back_lvl]);

        int prev_assigned_value = var_truth_table[dec_var[back_lvl]]; 
        //Undo all variable assignment after back_lvl
        for (int i = 0; i < NUM_VARS; i ++){
          if (dec_lvl[i] >= back_lvl){
            var_truth_table[i] = U;
            dec_lvl[i] = -1; 
            parent_cls[i] = -1;
            parent_lit[i][0] = 0;
            parent_lit[i][1] = 0;
          }
        }

        for (int i = 0; i < BUF_DEC_LVL; i++){
          if (i > back_lvl){
            dec_var[i] = 0; 
            //dec_lst_lvl[i] = -1; 
          }
        }

        new_var_idx = dec_var[back_lvl];
        var_truth_table[new_var_idx] = (prev_assigned_value == T) ? TF : FT;
        dec_lvl[new_var_idx] = back_lvl;
        curr_lvl = back_lvl;

        state = PROP; 
        break; 

     case SOLVED:
        printf("Solved\n");
        tot_conflict = 0;
        #pragma ACCEL parallel flatten reduction=tot_conflict
        for (int x = 0; x < NUM_ORG_CLAUSES; x++){
          int l1_tmp = local_clauses[x][0];
          int l2_tmp = local_clauses[x][1];
          int l3_tmp = local_clauses[x][2];
          bool unsat1 = l1_tmp >0 ? (var_truth_table[l1_tmp] == F || var_truth_table[l1_tmp] == TF) : (var_truth_table[-l1_tmp] == T || var_truth_table[-l1_tmp] == FT);
          bool unsat2 = l2_tmp >0 ? (var_truth_table[l2_tmp] == F || var_truth_table[l2_tmp] == TF) : (var_truth_table[-l2_tmp] == T || var_truth_table[-l2_tmp] == FT);
          bool unsat3 = l3_tmp >0 ? (var_truth_table[l3_tmp] == F || var_truth_table[l3_tmp] == TF) : (var_truth_table[-l3_tmp] == T || var_truth_table[-l3_tmp] == FT);
          tot_conflict |= (unsat1 && unsat2 && unsat3);
        }

        for (int x = 1; x < NUM_VARS; x++){
          if (var_truth_table[x] == U){
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
    if (result[0] == 0){
        cout<< "Failed"<<endl;
    }else{
        cout << "Succeed" << endl; 
    }
//}//Comment this out for testing

  auto end=std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> total=end-start;
  cout<< "Time(total) : "<<total.count() <<endl;
  free(c1);
  free(c2);
  free(c3);
  return 0;
}
