#include <fstream> 

struct SymbolInfo {
    string name;
    string type;
    int row;
    int col;

    string toString() const {
        return "<" + name + ", " + type + ", row: " + to_string(row) + ", col: " + to_string(col) + ">";
    }
};


class SymbolTable {
private:
    unordered_map<string, SymbolInfo> table;

public:
    void addSymbol(const Token &token) {
        if (token.type == "IDENTIFIER") {
            if (table.find(token.lexeme) == table.end()) {
                table[token.lexeme] = {token.lexeme, token.type, token.rowNum, token.colNum};
            }
        }
    }

    void writeToFile(const string &filename = "symboltable.txt") const {
        ofstream outFile(filename);
        if (!outFile.is_open()) {
            cerr << "Error: Could not open file " << filename << " for writing.\n";
            return;
        }

        outFile << "Symbol Table:\n";
        for (const auto &entry : table) {
            const SymbolInfo &info = entry.second;
            outFile << "<" << info.name << ", " << info.type
                    << ", row: " << info.row << ", col: " << info.col << ">\n";
        }

        outFile.close();
    }
};
