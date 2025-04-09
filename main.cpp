#include <bits/stdc++.h>

#include "lexer.hpp"
#include "symbolTable.hpp"
#include "CFG.hpp"
#include "parser.hpp"

int main(){
	Lexer lexer("test.py");
	lexer.runLexer();
	// lexer.printLexer();
	vector<Token> tokens = lexer.allTokens;
	
	cout << "---------------------------------\n";
	
	CFG grammar("grammar.txt");
	grammar.computeAllFirsts();
	grammar.computeAllFollows();
	SymbolTable symTable;
	
	for (const Token& tok : lexer.allTokens) {
	    if (tok.type == "IDENTIFIER") {
	        symTable.addSymbol(tok);  // Only add identifiers
	    }
	
	}
	symTable.writeToFile("symboltable.txt");
	// cout << "TERMINALS : ";
	// for(auto it : grammar.terminals){
	// 	cout << it << " ";
	// }
	// cout << endl;
	// cout << "---------------------------------\n";
	// cout << "NONTERMINALS : ";
	// for(auto it : grammar.nonTerminals){
	// 	if(it == "S'") continue;
	// 	cout << it << " ";
	// }
	// cout << endl;
	for (auto &entry : grammar.follow) {
		if(entry.first == "S'") continue;
	    cout << "FOLLOW(" << entry.first << ") = { ";
	    for (const string &val : entry.second) {
	        cout << val << " ";
	    }
	    cout << "}" << endl;
	}

	cout << "---------------------------------\n";


	for(auto &entry : grammar.first){
		if(entry.first == "S'") continue;
		 cout << "FIRST(" << entry.first << ") = { ";
	    for (const string &val : entry.second) {
	        cout << val << " ";
	    }
	    cout << "}" << endl;
	}

	cout << "---------------------------------\n";
	


	vector<DFA_State> dfa_states = buildCanonicalCollection(grammar);
	//printDFAStates(dfa_states);
}
