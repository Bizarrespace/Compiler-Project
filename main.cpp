/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <iostream>  // console error messages
#include <string>

#include "lexer.h"  // lexer
#include "syntax_analyzer.h"  // syntax analyzer


/*******************************************************************************
| The main file receives the names of the input and output files that the user |
| wants to use for syntax analysis of the Rat23S programming language. If the  |
| user enters a non-existing/invalid input/output file, an error message will  |
| be printed to the terminal. Otherwise, the file will go through syntax       |
| analysis. If it fails, an error message will be printed at the end of the    |
| output file. If it passes, the program will exit with code 0, and the output |
| file will have a list of all the productions used in the input file.         |
*******************************************************************************/
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


	// Lexical Analysis
	std::ifstream lexical_ifs;
	lexical_ifs.open(input_file_name);
	Lexer lexical_analyzer(&lexical_ifs);
	if (lexical_analyzer.Analyze() != 0) {
		std::cout << "ERROR: File failed lexical analysis.\n";
		system("pause");
		return -1;
	}

	// Syntax Analysis
	Syntax_Analyzer syntax_analyzer(&ifs, &ofs);
	syntax_analyzer.Rat23S();

	return 0;
}