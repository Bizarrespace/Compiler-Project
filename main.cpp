/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <iostream>  // console error messages
#include <string>

#include "lexer.h"  // lexer
#include "syntax_analyzer.h"  // syntax analyzer


/* -------------------------- FUNCTION PROTOTYPES -------------------------- */
void skip_BOM_encoding(std::ifstream& ifs);  // handles UTF-8 files with BOM



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
	// std::cin >> input_file_name;
	input_file_name = "bomba.txt";
	std::ifstream ifs;
	ifs.open(input_file_name);

	// check if input file exists
	if (!ifs.is_open()) {
		std::cout << "ERROR: Couldn't open file '" << input_file_name << "'\n";
		system("pause");
		return -1;
	}

	// if the input file uses UTF-8 with BOM, skip the BOM encoding
	skip_BOM_encoding(ifs);  // if not, the function will reset ifs to beginning


	// get output file
	std::cout << "Enter the name of the output file you want to create/edit: ";
	std::string output_file_name;
	// std::cin >> output_file_name;
	output_file_name = "output.txt";
	std::ofstream ofs;
	ofs.open(output_file_name);

	// check if output file was successfully opened/created
	if (!ofs.is_open()) {
		std::cout << "ERROR: Couldn't create/edit file '" << output_file_name
			      << "'\n";
		system("pause");
		return -1;
	}
	std::cout << "\n";


	// Lexical Analysis
	std::ifstream lexical_ifs;  // separate reader from SA phase
	lexical_ifs.open(input_file_name);
	skip_BOM_encoding(lexical_ifs);  // skip BOM then do LA phase
	Lexer lexical_analyzer(&lexical_ifs);
	if (lexical_analyzer.Analyze() != 0) {
		std::cout << "ERROR: File failed lexical analysis on line " <<
			         lexical_analyzer.get_line_number() << ".\n";
		ofs << "ERROR: File failed lexical analysis on line " <<
			   lexical_analyzer.get_line_number() << ".\n";
		system("pause");
		return -1;
	}

	// Syntax Analysis
	Syntax_Analyzer syntax_analyzer(&ifs, &ofs);
	syntax_analyzer.Rat23S();

	return 0;
}

/*******************************************************************************
| This helper function skips over the BOM encoding (first 3 bytes) of a UTF-8  |
| file that uses BOM. The encoding has to be overpassed because the .get()     |
| method of ifstream objects can't tell the difference between a byte used for |
| encoding vs. a byte used for text. And when the encoding bytes are read by   |
| the lexer (ie. when isalpha() or isdigit() are called), an error will occur. |
| Thus, this function will help skip over those encoding bytes (if present).   |
*******************************************************************************/
void skip_BOM_encoding(std::ifstream& ifs) {
	// a UTF-8 file's 3 beginning bytes indicate if it uses BOM
	if (ifs.get() == 0xEF) {  // byte 1
		if (ifs.get() == 0xBB) {  // byte 2
			if (ifs.get() == 0xBF) {  // byte 3
				return;  // all 3 bytes matched, exit
			}
			else {  // only first 2 bytes matched
				ifs.unget();
				ifs.unget();
				ifs.unget();  // reset the 3 bytes that were peeked at
			}
		}
		else {  // only first byte matched
			ifs.unget();
			ifs.unget();  // reset the 2 bytes that were peeked at
		}
	}
	else {  // no bytes matched
		ifs.unget();  // reset the 1 byte that was peeked at
	}
}