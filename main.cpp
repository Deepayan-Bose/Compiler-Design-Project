#include <bits/stdc++.h>

#include "lexer.hpp"

#include "CFG.hpp"
#include "parser.hpp"

int main(){
	Lexer lexer("test.py");
	lexer.runLexer();
	lexer.printLexer();

	cout << "---------------------------------\n";
	
	CFG grammar("grammar.txt");
	grammar.computeAllFirsts();
	grammar.computeAllFollows();
	
// 	for (auto &entry : grammar.follow) {
// 	    cout << "FOLLOW(" << entry.first << ") = { ";
// 	    for (const string &val : entry.second) {
// 	        cout << val << " ";
// 	    }
// 	    cout << "}" << endl;
// 	}
// 
// 	for(auto &entry : grammar.first){
// 		 cout << "FIRST(" << entry.first << ") = { ";
// 	    for (const string &val : entry.second) {
// 	        cout << val << " ";
// 	    }
// 	    cout << "}" << endl;
// 	}
// 
// 	cout << "---------------------------------\n";
	


	// vector<DFA_State> dfa_states = canonicalCollection(grammar);
	// printDFAStates(dfa_states);
}
