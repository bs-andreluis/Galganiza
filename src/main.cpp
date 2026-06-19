#include "gal/lexer_generator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void printUsage(const char* program) {
    std::cerr << "Uso: " << program
              << " <definicoes.er> <fonte.txt> <tokens.txt> [diretorio-tabelas]\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 5) {
        printUsage(argv[0]);
        return 2;
    }
    try {
        std::ifstream definitions(argv[1]);
        if (!definitions) throw std::runtime_error("não foi possível abrir o arquivo de definições");
        std::ifstream source(argv[2]);
        if (!source) throw std::runtime_error("não foi possível abrir o arquivo fonte");
        std::ofstream tokens(argv[3]);
        if (!tokens) throw std::runtime_error("não foi possível criar o arquivo de tokens");

        gal::LexerGenerator generator;
        generator.loadDefinitions(definitions);
        generator.build();
        generator.analyze(source, tokens);
        const std::filesystem::path tables = argc == 5 ? argv[4] : "tabelas";
        generator.exportTables(tables);

        std::cout << "Análise concluída: " << argv[3] << '\n'
                  << "Tabelas geradas em: " << tables.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Erro: " << error.what() << '\n';
        return 1;
    }
}

