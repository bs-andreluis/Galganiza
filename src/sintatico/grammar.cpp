#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "sintatico/parser.hpp"
#include "sintatico/grammar.hpp"

// Por convenção:
// - os nao terminais devem ser escritos em maiuscula e os terminais em minusculas ou outros simbolos;
// - producoes devem ser separadas por linha;
// - a primeira linha tem a producao inicial;
// - para representar uma producao com corpo vazio, devese descrever a cabeca
// e deixar o corpo vazio.
// - a producao deve separar os simbolos por espacos

using namespace std;

namespace sintatico {

void Grammar::build(vector<RawProduction> rawProductions)  {
	reset();
	addTerminal("$"); // final de sentenca
	// P' -> P
	// Base production
	addNonTerminal("START");
	addProduction("START", {rawProductions[0].head});

	for(auto p: rawProductions) {
		addProduction(p.head, p.body);
	}
}

void Grammar::reset() {
	symbols.clear();
	productions.clear();
	symbol_to_index.clear();
}

// adiciona terminal
int Grammar::addTerminal(string s) {
	int idx = findSymbol(s);
	if (idx >= 0) return idx;
	// nao existe ainda 
	idx = symbols.size();
	symbols.push_back({SymbolType::Terminal, s, idx});
	symbol_to_index[s] = idx;
	return idx;
}

int Grammar::addNonTerminal(string s) {
	int idx = findSymbol(s);
	if (idx >= 0) return idx;
	// nao existe ainda 
	idx = symbols.size();
	symbols.push_back({SymbolType::NonTerminal, s, idx});
	symbol_to_index[s] = idx;
	return idx;
}

int Grammar::onlyUppercase(string s) {
	for (char c: s) {
		if ('Z' < c || c < 'A') return false;
	}

	return true;
}

void Grammar::addProduction(string head, vector<string> body) {
	int idx_head = findSymbol(head);
	
	vector<int> productionBody;
	productionBody.reserve(body.size());
	for(auto part: body) {
		auto idx_part = findSymbol(part);
		if (idx_part < 0) {
			if (onlyUppercase(part)) {
				idx_part = addNonTerminal(part);
			} else {
				idx_part = addTerminal(part);
			}
		}
		productionBody.push_back(idx_part);
	}

	productions.push_back({idx_head, productionBody,(int)productions.size()}); 
}

	// retorna 
int Grammar::findSymbol(string s) {
	auto it = symbol_to_index.find(s);
	return (it == symbol_to_index.end() ? -1 : it->second);
}

pair<vector<set<int>>, vector<int>> Grammar::calcFirsts() {
	vector<set<int>> first(symbols.size());
	vector<int> first_null(symbols.size(), 0);
	
	for(auto s: symbols) {
		if (s.type == SymbolType::Terminal)
			first[s.index].insert(s.index);
	}

	int mudanca = 1;
	while(mudanca) {
		mudanca = 0;
		for(Production p: productions) {
			int eps = 1;
			for(auto part: p.body) {
				Symbol ac = symbols[part];
				if (ac.type == SymbolType::Terminal) {
					auto [it, preenchido] = first[p.head].insert(part);
					mudanca |= preenchido;
				} else {
					size_t tam = first[p.head].size();
					first[p.head].insert(first[part].begin(), first[part].end());
					
					if (tam != first[p.head].size()) {
						mudanca = 1;	
					}

					if (!first_null[part]) {
						eps = 0;
						break;
					}
				}
			}
			if (eps && !first_null[p.head]) first_null[p.head] = 1, mudanca = 0;
							
			if (p.body.empty()) {
				if (!first_null[p.head])
					first_null[p.head] = 1, mudanca = 1;
			}
		}
	}		
	return {first, first_null};
}

vector<set<int>> Grammar::calcFollow() {
	vector<set<int>> follow(symbols.size());

	auto [first, first_null] = calcFirsts();

	// Adiciona end symbol na producao inicial
	for(int i=0;i<(int)symbols.size();i++) {
		if (symbols[i].name == "START") follow[i].insert(findSymbol("$"));
	}

	int mudanca = 1;
	while (mudanca) {
		mudanca=0;
		for(auto symbol: symbols) {
			if (symbol.type == SymbolType::Terminal) continue;

			for(auto production: productions) {
				vector<int> &body = production.body;
				int i;
				for(i=0; i<(int)body.size(); i++) {
					if (body[i] == symbol.index) {
						int eps = 1;
						int tam = follow[symbol.index].size();
						i++;
						for(;i<(int)body.size();i++) {
							if (symbols[body[i]].type == SymbolType::Terminal) {
								follow[symbol.index].insert(body[i]);
								eps = 0;
								break;
							}
							follow[symbol.index].insert(first[body[i]].begin(), first[body[i]].end());

							if (!first_null[body[i]]) {
								eps = 0;
								break;
							}
						}
						if (eps) {
							follow[symbol.index].insert(follow[production.head].begin()
								, follow[production.head].end());
						}

						if (tam != (int)follow[symbol.index].size()) {
							mudanca = 1;
						}
					}	
				}
			}
		}
	}

	return follow;
}

ItemSet Grammar::closure (ItemSet I) {
	ItemSet items(I.begin(), I.end());

	int mudou = 1;
	while (mudou) {
		mudou = 0;
		for (Item item_c: items) {
			int id = item_c.dot;
			if (id == (int)item_c.p.body.size()) {
				continue;
			}

			int idx_body = item_c.p.body[id];
			
			for (auto p: productions) {
				if (p.head == idx_body) {
					Item item{p,0};

					if (items.count(item)) continue;
					items.insert(item);
					mudou = 1;
				}
			}
		}
	}

	return items;
}

vector<vector<Action>> Grammar::calcTable() {
	set<ItemSet> nao_visitadas;
	map<ItemSet, int> set_to_index;
	vector<ItemSet> itemSet;
	vector<tuple<int,int,int>> edge_list; // (u,v,s);

	// verificar existencia
	nao_visitadas.insert(closure({{productions[0], 0}}));
	set_to_index[closure({{productions[0], 0}})] = 0;
	itemSet.push_back(closure({{productions[0], 0}}));

	while(!nao_visitadas.empty()) {
		auto C = *nao_visitadas.begin(); nao_visitadas.erase(nao_visitadas.begin());

		// passa por cada simbolo
		for(auto symbol: symbols) {
			// base sao todos os items que tiveram uma transicao de (C, a) 
			ItemSet base;
			for(auto I: C) {
				// GOTO
				if ((int)I.p.body.size() > I.dot && I.p.body[I.dot] == symbol.index) {
					base.insert({I.p, I.dot+1});
				}
			}

			// Se pelo menos um item eh gerado entao ele adiciona como um ItemSet
			if (base.size()) {
				ItemSet clos = closure(base);

				// Caso seja a primeira vez encontrando aquele estado
				if (set_to_index.count(clos) == 0) {
					set_to_index[clos] = set_to_index.size();
					nao_visitadas.insert(clos);
					itemSet.push_back(clos);
				}

				// Adiciona uma aresta do ItemSet para o encontrado
				edge_list.push_back({set_to_index[C],set_to_index[clos], symbol.index});
			}
		}
	}

	// linhas = estados
	// colunas = simbolos
	vector<vector<Action>> table(itemSet.size(),
	vector<Action>(symbols.size(), {ActionType::Error, -1}));

	for(auto [u,v,s]: edge_list) {
		if (table[u][s].type != ActionType::Error) throw runtime_error("Gramática não é SLR(1).");
		table[u][s] = {ActionType::Shift, v};
	}

	auto follow = calcFollow();
	ItemSet finalState = {{productions[0], 1}};

	print_follow(follow);

	int idx = 0;
	for(int i=0;i<(int)symbols.size();i++) {
		if (symbols[i].name == "$") idx = i;
	}

	for(size_t i=0;i<itemSet.size();i++) {
		if (itemSet[i] == finalState) {
			table[i][idx] = {ActionType::Accept, 0};
			continue;
		}
		for(auto item: itemSet[i]) {
			if (item.dot == (int)item.p.body.size()) {
				for(auto p: follow[item.p.head]) {
					if (table[i][p].type != ActionType::Error) throw runtime_error("Gramática não é SLR(1).");
					
					table[i][p] = {ActionType::Reduce, item.p.index};
				}
			}
		}
	}

	print_collection(itemSet);

	return table;
}

void Grammar::print_follow(vector<set<int>> follow) {
	cout <<"Follows:"<<endl;
	for(auto symbol: symbols) {
		if (symbol.type == SymbolType::Terminal) continue;
		cout<<symbol.name<<": {";
		int first = 1;
		for(auto f: follow[symbol.index]) {
			if (!first) cout<<", ";
			first = 0;
			cout<<symbols[f].name;
		}
		cout<<"}"<<endl;
	}
	cout<<endl;
}

void Grammar::print_collection(vector<ItemSet> collection) {
	cout<<"Coleção LR(0) Canônica: "<<endl;
	int item = 0;
	for(auto I: collection) {
		cout<<"Item "<< item++<<":"<<endl;
		for(auto item: I) {
			auto &prod = item.p;
			cout<<symbols[prod.head].name<<" -> ";

			size_t i = 0;

			for(;i < prod.body.size(); i++) {
				if (i == (size_t)item.dot) break;
				cout<<symbols[prod.body[i]].name<<" ";
			}
			cout<<" . ";
			for(;i < prod.body.size(); i++) {
				cout<<symbols[prod.body[i]].name<<" ";
			}
			cout<<endl;
		}
		cout<<"--------"<<endl<<endl;
	}
}

void Grammar::print_slr_table(vector<vector<Action>> table) {
	cout << "SLR table:\n";
	cout << setw(8) << "State";
	for (auto &sym : symbols) {
		cout << setw(10) << sym.name;
	}
	cout << "\n";
	for (size_t i = 0; i < table.size(); i++) {
		cout << setw(8) << i;
		for (size_t j = 0; j < symbols.size(); j++) {
			auto &action = table[i][j];
			string cell = ".";
			if (action.type == ActionType::Shift) {
				cell = "S" + to_string(action.next);
			} else if (action.type == ActionType::Reduce) {
				cell = "R" + to_string(action.next);
			} else if (action.type == ActionType::Accept) {
				cell = "acc";
			}
			cout << setw(10) << cell;
		}
		cout << "\n";
	}
}

void Grammar::SLRParser (vector<string> w, vector<vector<Action>> table) {
	vector<int> pilha;
	vector<string> simbolos;
	pilha.push_back(0);
	int qtd;
	
	while (true) {
		int word = findSymbol(w.front());
		
		Action action = table[pilha.back()][word];
		if (action.type == ActionType::Shift) {
			pilha.push_back(action.next);
			simbolos.push_back(w[0]);
			w.erase(w.begin());
		} else if(action.type == ActionType::Reduce) {
			Production p = productions[action.next];
			qtd = p.body.size();
			for (int i = 0; i < qtd; i++) {
				pilha.pop_back();
				simbolos.pop_back();
			}
			simbolos.push_back(symbols[p.head].name);
			Action go = table[pilha.back()][symbols[p.head].index];
			pilha.push_back(go.next); 		
		} else if (action.type == ActionType::Accept) {
			printf("Aceito!\n");
			break;
		} else {
			cout<<"Erro de sintaxe no token: "<<symbols[word].name<<endl;
			throw runtime_error("Erro!");			
		}
	}
}
}

