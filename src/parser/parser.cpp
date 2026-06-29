#include "parser/parser.hpp"
#include "parser/grammar.hpp"

#include <sstream>

// le o arquivo de regras e prepara para a insercao no grammar
// le a tabela de simbolos e retorna a ordem

using namespace std;

namespace parser {
	void Parser::carregar_regras(string caminho) {
		ifstream in(caminho);

		if (!in) {
			throw runtime_error("Caminho não encontrado");
		}

		vector<RawProduction> productions;

		string s;
		while(getline(in, s)) {
			stringstream linha(s); 
			string head, consumer;

			linha>>head>>consumer;
			vector<string> body;
			
			string part; 
			while(linha>>part) {
				body.push_back(part);
			}

			productions.push_back({head,body});
		}

		this->rawProductions = productions;
	}

	void Parser::carregar_tokens(string caminho) {
		ifstream in(caminho);

		if (!in) {
			throw runtime_error("Caminho não encontrado");
		}

		string s;
		while(getline(in, s)) {
			stringstream linha(s); 
			string lexema, token;

			linha>>lexema>>token;

			lexema = lexema.substr(1, lexema.size()-2);
			token = token.substr(0, (int)token.size()-1);

			tabelaDeSimbolos.adiciona_simbolo(lexema);

			this->listaTokens.push_back({token, tabelaDeSimbolos.verifica_token(lexema)});
		}		
	}

	void Parser::carregar_palavras_reservadas(string caminho) {
		ifstream in(caminho);

		if (!in) {
			throw runtime_error("Caminho não encontrado");
		}

		vector<string> reservadas;

		string s;
		while(in>>s) {
			reservadas.push_back(s);
		}

		// Carrega palavras reservadas na tabela de simbolos		
		for(auto a: reservadas) {
			tabelaDeSimbolos.adiciona_simbolo(a);
		}

		quantReservas = tabelaDeSimbolos.tamanho();
	}

	vector<RawProduction> Parser::get_regras() {
		return this->rawProductions;
	}

	void Parser::mostrar_simbolos() {
		cout<<"Lista de tokens: "<<endl;
		size_t i;

		// tabelaDeSimbolos.mostrar();
		for(i = 0;i < quantReservas; i++) {
			cout<<"<"<<tabelaDeSimbolos.get_posicao(i)<<", PR>"<<endl;
		}

		for(i = 0; listaTokens.size() > i; i++) {
			if ((size_t) listaTokens[i].position < quantReservas) continue;
			cout<<"<"<<listaTokens[i].token<<", "<<listaTokens[i].position<<">"<<endl;
		}
		cout<<endl;
	}
}