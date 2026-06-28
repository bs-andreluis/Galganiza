#include "regex/parser.hpp"
#include "automatos/common.hpp"

namespace regex{
    std::unique_ptr<Node> Parser::parse() {
        auto raiz = parseUniao();
        pulaEspaco();
        if (index_ != expressao_.size()) fail("símbolo inesperado");
        return raiz;
    }

    std::unique_ptr<Node> Parser::parseUniao() {
        auto left = parseConcatenacao();
        pulaEspaco();
        while (achar('|')) {
            ++index_;
            auto right = parseConcatenacao();
            left = binario(Tipo::Uniao, std::move(left), std::move(right));
            pulaEspaco();
        }
        return left;
    }

    std::unique_ptr<Node> Parser::parseConcatenacao() {
        pulaEspaco();
        if (!comecaAtomo()) fail("expressão ou subexpressão vazia");
        auto left = parseRepeticao();
        pulaEspaco();
        while (comecaAtomo()) {
            auto right = parseRepeticao();
            left = binario(Tipo::Concatenacao, std::move(left), std::move(right));
            pulaEspaco();
        }
        return left;
    }

    std::unique_ptr<Node> Parser::parseRepeticao() {
        auto val = parseAtomo();
        pulaEspaco();
        bool temOperador = true;
        while (temOperador && index_ < expressao_.size()) {
            Tipo tipo{};
            switch (expressao_[index_]) {
                case '*': tipo = Tipo::Fecho; break;
                case '+': tipo = Tipo::Plus; break;
                case '?': tipo = Tipo::Opcional; break;
                default: temOperador = false; continue;
            }
            ++index_;
            auto pai = std::make_unique<Node>();
            pai->tipo = tipo;
            pai->left = std::move(val);
            val = std::move(pai);
            pulaEspaco();
        }
        return val;
    }

    std::unique_ptr<Node> Parser::parseAtomo() {
        pulaEspaco();
        if (index_ >= expressao_.size()) fail("fim inesperado da expressão");
        if (expressao_[index_] == '(') {
            ++index_;
            auto val = parseUniao();
            pulaEspaco();
            if (!achar(')')) fail("parêntese ')' ausente");
            ++index_;
            return val;
        }
        if (expressao_[index_] == '[') return parseClasseCaracter();
        if (expressao_[index_] == '&') {
            ++index_;
            auto val = std::make_unique<Node>();
            val->tipo = Tipo::Epsilon;
            return val;
        }
        const unsigned char c = leSimbolo();
        return simboloFolha({c});
    }

    std::unique_ptr<Node> Parser::parseClasseCaracter() {
        ++index_;
        bool negado = false;
        if (achar('^')) { negado = true; ++index_; }
        std::set<unsigned char> simbolos;
        bool achou = false;
        while (index_ < expressao_.size() && expressao_[index_] != ']') {
            const unsigned char c = leClasseSimbolo();
            achou = true;
            if (index_ + 1 < expressao_.size() && expressao_[index_] == '-' &&
                expressao_[index_ + 1] != ']') {
                ++index_;
                const unsigned char last_c = leClasseSimbolo();
                if (c > last_c) fail("intervalo invertido na classe de caracteres");
                for (unsigned int val = c; val <= last_c; ++val) {
                    simbolos.insert(static_cast<unsigned char>(val));
                }
            } else {
                simbolos.insert(c);
            }
        }
        if (!achar(']')) fail("colchete ']' ausente");
        ++index_;
        if (!achou) fail("classe de caracteres vazia");
        if (negado) {
            std::set<unsigned char> complemento;
            for (unsigned int c = 0; c < automatos::tamanhoAlfabeto; ++c) {
                if (!simbolos.count(static_cast<unsigned char>(c))) {
                    complemento.insert(static_cast<unsigned char>(c));
                }
            }
            simbolos = std::move(complemento);
        }
        return simboloFolha(std::move(simbolos));
    }

    unsigned char Parser::leSimbolo() {
        if (index_ >= expressao_.size()) fail("símbolo ausente");
        if (expressao_[index_] == '\\') {
            ++index_;
            if (index_ >= expressao_.size()) fail("escape incompleto");
            return escaped(expressao_[index_++]);
        }
        const char c = expressao_[index_++];
        if (c == ')' || c == '|' || c == '*' || c == '+' || c == '?' || c == ']') {
            fail("operador sem operando (use '\\' para símbolo literal)");
        }
        return static_cast<unsigned char>(c);
    }

    unsigned char Parser::leClasseSimbolo() {
        if (index_ >= expressao_.size()) fail("classe de caracteres incompleta");
        if (expressao_[index_] == '\\') {
            ++index_;
            if (index_ >= expressao_.size()) fail("escape incompleto na classe");
            return escaped(expressao_[index_++]);
        }
        return static_cast<unsigned char>(expressao_[index_++]);
    }

    std::unique_ptr<Node> Parser::simboloFolha(std::set<unsigned char> simbolos) {
        auto node = std::make_unique<Node>();
        node->tipo = Tipo::Simbolo;
        node->posicao = posicoes_.size();
        posicoes_.push_back({std::move(simbolos), false});
        return node;
    }

    bool Parser::comecaAtomo() const {
        if (index_ >= expressao_.size()) return false;
        const char c = expressao_[index_];
        return c != ')' && c != '|' && c != '*' && c != '+' && c != '?';
    }

    void Parser::pulaEspaco() {
        while (index_ < expressao_.size() &&
            (expressao_[index_] == ' ' || expressao_[index_] == '\t' ||
                expressao_[index_] == '\r' || expressao_[index_] == '\n')) ++index_;
    }
}