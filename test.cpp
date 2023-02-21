#include <ctype.h>  // isalpha()  isdigit()
#include <fstream>  // input file
#include <iostream>  // I/O
#include <map>  // symbol table
#include <string>
#include <utility>  // token_type and lexem_value pairs
#include <vector>  // Lexeme_list


/* --------------- DEFINE STATEMENTS FOR STRUCTURES --------------- */
typedef std::string token_type;
typedef std::string lexeme_value;

// symbol table - format is {lexeme_value, token_type}
typedef std::map <lexeme_value, token_type> Symbol_table;

// records have format {token_type, lexeme_value} -> printed in output
typedef std::pair <token_type, lexeme_value> Record;

// store list of records (printed in output)
typedef std::vector <Record> Record_list;


/* --- GLOBAL VARIABLES AND STRUCTURES --- */
Symbol_table SYM;
// TODO: initialize SYM


/* --- FUNCTION PROTOTYPES --- */
// TODO: write prototypes
Record lexeme(std::ifstream& ifs);
Record DFSM_identifier(std::ifstream& ifs);
Record DFSM_int_real(std::ifstream& ifs);
Record check_operator_equals(std::ifstream& ifs);
Record check_operator_less_than(std::ifstream& ifs);
Record check_comment(std::ifstream& ifs);

bool is_white_space(char c);  // use '' instead of ""
void initialize_sym_table(Symbol_table& table);



int main() {
	
	// get input file
	std::cout << "Enter the name of an input text file:  ";
	std::string input_file_name;
	std::cin >> input_file_name;
	std::cout << "\n\n";
	std::ifstream ifs;
	ifs.open(input_file_name);

	// check if input file exists
	if (!ifs.is_open()) {
		std::cout << "ERROR: Couldn't open file '" << input_file_name << "'\n";
		return -1;
	}

	initialize_sym_table(SYM);

	Record_list List;
	while (ifs.get() && ifs.good()) {
		ifs.unget();
		Record r = lexeme(ifs);

		if (r.first == "ERROR") {  // invalid record
			std::cout << "ERROR: " << r.second << "\n";
			return -1;
		}
		else if (r.first == "EOF") {  // eof reached during lexeme()
			break;
		}
		else if (r.first != "comment") {
			List.push_back(r);
		}
	}
	std::cout << "Token\t\tLexeme\n";
	for (int pair = 0; pair < List.size(); pair++) {
		std::cout << List.at(pair).first << "\t" << List.at(pair).second << "\n";
	}
	std::cout << "\n";

	return 0;
}


Record lexeme(std::ifstream& ifs) {
	char buf = ifs.get();
	while (is_white_space(buf)) {  // skip white spaces
		buf = ifs.get();
		if (!ifs.good()) {
			return { "EOF", "" };  // EOF reached
		}
	}
	if (isalpha(buf)) {
		return DFSM_identifier(ifs);
	}
	else if (isdigit(buf)) {
		return DFSM_int_real(ifs);
	}
	else if (buf == '=') {
		return check_operator_equals(ifs);
	}
	else if (buf == '<') {
		return check_operator_less_than(ifs);
	}
	else if (buf == '!') {
		buf = ifs.get();
		if (buf == '=') {
			return { "operator", "!=" };
		}
		return { "ERROR", "Unrecognized symbol" };
	}
	else if (buf == '[') {
		return check_comment(ifs);
	}
	lexeme_value find(1, buf);
	if (SYM.find(find) != SYM.end()) {  // sym lookup for op/sep
		return { SYM.at(find), find };
	}
	else {
		return { "ERROR", "Unrecognized symbol" };
	}
}


Record DFSM_identifier(std::ifstream& ifs) {
	return { "DFSM_id", "" };
}


Record DFSM_int_real(std::ifstream& ifs) {
	return { "DFSM_int_real", "" };
}


Record check_operator_equals(std::ifstream& ifs) {
	char buf = ifs.get();
	if (buf == '=') {
		return { "operator", "==" };
	}
	else if (buf == '>') {
		return { "operator", "=>" };
	}
	else {
		ifs.unget();
		return { "operator", "=" };
	}
}


Record check_operator_less_than(std::ifstream& ifs) {
	char buf = ifs.get();
	if (buf == '=') {
		return { "operator", "<=" };
	}
	else {
		ifs.unget();
		return { "operator", "<" };
	}
}


Record check_comment(std::ifstream& ifs) {
	char buf = ifs.get();
	if (!ifs.good()) {
		return { "ERROR", "Unclosed comment" };
	}
	else if (buf == '*') {
		buf = ifs.get();
		while (ifs.good()) {
			if (buf == '*') {
				buf = ifs.get();
				if (!ifs.good()) {
					return { "ERROR", "Unclosed comment"};
				}
				else if (buf == ']') {
					return { "comment", "" };
				}
				ifs.unget();
			}
			buf = ifs.get();
		}
		return { "ERROR", "Unclosed comment" };
	}
	else {
		return { "ERROR", "Unclosed comment" };
	}
}


bool is_white_space(char c) {
	return c == ' ' || c == '\t' || c == '\v' || c == '\n';
}


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













// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file