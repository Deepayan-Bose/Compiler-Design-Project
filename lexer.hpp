
using namespace std;

struct Token {
    string lexeme;
    string type;
    int rowNum;
    int colNum;


    string stringifyToken(void){
    	// function which returns the token in a string format
    	return string(lexeme+'\t'+type+'\t'+to_string(rowNum) +'\t' + to_string(colNum)); 
    }
};

struct Lexer{
	// Contains a set of valid keywords for the python like language 

	string fileName;
	set<string> keywords; 
	vector<Token> allTokens;


	Lexer(const string &fileName){
		this->fileName = string(fileName);

		keywords = {
		  	"if", "else", "elif", "int", "bool", "float", "list", "str",
		};
		
		regex stringLiteralRegex(R"('([^'\\]|\\.)')");
		regex floatLiteralRegex(R"(\b\d+\.\d+\b)");
		regex intLiteralRegex(R"(\b\d+\b)");
		regex boolLiteralRegex(R"(\bTrue\b|\bFalse\b)");
		regex identifierRegex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
		regex operatorRegex(R"(==|=|<|>)");
		regex punctuatorRegex(R"([(){}\[\],:.;])");
		
		
		// list of regex mapped to its corresponding token type it recognizes in proper order
		tokenPatterns = {
		    {stringLiteralRegex, "STRING"},
		    {floatLiteralRegex, "FLOAT"},
		    {intLiteralRegex, "INTEGER"},
		    {boolLiteralRegex, "BOOLEAN"},
		    {identifierRegex, "IDENTIFIER"},
		    {operatorRegex, "OPERATOR"},
		    {punctuatorRegex, "PUNCTUATOR"}
		};
	}
	// Contains a list of pairs of regex and the token type it recognizes
	// Map not used to successfully match the regex in proper order specified during initialization
	vector<pair<regex, string>> tokenPatterns;

	//function to classify if the identifier regex identified a keyword or a regex
	string classifyKeywordOrIdentifier(const string&);
	// function to tokenize the contents of file on a line by line basis
	vector<Token> tokenizeCurrentLine(string&, int);

	// function to call for running the lexer 
	void runLexer(void);

	// print all the tokens
	void printLexer(void);

	
};



// function to check if the lexeme recognized by identifier regex is a keyword in the language
string Lexer::classifyKeywordOrIdentifier(const string& lexeme) {
    if (keywords.count(lexeme)) return "KEYWORD";
    return "IDENTIFIER";
}

vector<Token> Lexer::tokenizeCurrentLine(string& line, int rowNum) {
    vector<Token> tokens;
    int i = 0;
    while (i < (int)line.length()) {
        if (line[i] == '#') break; // Skip comments in the test code provided

        bool regexMatched = false;

        for (auto  ele : tokenPatterns) {
        	regex pattern = ele.first;
        	string type = ele.second;
            smatch match; // stores the position wrt sub (0-based) where match is found
            string sub = line.substr(i);

            if (regex_search(sub, match, pattern) && match.position() == 0) {
                string lexeme = match.str();
                string finalType = type;

                // if identifier regex is matched successfully then check if the string matched is a keyword or not 
                if (type == "IDENTIFIER") {
                    finalType = classifyKeywordOrIdentifier(lexeme);
                }

				// add the token to the list of tokens identified for this line
                tokens.push_back({lexeme, finalType, rowNum, i});
                // jump by number of characters matched 
                i += lexeme.length();
                // set regexMached to true indicating the row contains a valid python token
                regexMatched = true;
                break;
            }
        }

        if (!regexMatched) {
        	// Simply skip the token that is unrecognized
            i++;
        }
    }

    return tokens;
}


void Lexer::runLexer(void){
	ifstream file(fileName);

    if (!file.is_open()) {
        cerr << "Failed to open" << fileName << endl;
        exit(1);
    }

    string line;
    int rowNum = 0;
	allTokens.resize(0);

    while (getline(file, line)) {
        vector<Token> tokens = tokenizeCurrentLine(line, rowNum);
        allTokens.insert(allTokens.end(), tokens.begin(), tokens.end());
        string leftTrimmedString = line;
        leftTrimmedString.erase(0, leftTrimmedString.find_first_not_of(" \t"));
        if (!leftTrimmedString.empty() && leftTrimmedString[0] != '#') {
            allTokens.push_back({"\\n", "NEWLINE", rowNum, (int)(line.length())});
        }
    
        rowNum++;
    }

    file.close();

}

void Lexer::printLexer(void){
	 for (auto token : allTokens) cout << "<" << token.lexeme << ", " << token.type  << ", row: " << token.rowNum << ", col: " << token.colNum << ">" << endl;
    
}
