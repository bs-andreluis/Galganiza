#include "gal/regex.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>

namespace gal {
namespace {

enum class Kind { Symbols, Epsilon, EndMarker, Union, Concat, Star, Plus, Optional };

struct Node {
    Kind kind;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    std::size_t position{};
    bool nullable{};
    std::set<std::size_t> first;
    std::set<std::size_t> last;
};

struct Position {
    std::set<unsigned char> symbols;
    bool endMarker{false};
};

std::set<std::size_t> setUnion(std::set<std::size_t> left, const std::set<std::size_t>& right) {
    left.insert(right.begin(), right.end());
    return left;
}

class Parser {
public:
    Parser(const std::string& expression, std::vector<Position>& positions)
        : expression_(expression), positions_(positions) {}

    std::unique_ptr<Node> parse() {
        auto root = parseUnion();
        skipWhitespace();
        if (index_ != expression_.size()) fail("símbolo inesperado");
        return root;
    }

private:
    std::unique_ptr<Node> parseUnion() {
        auto left = parseConcat();
        skipWhitespace();
        while (peek('|')) {
            ++index_;
            auto right = parseConcat();
            left = binary(Kind::Union, std::move(left), std::move(right));
            skipWhitespace();
        }
        return left;
    }

    std::unique_ptr<Node> parseConcat() {
        skipWhitespace();
        if (!startsAtom()) fail("expressão ou subexpressão vazia");
        auto left = parseRepetition();
        skipWhitespace();
        while (startsAtom()) {
            auto right = parseRepetition();
            left = binary(Kind::Concat, std::move(left), std::move(right));
            skipWhitespace();
        }
        return left;
    }

    std::unique_ptr<Node> parseRepetition() {
        auto value = parseAtom();
        skipWhitespace();
        bool hasOperator = true;
        while (hasOperator && index_ < expression_.size()) {
            Kind kind{};
            switch (expression_[index_]) {
            case '*': kind = Kind::Star; break;
            case '+': kind = Kind::Plus; break;
            case '?': kind = Kind::Optional; break;
            default: hasOperator = false; continue;
            }
            ++index_;
            auto parent = std::make_unique<Node>();
            parent->kind = kind;
            parent->left = std::move(value);
            value = std::move(parent);
            skipWhitespace();
        }
        return value;
    }

    std::unique_ptr<Node> parseAtom() {
        skipWhitespace();
        if (index_ >= expression_.size()) fail("fim inesperado da expressão");
        if (expression_[index_] == '(') {
            ++index_;
            auto value = parseUnion();
            skipWhitespace();
            if (!peek(')')) fail("parêntese ')' ausente");
            ++index_;
            return value;
        }
        if (expression_[index_] == '[') return parseCharacterClass();
        if (expression_[index_] == '&') {
            ++index_;
            auto value = std::make_unique<Node>();
            value->kind = Kind::Epsilon;
            return value;
        }
        const unsigned char symbol = readSymbol();
        return symbolLeaf({symbol});
    }

    std::unique_ptr<Node> parseCharacterClass() {
        ++index_;
        bool negated = false;
        if (peek('^')) { negated = true; ++index_; }
        std::set<unsigned char> symbols;
        bool foundAny = false;
        while (index_ < expression_.size() && expression_[index_] != ']') {
            const unsigned char first = readClassSymbol();
            foundAny = true;
            if (index_ + 1 < expression_.size() && expression_[index_] == '-' &&
                expression_[index_ + 1] != ']') {
                ++index_;
                const unsigned char last = readClassSymbol();
                if (first > last) fail("intervalo invertido na classe de caracteres");
                for (unsigned int value = first; value <= last; ++value) {
                    symbols.insert(static_cast<unsigned char>(value));
                }
            } else {
                symbols.insert(first);
            }
        }
        if (!peek(']')) fail("colchete ']' ausente");
        ++index_;
        if (!foundAny) fail("classe de caracteres vazia");
        if (negated) {
            std::set<unsigned char> complement;
            for (unsigned int value = 0; value < tamanhoAlfabeto; ++value) {
                if (!symbols.count(static_cast<unsigned char>(value))) {
                    complement.insert(static_cast<unsigned char>(value));
                }
            }
            symbols = std::move(complement);
        }
        return symbolLeaf(std::move(symbols));
    }

    unsigned char readSymbol() {
        if (index_ >= expression_.size()) fail("símbolo ausente");
        if (expression_[index_] == '\\') {
            ++index_;
            if (index_ >= expression_.size()) fail("escape incompleto");
            return escaped(expression_[index_++]);
        }
        const char value = expression_[index_++];
        if (value == ')' || value == '|' || value == '*' || value == '+' || value == '?' || value == ']') {
            fail("operador sem operando (use '\\' para símbolo literal)");
        }
        return static_cast<unsigned char>(value);
    }

    unsigned char readClassSymbol() {
        if (index_ >= expression_.size()) fail("classe de caracteres incompleta");
        if (expression_[index_] == '\\') {
            ++index_;
            if (index_ >= expression_.size()) fail("escape incompleto na classe");
            return escaped(expression_[index_++]);
        }
        return static_cast<unsigned char>(expression_[index_++]);
    }

    static unsigned char escaped(char value) {
        switch (value) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        default: return static_cast<unsigned char>(value);
        }
    }

    std::unique_ptr<Node> symbolLeaf(std::set<unsigned char> symbols) {
        auto node = std::make_unique<Node>();
        node->kind = Kind::Symbols;
        node->position = positions_.size();
        positions_.push_back({std::move(symbols), false});
        return node;
    }

    static std::unique_ptr<Node> binary(Kind kind, std::unique_ptr<Node> left,
                                         std::unique_ptr<Node> right) {
        auto node = std::make_unique<Node>();
        node->kind = kind;
        node->left = std::move(left);
        node->right = std::move(right);
        return node;
    }

    bool startsAtom() const {
        if (index_ >= expression_.size()) return false;
        const char value = expression_[index_];
        return value != ')' && value != '|' && value != '*' && value != '+' && value != '?';
    }

    bool peek(char value) const { return index_ < expression_.size() && expression_[index_] == value; }
    void skipWhitespace() {
        while (index_ < expression_.size() &&
               (expression_[index_] == ' ' || expression_[index_] == '\t' ||
                expression_[index_] == '\r' || expression_[index_] == '\n')) ++index_;
    }
    [[noreturn]] void fail(const std::string& message) const {
        throw RegexError(message + " na posição " + std::to_string(index_ + 1));
    }

    const std::string& expression_;
    std::vector<Position>& positions_;
    std::size_t index_{0};
};

void annotate(Node& node, std::vector<std::set<std::size_t>>& follow) {
    if (node.left) annotate(*node.left, follow);
    if (node.right) annotate(*node.right, follow);

    switch (node.kind) {
    case Kind::Symbols:
    case Kind::EndMarker:
        node.nullable = false;
        node.first = node.last = {node.position};
        break;
    case Kind::Epsilon:
        node.nullable = true;
        break;
    case Kind::Union:
        node.nullable = node.left->nullable || node.right->nullable;
        node.first = setUnion(node.left->first, node.right->first);
        node.last = setUnion(node.left->last, node.right->last);
        break;
    case Kind::Concat:
        node.nullable = node.left->nullable && node.right->nullable;
        node.first = node.left->nullable ? setUnion(node.left->first, node.right->first) : node.left->first;
        node.last = node.right->nullable ? setUnion(node.left->last, node.right->last) : node.right->last;
        for (const auto position : node.left->last) {
            follow[position].insert(node.right->first.begin(), node.right->first.end());
        }
        break;
    case Kind::Star:
    case Kind::Plus:
        node.nullable = node.kind == Kind::Star || node.left->nullable;
        node.first = node.left->first;
        node.last = node.left->last;
        for (const auto position : node.last) {
            follow[position].insert(node.first.begin(), node.first.end());
        }
        break;
    case Kind::Optional:
        node.nullable = true;
        node.first = node.left->first;
        node.last = node.left->last;
        break;
    }
}

} // namespace

Dfa DirectRegexCompiler::compile(const std::string& expression) const {
    std::vector<Position> positions;
    Parser parser(expression, positions);
    auto expressionRoot = parser.parse();

    auto marker = std::make_unique<Node>();
    marker->kind = Kind::EndMarker;
    marker->position = positions.size();
    const std::size_t markerPosition = positions.size();
    positions.push_back({{}, true});

    auto root = std::make_unique<Node>();
    root->kind = Kind::Concat;
    root->left = std::move(expressionRoot);
    root->right = std::move(marker);

    std::vector<std::set<std::size_t>> follow(positions.size());
    annotate(*root, follow);

    std::set<unsigned char> alphabet;
    for (const auto& position : positions) {
        alphabet.insert(position.symbols.begin(), position.symbols.end());
    }

    Dfa dfa;
    std::map<std::set<std::size_t>, Estado> indices;
    std::queue<std::set<std::size_t>> work;
    indices.emplace(root->first, 0);
    work.push(root->first);
    dfa.states.emplace_back();
    if (root->first.count(markerPosition)) dfa.states[0].token = 0;

    while (!work.empty()) {
        const auto current = work.front();
        work.pop();
        const Estado currentIndex = indices.at(current);
        for (const unsigned char symbol : alphabet) {
            std::set<std::size_t> target;
            for (const auto position : current) {
                if (positions[position].symbols.count(symbol)) {
                    target.insert(follow[position].begin(), follow[position].end());
                }
            }
            if (target.empty()) continue;
            auto [it, inserted] = indices.emplace(target, indices.size());
            if (inserted) {
                dfa.states.emplace_back();
                if (target.count(markerPosition)) dfa.states[it->second].token = 0;
                work.push(target);
            }
            dfa.states[currentIndex].transicoes[symbol] = it->second;
        }
    }
    return dfa;
}

} // namespace gal
