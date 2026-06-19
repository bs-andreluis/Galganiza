#include "gal/automaton.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace gal {
namespace {

std::string symbolName(unsigned char symbol) {
    switch (symbol) {
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case ' ': return "space";
    default:
        if (symbol >= 33 && symbol <= 126) return std::string(1, static_cast<char>(symbol));
        std::ostringstream output;
        output << "0x" << std::hex << std::uppercase << static_cast<int>(symbol);
        return output.str();
    }
}

std::set<State> epsilonClosure(const Nfa& nfa, std::set<State> states) {
    std::vector<State> work(states.begin(), states.end());
    while (!work.empty()) {
        const State current = work.back();
        work.pop_back();
        for (const State next : nfa.states.at(current).epsilon) {
            if (states.insert(next).second) work.push_back(next);
        }
    }
    return states;
}

std::optional<std::size_t> selectedToken(const Nfa& nfa, const std::set<State>& states) {
    std::optional<std::size_t> selected;
    for (const State state : states) {
        const auto token = nfa.states.at(state).token;
        if (token && (!selected || *token < *selected)) selected = token;
    }
    return selected;
}

} // namespace

bool Dfa::accepts(const std::string& text) const { return recognize(text).has_value(); }

std::optional<std::size_t> Dfa::recognize(const std::string& text) const {
    if (states.empty()) return std::nullopt;
    State current = start;
    for (const unsigned char symbol : text) {
        const auto next = states.at(current).transitions[symbol];
        if (!next) return std::nullopt;
        current = *next;
    }
    return states.at(current).token;
}

void Dfa::writeTable(std::ostream& output, const std::vector<std::string>& tokenNames) const {
    std::set<unsigned char> alphabet;
    for (const auto& state : states) {
        for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
            if (state.transitions[symbol]) alphabet.insert(static_cast<unsigned char>(symbol));
        }
    }

    output << "estado";
    for (const auto symbol : alphabet) output << '\t' << symbolName(symbol);
    output << "\ttoken\n";
    for (State state = 0; state < states.size(); ++state) {
        output << (state == start ? "->" : "  ") << state;
        for (const auto symbol : alphabet) {
            output << '\t';
            if (states[state].transitions[symbol]) output << *states[state].transitions[symbol];
            else output << '-';
        }
        output << '\t';
        if (states[state].token) {
            const auto token = *states[state].token;
            output << ((token < tokenNames.size()) ? tokenNames[token] : std::to_string(token));
        } else {
            output << '-';
        }
        output << '\n';
    }
}

Dfa minimize(const Dfa& dfa) {
    if (dfa.states.empty()) return dfa;

    const State dead = dfa.states.size();
    const std::size_t count = dfa.states.size() + 1;
    std::vector<std::array<State, kAlphabetSize>> transition(count);
    for (State state = 0; state < dfa.states.size(); ++state) {
        for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
            transition[state][symbol] = dfa.states[state].transitions[symbol].value_or(dead);
        }
    }
    transition[dead].fill(dead);

    std::vector<std::optional<std::size_t>> tokens(count);
    for (State state = 0; state < dfa.states.size(); ++state) tokens[state] = dfa.states[state].token;

    std::map<std::optional<std::size_t>, std::size_t> initialGroups;
    std::vector<std::size_t> partition(count);
    for (State state = 0; state < count; ++state) {
        const auto [it, inserted] = initialGroups.emplace(tokens[state], initialGroups.size());
        partition[state] = it->second;
        (void)inserted;
    }

    while (true) {
        std::map<std::vector<std::size_t>, std::size_t> groups;
        std::vector<std::size_t> refined(count);
        for (State state = 0; state < count; ++state) {
            std::vector<std::size_t> signature;
            signature.reserve(kAlphabetSize + 1);
            signature.push_back(partition[state]);
            for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
                signature.push_back(partition[transition[state][symbol]]);
            }
            const auto [it, inserted] = groups.emplace(std::move(signature), groups.size());
            refined[state] = it->second;
            (void)inserted;
        }
        if (refined == partition) break;
        partition = std::move(refined);
    }

    const auto classCount = *std::max_element(partition.begin(), partition.end()) + 1;
    const auto deadGroup = partition[dead];
    if (partition[dfa.start] == deadGroup) {
        Dfa emptyLanguage;
        emptyLanguage.states.emplace_back();
        return emptyLanguage;
    }

    std::vector<std::optional<State>> remap(classCount);
    State nextIndex = 0;
    for (State group = 0; group < classCount; ++group) {
        if (group != deadGroup) remap[group] = nextIndex++;
    }
    Dfa result;
    result.states.resize(nextIndex);
    result.start = *remap[partition[dfa.start]];
    std::vector<State> representative(classCount, dead);
    for (State state = 0; state < count; ++state) representative[partition[state]] = state;
    for (State group = 0; group < classCount; ++group) {
        if (group == deadGroup) continue;
        const State state = representative[group];
        auto& minimizedState = result.states[*remap[group]];
        minimizedState.token = tokens[state];
        for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
            const auto targetGroup = partition[transition[state][symbol]];
            if (targetGroup != deadGroup) minimizedState.transitions[symbol] = *remap[targetGroup];
        }
    }
    return result;
}

Nfa uniteWithEpsilon(const std::vector<Dfa>& automata) {
    Nfa result;
    result.states.emplace_back();
    result.start = 0;
    for (const auto& dfa : automata) {
        if (dfa.states.empty()) continue;
        const State offset = result.states.size();
        result.states.resize(offset + dfa.states.size());
        result.states[result.start].epsilon.insert(offset + dfa.start);
        for (State state = 0; state < dfa.states.size(); ++state) {
            result.states[offset + state].token = dfa.states[state].token;
            for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
                if (const auto next = dfa.states[state].transitions[symbol]) {
                    result.states[offset + state].transitions[symbol].insert(offset + *next);
                }
            }
        }
    }
    return result;
}

Dfa determinize(const Nfa& nfa) {
    if (nfa.states.empty()) return {};
    Dfa result;
    std::map<std::set<State>, State> indices;
    std::queue<std::set<State>> work;
    const auto initial = epsilonClosure(nfa, {nfa.start});
    indices.emplace(initial, 0);
    work.push(initial);
    result.states.emplace_back();
    result.states[0].token = selectedToken(nfa, initial);

    while (!work.empty()) {
        const auto current = work.front();
        work.pop();
        const State currentIndex = indices.at(current);
        for (std::size_t symbol = 0; symbol < kAlphabetSize; ++symbol) {
            std::set<State> reached;
            for (const State state : current) {
                const auto& targets = nfa.states[state].transitions[symbol];
                reached.insert(targets.begin(), targets.end());
            }
            if (reached.empty()) continue;
            reached = epsilonClosure(nfa, std::move(reached));
            auto [it, inserted] = indices.emplace(reached, indices.size());
            if (inserted) {
                result.states.emplace_back();
                result.states[it->second].token = selectedToken(nfa, reached);
                work.push(reached);
            }
            result.states[currentIndex].transitions[symbol] = it->second;
        }
    }
    return result;
}

} // namespace gal
