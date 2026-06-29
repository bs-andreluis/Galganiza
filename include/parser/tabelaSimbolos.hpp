#pragma once

#include <bits/stdc++.h>


using namespace std;

namespace parser {
	class TabelaSimbolos {
		map<string,int> tabelaSimbolos_indexer;
		vector<string> tabela;
		
	public:
		int verifica_token(string v); //verifica se tem simbolo e retorna indice 

		int adiciona_simbolo(string v); //adiciona simbolo e retorna posicao

		string get_posicao(size_t pos);

		void mostrar(); // printa a tabela de simbolos

		size_t tamanho();
	};
}