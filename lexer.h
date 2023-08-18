#pragma once
#ifndef LEXER_H_
#define LEXER_H_

/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <map>  // symbol table (SYM)
#include <string>
#include <utility>  // token_type and lexeme_value pairs (ie. tokens)

/* ------------------- DEFINE STATEMENTS FOR STRUCTURES ------------------- */
typedef std::string token_type;
typedef std::string lexeme_value;
typedef int DFSM_state;

// SYM table format is { key = lexeme_value, value = token_type }
// it is used to store the list of exisiting tokens (ie. ops, seps, keywords)
typedef std::map <lexeme_value, token_type> Symbol_table;

// Tokens use format { token_type, lexeme_value }
typedef std::pair <token_type, lexeme_value> Token;


/* -------------------------------- CLASSES -------------------------------- */
class Lexer {  // used for an input file's Lexical Analysis
	private:
		// Contains a list of all tokens (operators, separators, keywords, etc.)
		Symbol_table SYM;
		std::ifstream* ifs;  // reads input file
		int line_number = 1;  // records the line that the lexer is currently on

		// DFSM transition table (N) for identifiers/keywords
		DFSM_state DFSM_id_table[5][4] =
		{
			{1, 2, 3, 4},
			{1, 2, 3, 4},
			{1, 2, 3, 4},
			{1, 2, 3, 4},
			{4, 4, 4, 4}
		};

		// DFSM transition table (N) for integers/reals
		DFSM_state DFSM_int_real_table[4][3] =
		{
			{0, 1, 3},
			{2, 3, 3},
			{2, 3, 3},
			{3, 3, 3},
		};

		// lexer helper functions (implementations in Lexer.cpp)
		Token DFSM_identifier();
		Token DFSM_int_real();
		Token check_operator_equals();
		Token check_operator_less_than();
		Token check_comment();

		bool is_white_space(char c);
		void initialize_sym_table(Symbol_table& table);


	public:
		Lexer(std::ifstream* input_file_stream);  // constructor
		Token get_token();  // extract (next) token from input file
		int get_line_number();  // get position of lexer
		int Analyze();  // returns -1 if LA error, returns 0 if file is good
		void close_ifs();  // close input file stream
};

#endif