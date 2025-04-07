

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
		while(getline(fptr, line)){
			if(line == "") continue;
			stringstream ss(line);
			string s1, s2;
			// contains lefthand side of production
			getline(ss,s1, ':');
			s1 = trim(s1);
			nonTerminals.insert(s1);
			// contains righthand side of production
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
			production.push_back({s1, arr});
			
		}
		follow["program"] = "$";
// 		for(auto ele : production){
// 			cout << ele.first << "->" ;
// 			for(auto str : ele.second) cout << str << " ";
// 			cout << endl;
// 		}		
// 
// 		for(auto it : terminals){
// 			cout << it << " ";
// 		}
// 		cout << endl;
	}

	bool isTerminal(const string &s){
		if(terminals.find(s) != terminals.end()) return true;
		return false;
	}

	bool isNonTerminal(const string &s){
		if(nonTerminals.find(s) != nonTerminals.end()) return true;
		return false;
	}

	void findFirst(const string &s){
		if(first.find(s) != first.end()) return;
		for(auto p : production){
			if(p.first != s) continue;
			// now check if
			// 1. production has EPSILON
			if((p.second).size() == 1 && (p.second).front() == "EPSILON" ){
				first[s].insert("EPSILON");
				continue;
			}
			// 2. if the production starts with a non terminal
			if(isTerminal((p.second).front())){
				first[s].insert((p.second).front());
				continue;
			}
			else{
				int epsilonCnt = 0;
				for(int i =0; i < (p.second).size() ; i++){
					// check if follow is already computed or not
					bool foundEpsilon = false;
					string str = (p.second)[i];
					if(isTerminal(str)) {
						first[s].insert((p.second)[i]);
						break;
					}
					
				}
			}
		}
	}
};
