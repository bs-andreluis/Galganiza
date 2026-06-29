#pragma once

#include <bits/stdc++.h>
#include <parser/grammar.hpp>
#include <parser/tabelaSimbolos.hpp>

using namespace std;



namespace parser {
	struct RawProduction {
		string head;
		vector<string> body;
	};

	struct TokenListItem {
		string token;
		int position;
	};

	class Parser {
	
	public:
		TabelaSimbolos tabelaDeSimbolos;
		vector<TokenListItem> listaTokens;
		size_t quantReservas = 0;

		vector<RawProduction> rawProductions;
		Grammar gramatica;
		void carregar_regras(string caminho); // OK
		void carregar_tokens(string caminho); // OK
		void carregar_palavras_reservadas(string caminho); // OK
		vector<RawProduction> get_regras(); // OK
		void mostrar_simbolos(); // OK
	};
}

