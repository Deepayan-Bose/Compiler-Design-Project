struct Item {
    string lhs;
    vector<string> rhs;
    int dotPos;
    string lookahead; // important for CLR(1)

	// used to check if two Items are equal or not : useful for storing items in a set

    bool operator==(const Item &other) const {
    	if(dotPos != other.dotPos) return false;
        if(lhs != other.lhs) return false;
        if(rhs.size() != other.rhs.size()) return false;
        for(int i= 0; i < min(rhs.size(), other.rhs.size()); i++) if(rhs[i] != other.rhs[i]) return false;
        if(lookahead != other.lookahead) return false;

        return true;
    }

	// for ordering of items in the set
    bool operator<(const Item &other) const {
        if (lhs != other.lhs) return lhs < other.lhs;
        if (rhs != other.rhs) return rhs < other.rhs;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }

    string toString() const {
        string result = lhs + " → ";
        for (int i = 0; i <= rhs.size(); ++i) {
            if (i == dotPos) result += "• ";
            if (i < rhs.size()) result += rhs[i] + " ";
        }
        result += ", " + lookahead;
        return result;
    }

    void initItem(string lhs, vector<string> rhs, int dotPos, string lookahead){
    	this->lhs = lhs;
    	this->rhs = rhs;
    	this->dotPos = dotPos;
    	this->lookahead = lookahead;
    }
};

struct ItemSet {
    set<Item> items;

    bool operator==(const ItemSet &other) const {
        return items == other.items;
    }

	ItemSet(const ItemSet & other) {
		for(auto it : other.items) items.insert(it);
	}
	ItemSet(){
		
	}
    string toString() const {
        string res = "{\n";
        for (const auto &item : items) {
            res += "  " + item.toString() + "\n";
        }
        res += "}";
        return res;
    }

    bool operator<(const ItemSet &other) const {
	
    	return items.size() < other.items.size();
    	
    }

    void addItem(const Item &item) {
        items.insert(item);
    }

    bool contains(const Item &item) const {
        return items.find(item) != items.end();
    }
};

set<string> computeFirstFromSeq(vector<string> sequence, CFG &grammar) {
    set<string> ans;
    bool epsilonTillEnd = true;

    for (const auto& symbol : sequence) {

        // Terminal
        if (grammar.isTerminal(symbol)) {
            if (symbol != "EPSILON")
                ans.insert(symbol);
            epsilonTillEnd = false;
            //break; WRONG- PLZ DEBUG
        }

        // Non-terminal: get FIRST set
        const set<string>& firstSet = grammar.first[symbol];

        // Add all except EPSILON
        for (const string& tok : firstSet)
            if (tok != "EPSILON")
                ans.insert(tok);

        // If EPSILON not in FIRST(Xi), stop
        if (firstSet.find("EPSILON") == firstSet.end()) {
            epsilonTillEnd = false;
            break;
        }
    }

    if (epsilonTillEnd)
        ans.insert("EPSILON");

    return ans;
}


ItemSet computeClosure(ItemSet a, CFG &grammar) {
    ItemSet closureSet;
    queue<Item> q;

    // Add initial items to closure and queue
    for (const Item& it : a.items) {
        closureSet.addItem(it);
        q.push(it);
    }

    while (!q.empty()) {
        Item curr = q.front(); q.pop();

        if (curr.dotPos >= curr.rhs.size()) continue;

        string B = curr.rhs[curr.dotPos];
        if (!grammar.isNonTerminal(B)) continue;

        // Compute FIRST(βa)
        vector<string> beta;
        for (int i = curr.dotPos + 1; i < curr.rhs.size(); i++)
            beta.push_back(curr.rhs[i]);
        beta.push_back(curr.lookahead);

        set<string> lookaheads = computeFirstFromSeq(beta, grammar);
		if(lookaheads.empty()) lookaheads.insert("$");
        for (const auto& prod : grammar.production) {
            if (prod.first == B) {
                for (const string& la : lookaheads) {
                    Item newItem;
                    newItem.initItem(B, prod.second, 0, la);
                    if (!closureSet.contains(newItem)) {
                        closureSet.addItem(newItem);
                        q.push(newItem);
                    }
                }
            }
        }
    }

    cout << "CLOSURE SET : " << closureSet.toString() << endl;

    return closureSet;
}

ItemSet GOTO(ItemSet I, const string &X, CFG &grammar) {
    ItemSet J;

    for (const Item &item : I.items) {
        // Check if dot is before X
        if (item.dotPos < item.rhs.size() && item.rhs[item.dotPos] == X) {
            // Create a new item with dot moved one step ahead
            Item newItem = item;
            newItem.dotPos++;
            J.addItem(newItem);
        }
    }

    // Compute closure of the result
    return computeClosure(J, grammar);
}

struct DFA_State {
    int id;
    ItemSet items;
    unordered_map<string, int> transitions; // symbol -> next state ID

    DFA_State(int id, const ItemSet& items) {
        this->id = id;
        this->items = items;
    }

    // For comparing two states based on their items
    bool operator==(const DFA_State &other) const {
        return items == other.items;
    }
};

vector<DFA_State> canonicalCollection(CFG &grammar) {
    vector<DFA_State> states;
    map<ItemSet, int> stateMap; // maps ItemSet to state ID for uniqueness
    queue<ItemSet> q;

    // Augmented start production: S' -> •S, $
    string startSym = "program"; // assuming "program" is start symbol
    Item startItem;
    startItem.initItem("S'", {startSym}, 0, "$");

    ItemSet startSet;
    startSet.addItem(startItem);
    startSet = computeClosure(startSet, grammar);

    // Add first state
    states.emplace_back(0, startSet);
    stateMap[startSet] = 0;
    q.push(startSet);

    int stateCounter = 1;

    while (!q.empty()) {
        ItemSet current = q.front(); q.pop();
        int currentStateID = stateMap[current];

        set<string> symbols;
        // Collect all grammar symbols after dot
        for (const Item &item : current.items) {
            if (item.dotPos < item.rhs.size()) {
                symbols.insert(item.rhs[item.dotPos]);
            }
        }

        for (const string &symbol : symbols) {
            ItemSet gotoSet = GOTO(current, symbol, grammar);
            if (gotoSet.items.empty()) continue;
			cout << "ITEMSET : " << gotoSet.toString() << endl;
            if (stateMap.find(gotoSet) == stateMap.end()) {
                // New state
                stateMap[gotoSet] = stateCounter;
                states.emplace_back(stateCounter, gotoSet);
                q.push(gotoSet);
                states[currentStateID].transitions[symbol] = stateCounter;
                stateCounter++;
            } else {
                // Already exists
                states[currentStateID].transitions[symbol] = stateMap[gotoSet];
            }
        }
    }

    return states;
}

void printDFAStates(const vector<DFA_State> &states) {
    for (const DFA_State &state : states) {
        cout << "State " << state.id << ":\n";

        // Print each item in this state
        for (const Item &item : state.items.items) {
            cout << "  " << item.lhs << " -> ";
            for (int i = 0; i <= item.rhs.size(); ++i) {
                if (i == item.dotPos) cout << "• ";
                if (i < item.rhs.size()) cout << item.rhs[i] << " ";
            }
            cout << ", " << item.lookahead << "\n";
        }

        // Print transitions
        if (!state.transitions.empty()) {
            cout << "  Transitions:\n";
            for (const auto &trans : state.transitions) {
                cout << "    " << trans.first << " -> State " << trans.second << "\n";
            }
        }

        cout << "-----------------------------\n";
    }
}

