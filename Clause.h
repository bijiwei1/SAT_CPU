#include <iostream>
#include <string>
#include <chrono>
#include <vector>

class Clause{

public:
	int id; 
	Vector<Variable*> lits; //This variable without sign
	Vector<bool> sign; 
	int freq;
	bool islearnt; 
	
	Clause(int _id){
		id = _id;  
		freq = 0; 
		islearnt = 1; 
	}

	Clause(int _id, Variable* c1, Variable* c2, Variable* c3, bool sign1, bool sign2, bool sign3){
		id = _id; 
		lits.push_back(c1);
		lits.push_back(c2);
		lits.push_back(c3);
		sign.push_back(sign1);
		sign.push_back(sign2);
		sign.push_back(sign3);
		len = 3; 
		islearnt = 0;
		freq = 0; 
	}

	void addLit(Variable* _lits, bool _sign){
		lits.push_back(_lits);
		sign.push_back(_sign);
	}

/*
	void print(){
		printf("Clause is : ");
		for (int i = 0; i < len; i++){
			printf("%d, \n", lits[i]);
		}
		printf("\n");
	}	*/

	// return: ded_lit > 0 or ded_lit < -1 : deduction variable ; 
	//         ded_lit = 0 : sat/ more than 2 unassigned variable ;
	//         ded_lit = -1 : unsat (because we can never deduct var 1)

	/*
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
*/
	string toString(){
		string str; 
		for (int i = 0 ; i < lits.size(); i ++){
			Variable* curr_var = lits.at(i); 
			if (!sign[i]){
				str += "-";
			}
			str += to_string(curr_var-> get_id()) + ", "; 
		}
		return str;
	}
	

	int get_id(){
		return id;
	}

	int get_freq(){
		return freq;
	}

	bool get_islearnt(){
		return islearnt; 
	}

};
