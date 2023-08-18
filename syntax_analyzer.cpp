/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <cstring>  // tolower
#include <string>  // substring

#include "lexer.h"
#include "syntax_analyzer.h"

/*****************************************************************************
| The constructor creates a lexer (to get tokens from the input file) using  |
| the given input file stream. It also initializes ofs with the given output |
| file stream.                                                               |
*****************************************************************************/
Syntax_Analyzer::Syntax_Analyzer(std::ifstream* input_file_stream,
								 std::ofstream* output_file_stream) {
	lexer = new Lexer(input_file_stream);
	ofs = output_file_stream;
}

/******************************************************************************
| Rat23S(), the "main" method of the Syntax Analyzer, represents the starting |
| production <Rat23S>. It essentially reads the entire input file and checks  |
| whether it passes the Syntax Analysis phase or not. When the program        |
| finishes (either passing or failing), it closes all file streams. Each      |
| production rule has its own function.                                       |
******************************************************************************/
void Syntax_Analyzer::Rat23S() {
	// add <Rat23S> to list of productions
	Productions.push_back("\t<Rat23S> -> <Opt Function Definitions> #"
						  " <Opt Declaration List> # <Statement List Start>");

	// <Rat23S> -> <Opt Function Definitions> # <Opt Declaration List> #
	//             <Statement List> $
	Opt_Function_Definitions();

	try { check_symbol("#"); }
	catch (int err) {
		print_error("Missing '#' or 'function' between optional function"
					" definitions and an optional declaration list");
	}

	Opt_Declaration_List();

	try { check_symbol("#"); }
	catch (int err) {
		print_error("Missing '#' between an optional declaration list and the"
					" list of program statements (main body)");
	}

	try { Statement_List_Start(); }
	catch (int err) {
		print_error("Missing statement(s) for program's main body");
	}

	try { check_symbol("EOF"); }
	catch (int err) {
		print_error("File should reach end after main body's statements");
	}

	// close file streams
	lexer->close_ifs();
	ofs->close();
}

/*****************************************************************************
| A production rule's function calls the function(s) of the other production |
| rule(s) used within it. In this case, since <Opt Function Definitions> ->  |
| <Function Definitions Start>, the function Opt_Function_Definitions() will |
| call the function Function_Definitions_Start().                            |
*****************************************************************************/
void Syntax_Analyzer::Opt_Function_Definitions() {
	// when a production has multiple rules (ie. E -> A | B | C), an
	// initial_list is created to save the list of productions used up to
	// this point. When one case doesn't work (ie. E -> A), the
	// initial_list will restore the list of Productions to its starting
	// point (for backtracking and testing the other cases).
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Opt Function Definitions> -> <Function Definitions Start>
	try {  // add Case 1 to list of productions and try it
		Productions.push_back("\t<Opt Function Definitions> ->"
							  " <Function Definitions Start>");
		Function_Definitions_Start();
		return;
	}
	catch (int err) {  // if Case 1 didn't work -> revert to initial_list
		copy_list(initial_list, Productions);
	}
	
	// Case 2: <Opt Function Definitions> -> <Empty>
	Productions.push_back("\t<Opt Function Definitions> -> <Empty>");
}


void Syntax_Analyzer::Function_Definitions_Start() {
	Productions.push_back("\t<Function Definitions Start> -> <Function>"
						  " <Function Definitions Cont>");

	// <Function Definitions Start> will try to use <Function>. If Function()
	// doesn't work, it will throw an error, and this error will be caught.
	// If <Function> cannot be used, <Function Definitions Start> will not
	// work either. Therefore, it will also throw an error to the
	// function that calls it. (This try-catch "algorithm" is present
	// for a lot of productions involved in backtracking).
	try { Function(); }
	catch (int err) { throw -1; }

	Function_Definitions_Cont();
	// some production rules have functions
}


void Syntax_Analyzer::Function_Definitions_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Function Definitions Cont> -> <Function Definitions Start>
	try {
		Productions.push_back("\t<Function Definitions Cont> ->"
							  " <Function Definitions Start>");
		Function_Definitions_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Function Definitions Cont> -> <Empty>
	Productions.push_back("\t<Function Definitions Cont> -> <Empty>");
}


void Syntax_Analyzer::Function() {
	// if 'function' is not present, then <Function> will not be used in the
	// list of productions (throw exception -1)
	try {
		Productions.push_back("\t<Function> -> function <Identifier> ("
							  " <Opt Parameter List> ) <Opt Declaration List>"
							  " <Body>");
		check_symbol("function");
	}
	catch (int err) { throw -1; }

	// however, if 'function' IS present, then <Function> will be used.
	// Therefore, the rest of the production MUST work (or else <Function>
	// will have improper syntax, and the Syntax Analysis will fail):
	// in this case, <Identifier>, '(', <Opt Parameter List>, ')',
	// <Opt Declaration List>, and <Body> must come after 'function'
	try { check_symbol("<identifier>"); }
	catch (int err) {
		// if something is missing/out of place, an exception will be caught
		// and an error message will be printed
		print_error("Missing identifier: function needs a name");
	}

	try { check_symbol("("); }
	catch (int err) {
		print_error("Missing '(' for function's parameters");
	}

	Opt_Parameter_List();

	try { check_symbol(")"); }
	catch (int err) {
		print_error("Missing identifier(s) or ')' for function's parameters");
	}

	Opt_Declaration_List();
	Body();
	// note: some functions do not throw exceptions because they will either
	// work all the time (such as with production rules that use <Empty>), OR
	// the function call will handle the error. For example, Body() accounts
	// for all of its own exceptions errors
}


void Syntax_Analyzer::Opt_Parameter_List() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Opt Parameter List> -> <Parameter List Start>
		Productions.push_back("\t<Opt Parameter List> ->"
							  " <Parameter List Start>");
		Parameter_List_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Opt Parameter List> -> <Empty>
	Productions.push_back("\t<Opt Parameter List> -> <Empty>");
}


void Syntax_Analyzer::Parameter_List_Start() {
	Productions.push_back("\t<Parameter List Start> -> <Parameter>"
						  " <Parameter List Cont>");

	try { Parameter(); }
	catch (int err) { throw -1; }

	Parameter_List_Cont();
}


void Syntax_Analyzer::Parameter_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Parameter List Cont> -> , <Parameter List Start>
		Productions.push_back("\t<Parameter List Cont> -> ,"
							  " <Parameter List Start>");
		check_symbol(",");
		try { Parameter_List_Start(); }
		catch (int err) {
			print_error("Missing parameter(s) after ',' in parameter list");
		}
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Parameter List Cont> -> <Empty>
	Productions.push_back("\t<Parameter List Cont> -> <Empty>");
}


void Syntax_Analyzer::Parameter() {
	Productions.push_back("\t<Parameter> -> <IDs Start> <Qualifier>");

	try { IDs_Start(); }
	catch (int err) { throw -1; }

	try { Qualifier(); }
	catch (int err) {
		print_error("Parameter(s) missing qualifier: 'int', 'bool', or 'real'");
	}
}


void Syntax_Analyzer::Qualifier() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Qualifier> -> int
		Productions.push_back("\t<Qualifier> -> int");
		check_symbol("int");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Qualifier> -> bool
		Productions.push_back("\t<Qualifier> -> bool");
		check_symbol("bool");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 3: <Qualifier> -> real
		Productions.push_back("\t<Qualifier> -> real");
		check_symbol("real");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<Qualifier> -> int | bool | real");
	throw -1;
}


void Syntax_Analyzer::Body() {
	Productions.push_back("\t<Body> -> { <Statement List Start> }");

	try { check_symbol("{"); }
	catch (int err) {
		print_error("Missing '{' for beginning of function's body");
	}

	try { Statement_List_Start(); }
	catch (int err) {
		print_error("Function body does not have any statements");
	}

	try { check_symbol("}"); }
	catch (int err) {
		print_error("Missing '}' for ending of function's body");
	}

	// note: no exceptions are thrown because Body() is always expected to work.
	// some functions follow this behavior (such as If_Cont and Return_Cont)
}


void Syntax_Analyzer::Opt_Declaration_List() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Opt Declaration List> -> <Declaration List Start>
		Productions.push_back("\t<Opt Declaration List> ->"
							  " <Declaration List Start>");
		Declaration_List_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Opt Declaration List> -> <Empty>
	Productions.push_back("\t<Opt Declaration List> -> <Empty>");
}


void Syntax_Analyzer::Declaration_List_Start() {
	Productions.push_back("\t<Declaration List Start> -> <Declaration> ;"
						  " <Declaration List Cont>");

	try { Declaration(); }
	catch (int err) { throw -1; }

	try { check_symbol(";"); }
	catch (int err) {
		print_error("Missing ';' at end of declaration");
	}

	Declaration_List_Cont();
}


void Syntax_Analyzer::Declaration_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Declaration List Cont> -> <Declaration List Start>
		Productions.push_back("\t<Declaration List Cont> ->"
							  " <Declaration List Start>");
		Declaration_List_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Declaration List Cont> -> <Empty>
	Productions.push_back("\t<Declaration List Cont> -> <Empty>");
}


void Syntax_Analyzer::Declaration() {
	Productions.push_back("\t<Declaration> -> <Qualifier> <IDs Start>");

	try { Qualifier(); }
	catch (int err) { throw -1; }

	try { IDs_Start(); }
	catch (int err) {
		print_error("Missing identifier(s) in declaration (after qualifier)");
	}
}


void Syntax_Analyzer::IDs_Start() {
	Productions.push_back("\t<IDs Start> -> <Identifer> <IDs Cont>");

	try { check_symbol("<identifier>"); }
	catch (int err) {
		throw -1;
	}

	IDs_Cont();
}


void Syntax_Analyzer::IDs_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <IDs Cont> -> , <IDs Start>
		Productions.push_back("\t<IDs Cont> -> , <IDs Start>");
		check_symbol(",");
		try { IDs_Start(); }
		catch (int err) {
			print_error("Missing identifier(s) after ','");
		}
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <IDs Cont> -> <Empty>
	Productions.push_back("\t<IDs Cont> -> <Empty>");
}


void Syntax_Analyzer::Statement_List_Start() {
	Productions.push_back("\t<Statement List Start> -> <Statement>"
						  " <Statement List Cont>");

	try { Statement(); }
	catch (int err) { throw -1; }

	Statement_List_Cont();
}


void Syntax_Analyzer::Statement_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Statement List Cont> -> <Statement List Start>
		Productions.push_back("\t<Statement List Cont> ->"
							  " <Statement List Start>");
		Statement_List_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Statement List Cont> -> <Empty>
	Productions.push_back("\t<Statement List Cont> -> <Empty>");
}


void Syntax_Analyzer::Statement() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Statement> -> <Compound>
		Productions.push_back("\t<Statement> -> <Compound>");
		Compound();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Statement> -> <Assign>
		Productions.push_back("\t<Statement> -> <Assign>");
		Assign();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 3: <Statement> -> <If Start>
		Productions.push_back("\t<Statement> -> <If Start>");
		If_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 4: <Statement> -> <Return Start>
		Productions.push_back("\t<Statement> -> <Return Start>");
		Return_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 5: <Statement> -> <Print>
		Productions.push_back("\t<Statement> -> <Print>");
		Print();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 6: <Statement> -> <Scan>
		Productions.push_back("\t<Statement> -> <Scan>");
		Scan();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 7: <Statement> -> <While>
		Productions.push_back("\t<Statement> -> <While>");
		While();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<Statement> -> <Compound> | <Assign> |"
						  " <If Start> | <Return Start> | <Print> | <Scan>"
						  " | <While>");
	throw -1;
}


void Syntax_Analyzer::Compound() {
	Productions.push_back("\t<Compound> -> { <Statement List Start> }");

	try { check_symbol("{"); }
	catch (int err) { throw -1; }

	try { Statement_List_Start(); }
	catch (int err) {
		print_error("Compound statement is missing inside statement(s)");
	}

	try { check_symbol("}"); }
	catch (int err) {
		print_error("Missing '}' at end of Compound statement");
	}
}


void Syntax_Analyzer::Assign() {
	Productions.push_back("\t<Assign> -> <Identifier> = <Expression Start> ;");

	try { check_symbol("<identifier>"); }
	catch (int err) { throw -1; }

	try { check_symbol("="); }
	catch (int err) {
		print_error("Missing '=' for assign statement");
	}

	try { Expression_Start(); }
	catch (int err) {
		print_error("Missing expression for assign statement");
	}

	try { check_symbol(";"); }
	catch (int err) {
		print_error("Missing ';' at end of assign statement");
	}
}


void Syntax_Analyzer::If_Start() {
	Productions.push_back("\t<If Start> -> if ( <Condition> ) <Statement>"
						  " <If Cont>");

	try { check_symbol("if"); }
	catch (int err) {
		throw -1;
	}

	try { check_symbol("("); }
	catch (int err) {
		print_error("Missing '(' before condition of if statement");
	}

	Condition();

	try { check_symbol(")"); }
	catch (int err) {
		print_error("Missing ')' after condition of if statement");
	}

	try { Statement(); }
	catch (int err) {
		print_error("Missing statement for satisfied if condition");
	}

	If_Cont();
}


void Syntax_Analyzer::If_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <If Cont> -> else <Statement> fi
		Productions.push_back("\t<If Cont> -> else <Statement> fi");
		check_symbol("else");
		try { Statement(); }
		catch (int err) {
			print_error("Missing statement for"
						" satisfied else condition of if statement");
		}
		try { check_symbol("fi"); }
		catch (int err) {
			print_error("Missing 'fi' at end of if statement");
		}
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <If Cont> -> fi
		Productions.push_back("\t<If Cont> -> fi");
		check_symbol("fi");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<If Cont> -> fi | else <Statement> fi");
	print_error("if statement is missing 'fi' or 'else' statement 'fi' at end");
}


void Syntax_Analyzer::Return_Start() {
	Productions.push_back("\t<Return Start> -> return <Return Cont>");

	try { check_symbol("return"); }
	catch (int err) { throw -1; }

	Return_Cont();
}


void Syntax_Analyzer::Return_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Return Cont> -> <Expression Start> ;
		Productions.push_back("\t<Return Cont> -> <Expression Start> ;");
		Expression_Start();
		try { check_symbol(";"); }
		catch (int err) {
			print_error("Missing ';' at end of return statement's expression");
		}
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Return Cont> -> ;
		Productions.push_back("\t<Return Cont> -> ;");
		check_symbol(";");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}
	
	// No matches
	Productions.push_back("\t<Return Cont> -> <Expression Start> ; | ;");
	print_error("Missing ';' or expression and ';' at end of return statement");
}


void Syntax_Analyzer::Print() {
	Productions.push_back("\t<Print> -> put ( <Expression Start> ) ;");

	try { check_symbol("put"); }
	catch (int err) { throw -1; }

	try { check_symbol("("); }
	catch (int err) {
		print_error("Missing '(' after 'put' of print statement");
	}

	try { Expression_Start(); }
	catch (int err) {
		print_error("Missing expression inside print statement");
	}

	try { check_symbol(")"); }
	catch (int err) {
		print_error("Missing ')' after expression of print statement");
	}

	try { check_symbol(";"); }
	catch (int err) {
		print_error("Missing ';' at end of print statement");
	}
}


void Syntax_Analyzer::Scan() {
	Productions.push_back("\t<Scan> -> get ( <IDs Start> ) ;");

	try { check_symbol("get"); }
	catch (int err) { throw -1; }

	try { check_symbol("("); }
	catch (int err) {
		print_error("Missing '(' after 'get' of scan statement");
	}

	try { IDs_Start(); }
	catch (int err) {
		print_error("Missing identifier(s) inside scan statement");
	}

	try { check_symbol(")"); }
	catch (int err) {
		print_error("Missing ')' after identifier(s) of scan statement");
	}

	try { check_symbol(";"); }
	catch (int err) {
		print_error("Missing ';' at end of scan statement");
	}
}


void Syntax_Analyzer::While() {
	Productions.push_back("\t<While> -> while ( <Condition> )"
						  " <Statement> endwhile");

	try { check_symbol("while"); }
	catch (int err) { throw -1; }

	try { check_symbol("("); }
	catch (int err) {
		print_error("Missing '(' before condition of while statement");
	}

	Condition();

	try { check_symbol(")"); }
	catch (int err) {
		print_error("Missing ')' after condition of while statement");
	}

	try { Statement(); }
	catch (int err) {
		print_error("Missing statement(s) inside of while loop");
	}

	try { check_symbol("endwhile"); }
	catch (int err) {
		print_error("Missing 'endwhile' at end of while statement");
	}
}


void Syntax_Analyzer::Condition() {
	Productions.push_back("\t<Condition> -> <Expression Start> <Relop>"
						  " <Expression Start>");

	try { Expression_Start(); }
	catch (int err) {
		print_error("Missing LHS expression for condition");
	}

	Relop();

	try { Expression_Start(); }
	catch (int err) {
		print_error("Missing RHS expression for condition");
	}
}


void Syntax_Analyzer::Relop() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: ==
		Productions.push_back("\t<Relop> -> ==");
		check_symbol("==");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: !=
		Productions.push_back("\t<Relop> -> !=");
		check_symbol("!=");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 3: >
		Productions.push_back("\t<Relop> -> >");
		check_symbol(">");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 4: <
		Productions.push_back("\t<Relop> -> <");
		check_symbol("<");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 5: <=
		Productions.push_back("\t<Relop> -> <=");
		check_symbol("<=");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 6: =>
		Productions.push_back("\t<Relop> -> =>");
		check_symbol("=>");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<Relop> -> == | != | > | < | <= | =>");
	print_error("Missing relational operator for condition");
}


void Syntax_Analyzer::Expression_Start() {
	Productions.push_back("\t<Expression Start> -> <Term Start> <Expression"
						  " Cont>");

	try { Term_Start(); }
	catch (int err) { throw -1; }

	Expression_Cont();
}


void Syntax_Analyzer::Expression_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Expression Cont> -> + <Term Start> <Expression Cont>
		Productions.push_back("\t<Expression Cont> -> + <Term Start>"
							  " <Expression Cont>");

		check_symbol("+");

		try { Term_Start(); }
		catch (int err) {
			print_error("Missing Term after '+'");
		}

		Expression_Cont();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Expression Cont> -> - <Term Start> <Expression Cont>
		Productions.push_back("\t<Expression Cont> -> - <Term Start>"
							  " <Expression Cont>");

		check_symbol("-");

		try { Term_Start(); }
		catch (int err) {
			print_error("Missing Term after '-'");
		}

		Expression_Cont();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 3: <Expression Cont> -> <Empty>
	Productions.push_back("\t<Expression Cont> -> <Empty>");
}


void Syntax_Analyzer::Term_Start() {
	Productions.push_back("\t<Term Start> -> <Factor> <Term Cont>");

	try { Factor(); }
	catch (int err) { throw -1; }

	Term_Cont();
}


void Syntax_Analyzer::Term_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Term Cont> -> * <Factor> <Term Cont>
		Productions.push_back("\t<Term Cont> -> * <Factor> <Term Cont>");

		check_symbol("*");

		try { Factor(); }
		catch (int err) {
			print_error("Missing Factor after '*'");
		}

		Term_Cont();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Term Cont> -> / <Factor> <Term Cont>
		Productions.push_back("\t<Term Cont> -> / <Factor> <Term Cont>");

		check_symbol("/");

		try { Factor(); }
		catch (int err) {
			print_error("Missing Factor after '/'");
		}

		Term_Cont();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 3: <Term Cont> -> <Empty>
	Productions.push_back("\t<Term Cont> -> <Empty>");
}


void Syntax_Analyzer::Factor() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Factor> -> - <Primary>
		Productions.push_back("\t<Factor> -> - <Primary Start>");

		check_symbol("-");

		try { Primary_Start(); }
		catch (int err) {
			print_error("Missing Primary expression after '-'");
		}

		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Factor> -> <Primary>
		Productions.push_back("\t<Factor> -> <Primary Start>");

		Primary_Start();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<Factor> -> - <Primary Start> | <Primary Start>");
	throw -1;
}


void Syntax_Analyzer::Primary_Start() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Primary Start> -> <Identifier> <Primary Cont>
		Productions.push_back("\t<Primary Start> -> <Identifier> <Primary"
							  " Cont>");

		check_symbol("<identifier>");
		Primary_Cont();
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 2: <Primary Start> -> <Integer>
		Productions.push_back("\t<Primary Start> -> <Integer>");

		check_symbol("<integer>");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 3: <Primary Start> -> ( <Expression Start> )
		Productions.push_back("\t<Primary Start> -> ( <Expression Start> )");
		check_symbol("(");

		try { Expression_Start(); }
		catch (int err) {
			print_error("Missing expression between parantheses");
		}

		try { check_symbol(")"); }
		catch (int err) {
			print_error("Missing ')' to close expression");
		}

		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 4: <Primary Start> -> <Real>
		Productions.push_back("\t<Primary Start> -> <Real>");

		check_symbol("<real>");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 5: <Primary Start> -> true
		Productions.push_back("\t<Primary Start> -> true");

		check_symbol("true");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	try {  // Case 6: <Primary Start> -> false
		Productions.push_back("\t<Primary Start> -> false");

		check_symbol("false");
		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// No matches
	Productions.push_back("\t<Primary Start> -> <Identifier> <Primary"
						  " Cont> | <Integer> | ( <Expression Start> ) |"
						  " <Real> | true | false");
	throw -1;
}


void Syntax_Analyzer::Primary_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	try {  // Case 1: <Primary Cont> -> ( <IDs Start> )
		Productions.push_back("\t<Primary Cont> -> ( <IDs Start> )");

		check_symbol("(");

		try { IDs_Start(); }
		catch (int err) {
			print_error("Missing identifier(s) for Primary function() call");
		}

		try { check_symbol(")"); }
		catch (int err) {
			print_error("Missing ')' for Primary function() call");
		}

		return;
	}
	catch (int err) {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Primary Cont> -> <Empty>
	Productions.push_back("\t<Primary Cont> -> <Empty>");
}

/*******************************************************************************
| check_symbol checks if the current token matches a terminal symbol (id, int, |
| real, keyword, sep, op). if it doesn't, it will throw an exception of -1.    |
| otherwise, if the current token matches the terminal symbol, then the token  |
| and the list of productions it uses will be printed.                         |
|                                                                              |
| in the event that the matched symbol appears on a new line, err_line_number  |
| is updated to = that new line's number. this update marks where the next     |
| token is expected to appear (and if it doesn't appear there, an error        |
| message will print the # of the line where it's expected to appear).         |
*******************************************************************************/
void Syntax_Analyzer::check_symbol(std::string symbol) {
	// if a token hasn't been read from the lexer yet, call get_token().
	// otherwise, STAY on the same (current) token - don't skip tokens!!!
	if (current_token.first == "") {
		while (true) {
			current_token = lexer->get_token();
			// when looking for new tokens, skip over comments
			if (current_token.first != "comment") {
				break;
			}
		}
	}

	// <Identifier>, <Integer>, or <Real>
	if (symbol == "<identifier>" || symbol == "<integer>"
		|| symbol == "<real>") {

		// if current token doesn't match expected symbol, throw exception
		if (current_token.first != symbol.substr(1, symbol.find(">") - 1)) {
			throw -1;
		}
		else {  // otherwise, print the token and its productions
			print_current_token();
			print_productions();

			// reset everything for next token
			current_token = { "", "" };
			Productions.clear();

			// update where next symbol is expected to appear (line #)
			err_line_number = lexer->get_line_number();
			return;
		}
	}
	// EOF
	else if (symbol == "EOF") {
		if (current_token.first != "EOF") {
			throw -1;
		}
		else {
			print_current_token();
			print_productions();
			current_token = { "", "" };
			Productions.clear();
			err_line_number = lexer->get_line_number();
			return;
		}
	}
	// keywords, operators, separators
	// (note: Rat23S is NOT case sensitive. i.e. both 'function' and 'FuNCtiOn'
	// are equivalent keywords. so, current symbol is converted to all lowercase
	// before checking)
	std::string lower_lexeme = convert_to_lowercase(current_token.second);
	if (lower_lexeme != symbol) {
		throw -1;
	}
	else {
		print_current_token();
		print_productions();
		current_token = { "", "" };
		Productions.clear();
		err_line_number = lexer->get_line_number();
		return;
	}
}

/*****************************************************************************
| This function makes the list 'copy' have the same contents as 'original'.  |
| It is used for backtracking (i.e. reverting the list of productions to its |
| former steps).                                                             |
*****************************************************************************/
void Syntax_Analyzer::copy_list(Rule_list original, Rule_list& copy) {
	copy.clear();
	for (int i = 0; i < original.size(); i++) {
		copy.push_back(original.at(i));
	}
}

/*******************************************************************************
| This function returns the lowercase version of 's'. It is called during      |
| symbol checks since Rat23S is not a case sensitive language (which means     |
| that COMPARISONS between symbols should be in the same case, i.e. lowercase) |
*******************************************************************************/
std::string Syntax_Analyzer::convert_to_lowercase(std::string s) {
	std::string lower = "";
	for (int i = 0; i < s.size(); i++) {
		lower += tolower(s[i]);
	}
	return lower;
}

// prints the current token onto the output file
void Syntax_Analyzer::print_current_token() {
	if (current_token.first == "") {
		return;
	}
	else if (current_token.first == "identifier" ||
		current_token.first == "separator") {
		*ofs << "Token: " << current_token.first << "\tLexeme: "
			<< current_token.second << "\n";
	}
	else {  // number of tabs adjusted based on token type length
		*ofs << "Token: " << current_token.first << "\t\tLexeme: "
			<< current_token.second << "\n";
	}
}

// prints the list of productions used by the current token onto the output file
void Syntax_Analyzer::print_productions() {
	for (int i = 0; i < Productions.size(); i++) {
		*ofs << Productions.at(i) << "\n";
	}
}

/*******************************************************************************
| This function prints the given error message (err_msg) onto the output file. |
| The format of an error message is the line number it occurred, its           |
| description, the list of productions leading to the error, and the           |
| unexpected token. Additionally, reaching an error means that the SA phase    |
| has failed. Thus, the input and output file streams are closed and the       |
| program exits with an error code of -1.                                      |
*******************************************************************************/
void Syntax_Analyzer::print_error(std::string err_msg) {
	*ofs << err_line_number << ": ERROR - " << err_msg << "\n";
	print_productions();
	*ofs << "\t";
	print_current_token();
	lexer->close_ifs();
	ofs->close();
	exit(-1);
}