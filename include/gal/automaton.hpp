#pragma once

#include <array>
#include <cstddef>
#include <iosfwd>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace gal {

constexpr std::size_t kAlphabetSize = 256;
using State = std::size_t;

struct DfaState {
    std::array<std::optional<State>, kAlphabetSize> transitions{};
    std::optional<std::size_t> token;
};

class Dfa {
public:
    State start{0};
    std::vector<DfaState> states;

    [[nodiscard]] bool accepts(const std::string& text) const;
    [[nodiscard]] std::optional<std::size_t> recognize(const std::string& text) const;
    void writeTable(std::ostream& output, const std::vector<std::string>& tokenNames = {}) const;
};

struct NfaState {
    std::array<std::set<State>, kAlphabetSize> transitions{};
    std::set<State> epsilon;
    std::optional<std::size_t> token;
};

class Nfa {
public:
    State start{0};
    std::vector<NfaState> states;
};

[[nodiscard]] Dfa minimize(const Dfa& dfa);
[[nodiscard]] Nfa uniteWithEpsilon(const std::vector<Dfa>& automata);
[[nodiscard]] Dfa determinize(const Nfa& nfa);

} // namespace gal

