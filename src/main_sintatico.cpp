#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "sintatico/grammar.hpp"
#include "sintatico/parser.hpp"

using namespace std;

namespace {

void printaUso(const char* programa) {
	cerr << "Uso: " << programa
	     << " [regras.txt palavras_reservadas.txt tokens.txt]\n";
}

} // namespace

int main(int argc, char* argv[]) {
	if (argc != 1 && argc != 4) {
		printaUso(argv[0]);
		return 2;
	}

	// Sem argumentos usa os exemplos do repositório; com 3 argumentos lê os
	// caminhos informados (a tabela de tokens costuma vir do analisador léxico).
	const string regras   = argc == 4 ? argv[1] : "exemplos_sintatico/regras1.txt";
	const string palavras = argc == 4 ? argv[2] : "exemplos_sintatico/palavras1.txt";
	const string tokens   = argc == 4 ? argv[3] : "exemplos_sintatico/tokens1.txt";

	try {
		sintatico::Parser parser;

		parser.carregar_regras(regras);
		parser.carregar_palavras_reservadas(palavras);
		parser.carregar_tokens(tokens);

		vector<string> lista;
		for (auto& s : parser.listaTokens) lista.push_back(s.token);
		lista.push_back("$");

		parser.gramatica = sintatico::Grammar();
		parser.gramatica.build(parser.get_regras());

		parser.gramatica.SLRParser(lista, parser.gramatica.calcTable());

		parser.mostrar_simbolos();
		return 0;
	} catch (const exception& error) {
		cerr << "Erro: " << error.what() << '\n';
		return 1;
	}
}
