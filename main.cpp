#include <bits/stdc++.h>

#include "lexer.hpp"
#include "CFG.hpp"


int main(){
	Lexer lexer("test.py");
	lexer.runLexer();
	lexer.printLexer();
	CFG cfg("grammar.txt");
}
