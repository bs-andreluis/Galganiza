#pragma once

#include "gal/automaton.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace gal {

class RegexError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DirectRegexCompiler {
public:
    [[nodiscard]] Dfa compile(const std::string& expression) const;
};

} // namespace gal
