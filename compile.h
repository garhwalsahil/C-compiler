#include <bits/stdc++.h>

#define DEBUG if(0)

using namespace std;

extern int yylineno;

enum D_type{ //dt_none to datatype_none
	datatype_none,
	datatype_int,
	datatype_char,
	datatype_string,
	datatype_float,
	datatype_bool,
	datatype_func,
	datatype_err
};

// Nodes of the AST
class Node {
private:
	string type;			// lexeme class
	string value;			// lexeme
	D_type d_type;		// datatype of the node(if required)

public:
	int line_no;		// line number where the node is occuring

	// Children of the Nodes
	Node *ch_1; 
	Node *ch_2;
	Node *ch_3;
	Node *ch_4;

	Node (string t, string v, Node *c1, Node *c2, Node *c3) {
		type = t;
		value = v;
		d_type = datatype_none;
		ch_3 = c3;
		ch_2 = c2;
		ch_1 = c1;
		ch_4 = NULL;
		line_no = yylineno;
	}

	void adding_ch4(Node *c4){
		ch_4 = c4;
	}

	string getValue(){
		return value;
	}

	string getType(){
		return type;
	}

	D_type getD_type(){
		return d_type;
	}

	void setD_type(D_type dt){
		d_type = dt;
	}
	// ~Node();
};


// Parameter of a function
class Parameter
{
private:
	string name;		// parameter name
	D_type d_type;	// parameter data type
public:
	Parameter(){}

	Parameter(string id, D_type dt)
	:name(id), d_type(dt)
	{}

	D_type getD_type(){
		return d_type;
	}

	string getValue(){
		return name;
	}

	// ~Parameter();
};


// Class for the Meta data of the symbol table
class Symbol_table_meta
{
private:
	D_type d_type;		// datatype of the symbol

	// if symbol is a function, then following are also required - return data type, parameter list, number of parameters
	// i.e. d_type = datatype_func
	D_type return_type;
	vector <Parameter> p_list; //parameter list
	int p_count;

public:

	Symbol_table_meta(){

	}

	Symbol_table_meta(D_type dt)
	:d_type(dt) {

	}

	Symbol_table_meta(D_type dt, D_type rtd, vector <Parameter> params)
	:d_type(dt), return_type(rtd), p_list(params), p_count(params.size()) {

	}

	D_type getD_type(){
		return d_type;
	}

	D_type getReturnD_type(){
		return return_type;
	}

	vector<Parameter> getParameterList(){
		return p_list;
	}

	int getParameterCount(){
		return p_count;
	}

	// ~Symbol_table_meta();
};


class Symbol_table
{
private:
	int scope;	// current maximum scope
				// 0 => Global scope

	vector < map < string, Symbol_table_meta > > symbols, backup_symbols;	// vector of maps at different scopes. vector[i] => map of symbols at scope i

	string Type_to_string[8] = {"none", "int", "char", "string", "float", "bool", "func", "err"};


public:
	Symbol_table(){
		scope = 0;	// global
		symbols.push_back(map<string, Symbol_table_meta>());	// empty symbols table at global scope
	}

	bool findInCurrentScope(string id){
		if(symbols[scope].find(id) !=  symbols[scope].end()){
			return true;
		} else {
			return false;
		}
	}

	bool find(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				return true;
			}
		}
		return false;
	}

	void addVariableInCurrentScope(string id, D_type dt){
		symbols[scope][id] = Symbol_table_meta(dt);
	}

	Symbol_table_meta* addFunction(string id, D_type rdt, vector<Parameter> params){
		symbols[0][id] = Symbol_table_meta(datatype_func, rdt, params);
		return &symbols[0][id];
	}

	void addScope(){
		scope++;
		map<string, Symbol_table_meta> newMap;
		newMap.clear();
		symbols.push_back(newMap);
	}

	void removeScope(){
		if(scope == 0)	return ;
		scope--;
		symbols.pop_back();
	}

	D_type getD_type(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				return (symbols[i].find(id))->second.getD_type();
			}
		}
		return datatype_none;
	}

	D_type getFunctionD_type(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				return (symbols[i].find(id))->second.getReturnD_type();
			}
		}
		return datatype_none;
	}

	bool checkFunctionArgs(string id, vector<D_type> args_list) {
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				Symbol_table_meta temp = symbols[i].find(id)->second;
				if(temp.getD_type() != datatype_func){continue;}
				if(temp.getParameterCount() != args_list.size()){continue;}
				bool flag = true;
				int x = 0;
				for(vector <Parameter>::iterator i = temp.getParameterList().begin(); i != temp.getParameterList().end() && x < args_list.size(); i++, x++){
					if (i->getD_type() != args_list[x]){
						flag = false;
						break;
					}
				}
				if(flag){
					return true;
				}
			}
		}
		return false;
	}

	vector<string> getFunctionParameters(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				vector<Parameter> param_list =  symbols[i].find(id)->second.getParameterList();
				vector<string> res;
				for (std::vector<Parameter>::iterator x = param_list.begin(); x != param_list.end(); ++x)
				{
					res.push_back(x->getValue() + ".int.1");
				}
				return res;
			}
		}
	}

	string gen_mips(string id){
		// return name.datatype.scope
		for (int i = scope; i >= 0; i--) {
			if(symbols[i].find(id) !=  symbols[i].end()){
				return id + "." +  Type_to_string[symbols[i].find(id)->second.getD_type()] + "." + to_string(i);
			}
		}
		return "";
	}

	vector<string> backup()
	{
		vector<string> back_var;

		backup_symbols = symbols;

		// backup symbol names for scope >= 1
		for(int i = 1; i <= scope; i++)
			for(map<string,Symbol_table_meta>::iterator it = symbols[i].begin(); it != symbols[i].end(); ++it)
			{
				back_var.push_back(gen_mips(it->first));
			}

		// pop the symbols of scope >0 from the symbol table
		for(int i = scope ; i > 0; i--)
			symbols.pop_back();

		scope = 0;
		for(int i = 0; i < back_var.size(); i++){
			cout<<"ABHI" << back_var[i]<<endl;
		}
		return back_var;
	}

	void restore(vector<string> back_var)
	{
		symbols = backup_symbols;
		scope = symbols.size() - 1;
	}

	// ~Symbol_table();
};


class SemanticAnalysis
{
private:
	int error_count;
	stringstream error_message;
	bool inside_loop;
	Symbol_table symtab;
	Symbol_table_meta *active_fun_ptr;

public:

	SemanticAnalysis(Node *TreeRoot)
	:error_count(0), inside_loop(false), active_fun_ptr(NULL)
	{
		analyse(TreeRoot);
	}


	void analyse(Node *node){
		if(node == NULL)	return;

		string node_type = node->getType();
		DEBUG cerr << node_type<<endl;

		if (node_type == "program"){
			// Analyse the children
			analyse(node->ch_1);	// declaration list
			analyse(node->ch_2);	// main function

		} else if (node_type == "declaration_list" ) {
			// Analyse the children
			analyse(node->ch_1);	// declaration list
			analyse(node->ch_2);	// declaration

		} else if (node_type == "declaration" ){
			// Analyse the children
			analyse(node->ch_1);	// variable/function declaration

		} else if (node_type == "variable_declaration" ){

			if(node->ch_3 == NULL){ 		// type variablelist
				// get variables from the ch_2
				vector<string> vars = expandVariablesList(node->ch_2);

				// check if variable not already declared
				// add if not declared
				for (std::vector<string>::iterator i = vars.begin(); i != vars.end(); ++i){
					if(symtab.findInCurrentScope(*i)){
						error_message<<"Line Number "<< node->line_no << " : Variable '"<< *i <<"' already declared." <<endl;
						error_count++;
						node->setD_type(datatype_err);
					} else {
						symtab.addVariableInCurrentScope(*i, node->ch_1->getD_type());
					}
				}
			} else { 		// type variable = expression
				analyse(node->ch_3);
				if(symtab.findInCurrentScope(node->ch_2->getValue())) {
					error_message<<"Line Number "<< node->line_no << " : Variable '"<< node->ch_2->getValue() <<"' already declared." <<endl;
					error_count++;
					node->setD_type(datatype_err);
				} else if (!checkDatatypeCoercible(node->ch_3->getD_type(), node->ch_1->getD_type())) {
					error_message<<"Line Number "<< node->line_no << " : Type mismatch. Expected "<< node->ch_1->getD_type() << " but passed " << node->ch_3->getD_type()<<endl;
					error_count++;
					node->setD_type(datatype_err);
				} else {
					symtab.addVariableInCurrentScope(node->ch_2->getValue(), node->ch_1->getD_type());
				}
			}

		} else if (node_type == "variable" ){
			// Check if declared or not
			if(!symtab.find(node->ch_1->getValue())) {
				error_count++;
				error_message<<"Line Number "<< node->line_no<< " : Variable " << node->ch_1->getValue() <<" used before declaration."<<endl;
				node->setD_type(datatype_err);
			}else {
				node->setD_type(symtab.getD_type(node->ch_1->getValue()));
			}

		} else if (node_type == "function_declaration" ){
			if(!symtab.find(node->getValue())){ 	// declare the function if not already declared
				vector <Parameter> params = expandParameterList(node->ch_2);

				// set the active function pointer
				active_fun_ptr = symtab.addFunction(node->getValue(), node->ch_1->getD_type(), params);

				// add a scope
				symtab.addScope();
				for (std::vector<Parameter>::iterator i = params.begin(); i != params.end(); ++i)
				{
					symtab.addVariableInCurrentScope(i->getValue(), i->getD_type());
				}
				analyse(node->ch_3);
				//remove the scope
				symtab.removeScope();

			} else { //function overloading not implemented
				error_count++;
				error_message << "Line Number "<< node->line_no<<" : Function Already declared."<<endl;
				node->setD_type(datatype_err);
			}

		} else if (node_type == "main_function" ){
			// Analyse the children
			active_fun_ptr = symtab.addFunction("main", datatype_none, vector<Parameter> ());
			analyse(node->ch_1);

		} else if (node_type == "statements" ){
			// Analyse the children statements
			analyse(node->ch_1);
			analyse(node->ch_2);

		} else if (node_type == "statement" ){
			// Analyse the children
			if(node->getValue() == "break"){
				if (inside_loop){
					return ;
				} else {
					error_count++;
					error_message << "Line Number " << node->line_no << " : 'break' can only be used inside a loop." << endl;
				}
			} else if (node->getValue() == "continue"){
				if (inside_loop){
					return ;
				} else {
					error_count++;
					error_message << "Line Number " << node->line_no << " : 'continue' can only be used inside a loop." << endl;
				}
			} else if (node->getValue() == "scope"){
				symtab.addScope();
				analyse(node->ch_1);
				symtab.removeScope();
			} else {
				analyse(node->ch_1);
			}

		} else if (node_type == "condition" ){
			// Analyse the children
			analyse(node->ch_1);

			symtab.addScope();
			analyse(node->ch_2);
			symtab.removeScope();

			if(node->ch_3 != NULL){
				symtab.addScope();
				analyse(node->ch_3);
				symtab.removeScope();
			}

		} else if (node_type == "loop" ){
			// Analyse the children
			inside_loop = true;
			analyse(node->ch_1);
			inside_loop = false;

		} else if (node_type == "for_loop" ){
			symtab.addScope();
			analyse(node->ch_1);
			analyse(node->ch_2);
			analyse(node->ch_3);
			analyse(node->ch_4);
			symtab.removeScope();

		

		} else if (node_type == "while_loop" ){
			// Analyse the children
			analyse(node->ch_1);
			symtab.addScope();
			analyse(node->ch_2);
			symtab.removeScope();

		} else if (node_type == "return_statement" ){
			// Analyse the children
			if (active_fun_ptr == NULL) {
				error_count++;
				error_message<<"Line Number "<<node->line_no<<" : Return statement can only be used inside a function."<<endl;
				node->setD_type(datatype_err);
			} else {
				analyse(node->ch_2);
				if (node->ch_2->getD_type() == active_fun_ptr->getReturnD_type()) {
					active_fun_ptr = NULL;
				} else if (node->ch_2 == NULL && active_fun_ptr->getReturnD_type() == datatype_none){
					active_fun_ptr = NULL;
				} else {
					error_count++;
					error_message<<"Line Number "<<node->line_no<<" : Function returns wrong data type."<<endl;
				}
			}

		} else if (node_type == "read" || node_type == "write" ){
			// Analyse the children
			analyse(node->ch_1);

		} else if (node_type == "expression" ){
			// Analyse the children
			if(node->ch_2 != NULL){
				analyse(node->ch_2);
				analyse(node->ch_1);

				if(!checkDatatypeCoercible(node->ch_1->getD_type(), node->ch_2->getD_type())){
					error_count++;
					error_message<<"Line Number "<<node->line_no<<" : Type mismatch. Unable to type cast implicitly.(expression)"<<endl;
					node->setD_type(datatype_err);
				} else {
					node->setD_type(node->ch_1->getD_type());
				}
			} else {
				analyse(node->ch_1);
				node->setD_type(node->ch_1->getD_type());
			}

		} else if (node_type == "logical_expression" || node_type == "and_expression"){
			// Analyse the children
			if(node->ch_2 != NULL){
				analyse(node->ch_1);
				analyse(node->ch_2);
				node->setD_type(datatype_bool);
			} else {
				analyse(node->ch_1);
				node->setD_type(node->ch_1->getD_type());
			}

		} else if (node_type == "relational_expression" ){
			// Analyse the children
			analyse(node->ch_1);

			if(node->ch_3 != NULL){
				analyse(node->ch_3);
				if(checkDatatypeCoercible(node->ch_1->getD_type(), node->ch_3->getD_type())){
					analyse(node->ch_2);
					node->setD_type(datatype_bool);
				} else {
					error_count++;
					error_message<<"Line Number "<<node->line_no<<" : Data type mismatch. Unable to type cast implicitly.(relational_expression)"<<endl;
					node->setD_type(datatype_err);
				}
			} else {
				node->setD_type(node->ch_1->getD_type());
			}

		} else if (node_type == "simple_expression" || node_type == "divmul_expression" ){
			// Analyse the children
			analyse(node->ch_1);
			if(node->ch_2 != NULL){
				analyse(node->ch_3);
				analyse(node->ch_2);
				if(!checkDatatypeCoercible(node->ch_1->getD_type(), node->ch_3->getD_type())){
					error_count++;
					error_message<<"Line Number "<<node->line_no<<" : Data type mismatch. Unable to type cast implicitly.(simple_expression/divmul_expression)"<<endl;
					node->setD_type(datatype_err);
				} else {
					D_type dt1 = node->ch_1->getD_type();
					D_type dt2 = node->ch_3->getD_type();

					if((dt1 == datatype_int) && (dt2 == datatype_int)){
						node->setD_type(datatype_int);
					} else if((dt1 == datatype_int || dt1 == datatype_float) && ((dt2 == datatype_int || dt2 == datatype_float))){
						node->setD_type(datatype_float);
					} else {
						error_count++;
						error_message<<"Line Number : "<<node->line_no<<" : Invalid operands provided to '"<<node->ch_2->getValue()<<"' operator."<<endl;
						node->setD_type(datatype_err);
					}
				}
			}else{
				node->setD_type(node->ch_1->getD_type());
			}

		

		} else if (node_type == "term" ){
			// Analyse the children
			analyse(node->ch_1);
			node->setD_type(node->ch_1->getD_type());

		} else if (node_type == "function_call" ){
			// check if function is declared
			node->setD_type(datatype_none);

			if(!symtab.find(node->getValue())){
				error_count++;
				error_message<<"Line Number "<<node->line_no<<" : Function '"<< node->getValue() <<"' not declared."<<endl;
				node->setD_type(datatype_err);
			} else { // if declared, the arguments count and type should match
				vector<D_type> args_list = expandArgumentsList(node->ch_1);

				if(!symtab.checkFunctionArgs(node->getValue(), args_list)){
					error_count++;
					error_message<<"Line Number "<<node->line_no<<" : Incorrect arguments passed to the function '"<< node->getValue() <<"'."<<endl;
					node->setD_type(datatype_err);
				} else {
					node->setD_type(symtab.getFunctionD_type(node->getValue()));
				}
			}

		} else if (node_type == "operator1" || node_type == "operator2" || node_type == "operator3" || node_type == "constants" || node_type == "write_string"){
			// Analyse the children
			return ;

		} else {
			cout<<"---------"<<endl;
			cout<<node->getValue()<<endl;
			cout<<node->getType()<<endl;
			cout<<node->getD_type()<<endl;
			cout<<"---------"<<endl;
		}
	}


	vector<string> expandVariablesList(Node *tree){
		vector<string> res;
		if(tree->getType() != "variable_list"){
			return res;
		} else if(tree->ch_2 == NULL){
			res.push_back(tree->ch_1->getValue());
		} else {
			res.push_back(tree->ch_2->getValue());
			vector <string> temp = expandVariablesList(tree->ch_1);
			for (std::vector<string>::reverse_iterator i = temp.rbegin(); i != temp.rend(); ++i){
				res.insert(res.begin(), *i);
			}
		}
		return res;
	}

	vector <Parameter> expandParameterList(Node *tree){
		if(tree->getType() != "parameters" || tree->ch_1 == NULL){
			return vector<Parameter>();
		} else {
			Node *paramlist = tree->ch_1;
			return expandParameterListAux(paramlist);
		}
	}

	vector<Parameter> expandParameterListAux(Node *tree){
		vector<Parameter> res;
		if(tree->getType() != "parameters_list"){
			return res;
		}
		if(tree->ch_2 == NULL){
			res.push_back(Parameter(tree->ch_1->ch_2->getValue(), tree->ch_1->getD_type()));
			return res;
		}
		res = expandParameterListAux(tree->ch_1);
		res.push_back(Parameter(tree->ch_2->ch_2->getValue(), tree->ch_2->getD_type()));
		return res;
	}

	vector<D_type> expandArgumentsList(Node *Tree){
		vector<D_type> v;
		v.clear();
		if(Tree->getType() != "args" || Tree->ch_1 == NULL){return v;}
		Node *args_list = Tree->ch_1;
		return expandArgumentsListAux(args_list);
	}

	vector<D_type> expandArgumentsListAux(Node *tree){
		vector<D_type> res;
		res.clear();
		if(tree->getType() != "args_list"){
			return res;
		}
		if(tree->ch_2 == NULL){
			analyse(tree->ch_1);
			res.push_back(tree->ch_1->getD_type());
			return res;
		}
		res = expandArgumentsListAux(tree->ch_1);
		analyse(tree->ch_2);
		res.push_back(tree->ch_2->getD_type());
		return res;
	}

	bool checkDatatypeCoercible(D_type dt1, D_type dt2){
		if (dt1 == dt2) {
			return true;
		} else if((dt1 == datatype_int || dt1 == datatype_float) && ((dt2 == datatype_int || dt2 == datatype_float))){
			return true;
		} else {
			return false;
		}
	}

	void errors(){
		if(error_count == 0){
			cout<<"No Semantic Errors!"<<endl;
		} else {
			cout<<error_count<<"  error(s) found during semantic analysis!"<<endl;
			cout<<error_message.str()<<endl;
			exit(2);
		}
	}

	// ~SemanticAnalysis();
};


class MIPSCode
{
private:
	Symbol_table symtab, backup_symtab;	// backup symtab is used for pushing to stack during backup
	stringstream mips_1, mips_2, intermediate_code;
	int label_counter;
	int string_var_counter;
	int temp_counter;

	vector<string> breaks;
	vector<string> continues;

	vector< pair<string, D_type> > allVariables;
	vector< pair<string, string> > stringLiterals;

	string Type_to_string[8] = {"none", "int", "char", "string", "float", "bool", "func", "err"};

public:
	MIPSCode(){
		mips_1 << ".text" << endl;
		label_counter = 0;
		temp_counter = 0;
		string_var_counter = 0;

		breaks.clear();
		continues.clear();
		allVariables.clear();
	}

	string getNextLabel(){
		return "label_" + to_string(label_counter++);
	}

	pair<string, D_type> getNextTempVar(){
		return make_pair("temp" + to_string(temp_counter++), datatype_int);
	}

	string getStringVar(){
		return "_str" + to_string(string_var_counter++);
	}

	string putLabel(string label, int param_count){
		label = "_" + label + "_." + to_string(param_count);
		mips_1 << label << " : " << endl;
		return label;
	}

	string putLabel(string label){
		mips_1 << label << " : " << endl;
		return label;
	}

	void loadInRegister(string var, string reg, D_type type){
		// check if var is variable (load word) or number (load immediate)

		if(var[0] <= '9' and var[0] >= '0')	// if number
		{
			mips_1 << "li\t$" << reg <<", " << var << endl;

		} else if (var[0] == '\'') {
			mips_1 << "li\t$" << reg <<", " << var << endl;

		} else if (var[0] == '"') {
			string temp = getStringVar();
			stringLiterals.push_back(make_pair(temp, var));
			mips_1 << "la\t$" << reg <<", " << temp << endl;

		} else {					// variable name
			var = "_" + var;
			mips_1 << "lw\t$" << reg <<", " << var << endl;
			for (std::vector<pair<string, D_type> >::iterator i = allVariables.begin(); i != allVariables.end(); ++i) {
				if(i->first == var)
					return;
			}
			allVariables.push_back(make_pair(var, type));
		}
	}

	void storeInMemory(string var, D_type type){
		var = "_"+var;
		mips_1 << "sw\t$t0, " << var << endl;
		for (std::vector<pair<string, D_type> >::iterator i = allVariables.begin(); i != allVariables.end(); ++i) {
			if(i->first == var)
				return;
		}
		allVariables.push_back(make_pair(var, type));
	}

	void ReturnFunc(){
		mips_1 << "jr\t$ra"<<endl;
	}

	void Jump(string label){
		mips_1 << "j\t" << label << endl;
	}

	void condition(string reg, string label){
		mips_1 << "bgtz\t$" << reg <<", "<<label<<endl;
	}

	void functionReturnValue(string reg){
		mips_1 << "move\t$v0, $"<< reg << endl;
	}

	void readCode(string reg){
		mips_1 << "li\t$v0, 5" << endl;
		mips_1 << "syscall" << endl;
		mips_1 << "sw\t$v0, $" << reg << endl;
	}

	void writeCode(string reg){
		mips_1 << "li\t$v0, 1" << endl;
		mips_1 << "move\t$a0, $"<< reg << endl;
		mips_1 << "syscall" << endl;
	}

	void writeStringCode(string str_var){
		mips_1 << "li\t$v0, 4" << endl;
		mips_1 << "la\t$a0, ($" << str_var << ")" << endl;
		mips_1 << "syscall" << endl;
	}

	void pushReturnCode(){
		mips_1 << "addi\t$sp,$sp,-4" << endl;
		mips_1 << "sw\t$ra,0($sp)" << endl;
	}

	void popReturnCode(){
		mips_1 << "lw\t$ra,0($sp)" << endl;
		mips_1 << "addi\t$sp,$sp,4" << endl;	}

	void pushCode(string loc){
		mips_1 << "addi	$sp,$sp,-4" << endl;
		mips_1 << "lw	$t8, " << loc << endl;
		mips_1 << "sw	$t8,0($sp)" << endl;
	}

	void popCode(string loc){
		mips_1 << "lw	$t8,0($sp)" << endl;
		mips_1 << "sw	$t8, " << loc << endl;
		mips_1 << "addi	$sp,$sp,4" << endl;
	}

	void copy(string src, string dst){
		mips_1 << "lw\t$t8, " << src << endl;
		mips_1 << "sw\t$t8, " << dst << endl;
	}

	void functionCall(string fun){
		mips_1 << "jal\t" << fun << endl;
	}

	void restoreReturn(string ret){
		mips_1 << "sw\t$v0, _" << ret << endl;
	}

	void operate(string op){
		if(op == "&&") {
			mips_1 << "and\t$t0, $t1, $t2" << endl;
		} else if(op == "||") {
			mips_1 << "or\t$t0, $t1, $t2" << endl;

		} else if(op == "+") {
			mips_1 << "add\t$t0, $t1, $t2" << endl;
		} else if(op == "-") {
			mips_1 << "sub\t$t0, $t1, $t2" << endl;

		} else if(op == "*") {
			mips_1 << "mult\t$t1, $t2" << endl;		// store the result in $LO
			mips_1 << "mflo\t$t0" << endl;			// load the contents of $LO to $t0
		} else if(op == "/") {
			mips_1 << "div\t$t1, $t2" << endl;
			mips_1 << "mflo\t$t0" << endl;
		} else if(op == "%") {
			mips_1 << "div\t$t1, $t2" << endl;
			mips_1 << "mfhi\t$t0" << endl;

		} else if(op == ">=") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;		// set t3 if t1 less than t2
			mips_1 << "xori\t$t0, $t3, 1" << endl;		// compliment t3
		} else if(op == "<=") {
			mips_1 << "slt\t$t3, $t2, $t1" << endl;		// set t3 if t2 less than t1
			mips_1 << "xori\t$t0, $t3, 1" << endl;		// compliment t3

		} else if(op == ">") {
			mips_1 << "slt\t$t0, $t2, $t1" << endl;		// set t0 if t2 less than t1
		} else if(op == "<") {
			mips_1 << "slt\t$t0, $t1, $t2" << endl;

		} else if(op == "==") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;
			mips_1 << "slt\t$t4, $t2, $t1" << endl;
			mips_1 << "or\t$t5, $t3, $t4" << endl;
			mips_1 << "xori\t$t0, $t5, 1" << endl;
		} else if(op == "!=") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;
			mips_1 << "slt\t$t4, $t2, $t1" << endl;
			mips_1 << "or\t$t0, $t3, $t4" << endl;
		} else {
			cerr<<"WRONG OPERATOR PASSED!";
		}
	}

	pair<string, D_type> generateCode(Node *tree){
		if(tree == NULL)	return make_pair("", datatype_int);

		string node_type = tree->getType();

		DEBUG cerr << node_type << endl;

		if (node_type == "") { return make_pair("", datatype_int);}


		if (node_type == "variable_declaration") {
			if(tree->ch_3 == NULL){
				vector<string> vars = expandVariablesList(tree->ch_2);
				for(int i=0; i < vars.size(); i++)
				{
					symtab.addVariableInCurrentScope(vars[i], tree->ch_1->getD_type());
					intermediate_code << Type_to_string[tree->ch_1->getD_type()] << " " << vars[i] <<endl;
				}
			} else {
				// add to symtab
				symtab.addVariableInCurrentScope(tree->ch_2->getValue(), tree->ch_1->getD_type());
				pair<string, D_type> exp = generateCode(tree->ch_3);
				intermediate_code << Type_to_string[tree->ch_1->getD_type()] << " " << tree->ch_2->getValue() << " = " << exp.first << endl;
				// store the expression register to the memory
				loadInRegister(exp.first, "t0", exp.second);
				storeInMemory(symtab.gen_mips(tree->ch_2->getValue()), tree->ch_1->getD_type());
			}
			return make_pair("", datatype_int);

		} else if ( node_type == "variable") {
			// return name.type.scope
			return make_pair(symtab.gen_mips(tree->ch_1->getValue()), tree->ch_1->getD_type());

		} else if ( node_type == "function_declaration") {

			vector<Parameter> params = expandParameterList(tree->ch_2);

			string label = putLabel(tree->getValue(), params.size());
			intermediate_code << tree->getValue() << " : " <<endl;

			symtab.addFunction(label, datatype_int, params);

			symtab.addScope();
			for(int i=0;i<params.size();i++)
			{
				symtab.addVariableInCurrentScope(params[i].getValue(), params[i].getD_type());
				intermediate_code << "param " << Type_to_string[params[i].getD_type()] << " " << params[i].getValue() <<endl;
			}

			pair<string, D_type> a = generateCode(tree->ch_3);
			symtab.removeScope();

			ReturnFunc();
			intermediate_code << "return" << endl;

			return make_pair("", datatype_int);

		} else if ( node_type == "main_function") {
			mips_1 << "main : "<<endl;
			intermediate_code << "main : "<<endl;

			symtab.addScope();
			pair<string, D_type> a = generateCode(tree->ch_1);
			symtab.removeScope();
			ReturnFunc();
			intermediate_code << "return" << endl;
			return make_pair("", datatype_int);

		} else if ( node_type == "statement") {
			if(tree->getValue() == "break"){
				Jump(breaks.back());
				intermediate_code << "break" <<endl;
			} else if (tree->getValue() == "continue") {
				Jump(continues.back());
				intermediate_code << "continue" <<endl;
			} else if(tree->getValue() == "scope"){
				symtab.addScope();
				intermediate_code << "{" <<endl;
				generateCode(tree->ch_1);
				symtab.removeScope();
				intermediate_code << "}" <<endl;
			} else {
				generateCode(tree->ch_1);
			}
			return make_pair("", datatype_int);

		} else if ( node_type == "condition") {
			string start = getNextLabel();
			string end = getNextLabel();

			pair<string, D_type> a = generateCode(tree->ch_1);	// expression

			// load the expression's value in t1
			loadInRegister(a.first, "t1", a.second);
			intermediate_code << "if ( " << a.first <<" > 0 ) jump " << start <<endl;
			// branch to start if $t1>0
			condition("t1", start);
			// otherwise go to end
			intermediate_code << "jump " << end <<endl;
			Jump(end);
			// now add the if code (i.e. the start label)
			putLabel(start);
			intermediate_code << start << " : "<<endl;
			symtab.addScope();
			pair<string, D_type> b = generateCode(tree->ch_2);
			symtab.removeScope();
			putLabel(end);
			intermediate_code << end << " : "<<endl;

			// if else part
			if(tree->ch_3 != NULL)
			{
				symtab.addScope();
				pair<string, D_type> c = generateCode(tree->ch_3);
				symtab.removeScope();
			}
			return make_pair("", datatype_int);

		} else if ( node_type == "for_loop") {
			string start = getNextLabel();		// start of the loop
			string middle = getNextLabel();		// code statements
			string cond = getNextLabel();		// code statements
			string end = getNextLabel();		// end

			breaks.push_back(end);		// Label to jump to on break
			continues.push_back(cond); 	// Label to jump to on continue

			// code for expression
			generateCode(tree->ch_1);

			putLabel(start);
			intermediate_code << start << " : " <<endl;

			// code for condition
			pair<string, D_type> a = generateCode(tree->ch_2);
			loadInRegister(a.first, "t0", a.second);

			// if condition true, jump to code statements
			condition("t0", middle);
			intermediate_code << "if ( " << a.first << " > 0 ) jump "<< middle <<endl;

			// else jump to exit
			Jump(end);
			intermediate_code << "jump " << end <<endl;
			// code statements
			putLabel(middle);
			intermediate_code << middle <<" : " <<endl;

			generateCode(tree->ch_4);

			putLabel(cond);
			intermediate_code << cond <<" : " <<endl;
			generateCode(tree->ch_3);

			// code for jump to start
			Jump(start);
			intermediate_code << "jump " << start <<endl;
			putLabel(end);
			intermediate_code << end <<" : " <<endl;

			breaks.pop_back();
			continues.pop_back();
			return(make_pair("", datatype_int));

		

		} else if ( node_type == "while_loop") {
			string start = getNextLabel();	// start of the loop
			string middle = getNextLabel();		// loop statements
			string end = getNextLabel();		// terminate loop

			breaks.push_back(end);
			continues.push_back(start);

			putLabel(start);
			intermediate_code << start << " : "<<endl;
			pair<string, D_type> a = generateCode(tree->ch_1);

			loadInRegister(a.first, "t1", a.second);
			condition("t1", middle);
			Jump(end);
			intermediate_code << "if ( " << a.first << " > 0 ) jump " << middle << endl;
			intermediate_code << "jump " << end<<endl;
			putLabel(middle);
			intermediate_code << middle << " : "<<endl;
			symtab.addScope();
			pair<string, D_type> b = generateCode(tree->ch_2);
			symtab.removeScope();

			Jump(start);
			putLabel(end);

			intermediate_code << "jump " << start <<endl;
			intermediate_code << end << " : "<<endl;

			breaks.pop_back();
			continues.pop_back();

			return make_pair("", datatype_int);

		} else if ( node_type == "return_statement") {
			if(tree->ch_2 == NULL)
			{
				pair<string, D_type> a = generateCode(tree->ch_2);

				loadInRegister(a.first, "t1", a.second);
				functionReturnValue("t1");
				intermediate_code << "return " << a.first << endl;
			}
			ReturnFunc();

			return make_pair("", datatype_int);

		} else if ( node_type == "read") {
			pair<string, D_type> a = generateCode(tree->ch_1);
			loadInRegister(a.first, "t1", a.second);
			readCode("t1");
			intermediate_code << "read " << a.first << endl;
			return a;

		} else if ( node_type == "write") {
			pair<string, D_type> a = generateCode(tree->ch_1);
			loadInRegister(a.first, "t1", a.second);
			writeCode("t1");
			intermediate_code << "write " << a.first << endl;
			return a;

		} else if ( node_type == "expression") {
			pair<string, D_type> b;
			if(tree->getValue() == "=") {
				pair<string, D_type> a = generateCode(tree->ch_2);
				b = generateCode(tree->ch_1);
				loadInRegister(a.first, "t0", a.second);
				storeInMemory(b.first, b.second);
				intermediate_code << b.first << " = " << a.first <<endl;
			} else {
				b = generateCode(tree->ch_1);
			}
			pair<string, D_type> ret = getNextTempVar();

			loadInRegister(b.first, "t0", b.second);
			storeInMemory(ret.first, b.second);
			intermediate_code << ret.first << " = " << b.first <<endl;

			return ret;

		} else if ( node_type == "logical_expression") {
			if(tree->getValue() == "or") {
				pair<string, D_type> a = generateCode(tree->ch_2);
				pair<string, D_type> b = generateCode(tree->ch_1);
				pair<string, D_type> ret = getNextTempVar();

				loadInRegister(b.first, "t1", b.second);
				loadInRegister(a.first, "t2", a.second);
				operate("||");
				storeInMemory(ret.first, datatype_bool);

				intermediate_code << ret.first << " = " << a.first << " || " << b.first << endl;

				return ret;

			} else {
				return generateCode(tree->ch_1);
			}

		} else if ( node_type == "and_expression") {
			if(tree->getValue() == "and") {
				pair<string, D_type> a = generateCode(tree->ch_2);
				pair<string, D_type> b = generateCode(tree->ch_1);
				pair<string, D_type> ret = getNextTempVar();

				loadInRegister(b.first, "t1", b.second);
				loadInRegister(a.first, "t2", a.second);
				operate("&&");
				storeInMemory(ret.first, datatype_bool);
				intermediate_code << ret.first << " = " << a.first << " && " << b.first << endl;
				return ret;

			} else {
				return generateCode(tree->ch_1);
			}

		} else if ( node_type == "relational_expression") {
			if(tree->getValue() == "op")
			{
				pair<string, D_type> a = generateCode(tree->ch_3);
				pair<string, D_type> b = generateCode(tree->ch_1);
				pair<string, D_type> c = generateCode(tree->ch_2);
				pair<string, D_type> ret = getNextTempVar();

				loadInRegister(b.first, "t1", b.second);
				loadInRegister(a.first, "t2", a.second);
				operate(c.first);
				storeInMemory(ret.first, datatype_bool);
				intermediate_code << ret.first << " = " << a.first << " " << c.first << " " << b.first << endl;
				return ret;
			} else {
				return generateCode(tree->ch_1);
			}

		} else if ( node_type == "simple_expression") {
			if(tree->ch_3 != NULL)
			{
				pair<string, D_type> a = generateCode(tree->ch_3);
				pair<string, D_type> b = generateCode(tree->ch_1);
				pair<string, D_type> c = generateCode(tree->ch_2);
				pair<string, D_type> ret = getNextTempVar();

				loadInRegister(b.first, "t1", b.second);
				loadInRegister(a.first, "t2", a.second);
				operate(c.first);
				storeInMemory(ret.first, DatatypeCoercible(a.second, b.second));
				intermediate_code << ret.first << " = " << a.first << " " << c.first << " " << b.first << endl;
				return ret;
			} else {
				return generateCode(tree->ch_1);
			}

		} else if ( node_type == "divmul_expression") {
			if(tree->ch_3 != NULL)
			{
				pair<string, D_type> a = generateCode(tree->ch_3);
				pair<string, D_type> b = generateCode(tree->ch_1);
				pair<string, D_type> c = generateCode(tree->ch_2);
				pair<string, D_type> ret = getNextTempVar();

				loadInRegister(b.first, "t1", b.second);
				loadInRegister(a.first, "t2", a.second);
				operate(c.first);
				storeInMemory(ret.first, DatatypeCoercible(a.second, b.second));
				intermediate_code << ret.first << " = " << a.first << " " << c.first << " " << b.first << endl;
				return ret;
			} else {
				return generateCode(tree->ch_1);
			}

		}  else if ( node_type == "term") {
			return generateCode(tree->ch_1);

		} else if ( node_type == "constants") {
			return make_pair(tree->getValue(), tree->getD_type());;

		} else if ( node_type == "function_call") {
			//evaluate arguments
			intermediate_code << "call " << tree->getValue() <<endl;
			vector<string> args = expandArgumentsList(tree->ch_1);
			vector<string> pars = symtab.getFunctionParameters("_" + tree->getValue() + "_." + to_string(args.size()));

			//backup variables
			// push the return address to stack
			pushReturnCode();

			// push the current register values to stack
			vector<string> backvars = symtab.backup();
			for(int i=0;i<backvars.size();i++)
			{
				pushCode(backvars[i]);
			}

			//set arguments

			// copy args[i] to pars[i]
			for(int i=0;i < args.size(); i++)
			{
				copy(args[i], pars[i]);
			}

			//call function
			functionCall("_" + tree->getValue() + "_." + to_string(args.size()));

			//restore variables
			// pop back the stored register values from stack
			for(int i = backvars.size()-1;i>=0;i--)
			{
				popCode(backvars[i]);
			}
			// pop back the return address from stack
			symtab.restore(backvars);

			popReturnCode();

			pair<string, D_type> ret = getNextTempVar();

			// restore the return value
			restoreReturn(ret.first);
			ReturnFunc();

			return ret;
		} else if (node_type == "write_string") {
			loadInRegister(tree->ch_1->getValue(), "t0", datatype_string);
			writeStringCode("t0");
			intermediate_code << "puts " << tree->ch_1->getValue() <<endl;
			return make_pair("", datatype_int);

		} else if ( node_type == "operator3" || node_type == "operator2" || node_type == "operator1") {
			return make_pair(tree->getValue(), tree->getD_type());
		} else {
			generateCode(tree->ch_1);
			generateCode(tree->ch_2);
			generateCode(tree->ch_3);
			return make_pair("", datatype_int);
		}
	}

	void generateDataSection(){
		mips_2 << ".data" << endl;
		for(vector<pair< string, string> >::iterator i = stringLiterals.begin(); i != stringLiterals.end(); i++)
		{
			mips_2 << i->first << ":\t\t.asciiz " << i->second << endl;
		}

		for( vector < pair < string , D_type > > :: iterator i = allVariables.begin(); i != allVariables.end(); i++){
			if(i->second == datatype_none || i->second == datatype_int || i->second == datatype_bool){
				mips_2 << i->first << ":\t\t.word 0" << endl;
			} else if(i->second == datatype_float){
				mips_2 << i->first << ":\t\t.float 0.0" << endl;
			} else if (i->second == datatype_string){
				mips_2 << i->first << ":\t\t.space 20" << endl;
			}
		}
		mips_2 << mips_1.str();
		return;
	}

	void generateOutput(){
		fstream InterFile, MIPSFile;
		MIPSFile.open("mips.s", fstream::out);
		MIPSFile << mips_2.str() ;
		MIPSFile.close();
		InterFile.open("intermediate.txt", fstream::out);
		InterFile << intermediate_code.str() ;
		InterFile.close();
		return ;
	}

	vector<string> expandVariablesList(Node *tree){
		vector<string> res;
		if(tree->getType() != "variable_list"){
			return res;
		} else if(tree->ch_2 == NULL){
			res.push_back(tree->ch_1->getValue());
		} else {
			res.push_back(tree->ch_2->getValue());
			vector <string> temp = expandVariablesList(tree->ch_1);
			for (std::vector<string>::reverse_iterator i = temp.rbegin(); i != temp.rend(); ++i){
				res.insert(res.begin(), *i);
			}
		}
		return res;
	}

	vector <Parameter> expandParameterList(Node *tree){
		if(tree->getType() != "parameters" || tree->ch_1 == NULL){
			return vector<Parameter>();
		} else {
			Node *paramlist = tree->ch_1;
			return expandParameterListAux(paramlist);
		}
	}

	vector<Parameter> expandParameterListAux(Node *tree){
		vector<Parameter> res;
		if(tree->getType() != "parameters_list"){
			return res;
		}
		if(tree->ch_2 == NULL){
			res.push_back(Parameter(tree->ch_1->ch_2->getValue(), tree->ch_1->getD_type()));
			return res;
		}
		res = expandParameterListAux(tree->ch_1);
		res.push_back(Parameter(tree->ch_2->ch_2->getValue(), tree->ch_2->getD_type()));
		return res;
	}

	vector<string> expandArgumentsList(Node *Tree){
		vector<string> v;
		v.clear();
		if(Tree->getType() != "args" || Tree->ch_1 == NULL) {return v;}
		Node *args_list = Tree->ch_1;
		return expandArgumentsListAux(args_list);
	}

	vector<string> expandArgumentsListAux(Node *tree){
		vector<string> res;
		res.clear();
		if(tree->getType() != "args_list"){
			return res;
		}
		if(tree->ch_2 == NULL){
			res.push_back(tree->ch_1->getValue());
			return res;
		}

		res = expandArgumentsListAux(tree->ch_1);
		res.push_back(tree->ch_2->getValue());

		return res;
	}

	D_type DatatypeCoercible(D_type dt1, D_type dt2){
		if (dt1 == dt2) {
			return dt1;
		} else if((dt1 == datatype_int || dt1 == datatype_float) && ((dt2 == datatype_int || dt2 == datatype_float))){
			return datatype_float;
		} else {
			return datatype_int;
		}
	}


	// ~MIPSCode();
};
