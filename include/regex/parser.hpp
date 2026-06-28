#pragma once

#include <memory>
#include <set>
#include <vector>
#include <string>
#include "regex.hpp"

namespace regex {
    enum class Tipo { Simbolo, Epsilon, MarcadorFim, Uniao, Concatenacao, Fecho, Plus, Opcional };

    struct Node {
        Tipo tipo;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::size_t posicao{};
        bool nullable{};
        std::set<std::size_t> first;
        std::set<std::size_t> last;
    };

    struct Posicao {
        std::set<unsigned char> simbolos;
        bool marcadorFim{false};
    };

    class Parser {
    public:
    
        Parser(const std::string& expressao, std::vector<Posicao>& posicoes)
            : expressao_(expressao), posicoes_(posicoes) {}

        std::unique_ptr<Node> parse();

    private:
        const std::string& expressao_;
        std::vector<Posicao>& posicoes_;
        std::size_t index_{0};

        std::unique_ptr<Node> parseUniao();
        std::unique_ptr<Node> parseConcatenacao();
        std::unique_ptr<Node> parseRepeticao();
        std::unique_ptr<Node> parseAtomo();
        std::unique_ptr<Node> parseClasseCaracter();

        unsigned char leSimbolo();
        unsigned char leClasseSimbolo();
        bool achar(char c) const { return index_ < expressao_.size() && expressao_[index_] == c; }

        std::unique_ptr<Node> simboloFolha(std::set<unsigned char> simbolos);
        bool comecaAtomo() const;
        void pulaEspaco();
        
        void fail(const std::string& mensagem) const { throw ErroRegex(mensagem + " na posição " + std::to_string(index_ + 1)); }

        static unsigned char escaped(char c) {
            switch (c) {
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            default: return static_cast<unsigned char>(c);
            }
        }

        static std::unique_ptr<Node> binario(Tipo tipo, std::unique_ptr<Node> left, std::unique_ptr<Node> right) {
            auto node = std::make_unique<Node>();
            node->tipo = tipo;
            node->left = std::move(left);
            node->right = std::move(right);
            return node;
        }
    };   
}