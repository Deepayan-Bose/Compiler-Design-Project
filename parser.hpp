#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <iostream>

using namespace std;

struct Item {
    string lhs;
    vector<string> rhs;
    int dotPos;
    string lookahead;

    bool operator==(const Item &other) const {
        if (lhs != other.lhs) return false;
        if (rhs != other.rhs) return false;
        if (dotPos != other.dotPos) return false;
        return lookahead == other.lookahead;
    }

    bool operator<(const Item &other) const {
        if (lhs != other.lhs) return lhs < other.lhs;
        if (rhs != other.rhs) return rhs < other.rhs;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }

    string toString() const {
        string result = lhs + " → ";
        for (int i = 0; i <= rhs.size(); ++i) {
            if (i == dotPos) result += "•";
            if (i < rhs.size()) result += rhs[i] + " ";
        }
        result += ", " + lookahead;
        return result;
    }
    void initItem(const string LHS,
                   const vector<string> RHS,
                   int dot,
                   const string LA) {
         lhs       = LHS;
         rhs       = RHS;
         dotPos    = dot;
         lookahead = LA;
     }
};

struct ItemSet {
    set<Item> items;

    bool operator==(const ItemSet &other) const {
        return items == other.items;
    }

    string toString() const {
        string res = "{\n";
        for (const auto &item : items) {
            res += "  " + item.toString() + "\n";
        }
        res += "}";
        return res;
    }

    void addItem(const Item &item) {
        items.insert(item);
    }

    bool contains(const Item &item) const {
        return items.find(item) != items.end();
    }
     bool operator<(const ItemSet &other) const {
        return items < other.items;
    }
};

struct DFA_State {
    int id;
    ItemSet items;
    unordered_map<string, int> transitions;  // Symbol -> next state ID
    bool isAccepting;  // True if this state contains S' → program•

    // Constructor
    DFA_State(int id, const ItemSet& items) : id(id), items(items) {
        // Check if this is an accepting state (contains S' → program•, $)
        isAccepting = false;
        for (const Item& item : items.items) {
            if (item.lhs == "S'" && 
                item.rhs.size() == 1 && 
                item.rhs[0] == "program" && 
                item.dotPos == 1 && 
                item.lookahead == "$") {
                isAccepting = true;
                break;
            }
        }
    }

    // For printing the state
    string toString() const {
        string result = "State " + to_string(id) + ":\n";
        result += (isAccepting ? "[ACCEPTING]\n" : "");
        
        // Print all items
        for (const Item& item : items.items) {
            result += "  " + item.toString() + "\n";
        }

        // Print transitions
        if (!transitions.empty()) {
            result += "  Transitions:\n";
            for (const auto& trans : transitions) {
                result += "    " + trans.first + " → State " + to_string(trans.second) + "\n";
            }
        }

        return result;
    }

    // For comparing states (used in stateMap)
    bool operator==(const DFA_State& other) const {
        return items == other.items;
    }
};

set<string> computeFirstFromSeq(vector<string> sequence, CFG &grammar) {
    set<string> ans;
    bool allEpsilon = true;

    for (const auto& symbol : sequence) {
        if (grammar.isTerminal(symbol)) {
            if (symbol != "EPSILON") {
                ans.insert(symbol);
            }
            allEpsilon = false;
            break; // Stop at first terminal (unless EPSILON)
        }

        const set<string>& firstSet = grammar.first[symbol];
        for (const string& tok : firstSet) {
            if (tok != "EPSILON") ans.insert(tok);
        }

        if (firstSet.find("EPSILON") == firstSet.end()) {
            allEpsilon = false;
            break;
        }
    }

    if (allEpsilon) ans.insert("EPSILON");
    
    return ans;
}

ItemSet computeClosure(const ItemSet &inputSet, CFG &grammar) {

    ItemSet closureSet = inputSet;
    queue<Item> workQ;

    for (const Item &it : inputSet.items) {
        workQ.push(it);
    }
	
    while (!workQ.empty()) {
        Item curr = workQ.front();
        workQ.pop();

        if (curr.dotPos >= curr.rhs.size()) 
            continue;

        string B = curr.rhs[curr.dotPos];
        if (!grammar.isNonTerminal(B)) 
            continue;

        vector<string> beta;
        for (int i = curr.dotPos + 1; i < curr.rhs.size(); ++i) {
            beta.push_back(curr.rhs[i]);
        }
        beta.push_back(curr.lookahead);

        // Computing first
        set<string> lookaheads = computeFirstFromSeq(beta, grammar);
		if(lookaheads.empty()) lookaheads.insert("$");

        for (const auto &prod : grammar.production) {
            if (prod.first != B) 
                continue;

            for (const string &la : lookaheads) {
                Item newItem;
                newItem.lhs       = B;
                newItem.rhs       = prod.second;
                newItem.dotPos    = 0;
                newItem.lookahead = la;

                if (!closureSet.contains(newItem)) {
                    closureSet.addItem(newItem);
                    workQ.push(newItem);
                }
            }
        }
    }

    return closureSet;
}


ItemSet GOTO(ItemSet I, const string &X, CFG &grammar) {
    ItemSet J;
    for (const Item &item : I.items) {
        if (item.dotPos < item.rhs.size() && item.rhs[item.dotPos] == X) {
            Item newItem = item;
            newItem.dotPos++;
            J.addItem(newItem);
        }
    }
    return computeClosure(J, grammar);
}

vector<DFA_State> buildCanonicalCollection(CFG &grammar) {
    vector<DFA_State> states;
    map<ItemSet, int> stateMap;    // ItemSet → state ID
    queue<ItemSet> workQueue;

    // 1) Create the initial item S' → • program, $
    Item startItem;
    startItem.lhs       = "S'";
    startItem.rhs       = {"program"};
    startItem.dotPos    = 0;
    startItem.lookahead = "$";

    ItemSet startSet;
    startSet.addItem(startItem);
    startSet = computeClosure(startSet, grammar);

    // 2) Create initial DFA_State
    states.emplace_back(0, startSet);
    stateMap[startSet] = 0;
    workQueue.push(startSet);

    // 3) Process queue
    while (!workQueue.empty()) {
        ItemSet currentSet = workQueue.front(); 
        workQueue.pop();

        int currID = stateMap[currentSet];
        DFA_State &currState = states[currID];

        // 4) Gather all symbols X that appear immediately after a dot
        set<string> symbols;
        for (const Item &it : currentSet.items) {
            if (it.dotPos < it.rhs.size()) {
                symbols.insert(it.rhs[it.dotPos]);
            }
        }

        // 5) For each symbol, compute GOTO
        for (const string &X : symbols) {
            ItemSet gotoSet = GOTO(currentSet, X, grammar);
            if (gotoSet.items.empty()) continue;

            // 6) If new, assign ID and enqueue
            if (!stateMap.count(gotoSet)) {
                int newID = states.size();
                states.emplace_back(newID, gotoSet);
                stateMap[gotoSet] = newID;
                workQueue.push(gotoSet);
            }

            // 7) Record transition
            int targetID = stateMap[gotoSet];
            currState.transitions[X] = targetID;
        }
    }

    return states;
}

void printDFAStates(const vector<DFA_State> states) {
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

