/* ------------------------------- LIBRARIES ------------------------------- */
#include <ctype.h>  // isalpha()  isdigit()
#include <fstream>  // input file
#include <iostream>
#include <map>  // symbol table (SYM)
#include <string>
#include <cstring>  // tolower()
#include <utility>  // token_type and lexeme_value pairs (ie. records)
#include <vector>  // Lexeme_list


/* ------------------- DEFINE STATEMENTS FOR STRUCTURES ------------------- */
typedef std::string token_type;
typedef std::string lexeme_value;
typedef int DFSM_state;

// SYM table format is { key = lexeme_value, value = token_type }
// it is used to store the list of exisiting tokens (ie. ops, seps, keywords)
typedef std::map <lexeme_value, token_type> Symbol_table;

// Records use format { token_type, lexeme_value } -> printed in output file
typedef std::pair <token_type, lexeme_value> Record;

// main function stores a list of Records (extracted from input file)
// the list is printed onto an output file
typedef std::vector <Record> Record_list;


/* -------------------- GLOBAL VARIABLES AND STRUCTURES -------------------- */
// Contains a list of all tokens (operators, separators, keywords, etc.)
Symbol_table SYM;

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

// Note: The DFSM tables are a little different from the ones in documentation
//       due to optimizations (ie. starting state already checked in lexer())


/* -------------------------- FUNCTION PROTOTYPES -------------------------- */
Record lexer(std::ifstream& ifs);  // extract token from input file
Record DFSM_identifier(std::ifstream& ifs);  // validate id/keyword
Record DFSM_int_real(std::ifstream& ifs);  // validate int/real
Record check_operator_equals(std::ifstream& ifs);  // differentiate = == and =>
Record check_operator_less_than(std::ifstream& ifs);  // differentiate < and <=
Record check_comment(std::ifstream& ifs);  // see if comment is closed

bool is_white_space(char c);  // used to skip over white spaces in while loops
void initialize_sym_table(Symbol_table& table);  // initialize the SYM table



/******************************************************************************
| The main function asks the user to enter the name of an input file. If the  |
| file exists in the same directory, the main function will open it. Then, it |
| will keep calling lexer() to extract tokens from the input file.            |
| Each token will have its token type and lexeme value printed onto an output |
| file chosen by the user. Once the file reader reaches the end of the input  |
| file, the main function will stop calling lexer() and exit the program.     |
******************************************************************************/
int main() {
	// get input file
	std::cout << "Enter the name of an input text file: ";
	std::string input_file_name;
	std::cin >> input_file_name;
	std::ifstream ifs;
	ifs.open(input_file_name);

	// check if input file exists
	if (!ifs.is_open()) {
		std::cout << "ERROR: Couldn't open file '" << input_file_name << "'\n";
		system("pause");
		return -1;
	}

	// get output file
	std::cout << "Enter the name of the output file you want to create/edit: ";
	std::string output_file_name;
	std::cin >> output_file_name;
	std::ofstream ofs;
	ofs.open(output_file_name);

	// check if output file was successfully opened/created
	if (!ofs.is_open()) {
		std::cout << "ERROR: Couldn't create/edit file '" << output_file_name << "'\n";
		system("pause");
		return -1;
	}
	std::cout << "\n";


	// initialize the SYM table (add operators, separators, keywords)
	initialize_sym_table(SYM);

	// store the list of tokens extracted from the input file
	Record_list List_of_tokens;

	// keep extracting tokens from input file using lexer() until EOF reached
	// the condition peeks at the next character to see if EOF will be reached
	while (ifs.get() && ifs.good()) {
		ifs.unget();  // if not, reset the reader (move back) and call lexer()
		Record token = lexer(ifs);

		// if token is invalid, print ERROR message
		if (token.first == "ERROR") {
			std::cout << "ERROR: " << token.second << "\n";
			system("pause");
			return -1;
		}

		// if lexer() reaches the EOF or a comment, don't add it to the list
		else if (token.first != "comment" && token.first != "EOF") {
			// otherwise, add the valid token to the list
			List_of_tokens.push_back(token);
		}
	}


	// print the list of tokens onto the output file
	ofs << "Token\t\tLexeme\n";
	for (int pair = 0; pair < List_of_tokens.size(); pair++) {
		token_type type = List_of_tokens.at(pair).first;
		lexeme_value value = List_of_tokens.at(pair).second;

		// formatting output (use more tabs for smaller token type names)
		if (type == "keyword" || type == "integer" || type == "real") {
			ofs << type << "\t\t" << value << "\n";
		}
		else {
			ofs << type << "\t" << value << "\n";
		}
	}

	// close files
	ifs.close();
	ofs.close();
	return 0;
}


/*****************************************************************************
| The lexer function extracts tokens from the input file ifs. First, it      |
| skips over any white spaces (spacebar, tab, newline). Then, it may call    |
| helper functions to see if the current token is valid. If it is, it will   |
| return the token type and the lexeme value. If not, it will return the     |
| "ERROR" token type, which will let the main function know to stop running. |
*****************************************************************************/
Record lexer(std::ifstream& ifs) {
	char buf = ifs.get();

	// skip white spaces
	while (is_white_space(buf)) {
		buf = ifs.get();
		if (!ifs.good()) {
			return { "EOF", "" };  // EOF reached
		}
	}

	// first character is a letter -> check if token is ID or keyword
	if (isalpha(buf)) {
		return DFSM_identifier(ifs);  // use DFSM_id
	}

	// first character is a number -> check if token is int or real
	else if (isdigit(buf)) {
		return DFSM_int_real(ifs);  // use DFSM_int_real
	}

	// if first char is =, token could either be =, ==, or =>
	// use check_operator_equals() to determine which one it is
	else if (buf == '=') {
		return check_operator_equals(ifs);
	}

	// same case as =, but for < and <=
	else if (buf == '<') {
		return check_operator_less_than(ifs);
	}

	// the only time ! can be used is for !=. otherwise, return ERROR msg
	else if (buf == '!') {
		buf = ifs.get();
		if (buf == '=') {
			return { "operator", "!=" };
		}
		return { "ERROR", "! is an unrecognized symbol." };
	}

	// the only time [ can be used is for comments. use check_comment()
	// to see if [**] is used properly
	else if (buf == '[') {
		return check_comment(ifs);
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
Record DFSM_identifier(std::ifstream& ifs) {
	// reset reader to get entire lexeme
	ifs.unget();
	lexeme_value lexeme = "";

	// use the DFSM_id transition table when reading the token.
	// if it ends on a valid state, then the token is a valid id/keyword
	DFSM_state current_state = 0;
	while (true) {
		char buf = ifs.get();
		// stop when whitespaces/operators/separators/EOF are reached
		if (is_white_space(buf) || SYM.find(std::string(1, buf)) != SYM.end()
			|| buf == '!' || !ifs.good()) {  // ! is for the != operator

			// reset reader in case operators/separators are next
			ifs.unget();
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
Record DFSM_int_real(std::ifstream& ifs) {
	// reset reader to get entire lexeme
	ifs.unget();
	lexeme_value lexeme = "";

	// use the DFSM_int_real transition table when reading the token
	// if it ends on a valid state, then the token is a valid int/real
	DFSM_state current_state = 0;
	while (true) {
		char buf = ifs.get();
		// stop when whitespaces/operators/separators/EOF are reached
		if (is_white_space(buf) || SYM.find(std::string(1, buf)) != SYM.end()
			|| buf == '!' || !ifs.good()) {  // ! is for the != operator

			// reset reader in case operators/separators are next
			ifs.unget();
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
Record check_operator_equals(std::ifstream& ifs) {
	char buf = ifs.get();  // get next character and check it
	if (buf == '=') {
		return { "operator", "==" };
	}
	else if (buf == '>') {
		return { "operator", "=>" };
	}
	else {
		ifs.unget();  // reset the reader since = is only one character
		return { "operator", "=" };
	}
}


/****************************************************************************
| If a token begins with <, it can either be the < or <= operator.          |
| This function will look at the token's next character to see if it is     |
| <=. If not, then the token will be the < operator.                        |
****************************************************************************/
Record check_operator_less_than(std::ifstream& ifs) {
	char buf = ifs.get();  // get next character and check it
	if (buf == '=') {
		return { "operator", "<=" };
	}
	else {
		ifs.unget();  // reset the reader since < is only one character
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
Record check_comment(std::ifstream& ifs) {
	char buf = ifs.get();
	if (!ifs.good()) {  // file ends before comment is closed with **]
		return { "ERROR", "Unclosed comment." };
	}
	else if (buf == '*') {
		buf = ifs.get();
		while (ifs.good()) {  // keep reading until EOF or comment ends
			if (buf == '*') {
				buf = ifs.get();
				if (!ifs.good()) {
					// file ended before comment was closed with "*]"
					return { "ERROR", "Unclosed comment."};
				}
				else if (buf == ']') {  // file reaches "*]" -> comment ends
					return { "comment", "" };
				}
				ifs.unget();  // if something else (like *a *b), reset reader
			}
			buf = ifs.get();  // read the comment's next character
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
bool is_white_space(char c) {
	return c == ' ' || c == '\t' || c == '\v' || c == '\n';
}


/****************************************************************************
| The initialize_sym_table function will add the operators, separators, and |
| keywords from the RAT23S programming language to the SYM table. The SYM   |
| table is used for lookups of existing tokens (ie. existing keywords)      |
****************************************************************************/
void initialize_sym_table(Symbol_table& table) {
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