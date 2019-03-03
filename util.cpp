#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>

#include <bits/stdc++.h> 
using namespace std; 

using std::vector;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::string;
using std::sort;
using std::istringstream;


void read_clause_file(string filename, int *c1, int *c2, int *c3, int *max_size, 
  const int num_var, const int num_clauses){

  ifstream f;
  int l1, l2, l3; 
  int max_cls_size[num_var]; 
  int *c1_local = (int *)malloc(sizeof(int) * num_clauses);
  int *c2_local = (int *)malloc(sizeof(int) * num_clauses);
  int *c3_local = (int *)malloc(sizeof(int) * num_clauses); 

  for (int i = 0; i < num_var; i ++)
    max_cls_size[i] = 0; 

 
  f.open(filename.c_str(), ios::in);
  if (!f.is_open()) {
    cout << "Open " << filename << " failed" << endl;
    exit(1);
  }

  int cnt = 0;
  cout << "Start to read file" <<endl; 
  string line;
  while (std::getline(f, line)) {
    if (line == "")
      continue;
    if (cnt == num_clauses)
      break;
    
    if (line.at(0) != 'p' and line.at(0) != 'c') {
      vector<string> substrs;
      istringstream iss(line);
      for(string s; iss >> s;)
        substrs.push_back(s);

      if (substrs.size() < 2)
        continue;

      l1 = stoi(substrs.at(0));
      l2 = stoi(substrs.at(1));
      l3 = stoi(substrs.at(2));
      c1_local[cnt] = l1;
      c2_local[cnt] = l2;
      c3_local[cnt] = l3;

      c1[cnt] = l1;
      c2[cnt] = l2;
      c3[cnt] = l3;

      max_cls_size[abs(l1)] ++; 
      max_cls_size[abs(l2)] ++; 
      max_cls_size[abs(l3)] ++; 
      //cout << "Clause :"<< c1_local[cnt] << " " << c2_local[cnt]<< " " <<c3_local[cnt] << "\n"; 
      cnt ++; 
    }
  }

  int max = 0;
  for (int x = 1; x < num_var; x++){
    max = (max_cls_size[x] > max) ? max_cls_size[x] : max; 
  }
  max_size[0] = max; 
  cout << "Max size " << max << endl; 
  cout << "Number of clauses : " << cnt << endl << "Finish reading file" << endl;

  f.close();
  return ;
}

int vacate_learned(int** learned_clauses, int learned_cls_len[NUM_LEARN_1], 
    int learned_cls_freq[NUM_LEARN_1], int learned_end, int freq){

  int new_learned_end = -1;
  delete[] learned_clauses[0];

  for (int i = 1; i <= learned_end; i++){
    if (learned_cls_freq[i] > freq){
      new_learned_end ++; 
      learned_clauses[new_learned_end] = new int[learned_cls_len[i]];
      for (int j = 0; j < learned_cls_len[i]; j++){
        learned_clauses[new_learned_end][j] = learned_clauses[i][j];
      } 
      learned_cls_len[new_learned_end] = learned_cls_len[i]; 
    }
    delete[] learned_clauses[i];
    learned_cls_freq[i] = 0; 
  }

  return new_learned_end; 
}


void find_decvar(vector<int> &buf_dec_lit, vector<int> &buf_ded_lit, 
  int parent_lit[NUM_VARS][2], bool dec_ded[NUM_VARS]){

  int par_lit1, par_lit2; 
   while (!buf_ded_lit.empty()){
      int curr_ded_lit = buf_ded_lit.back(); 
      buf_ded_lit.pop_back();
      par_lit1 = parent_lit[abs(curr_ded_lit)][0];
      par_lit2 = parent_lit[abs(curr_ded_lit)][1];
      //printf("Var(%d)- par1 %d , par2 %d\n", curr_ded_lit, par_lit1, par_lit2);

      vector<int>::iterator it; 
      if (dec_ded[abs(par_lit1)]){
        it = find(buf_dec_lit.begin(), buf_dec_lit.end(),par_lit1);
        if (it == buf_dec_lit.end()){
          buf_dec_lit.push_back(par_lit1);
        }
      }else{
        it = find(buf_ded_lit.begin(), buf_ded_lit.end(),par_lit1);
        if (it == buf_ded_lit.end()){
          buf_ded_lit.push_back(par_lit1);
        }
      }

      if (dec_ded[abs(par_lit2)]){
        it = find(buf_dec_lit.begin(), buf_dec_lit.end(),par_lit2);
        if (it == buf_dec_lit.end()){
          buf_dec_lit.push_back(par_lit2);
        }
      }else{
        it = find(buf_ded_lit.begin(), buf_ded_lit.end(),par_lit2);
        if (it == buf_ded_lit.end()){
          buf_ded_lit.push_back(par_lit2);
        }
      }
    }

}