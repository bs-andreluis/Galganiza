#include "automatos/afd.hpp"
#include <set>

namespace automatos {

    bool AFD::aceita(const std::string& text) const { return reconhece(text).has_value(); }

    std::optional<std::size_t> AFD::reconhece(const std::string& text) const {
        if (estados.empty()) return std::nullopt;
        Estado atual = inicio;
        for (const unsigned char c : text) {
            const auto prox = estados.at(atual).transicoes[c];
            if (!prox) return std::nullopt;
            atual = *prox;
        }
        return estados.at(atual).token;
    }

    void AFD::escreveTabela(std::ostream& output, const std::vector<std::string>& tokenNames) const {
        std::set<unsigned char> alfabeto;
        for (const auto& estado : estados) {
            for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                if (estado.transicoes[c]) alfabeto.insert(static_cast<unsigned char>(c));
            }
        }

        output << "estado";
        for (const auto c : alfabeto) output << '\t' << nomeSimbolo(c);
        output << "\ttoken\n";
        for (Estado estado = 0; estado < estados.size(); ++estado) {
            output << (estado == inicio ? "->" : "  ") << estado;
            for (const auto c : alfabeto) {
                output << '\t';
                if (estados[estado].transicoes[c]) output << *estados[estado].transicoes[c];
                else output << '-';
            }
            output << '\t';
            if (estados[estado].token) {
                const auto token = *estados[estado].token;
                output << ((token < tokenNames.size()) ? tokenNames[token] : std::to_string(token));
            } else {
                output << '-';
            }
            output << '\n';
        }
    }
}