#include "gal/lexer_generator.hpp"
#include "gal/regex.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

namespace {

void directConstructionSupportsRequiredOperators() {
    gal::DirectRegexCompiler compiler;
    const auto dfa = compiler.compile("a?(a | b)+");
    assert(dfa.accepts("a"));
    assert(dfa.accepts("bbb"));
    assert(dfa.accepts("abab"));
    assert(!dfa.accepts(""));
    assert(!dfa.accepts("c"));
}

void rangesAndMinimizationWork() {
    gal::DirectRegexCompiler compiler;
    const auto original = compiler.compile("[a-zA-Z]([a-zA-Z] | [0-9])*");
    const auto minimized = gal::minimize(original);
    assert(minimized.accepts("alpha123"));
    assert(minimized.accepts("Z"));
    assert(!minimized.accepts("43teste"));
    assert(minimized.states.size() <= original.states.size() + 1);
}

void completePipelineAndPriorityWork() {
    std::istringstream definitions(
        "id: [a-zA-Z]([a-zA-Z] | [0-9])*\n"
        "num: [1-9]([0-9])* | 0\n"
        "word: [a-z]+\n");
    gal::LexerGenerator generator;
    generator.loadDefinitions(definitions);
    generator.build();
    std::istringstream source("a1 0 teste 21 43teste");
    std::ostringstream output;
    generator.analyze(source, output);
    assert(output.str() ==
           "<a1, id>\n<0, num>\n<teste, id>\n<21, num>\n<43teste, erro!>\n");
}

void epsilonAndEscapesWork() {
    gal::DirectRegexCompiler compiler;
    assert(compiler.compile("(& | a)b?").accepts(""));
    assert(compiler.compile("(& | a)b?").accepts("ab"));
    assert(compiler.compile("\\+\\?").accepts("+?"));
}

void malformedInputIsRejected() {
    gal::LexerGenerator generator;
    std::istringstream malformed("sem separador\n");
    bool rejected = false;
    try { generator.loadDefinitions(malformed); }
    catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);

    gal::DirectRegexCompiler compiler;
    rejected = false;
    try { (void)compiler.compile("[z-a]"); }
    catch (const gal::RegexError&) { rejected = true; }
    assert(rejected);
}

} // namespace

int main() {
    directConstructionSupportsRequiredOperators();
    rangesAndMinimizationWork();
    completePipelineAndPriorityWork();
    epsilonAndEscapesWork();
    malformedInputIsRejected();
    std::cout << "Todos os testes passaram.\n";
}
