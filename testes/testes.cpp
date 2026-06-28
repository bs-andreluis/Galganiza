#include "automatos/algoritmos.hpp"
#include "lexico/gerador_lexico.hpp"
#include "regex/compilador_regex.hpp"
#include "regex/regex.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

namespace {

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

} // namespace

int main() {
    construcaoDiretaAceitaOperadoresNecessarios();
    testarMinimizacao();
    testarGeradorLexicoCompleto();
    testarBarraEEpsilon();
    rejeitaInputMalFormatado();
    std::cout << "Todos os testes passaram.\n";
}
