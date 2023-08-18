#pragma once
#ifndef SYNTAX_ANALYZER_H_
#define SYNTAX_ANALYZER_H_

/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input and output files
#include <string>  // substring
#include <vector>  // Rule_list

#include "lexer.h"  // Lexer (get tokens)

/* ------------------- DEFINE STATEMENTS FOR STRUCTURES ------------------- */
typedef std::string Production;  // ex. "E -> T"
typedef std::vector<Production> Rule_list;  // productions used by a token


/* -------------------------------- CLASSES -------------------------------- */
class Syntax_Analyzer {  // used for an input file's Syntax Analysis
	private:
		Lexer* lexer;  // get tokens from input file
		Token current_token = { "", "" };  // used for backtracking/symbol check
		Rule_list Productions;  // productions used by current_token
		std::ofstream* ofs;  // write to output file
		int err_line_number = 1;  // keep track of where error occurs

		// productions
		void Opt_Function_Definitions();
		void Function_Definitions_Start();
		void Function_Definitions_Cont();
		void Function();
		void Opt_Parameter_List();
		void Parameter_List_Start();
		void Parameter_List_Cont();
		void Parameter();
		void Qualifier();
		void Body();
		void Opt_Declaration_List();
		void Declaration_List_Start();
		void Declaration_List_Cont();
		void Declaration();
		void IDs_Start();
		void IDs_Cont();
		void Statement_List_Start();
		void Statement_List_Cont();
		void Statement();
		void Compound();
		void Assign();
		void If_Start();
		void If_Cont();
		void Return_Start();
		void Return_Cont();
		void Print();
		void Scan();
		void While();
		void Condition();
		void Relop();
		void Expression_Start();
		void Expression_Cont();
		void Term_Start();
		void Term_Cont();
		void Factor();
		void Primary_Start();
		void Primary_Cont();

		// helper functions
		void check_symbol(std::string symbol);  // match expected symbol
		void copy_list(Rule_list original, Rule_list& copy);  // backtrack
		std::string convert_to_lowercase(std::string s);
		void print_current_token();  // print the current token
		void print_productions();  // print the productions of current token
		void print_error(std::string err_msg);  // write error message

	public:
		Syntax_Analyzer(std::ifstream* input_file_stream,
						std::ofstream* output_file_stream);  // constructor
		void Rat23S();  // start Syntax Analysis
};

#endif