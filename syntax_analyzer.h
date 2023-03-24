#pragma once
#ifndef SYNTAX_ANALYZER_H_
#define SYNTAX_ANALYZER_H_

/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <string>
#include <utility>  // pair
#include <vector>  // message list

#include "lexer.h"

/* ------------------- DEFINE STATEMENTS FOR STRUCTURES ------------------- */
typedef int flag;  // switch
typedef std::string Message;
typedef std::vector<Message> Rule_list;  // list of productions used for a token


/* -------------------------------- CLASSES -------------------------------- */
class Syntax_Analyzer {
	private:
		Lexer* lexer;  // get tokens from input file
		Token current_token = { "", "" };  // keep track of most recent token
		Rule_list Productions;  // record the productions used for each token
		std::ofstream* ofs;  // print tokens + rules to output file

		// syntax rules
		void Opt_Function_Definitions();
		int Function_Definitions_Start(int exit_on_error);
		void Function_Definitions_Cont();
		int Function(int exit_on_error);
		void Opt_Parameter_List();
		int Parameter_List_Start(int exit_on_error);
		void Parameter_List_Cont();
		int Parameter(int exit_on_error);
		int Qualifier(int exit_on_error);
		void Body();
		void Opt_Declaration_List();
		int Declaration_List_Start(int exit_on_error);
		void Declaration_List_Cont();
		int Declaration(int exit_on_error);
		int IDs_Start(int exit_on_error);
		void IDs_Cont();
		int Statement_List_Start(int exit_on_error);
		void Statement_List_Cont();
		int Statement(int exit_on_error);
		int Compound(int exit_on_error);
		int Assign(int exit_on_error);
		int If_Start(int exit_on_error);
		int If_Cont(int exit_on_error);
		int Return_Start(int exit_on_error);
		int Return_Cont(int exit_on_error);
		int Print(int exit_on_error);
		int Scan(int exit_on_error);
		int While(int exit_on_error);
		int Condition(int exit_on_error);
		int Relop(int exit_on_error);
		int Expression_Start(int exit_on_error);
		void Expression_Cont();
		int Term_Start(int exit_on_error);
		void Term_Cont();
		int Factor(int exit_on_error);
		int Primary_Start(int exit_on_error);
		void Primary_Cont();

		// helper functions
		int check_symbol(std::string symbol, int exit_on_error,
			             int& line_number);  // terminal symbols (tokens)
		void copy_list(Rule_list original, Rule_list& copy);  // copy rule list
		std::string convert_to_lowercase(std::string s);
		int get_line_number();  // get position of lexer
		void print_current_token();
		void print_productions();
		void print_error(Message err_msg, int line_number);


	public:
		Syntax_Analyzer(std::ifstream* input_file_stream,
			            std::ofstream* output_file_stream);  //  constructor
		void Rat23S();  //  start syntax analysis
};

#endif