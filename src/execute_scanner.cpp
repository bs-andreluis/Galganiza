#include "lexico/gerador_lexico.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

void printaUso(const char* programa) {
    std::cerr << "Uso: " << programa
              << " <definicoes.er> <fonte.txt> [tokens.txt]\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printaUso(argv[0]);
        return 2;
    }
    try {
        std::ifstream definicoes(argv[1]);
        if (!definicoes) throw std::runtime_error("não foi possível abrir o arquivo de definições");
        std::ifstream fonte(argv[2]);
        if (!fonte) throw std::runtime_error("não foi possível abrir o arquivo fonte");

        lexico::GeradorLexico gerador;
        gerador.carregarDefinicoes(definicoes);
        gerador.construir();

        std::ostringstream saidaTokens;
        gerador.analise(fonte, saidaTokens);
        std::cout << saidaTokens.str();

        if (argc == 4) {
            std::ofstream tokens(argv[3]);
            if (!tokens) throw std::runtime_error("não foi possível criar o arquivo de tokens");
            tokens << saidaTokens.str();
        }

        const std::filesystem::path tables = "tabelas";
        gerador.exportarTabelas(tables);

        if (argc == 4) std::cout << "Tokens salvos em: " << argv[3] << '\n';
        std::cout << "Tabelas geradas em: " << tables.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Erro: " << error.what() << '\n';
        return 1;
    }
}
