#pragma once

#include "gal/automaton.hpp"

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace gal {

struct TokenDefinition {
    std::string name;
    std::string expression;
};

class LexerGenerator {
public:
    void loadDefinitions(std::istream& input);
    void build();
    void analyze(std::istream& source, std::ostream& output) const;
    void exportTables(const std::filesystem::path& directory) const;

    [[nodiscard]] const std::vector<TokenDefinition>& definitions() const noexcept;
    [[nodiscard]] const std::vector<Dfa>& individualAutomata() const noexcept;
    [[nodiscard]] const Dfa& analysisTable() const;

private:
    std::vector<TokenDefinition> definitions_;
    std::vector<Dfa> individual_;
    Dfa analysis_;
    bool built_{false};
};

} // namespace gal
