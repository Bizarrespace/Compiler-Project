#pragma once

/* ------------------------------- LIBRARIES ------------------------------- */
#include <fstream>  // input file
#include <cstring>  // tolower

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
| finishes (either passing or failing), it closes all file streams.           |
******************************************************************************/
void Syntax_Analyzer::Rat23S() {
	// add <Rat23S> to list of productions
	Productions.push_back("\t<Rat23S> -> <Opt Function Definitions> #"
		                  " <Opt Declaration List> # <Statement List Start>");
	
	// <Rat23S> -> <Opt Function Definitions> # <Opt Declaration List> #
	// <Statement List> $
	Opt_Function_Definitions();
	check_symbol("#", 1);
	Opt_Declaration_List();
	check_symbol("#", 1);
	Statement_List_Start(1);
	check_symbol("EOF", 1);

	// close file streams
	lexer->close_ifs();
	ofs->close();
}

/******************************************************************************
| This function imitates the <Opt Function Definitions> production rule.      |
| It calls the functions of the other productions used in it                  |
| (ie. <Function Definitions> since                                           |
| <Opt Function Definitions> -> <Function Definitions>). All other production |
| functions (one for each production rule) follow a similar process.          |
******************************************************************************/
void Syntax_Analyzer::Opt_Function_Definitions() {
	// initial_list saves the current list of Productions up to this point
	// (where Opt_Function_Definitions is called). It is saved so that when
	// the SA backtracks (to try all possible rules), the list of Productions
	// won't include the unused rules. This "algorithm" is present for all
	// productions that have multiple cases (options/alternatives)
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Opt Function Definitions> -> <Function Definitions>
	Productions.push_back("\t<Opt Function Definitions> ->"
		                  " <Function Definitions>");

	// if the Case 1 production rule works (ie. returns 0), then you
	// can exit this function (return).
	if (Function_Definitions_Start(0) == 0) {
		return;
	}

	// if the case doesn't work, you have to revert the Productions list
	// to what it was originally when the function first started (backtrack).
	// then, you have to try all the other cases (and backtrack when necessary).
	// if none of the function's cases work, you have to print an error message
	// (unless the function is already part of another case testing process.
	// This "algorithm" is present for all productions that have multiple cases
	else {
		copy_list(initial_list, Productions);
	}


	// Case 2: <Opt Function Definitions> -> <Empty>
	Productions.push_back("\t<Opt Function Definitions> -> <Empty>");
	// note: Opt Function Definitions always works because it has the <Empty>
	// production rule (ie. can return nothing), so it doesn't need to return
	// an error code (which is why it's a void function). all other production
	// functions that can return <Empty> are also void.
}

// <Function Definitions Start>
int Syntax_Analyzer::Function_Definitions_Start(int exit_on_error) {
	Productions.push_back("\t<Function Definitions Start> -> <Function>"
		                  " <Function Definitions Cont>");

	// if <Function> doesn't work, and 'exit_on_error' is disabled ( == 0)
	// then <Function> will return the error code 1. since it doesn't work,
	// <Function Definitions Start> won't work either, so it also has to
	// return the error code 1. many other production functions follow
	// this test case (backtracking) process.

	// note: if exit_on_error is enabled ( == 1) AND the function that is
	// being called runs into an error, that function will be responsible
	// for ending the Syntax Analyzer.
	if (Function(exit_on_error) != 0) {
		return 1;
	}

	Function_Definitions_Cont();
	// the <Function Definitions Start> production rule was successful
	return 0;
}

// <Function Definitions Cont>
void Syntax_Analyzer::Function_Definitions_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Function Definitions Cont> -> <Function Definitions Start>
	Productions.push_back("\t<Function Definitions Cont> ->"
		                  " <Function Definitions Start>");

	if (Function_Definitions_Start(0) == 0) {
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Function Definitions Cont> -> <Empty>
	Productions.push_back("\t<Function Definitions Cont> -> <Empty>");
}

// <Function>
int Syntax_Analyzer::Function(int exit_on_error) {
	Productions.push_back("\t<Function> -> function <Identifier> ("
		                  " <Opt Parameter List> ) <Opt Declaration List>"
		                  " <Body>");

	// if check_symbol() didn't work, and exit_on_error is disabled,
	// return error code 1
	if (check_symbol("function", exit_on_error) != 0) {
		return 1;
	}

	// if it DID work, then the test case MUST follow through until completion
	// (ie. you can't have the keyword 'function' without (<Opt Parameter List>)
	// following it). since the test case has to follow through, exit_on_error
	// is enabled for all subsequent production rules/symbol checks. many other
	// production functions follow this "process."
	check_symbol("identifier", 1);  // enable exit_on_error by passing '1'
	check_symbol("(", 1);
	Opt_Parameter_List();
	check_symbol(")", 1);
	Opt_Declaration_List();
	Body();
	return 0;
}

// <Opt Parameter List>
void Syntax_Analyzer::Opt_Parameter_List() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Opt Parameter List> -> <Parameter List Start>
	Productions.push_back("\t<Opt Parameter List> ->"
		                  " <Parameter List Start>");

	if (Parameter_List_Start(0) == 0) {
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Opt Parameter List> -> <Empty>
	Productions.push_back("\t<Opt Parameter List> -> <Empty>");
}

// <Parameter List Start>
int Syntax_Analyzer::Parameter_List_Start(int exit_on_error) {
	Productions.push_back("\t<Parameter List Start> -> <Parameter>"
		                  " <Parameter List Cont>");

	if (Parameter(exit_on_error) != 0) {
		return 1;
	}

	Parameter_List_Cont();
	return 0;
}

// <Parameter List Cont>
void Syntax_Analyzer::Parameter_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Parameter List Cont> -> , <Parameter List Start>
	Productions.push_back("\t<Parameter List Cont> -> ,"
		                  " <Parameter List Start>");

	if (check_symbol(",", 0) == 0) {
		// if ',' matched, SA is expecting <Parameter List Start>
		// so exit_on_error has to be enabled
		Parameter_List_Start(1);
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Parameter List Cont> -> <Empty>
	Productions.push_back("\t<Parameter List Cont> -> <Empty>");
}

// <Parameter>
int Syntax_Analyzer::Parameter(int exit_on_error) {
	Productions.push_back("\t<Parameter> -> <IDs Start> <Qualifier>");

	if (IDs_Start(exit_on_error) != 0) {
		return 1;
	}

	Qualifier(1);
	return 0;
}

// <Qualifier>
int Syntax_Analyzer::Qualifier(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Qualifier> -> int
	Productions.push_back("\t<Qualifier> -> int");

	if (check_symbol("int", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Qualifier> -> bool
	Productions.push_back("\t<Qualifier> -> bool");

	if (check_symbol("bool", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 3: <Qualifier> -> real
	Productions.push_back("\t<Qualifier> -> real");

	if (check_symbol("real", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Qualifier> -> int | bool | real");
		print_error("expected qualifier");
	}
	return 1;
}

// <Body>
void Syntax_Analyzer::Body() {
	Productions.push_back("\t<Body> -> { <Statement List> }");

	// <Body> is always expected to work (ie. only called when a test case
	// matches and exit_on_error is enabled), so it doesn't need to return
	// any exit codes. some other production functions do this too.
	check_symbol("{", 1);
	Statement_List_Start(1);
	check_symbol("}", 1);
}

// <Opt Declaration List>
void Syntax_Analyzer::Opt_Declaration_List() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Opt Declaration List> -> <Declaration List Start>
	Productions.push_back("\t<Opt Declaration List> ->"
		                  " <Declaration List Start>");

	if (Declaration_List_Start(0) == 0) {
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Opt Declaration List> -> <Empty>
	Productions.push_back("\t<Opt Declaration List> -> <Empty>");
}

// <Declaration List Start>
int Syntax_Analyzer::Declaration_List_Start(int exit_on_error) {
	Productions.push_back("\t<Declaration List Start> -> <Declaration> ;"
		                  " <Declaration List Cont>");

	if (Declaration(exit_on_error) != 0) {
		return 1;
	}

	check_symbol(";", 1);
	Declaration_List_Cont();
	return 0;
}

// <Declaration List Cont>
void Syntax_Analyzer::Declaration_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Declaration List Cont> -> <Declaration List Start>
	Productions.push_back("\t<Declaration List Cont> ->"
		                  " <Declaration List Start>");
	
	if (Declaration_List_Start(0) == 0) {
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Declaration List Cont> -> <Empty>
	Productions.push_back("\t<Declaration List Cont> -> <Empty>");
}

// <Declaration>
int Syntax_Analyzer::Declaration(int exit_on_error) {
	Productions.push_back("\t<Declaration> -> <Qualifier> <IDs Start>");
	if (Qualifier(exit_on_error) != 0) {
		return 1;
	}

	IDs_Start(1);
	return 0;
}

// <IDs Start>
int Syntax_Analyzer::IDs_Start(int exit_on_error) {
	Productions.push_back("\t<IDs Start> -> <Identifer> <IDs Cont>");

	if (check_symbol("identifier", exit_on_error) != 0) {
		return 1;
	}

	IDs_Cont();
	return 0;
}

// <IDs Cont>
void Syntax_Analyzer::IDs_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <IDs Cont> -> , <IDs Start>
	Productions.push_back("\t<IDs Cont> -> , <IDs Start>");

	if (check_symbol(",", 0) == 0) {
		IDs_Start(1);
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <IDs Cont> -> <Empty>
	Productions.push_back("\t<IDs Cont> -> <Empty>");
}

// <Statement List Start>
int Syntax_Analyzer::Statement_List_Start(int exit_on_error) {
	Productions.push_back("\t<Statement List Start> -> <Statement>"
		                  " <Statement List Cont>");

	if (Statement(exit_on_error) != 0) {
		return 1;
	}

	Statement_List_Cont();
	return 0;
}

// <Statement List Cont>
void Syntax_Analyzer::Statement_List_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Statement List Cont> -> <Statement List Start>
	Productions.push_back("\t<Statement List Cont> ->"
		                  " <Statement List Start>");

	if (Statement_List_Start(0) == 0) {
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Statement List Cont> -> <Empty>
	Productions.push_back("\t<Statement List Cont> -> <Empty>");
}

// <Statement>
int Syntax_Analyzer::Statement(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Statement> -> <Compound>
	Productions.push_back("\t<Statement> -> <Compound>");

	if (Compound(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 2: <Statement> -> <Assign>
	Productions.push_back("\t<Statement> -> <Assign>");

	if (Assign(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 3: <Statement> -> <If Start>
	Productions.push_back("\t<Statement> -> <If Start>");

	if (If_Start(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 4: <Statement> -> <Return Start>
	Productions.push_back("\t<Statement> -> <Return Start>");

	if (Return_Start(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 5: <Statement> -> <Print>
	Productions.push_back("\t<Statement> -> <Print>");

	if (Print(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 6: <Statement> -> <Scan>
	Productions.push_back("\t<Statement> -> <Scan>");

	if (Scan(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 7: <Statement> -> <While>
	Productions.push_back("\t<Statement> -> <While>");

	if (While(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Statement> -> <Compound> | <Assign> |"
			                  " <If Start> | <Return Start> | <Print> | <Scan>"
			                  " | <While>");

		print_error("expected statement");
	}
	return 1;
}

// <Compound>
int Syntax_Analyzer::Compound(int exit_on_error) {
	Productions.push_back("\t<Compound> -> { <Statement List Start> }");

	if (check_symbol("{", exit_on_error) != 0) {
		return 1;
	}

	Statement_List_Start(1);
	check_symbol("}", 1);
	return 0;
}

// <Assign>
int Syntax_Analyzer::Assign(int exit_on_error) {
	Productions.push_back("\t<Assign> -> <Identifier> = <Expression Start>"
		                  " ;");

	if (check_symbol("identifier", exit_on_error) != 0) {
		return 1;
	}

	check_symbol("=", 1);
	Expression_Start(1);
	check_symbol(";", 1);
	return 0;
}

// <If Start>
int Syntax_Analyzer::If_Start(int exit_on_error) {
	Productions.push_back("\t<If Start> -> if ( <Condition> ) <Statement>"
		                  " <If Cont>");

	if (check_symbol("if", exit_on_error) != 0) {
		return 1;
	}

	check_symbol("(", 1);
	Condition(1);
	check_symbol(")", 1);
	Statement(1);
	If_Cont(1);
	return 0;
}

// <If Cont>
int Syntax_Analyzer::If_Cont(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <If Cont> -> else <Statement> fi
	Productions.push_back("\t<If Cont> -> else <Statement> fi");
	
	if (check_symbol("else", 0) == 0) {
		Statement(1);
		check_symbol("fi", 1);
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <If Cont> -> fi
	Productions.push_back("\t<If Cont> -> fi");
	
	if (check_symbol("fi", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<If Cont> -> fi | else <Statement> fi");
		print_error("expected 'fi' or 'else'");
	}
	return 1;
}

// <Return Start>
int Syntax_Analyzer::Return_Start(int exit_on_error) {
	Productions.push_back("\t<Return Start> -> return <Return Cont>");

	if (check_symbol("return", exit_on_error) != 0) {
		return 1;
	}

	Return_Cont(1);
	return 0;
}

// <Return Cont>
int Syntax_Analyzer::Return_Cont(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Return Cont> -> <Expression Start> ;
	Productions.push_back("\t<Return Cont> -> <Expression Start> ;");
	if (Expression_Start(0) == 0) {
		check_symbol(";", 1);
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Return Cont> -> ;
	Productions.push_back("\t<Return Cont> -> ;");
	if (check_symbol(";", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Return Cont> -> <Expression Start> ; | ;");
		print_error("expected expression or ';'");
	}
	return 1;
}

// <Print>
int Syntax_Analyzer::Print(int exit_on_error) {
	Productions.push_back("\t<Print> -> put ( <Expression Start> ) ;");

	if (check_symbol("put", exit_on_error) != 0) {
		return 1;
	}

	check_symbol("(", 1);
	Expression_Start(1);
	check_symbol(")", 1);
	check_symbol(";", 1);
	return 0;
}

// <Scan>
int Syntax_Analyzer::Scan(int exit_on_error) {
	Productions.push_back("\t<Scan> -> get ( <IDs Start> ) ;");

	if (check_symbol("get", exit_on_error) != 0) {
		return 1;
	}

	check_symbol("(", 1);
	IDs_Start(1);
	check_symbol(")", 1);
	check_symbol(";", 1);
	return 0;
}

// <While>
int Syntax_Analyzer::While(int exit_on_error) {
	Productions.push_back("\t<While> -> while ( <Condition> )"
		                  " <Statement> endwhile");

	if (check_symbol("while", exit_on_error) != 0) {
		return 1;
	}

	check_symbol("(", 1);
	Condition(1);
	check_symbol(")", 1);
	Statement(1);
	check_symbol("endwhile", 1);
	return 0;
}

// <Condition>
int Syntax_Analyzer::Condition(int exit_on_error) {
	Productions.push_back("\t<Condition> -> <Expression Start> <Relop>"
		                  " <Expression Start>");

	if (Expression_Start(exit_on_error) != 0) {
		return 1;
	}

	Relop(1);
	Expression_Start(1);
	return 0;
}

// <Relop>
int Syntax_Analyzer::Relop(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: ==
	Productions.push_back("\t<Relop> -> ==");

	if (check_symbol("==", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 2: !=
	Productions.push_back("\t<Relop> -> !=");

	if (check_symbol("!=", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 3: >
	Productions.push_back("\t<Relop> -> >");

	if (check_symbol(">", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 4: <
	Productions.push_back("\t<Relop> -> <");

	if (check_symbol("<", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 5: <=
	Productions.push_back("\t<Relop> -> <=");

	if (check_symbol("<=", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 6: =>
	Productions.push_back("\t<Relop> -> =>");

	if (check_symbol("=>", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Relop> -> == | != | > | < | <= | =>");
		print_error("expected relational operator");
	}
	return 1;
}

// <Expression Start>
int Syntax_Analyzer::Expression_Start(int exit_on_error) {
	Productions.push_back("\t<Expression Start> -> <Term Start> <Expression"
		                  " Cont>");

	if (Term_Start(exit_on_error) != 0) {
		return 1;
	}

	Expression_Cont();
	return 0;
}

// <Expression Cont>
void Syntax_Analyzer::Expression_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Expression Cont> -> + <Term Start> <Expression Cont>
	Productions.push_back("\t<Expression Cont> -> + <Term Start> <Expression"
		                  " Cont>");

	if (check_symbol("+", 0) == 0) {
		Term_Start(1);
		Expression_Cont();
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Expression Cont> -> - <Term Start> <Expression Cont>
	Productions.push_back("\t<Expression Cont> -> - <Term Start> <Expression"
		                  " Cont>");

	if (check_symbol("-", 0) == 0) {
		Term_Start(1);
		Expression_Cont();
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 3: <Expression Cont> -> <Empty>
	Productions.push_back("\t<Expression Cont> -> <Empty>");
}

// <Term Start>
int Syntax_Analyzer::Term_Start(int exit_on_error) {
	Productions.push_back("\t<Term Start> -> <Factor> <Term Cont>");

	if (Factor(exit_on_error) != 0) {
		return 1;
	}

	Term_Cont();
	return 0;
}

// <Term Cont>
void Syntax_Analyzer::Term_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Term Cont> -> * <Factor> <Term Cont>
	Productions.push_back("\t<Term Cont> -> * <Factor> <Term Cont>");

	if (check_symbol("*", 0) == 0) {
		Factor(1);
		Term_Cont();
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Term Cont> -> / <Factor> <Term Cont>
	Productions.push_back("\t<Term Cont> -> / <Factor> <Term Cont>");

	if (check_symbol("/", 0) == 0) {
		Factor(1);
		Term_Cont();
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 3: <Term Cont> -> <Empty>
	Productions.push_back("\t<Term Cont> -> <Empty>");
}

// <Factor>
int Syntax_Analyzer::Factor(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Factor> -> - <Primary>
	Productions.push_back("\t<Factor> -> - <Primary>");

	if (check_symbol("-", 0) == 0) {
		Primary_Start(1);
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 2: <Factor> -> <Primary>
	Productions.push_back("\t<Factor> -> <Primary>");

	if (Primary_Start(0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Factor> -> - <Primary> | <Primary>");
		print_error("expected primary (<identifier>, <integer>, <real>,"
			        " (expression), true, or false)");
	}
	return 1;
}

// <Primary Start>
int Syntax_Analyzer::Primary_Start(int exit_on_error) {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Primary Start> -> <Identifier> <Primary Cont>
	Productions.push_back("\t<Primary Start> -> <Identifier> <Primary"
		                  " Cont>");

	if (check_symbol("identifier", 0) == 0) {
		Primary_Cont();
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 2: <Primary Start> -> <Integer>
	Productions.push_back("\t<Primary Start> -> <Integer>");

	if (check_symbol("integer", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 3: <Primary Start> -> ( <Expression Start> )
	Productions.push_back("\t<Primary Start> -> ( <Expression Start> )");

	if (check_symbol("(", 0) == 0) {
		Expression_Start(1);
		check_symbol(")", 1);
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 4: <Primary Start> -> <Real>
	Productions.push_back("\t<Primary Start> -> <Real>");

	if (check_symbol("<real>", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}
	
	// Case 5: <Primary Start> -> true
	Productions.push_back("\t<Primary Start> -> true");

	if (check_symbol("true", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 6: <Primary Start> -> false
	Productions.push_back("\t<Primary Start> -> false");

	if (check_symbol("false", 0) == 0) {
		return 0;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// No matches
	if (exit_on_error == 1) {
		Productions.push_back("\t<Primary Start> -> <Identifier> <Primary"
			                  " Cont> | <Integer> | ( <Expression Start> ) |"
			                  " <Real> | true | false");
		print_error("expected primary (<identifier>, <integer>, <real>,"
			        " (expression), true, or false)");
	}
	return 1;
}

// <Primary Cont>
void Syntax_Analyzer::Primary_Cont() {
	Rule_list initial_list;
	copy_list(Productions, initial_list);

	// Case 1: <Primary Cont> -> ( <IDs Start> )
	Productions.push_back("\t<Primary Cont> -> ( <IDs Start> )");

	if (check_symbol("(", 0) == 0) {
		IDs_Start(1);
		check_symbol(")", 1);
		return;
	}
	else {
		copy_list(initial_list, Productions);
	}

	// Case 2: <Primary Cont> -> <Empty>
	Productions.push_back("\t<Primary Cont> -> <Empty>");
}

/*******************************************************************************
| check_symbol checks if the current token matches a terminal symbol (id, int, |
| real, keyword, sep, op). if it doesn't, and exit_on_error is enabled, then   |
| it will print an error message using print_error(). if exit_on_error is      |
| disabled, then it will just return the error code 1.                         |
| otherwise, if the current token matches the terminal symbol, then it will    |
| return the success code 0. in the event that the matched symbol appears in   |
| a new line, line_number (passed by reference) is updated.                    |
*******************************************************************************/
int Syntax_Analyzer::check_symbol(std::string symbol, int exit_on_error) {
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

	// <Identifier> or <Integer>
	if (symbol == "identifier" || symbol == "integer") {
		if (current_token.first != symbol) {
			if (exit_on_error == 1) {  // doesn't match and exit_on_error on
				print_error("expected <" + symbol + ">");
			}
			else {
				return 1;
			}
		}
		else {  // terminal symbol matches -> print everything
			print_current_token();
			print_productions();
			current_token = { "", "" };  // reset everything for next token
			Productions.clear();
			// in case symbol appeared on new line, update err_line_number
			err_line_number = get_line_number();
			return 0;
		}  // all other cases follow this same matching process
	}
	// <Real>
	else if (symbol == "<real>") {
		if (current_token.first != "real") {
			if (exit_on_error == 1) {
				print_error("expected <real>");
			}
			else {
				return 1;
			}
		}
		else {
			print_current_token();
			print_productions();
			current_token = { "", "" };
			Productions.clear();
			err_line_number = get_line_number();
			return 0;
		}
	}
	// EOF
	else if (symbol == "EOF") {
		if (current_token.first != symbol) {
			if (exit_on_error == 1) {
				print_error("expected " + symbol);
			}
			else {
				return 1;
			}
		}
		else {
			print_current_token();
			print_productions();
			current_token = { "", "" };
			Productions.clear();
			err_line_number = get_line_number();
			return 0;
		}
	}
	// keywords, operators, separators (note: Rat23S is NOT case sensitive)
	// (ie. both 'function' and 'FuNCtiOn' are equivalent keywords)
	std::string lower_lexeme = convert_to_lowercase(current_token.second);
	if (lower_lexeme != symbol) {
		if (exit_on_error == 1) {
			print_error("expected '" + symbol + "'");
			return 1;
		}
		else {
			return 1;
		}
	}
	else {  // matching symbol
		print_current_token();
		print_productions();
		current_token = { "", "" };
		Productions.clear();
		err_line_number = get_line_number();
		return 0;
	}
}

// Makes the list 'copy' have the same contents as 'original'
void Syntax_Analyzer::copy_list(Rule_list original, Rule_list& copy) {
	copy.clear();
	for (int i = 0; i < original.size(); i++) {
		copy.push_back(original.at(i));
	}
}

// returns the lowercase version of string s (used for checking terminal
// symbols, since Rat23S is not case sensitive
std::string Syntax_Analyzer::convert_to_lowercase(std::string s) {
	std::string lower = "";
	for (int i = 0; i < s.size(); i++) {
		lower += tolower(s[i]);
	}
	return lower;
}

// returns the line that the lexer is currently on
int Syntax_Analyzer::get_line_number() {
	return lexer->get_line_number();
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
	else {
		*ofs << "Token: " << current_token.first << "\t\tLexeme: "
			<< current_token.second << "\n";
	}
}

// prints the list of productions used for a token onto the output file
void Syntax_Analyzer::print_productions() {
	for (int i = 0; i < Productions.size(); i++) {
		*ofs << Productions.at(i) << "\n";
	}
}

// called when a token causes an error. the given error message and line number
// are printed onto the output file
void Syntax_Analyzer::print_error(Message err_msg) {
	*ofs << err_line_number << ": ERROR - " << err_msg << "\n";
	print_productions();
	*ofs << "\t";
	print_current_token();
	lexer->close_ifs();
	ofs->close();
	exit(-1);
}