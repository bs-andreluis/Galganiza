#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace sintatico {
    struct RawProduction;
    enum class SymbolType {
        Terminal,
        NonTerminal,
        EndOfInput
    };

    struct Symbol {
        SymbolType type; // Terminal/nao terminal
        string name;
        int index;
    };

    struct Production {
        int head;
        vector<int> body; // Index dos simbolos
        int index;

        bool operator==(const Production& o) const { return o.head == head && body == o.body; }

        bool operator==(Production o) { return o.head == head && body == o.body; }

        bool operator<(Production o) {
            // delegate to the const overload
            return static_cast<const Production&>(*this) < o;
        }

        bool operator<(const Production &o) const {
            if (head == o.head) {
                if (body == o.body) {
                    return index < o.index;
                } 
                return body < o.body;
            } 
            return head < o.head;
        }
    };

    enum class ActionType {
        Shift,
        Reduce,
        Error,
        Accept
    };

    struct Action {
        ActionType type;
        int next{-1};
    };


    struct Item {
        Production p; // producao
        int dot; // onde o ponto esta na producao

        bool operator==(const Item& o) const { return (o.dot == dot && p == o.p); }

        bool operator==(Item o) { return (o.dot == dot && p == o.p); }

        bool operator<(const Item& o) const {
            if (p == o.p) return dot < o.dot;
            return  p < o.p;
        }

        bool operator<(Item o) {
            if (p == o.p) return dot < o.dot;
            return  p < o.p;
        }
    };

    using ItemSet = set<Item>;
    using ItemCanonical = set<ItemSet>;


    class Grammar {
        vector<Symbol> symbols;
        vector<Production> productions;
        map<string, int> symbol_to_index;

    public:
        void build(vector<RawProduction> v);
        void reset();
        
        int addTerminal(string s);
        int addNonTerminal(string s);
        int onlyUppercase(string s);
        void addProduction(string head, vector<string> body);
        
        int findSymbol(string s);
        
        pair<vector<set<int>>, vector<int>> calcFirsts();
        vector<set<int>> calcFollow();
        
        void print_follow(vector<set<int>>);
        void print_collection(vector<ItemSet> I);
        void print_slr_table(vector<vector<Action>>);
        
        ItemSet closure (ItemSet I);
        vector<vector<Action>> calcTable();
        
        void SLRParser(vector<string> w, vector<vector<Action>> table);
    };

} // namespace sintatico
