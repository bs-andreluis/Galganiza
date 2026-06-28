#pragma once
#include "common.hpp"
#include <set>
#include <array>
#include <optional>
#include <vector>


namespace automatos {
    /*
        Cada estado transiciona para um conjunto de estados, podendo possuir transição epsilon.
        Cada estado associa um token opcional da entrada ER (Número do token, nullopt = não há). 
    */
    struct EstadoAFND {
        std::array<std::set<Estado>, tamanhoAlfabeto> transicoes{};
        std::set<Estado> epsilon;
        std::optional<std::size_t> token;
    };

    /*
        classe de Automato Finito não determinístico.
    */
    class AFND {
    public:
        Estado inicio{0};
        std::vector<EstadoAFND> estados;

        std::set<Estado> epsilonFecho(std::set<Estado> states) const;

        // pega o maior token, dado um conjunto de possiveis (se existir).
        std::optional<std::size_t> tokenSelecionado(const std::set<Estado>& estados) const;
    };
}