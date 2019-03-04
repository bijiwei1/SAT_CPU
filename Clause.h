#include <iostream>
#include <string>
#include <chrono>
#include <vector>

class Clause{

public:
	int id; 
	int* lits;
	int len; 
	int freq;
	bool islearnt; 
	
	Clause(int new_id, int new_len){
		id = new_id; 
		len = new_len;
		lits = new int[len]; 
		freq = 0; 
		islearnt = 1; 
	}

	Clause(int new_id, int c1, int c2, int c3){
		id = new_id; 
		lits = new int[3];
		lits[0] = c1;
		lits[1] = c2; 
		lits[2] = c3;
		len = 3; 
		islearnt = 0;
		freq = 0; 
	}

	void set_value(int idx, int lit){
		lits[idx] = lit; 
	}

	void print(){
		printf("Clause is : ");
		for (int i = 0; i < len; i++){
			printf("%d, \n", lits[i]);
		}
		printf("\n");
	}	

	void delete_lit(){
		delete[] lits; 
	}

	// return: ded_lit > 0 or ded_lit < -1 : deduction variable ; 
	//         ded_lit = 0 : sat/ more than 2 unassigned variable ;
	//         ded_lit = -1 : unsat (because we can never deduct var 1)
	int deduct(Variable vars[NUM_VARS]) {
		bool sat = 0; 
		int ded_lit = 0;
  	int num_ded = 0;  
  	for (int i = 0; i < len; i++){
    	int curr_lit = lits[i]; 
    	int curr_var = vars[abs(curr_lit)].value;
    	sat = ((curr_lit > 0) && (curr_var == T || curr_var == FT)) || ((curr_lit < 0) && (curr_var == F || curr_var == TF));
    	if (sat){break;}
    	if (curr_var == U){
    		if (num_ded == 1){ break;} //Found 2 unassigned value
      	ded_lit = curr_lit;
      	num_ded = 1;
    	}
  	}

  	ded_lit = (~sat) ? -1 : (num_ded > 1) ? 0 : ded_lit;
  	return ded_lit;
	}

	string toString(){
		string str; 
		for (int i = 0 ; i < len; i ++){
			str += to_string(lits[i]) + ", "; 
		}
		return str;
	}
	
};
