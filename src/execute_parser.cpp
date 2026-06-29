#include <bits/stdc++.h>
#include "parser/grammar.hpp"
#include "parser/parser.hpp"

using namespace std;

int main() {
	parser::Grammar g;

	parser::Parser parser;
	
	parser.carregar_regras("exemplos_sintatico/regras1.txt");
	parser.carregar_palavras_reservadas("exemplos_sintatico/palavras1.txt");
	parser.carregar_tokens("exemplos_sintatico/tokens1.txt");

	vector<string> lista;
	for(auto& s: parser.listaTokens) lista.push_back(s.token);
	lista.push_back("$");
	
	parser.gramatica = parser::Grammar();
	parser.gramatica.build(parser.get_regras());

	parser.gramatica.SLRParser(lista, parser.gramatica.calcTable());

	parser.mostrar_simbolos();
}