#include "gal/lexer_generator.hpp"

#include "gal/regex.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gal {
namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string safeFilename(std::string name) {
    for (char& value : name) {
        if (!std::isalnum(static_cast<unsigned char>(value)) && value != '-' && value != '_') value = '_';
    }
    return name.empty() ? "token" : name;
}

} // namespace

void LexerGenerator::loadDefinitions(std::istream& input) {
    definitions_.clear();
    individual_.clear();
    analysis_ = {};
    built_ = false;

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        const auto clean = trim(line);
        if (clean.empty() || clean.front() == '#') continue;
        const auto separator = clean.find(':');
        if (separator == std::string::npos) {
            throw std::runtime_error("linha " + std::to_string(lineNumber) + ": ':' ausente");
        }
        TokenDefinition definition{trim(clean.substr(0, separator)), trim(clean.substr(separator + 1))};
        if (definition.name.empty()) {
            throw std::runtime_error("linha " + std::to_string(lineNumber) + ": nome do padrão vazio");
        }
        if (definition.expression.empty()) {
            throw std::runtime_error("linha " + std::to_string(lineNumber) + ": expressão regular vazia");
        }
        for (const auto& existing : definitions_) {
            if (existing.name == definition.name) {
                throw std::runtime_error("linha " + std::to_string(lineNumber) +
                                         ": padrão duplicado '" + definition.name + "'");
            }
        }
        definitions_.push_back(std::move(definition));
    }
    if (definitions_.empty()) throw std::runtime_error("nenhuma definição regular encontrada");
}

void LexerGenerator::build() {
    if (definitions_.empty()) throw std::logic_error("carregue as definições antes de gerar o analisador");
    DirectRegexCompiler compiler;
    individual_.clear();
    individual_.reserve(definitions_.size());
    for (std::size_t token = 0; token < definitions_.size(); ++token) {
        try {
            auto dfa = minimize(compiler.compile(definitions_[token].expression));
            for (auto& state : dfa.states) {
                if (state.token) state.token = token;
            }
            individual_.push_back(std::move(dfa));
        } catch (const RegexError& error) {
            throw RegexError("padrão '" + definitions_[token].name + "': " + error.what());
        }
    }
    analysis_ = determinize(uniteWithEpsilon(individual_));
    built_ = true;
}

void LexerGenerator::analyze(std::istream& source, std::ostream& output) const {
    if (!built_) throw std::logic_error("o analisador léxico ainda não foi gerado");
    std::string lexeme;
    while (source >> lexeme) {
        const auto token = analysis_.recognize(lexeme);
        if (token) output << '<' << lexeme << ", " << definitions_.at(*token).name << ">\n";
        else output << '<' << lexeme << ", erro!>\n";
    }
}

void LexerGenerator::exportTables(const std::filesystem::path& directory) const {
    if (!built_) throw std::logic_error("o analisador léxico ainda não foi gerado");
    std::filesystem::create_directories(directory);
    std::vector<std::string> names;
    for (const auto& definition : definitions_) names.push_back(definition.name);

    for (std::size_t index = 0; index < individual_.size(); ++index) {
        const auto path = directory / ("afd_" + std::to_string(index + 1) + "_" +
                                       safeFilename(definitions_[index].name) + ".tsv");
        std::ofstream output(path);
        if (!output) throw std::runtime_error("não foi possível criar '" + path.string() + "'");
        individual_[index].writeTable(output, names);
    }
    const auto path = directory / "tabela_analise_lexica.tsv";
    std::ofstream output(path);
    if (!output) throw std::runtime_error("não foi possível criar '" + path.string() + "'");
    analysis_.writeTable(output, names);
}

const std::vector<TokenDefinition>& LexerGenerator::definitions() const noexcept { return definitions_; }
const std::vector<Dfa>& LexerGenerator::individualAutomata() const noexcept { return individual_; }
const Dfa& LexerGenerator::analysisTable() const {
    if (!built_) throw std::logic_error("o analisador léxico ainda não foi gerado");
    return analysis_;
}

} // namespace gal

