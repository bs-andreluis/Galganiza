#include "sintatico/tabelaSimbolos.hpp"

#include <iostream>

using namespace std;

namespace sintatico {
	int TabelaSimbolos::verifica_token(string v) {
		if (tabelaSimbolos_indexer.count(v)) return tabelaSimbolos_indexer[v];
		return -1;
	}

	string TabelaSimbolos::get_posicao(size_t pos) {
		if (tabela.size() <= pos)
			throw out_of_range("Sem posição na tabela de simbolos");
		return tabela[pos];
	}

	int TabelaSimbolos::adiciona_simbolo(string v) {
		if (verifica_token(v) == -1) {
			tabelaSimbolos_indexer[v] = tabela.size();
			tabela.push_back(v);
		}
		return tabelaSimbolos_indexer[v];
	} 

	void TabelaSimbolos::mostrar() {
		cout<<"Tabela de simbolos:"<<endl;
		for(size_t i = 0; i < tabela.size(); i++) {
			cout<<tabela[i]<<endl;
		}
		cout<<endl;
	}

	size_t TabelaSimbolos::tamanho() {
		return tabela.size();
	}
}