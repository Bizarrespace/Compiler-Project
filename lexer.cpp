#pragma once

/* ------------------------------- LIBRARIES ------------------------------- */
#include <cstring>  // tolower()
#include <ctype.h>  // isalpha()  isdigit()
#include <fstream>  // input file
#include <map>  // symbol table (SYM)
#include <string>
#include <utility>  // token_type and lexeme_value pairs (ie. tokens)

#include "lexer.h"

/************************************************************************
| The constructor initializes the SYM table, which will make lookups of |
| existing operators, separators, and keywords possible. It also        |
| initializes the input file reader.                                    |
************************************************************************/
Lexer::Lexer(std::ifstream* input_file_stream) {
	initialize_sym_table(SYM);
	ifs = input_file_stream;
}

/*****************************************************************************
| The lexer function extracts tokens from the input file ifs. First, it      |
| skips over any white spaces (spacebar, tab, newline). Then, it may call    |
| helper functions to see if the current token is valid. If it is, it will   |
| return the token type and the lexeme value. If not, it will return the     |
| "ERROR" token type.                                                        |
*****************************************************************************/
Token Lexer::get_token() {
	// check if EOF reached
	if (!ifs->good()) {
		return { "EOF", "" };
	}

	// overpass whitespaces
	char buf = ifs->get();
	if (!ifs->good()) {
		return { "EOF", "" };  // EOF reached (kinda repetitive, but needed)
	}
	while (is_white_space(buf)) {
		buf = ifs->get();
		if (!ifs->good()) {
			return { "EOF", "" };  // EOF reached
		}
	}


	// first character is a letter -> check if token is ID or keyword
	if (isalpha(buf)) {
		return DFSM_identifier();  // use DFSM_id
	}

	// first character is a number -> check if token is int or real
	else if (isdigit(buf)) {
		return DFSM_int_real();  // use DFSM_int_real
	}

	// if first char is =, token could either be =, ==, or =>
	// use check_operator_equals() to determine which one it is
	else if (buf == '=') {
		return check_operator_equals();
	}

	// same case as =, but for < and <=
	else if (buf == '<') {
		return check_operator_less_than();
	}

	// the only time ! can be used is for !=. otherwise, return ERROR msg
	else if (buf == '!') {
		buf = ifs->get();
		if (buf == '=') {
			return { "operator", "!=" };
		}
		return { "ERROR", "! is an unrecognized symbol." };
	}

	// the only time [ can be used is for comments. use check_comment()
	// to see if [**] is used properly
	else if (buf == '[') {
		return check_comment();
	}

	// if char is an existing op/sep in SYM, return it and its token type
	lexeme_value buf_string(1, buf);  // SYM uses strings -> need to convert
	if (SYM.find(buf_string) != SYM.end()) {  // char found in SYM
		return { SYM.at(buf_string), buf_string };
	}

	// char is not an int, id, real, op, sep, or keyword -> invalid
	else {
		return { "ERROR", buf_string + " is an unrecognized symbol." };
	}
}

/*****************************************************************************
| The DFSM_identifier function checks if a token is a valid id or keyword.   |
| If it is, the DFSM will return its type (id/keyword) and its lexeme value. |
| If not, the DFSM will return the "ERROR" token type, which will let the    |
| main function know to stop running.                                        |
*****************************************************************************/
Token Lexer::DFSM_identifier() {
	// reset reader to get entire lexeme
	ifs->unget();
	lexeme_value lexeme = "";

	// use the DFSM_id transition table when reading the token.
	// if it ends on a valid state, then the token is a valid id/keyword
	DFSM_state current_state = 0;
	while (true) {
		char buf = ifs->get();
		// stop when whitespaces/operators/separators/EOF are reached
		if (is_white_space(buf) || SYM.find(std::string(1, buf)) != SYM.end()
			|| buf == '!' || !ifs->good()) {  // ! is for the != operator

			// reset reader in case operators/separators are next
			ifs->unget();
			break;
		}

		// otherwise, traverse through DFSM table while reading rest of lexeme
		if (isalpha(buf)) {  // letters
			current_state = DFSM_id_table[current_state][0];
		}
		else if (isdigit(buf)) {  // digits
			current_state = DFSM_id_table[current_state][1];
		}
		else if (buf == '_') {  // underscores
			current_state = DFSM_id_table[current_state][2];
		}
		else {  // unaccepted characters
			current_state = DFSM_id_table[current_state][3];
		}

		// record entire lexeme
		lexeme += buf;
	}

	// check to see if token ended on an accepting state (not 4)
	if (current_state != 4) {
		lexeme_value lower_lexeme = "";
		for (int i = 0; i < lexeme.size(); i++) {
			if (isupper(lexeme[i])) {
				lower_lexeme += tolower(lexeme[i]);
			}
			else {
				lower_lexeme += lexeme[i];
			}
		}

		// if the lexeme already exists in SYM, it is a keyword
		if (SYM.find(lower_lexeme) != SYM.end()) {
			return { "keyword", lexeme };
		}
		// otherwise, it is an identifier
		else {
			return { "identifier", lexeme };
		}
	}
	else {  // invalid state
		return { "ERROR", lexeme + " is an invalid identifier name" };
	}
}

/*****************************************************************************
| The DFSM_int_real function checks if a token is a valid int or real. If it |
| is, the DFSM will return its type (int/real) and its lexeme value. If not, |
| the DFSM will return the "ERROR" token type, which will let the main       |
| function know to stop running.                                             |
*****************************************************************************/
Token Lexer::DFSM_int_real() {
	// reset reader to get entire lexeme
	ifs->unget();
	lexeme_value lexeme = "";

	// use the DFSM_int_real transition table when reading the token
	// if it ends on a valid state, then the token is a valid int/real
	DFSM_state current_state = 0;
	while (true) {
		char buf = ifs->get();
		// stop when whitespaces/operators/separators/EOF are reached
		if (is_white_space(buf) || SYM.find(std::string(1, buf)) != SYM.end()
			|| buf == '!' || !ifs->good()) {  // ! is for the != operator

			// reset reader in case operators/separators are next
			ifs->unget();
			break;
		}

		// otherwise, traverse through DFSM table while reading rest of lexeme
		if (isdigit(buf)) {  // digits
			current_state = DFSM_int_real_table[current_state][0];
		}
		else if (buf == '.') {  // decimals
			current_state = DFSM_int_real_table[current_state][1];
		}
		else {  // unaccepted characters
			current_state = DFSM_int_real_table[current_state][2];
		}

		// record entire lexeme
		lexeme += buf;
	}

	// check to see if token ended on an accepting state
	if (current_state == 0) {  // int
		return { "integer", lexeme };
	}
	else if (current_state == 2) {  // real
		return { "real", lexeme };
	}
	else {  // invalid state
		return { "ERROR", lexeme + " is an invalid integer/real value." };
	}
}

/****************************************************************************
| If a token begins with =, it can either be the =, ==, or => operator.     |
| This function will look at the token's next character to see if it is     |
| either == or =>. If it is neither, then the token will be the = operator. |
****************************************************************************/
Token Lexer::check_operator_equals() {
	char buf = ifs->get();  // get next character and check it
	if (buf == '=') {
		return { "operator", "==" };
	}
	else if (buf == '>') {
		return { "operator", "=>" };
	}
	else {
		ifs->unget();  // reset the reader since = is only one character
		return { "operator", "=" };
	}
}

/****************************************************************************
| If a token begins with <, it can either be the < or <= operator.          |
| This function will look at the token's next character to see if it is     |
| <=. If not, then the token will be the < operator.                        |
****************************************************************************/
Token Lexer::check_operator_less_than() {
	char buf = ifs->get();  // get next character and check it
	if (buf == '=') {
		return { "operator", "<=" };
	}
	else {
		ifs->unget();  // reset the reader since < is only one character
		return { "operator", "<" };
	}
}

/******************************************************************************
| If the lexer function comes across a [, then the program has to check       |
| if the [ is being properly used for a comment. All comments begin with "[*" |
| and end with "*]". This function, check_comment, makes sure that all        |
| comments follow this format. If a comment is not used properly, then this   |
| function will return the "ERROR" token type, which will let the main        |
| function know to stop running.                                              |
******************************************************************************/
Token Lexer::check_comment() {
	char buf = ifs->get();
	if (!ifs->good()) {  // file ends before comment is closed with **]
		return { "ERROR", "Unclosed comment." };
	}
	else if (buf == '*') {
		buf = ifs->get();
		while (ifs->good()) {  // keep reading until EOF or comment ends
			if (buf == '*') {
				buf = ifs->get();
				if (!ifs->good()) {
					// file ended before comment was closed with "*]"
					return { "ERROR", "Unclosed comment." };
				}
				else if (buf == ']') {  // file reaches "*]" -> comment ends
					return { "comment", "" };
				}
				ifs->unget();  // if something else (like *a *b), reset reader
			}
			else if (buf == '\n') {
				line_number++;  // keep track of line number
			}
			buf = ifs->get();  // read the comment's next character
		}
		// file ended before comment was closed with "*]"
		return { "ERROR", "Unclosed comment." };
	}
	else {  // comment does not start with "[*" (and [ is not a valid token)
		return { "ERROR", "[ is an unrecognized symbol" };
	}
}

/***********************************************************************
| The is_white_space function will return true whenever a character is |
| a whitespace (either a space, tab, or new line).                     |
***********************************************************************/
bool Lexer::is_white_space(char c) {
	if (c == '\n') {
		line_number++;
	}
	return c == ' ' || c == '\t' || c == '\v' || c == '\n';
}

/****************************************************************************
| The initialize_sym_table function will add the operators, separators, and |
| keywords from the RAT23S programming language to the SYM table. The SYM   |
| table is used for lookups of existing tokens (ie. existing keywords)      |
****************************************************************************/
void Lexer::initialize_sym_table(Symbol_table& table) {
	table["function"] = "keyword";
	table["int"] = "keyword";
	table["bool"] = "keyword";
	table["real"] = "keyword";
	table["if"] = "keyword";
	table["fi"] = "keyword";
	table["else"] = "keyword";
	table["return"] = "keyword";
	table["put"] = "keyword";
	table["get"] = "keyword";
	table["while"] = "keyword";
	table["endwhile"] = "keyword";
	table["true"] = "keyword";
	table["false"] = "keyword";

	table["="] = "operator";
	table["+"] = "operator";
	table["-"] = "operator";
	table["*"] = "operator";
	table["/"] = "operator";
	table["=="] = "operator";
	table["!="] = "operator";
	table[">"] = "operator";
	table["<"] = "operator";
	table["<="] = "operator";
	table["=>"] = "operator";

	table["("] = "separator";
	table[")"] = "separator";
	table["{"] = "separator";
	table["}"] = "separator";
	table[";"] = "separator";
	table["#"] = "separator";
	table[","] = "separator";
}

// returns where the lexer is currently pointing to (line number)
int Lexer::get_line_number() {
	return line_number;
}

/******************************************************************************
| Checks if a file passes the Lexical Analysis phase. Returns 0 when the file |
| is valid, returns -1 when it is not.                                        |
******************************************************************************/
int Lexer::Analyze() {
	// peek at next character
	while (ifs->get() && ifs->good()) {  // if EOF reached, stop reading
		ifs->unget();
		Token token = get_token();
		token_type type = token.first;

		// if token is invalid, return error code (-1)
		if (type == "ERROR") {
			return -1;
		}
	}

	// Analysis successful (close ifs)
	close_ifs();
	return 0;
}

// close the input file stream (for syntax analyzer's lexer too)
void Lexer::close_ifs() {
	ifs->close();
}