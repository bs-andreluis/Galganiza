#include "automatos/algoritmos.hpp"
#include "parser/parser.hpp"
#include "lexico/gerador_lexico.hpp"
#include "regex/compilador_regex.hpp"
#include "regex/regex.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

namespace {

std::filesystem::path criarDiretorioTemporario() {
    auto base = std::filesystem::temp_directory_path() / "galganiza_parser_testes";
    auto sufixo = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto dir = base / sufixo;
    std::filesystem::create_directories(dir);
    return dir;
}

void escreverArquivo(const std::filesystem::path& caminho, const std::string& conteudo) {
    std::ofstream saida(caminho);
    assert(saida);
    saida << conteudo;
}

std::string capturarSaidaStdout(const std::function<void()>& acao) {
    std::ostringstream buffer;
    auto* antigo = std::cout.rdbuf(buffer.rdbuf());
    acao();
    std::cout.rdbuf(antigo);
    return buffer.str();
}

void construcaoDiretaAceitaOperadoresNecessarios() {
    regex::CompiladorRegex compilador;
    const auto afd = compilador.compilar("a?(a | b)+");
    assert(afd.aceita("a"));
    assert(afd.aceita("bbb"));
    assert(afd.aceita("abab"));
    assert(!afd.aceita(""));
    assert(!afd.aceita("c"));
}

void testarMinimizacao() {
    regex::CompiladorRegex compilador;
    const auto original = compilador.compilar("[a-zA-Z]([a-zA-Z] | [0-9])*");
    const auto minimo = automatos::minimizar(original);
    assert(minimo.aceita("alpha123"));
    assert(minimo.aceita("Z"));
    assert(!minimo.aceita("43teste"));
    assert(minimo.estados.size() <= original.estados.size() + 1);
}

void testarGeradorLexicoCompleto() {
    std::istringstream definicoes(
        "id: [a-zA-Z]([a-zA-Z] | [0-9])*\n"
        "num: [1-9]([0-9])* | 0\n"
        "word: [a-z]+\n");
    lexico::GeradorLexico gerador;
    gerador.carregarDefinicoes(definicoes);
    gerador.construir();
    std::istringstream fonte("a1 0 teste 21 43teste");
    std::ostringstream output;
    gerador.analise(fonte, output);
    assert(output.str() ==
           "<a1, id>\n<0, num>\n<teste, id>\n<21, num>\n<43teste, erro!>\n");
}

void testarBarraEEpsilon() {
    regex::CompiladorRegex compilador;
    assert(compilador.compilar("(& | a)b?").aceita(""));
    assert(compilador.compilar("(& | a)b?").aceita("ab"));
    assert(compilador.compilar("\\+\\?").aceita("+?"));
}

void rejeitaInputMalFormatado() {
    lexico::GeradorLexico gerador;
    std::istringstream malformatado("sem separador\n");
    bool rejeitado = false;
    try { gerador.carregarDefinicoes(malformatado); }
    catch (const std::runtime_error&) { rejeitado = true; }
    assert(rejeitado);

    regex::CompiladorRegex compilador;
    rejeitado = false;
    try { (void)compilador.compilar("[z-a]"); }
    catch (const regex::ErroRegex&) { rejeitado = true; }
    assert(rejeitado);
}

void testarParserCarregarRegras() {
    auto dir = criarDiretorioTemporario();
    auto regras = dir / "regras.txt";
    escreverArquivo(regras,
        "E -> E + T\n"
        "E -> T\n"
        "T -> id\n");

    parser::Parser parser;
    parser.carregar_regras(regras.string());

    const auto raw = parser.get_regras();
    assert(raw.size() == 3);
    assert(raw[0].head == "E");
    assert((raw[0].body == std::vector<std::string>{"E", "+", "T"}));
    assert(raw[1].head == "E");
    assert((raw[1].body == std::vector<std::string>{"T"}));
    assert(raw[2].head == "T");
    assert((raw[2].body == std::vector<std::string>{"id"}));
}

void testarParserCarregarPalavrasReservadas() {
    auto dir = criarDiretorioTemporario();
    auto palavras = dir / "palavras.txt";
    escreverArquivo(palavras,
        "if\n"
        "while\n"
        "return\n");

    parser::Parser parser;
    parser.carregar_palavras_reservadas(palavras.string());

    assert(parser.quantReservas == 3);
    assert(parser.tabelaDeSimbolos.tamanho() == 3);
    assert(parser.tabelaDeSimbolos.get_posicao(0) == "if");
    assert(parser.tabelaDeSimbolos.get_posicao(1) == "while");
    assert(parser.tabelaDeSimbolos.get_posicao(2) == "return");
    assert(parser.tabelaDeSimbolos.verifica_token("if") == 0);
    assert(parser.tabelaDeSimbolos.verifica_token("return") == 2);
    assert(parser.tabelaDeSimbolos.verifica_token("else") == -1);
}

void testarParserCarregarTokens() {
    auto dir = criarDiretorioTemporario();
    auto palavras = dir / "palavras.txt";
    auto tokens = dir / "tokens.txt";
    escreverArquivo(palavras,
        "if\n"
        "while\n");
    escreverArquivo(tokens,
        "<if, IF>\n"
        "<id, ID>\n"
        "<while, WHILE>\n"
        "<id, ID>\n");

    parser::Parser parser;
    parser.carregar_palavras_reservadas(palavras.string());
    parser.carregar_tokens(tokens.string());

    assert(parser.tabelaDeSimbolos.tamanho() == 3);
    assert(parser.tabelaDeSimbolos.verifica_token("if") == 0);
    assert(parser.tabelaDeSimbolos.verifica_token("while") == 1);
    assert(parser.tabelaDeSimbolos.verifica_token("id") == 2);

    assert(parser.listaTokens.size() == 4);
    assert(parser.listaTokens[0].token == "IF");
    assert(parser.listaTokens[0].position == 0);
    assert(parser.listaTokens[1].token == "ID");
    assert(parser.listaTokens[1].position == 2);
    assert(parser.listaTokens[2].token == "WHILE");
    assert(parser.listaTokens[2].position == 1);
    assert(parser.listaTokens[3].position == 2);
}

void testarParserSLRCompletoAceitaExpressao() {
    auto dir = criarDiretorioTemporario();
    auto regras = dir / "regras.txt";
    auto palavras = dir / "palavras.txt";
    auto tokens = dir / "tokens.txt";

    escreverArquivo(regras,
        "S -> if id\n");
    escreverArquivo(palavras,
        "if\n"
        "while\n");
    escreverArquivo(tokens,
        "<if, IF>\n"
        "<id, ID>\n"
        "<while, WHILE>\n"
        "<id, ID>\n");

    parser::Parser parser;
    parser.carregar_regras(regras.string());
    parser.carregar_palavras_reservadas(palavras.string());
    parser.carregar_tokens(tokens.string());

    parser.gramatica.build(parser.get_regras());
    auto tabela = parser.gramatica.calcTable();

    bool aceitou = true;
    try {
        parser.gramatica.SLRParser({"if", "id", "$"}, tabela);
    } catch (...) {
        aceitou = false;
    }

    assert(aceitou);
}

void testarParserMostrarSimbolos() {
    auto dir = criarDiretorioTemporario();
    auto palavras = dir / "palavras.txt";
    auto tokens = dir / "tokens.txt";

    escreverArquivo(palavras,
        "if\n"
        "while\n");
    escreverArquivo(tokens,
        "<if, IF>\n"
        "<id, ID>\n"
        "<while, WHILE>\n");

    parser::Parser parser;
    parser.carregar_palavras_reservadas(palavras.string());
    parser.carregar_tokens(tokens.string());

    const auto saida = capturarSaidaStdout([&] {
        parser.mostrar_simbolos();
    });

    assert(saida.find("<if, PR>") != std::string::npos);
    assert(saida.find("<while, PR>") != std::string::npos);
    assert(saida.find("<ID, 2>") != std::string::npos);
}

} // namespace

int main() {
    construcaoDiretaAceitaOperadoresNecessarios();
    testarMinimizacao();
    testarGeradorLexicoCompleto();
    testarBarraEEpsilon();
    rejeitaInputMalFormatado();
    testarParserCarregarRegras();
    testarParserCarregarPalavrasReservadas();
    testarParserCarregarTokens();
    testarParserSLRCompletoAceitaExpressao();
    testarParserMostrarSimbolos();
    std::cout << "Todos os testes passaram.\n";
}
