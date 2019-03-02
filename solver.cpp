#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <assert.h>
#include <vector>

#include <config.h>
#include <util.cpp>

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

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
  char var_truth_table[NUM_VARS] = {U}; // T, F, U (Undef), TF(assigned to T first), FT(assigned to F first)
  int dec_lvl[NUM_VARS] = {-1};
  int dec_var[BUF_DEC_LVL]= {0}; // Variable idx at each decision lvl, we assume at most 100 decision level
  int parent_cls[NUM_VARS] = {0}; 
  int buf_ded[BUF_DED_SIZE] = {0};
  int buf_ded_cls[BUF_DED_SIZE] = {-1}; 

  vector<int> pos_cls[NUM_VARS]; 
  vector<int> neg_cls[NUM_VARS]; 

  int l_ded[BUF_CLS_SIZE];
  int cls_ded[BUF_CLS_SIZE]; 
  bool conflict[BUF_CLS_SIZE];


  //Other global variables
  int state = DECISION; 
  int prev_state = DECISION; 
  int new_var_idx = 1;
  int curr_lvl = -1; 


/*************************** Loading Clauses ***************************/
  for (int x = 0; x < NUM_ORG_CLAUSES; ++x) {
    local_clauses[x][0] = c1[x];
    local_clauses[x][1] = c2[x];
    local_clauses[x][2] = c3[x];

    if (c1[x] > 0){
      pos_cls[c1[x]].push_back(x);
    }else{
      neg_cls[-c1[x]].push_back(x); 
    }
  }

/********************************* FSM **********************************/





























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
