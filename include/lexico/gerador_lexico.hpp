#pragma once
#include "automatos/afd.hpp"

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace lexico {

    struct DefinicaoDoToken {
        std::string nome;
        std::string expressao;
    };

    class GeradorLexico {
    public:
        void carregarDefinicoes(std::istream& input);
        void construir();
        void analise(std::istream& fonte, std::ostream& output) const;
        void exportarTabelas(const std::filesystem::path& diretorio) const;

        const std::vector<DefinicaoDoToken>& definicoes() const noexcept;
        const std::vector<automatos::AFD>& automatoIndividual() const noexcept;
        const automatos::AFD& tabelaDeAnalise() const;

    private:
        std::vector<DefinicaoDoToken> definicoes_;
        std::vector<automatos::AFD> individual_;
        automatos::AFD analise_;
        bool construido_{false};
    };

}
