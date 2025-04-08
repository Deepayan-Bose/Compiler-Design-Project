

string ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : s.substr(start);
}

string rtrim(const string& s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string trim(const std::string& s) {
    return rtrim(ltrim(s));
}

bool isUpper(string &s){
	for(char c : s) if (islower(c)) return false;

	return true;
}

struct CFG{
	string srcFile;
	set<string> terminals;
	set<string> nonTerminals;
	vector<pair<string, vector<string> > > production;
	unordered_map<string, set<string> > first, follow;


	CFG(string fileName){
		srcFile = fileName;
		ifstream fptr(srcFile);
		string line;
	
		vector<pair<string, vector<string>>> rawProductions;
	
		while(getline(fptr, line)){
			if(line.empty()) continue;
	
			stringstream ss(line);
			string s1, s2;
			getline(ss, s1, ':');
			s1 = trim(s1);
			nonTerminals.insert(s1);
	
			getline(ss, s2, '\n');
			s2 = trim(s2);	
			stringstream ss2(s2);
			string temp;
			vector<string> arr;
	
			while(ss2 >> temp){
				if(isUpper(temp)) terminals.insert(temp);
				else nonTerminals.insert(temp);
				arr.push_back(temp);
			}
			rawProductions.push_back({s1, arr});
		}
	
		// Augment the grammar
		string originalStart = rawProductions[0].first;
		vector<string> augmentedRHS = {originalStart};
	
		production.push_back({"S'", augmentedRHS});
		nonTerminals.insert("S'");
	
		// Copy the rest of the productions
		for (auto &p : rawProductions) {
			production.push_back(p);
		}
	
		// Initialize FOLLOW set of augmented start symbol with $
		follow["S'"].insert("$");
	}

	bool isTerminal(const string &s){
		if(terminals.find(s) != terminals.end()) return true;
		return false;
	}

	bool isNonTerminal(const string &s){
		if(nonTerminals.find(s) != nonTerminals.end()) return true;
		return false;
	}

	void computeAllFirsts() {
	    for (const string &nt : nonTerminals) {
	        findFirst(nt);
	    }
	}

	void computeAllFollows() {
	    for (const string &nt : nonTerminals) {
	        findFollow(nt);
	    }
	}

	void findFirst(const string &s) {
	    if (first.find(s) != first.end()) return;
	
	    for (auto &p : production) {
	        if (p.first != s) continue;
	
	        const vector<string> &rhs = p.second;
	
	        // Case 1: EPSILON production
	        if (rhs.size() == 1 && rhs.front() == "EPSILON") {
	            first[s].insert("EPSILON");
	            continue;
	        }
	
	        // Case 2: Go through each symbol in RHS
	        bool allNullable = true;
	
	        for (size_t i = 0; i < rhs.size(); ++i) {
	            string symbol = rhs[i];
	
	            if (isTerminal(symbol)) {
	                first[s].insert(symbol);
	                allNullable = false;
	                break;
	            }
	
	            // Recursively find FIRST for non-terminal
	            findFirst(symbol);
	
	            // Add FIRST(symbol) - EPSILON to FIRST(s)
	            for (const string &f : first[symbol]) {
	                if (f != "EPSILON") {
	                    first[s].insert(f);
	                }
	            }
	
	            // If EPSILON not in FIRST(symbol), stop
	            if (first[symbol].find("EPSILON") == first[symbol].end()) {
	                allNullable = false;
	                break;
	            }
	        }
	
	        // If all symbols in RHS could derive EPSILON, add EPSILON
	        if (allNullable) {
	            first[s].insert("EPSILON");
	        }
	    }
	}
	void findFollow(const string &s) {
	    if (follow.find(s) != follow.end() && !follow[s].empty()) return;
	
	    for (auto &p : production) {
	        const string &lhs = p.first;
	        const vector<string> &rhs = p.second;
	
	        for (int i = 0; i < rhs.size(); ++i) {
	            if (rhs[i] != s) continue;
	
	            // Case: A → α B β
	            if (i + 1 < rhs.size()) {
	                string nextSymbol = rhs[i + 1];
	
	                // Case 1: next is terminal
	                if (isTerminal(nextSymbol)) {
	                    follow[s].insert(nextSymbol);
	                }
	                // Case 2: next is non-terminal
	                else if (isNonTerminal(nextSymbol)) {
	                    findFirst(nextSymbol);
	                    for (const string &f : first[nextSymbol]) {
	                        if (f != "EPSILON")
	                            follow[s].insert(f);
	                    }
	
	                    // If EPSILON is in FIRST(next), add FOLLOW(lhs)
	                    if (first[nextSymbol].count("EPSILON")) {
	                        findFollow(lhs);
	                        follow[s].insert(follow[lhs].begin(), follow[lhs].end());
	                    }
	                }
	            }
	            // Case: A → α B (B is at end)
	            else {
	                if (lhs != s) {
	                    findFollow(lhs);
	                    follow[s].insert(follow[lhs].begin(), follow[lhs].end());
	                }
	            }
	        }
	    }
	}
	
	
};
