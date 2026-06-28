#pragma once
#include "common.hpp"
#include <array>
#include <optional>
#include <vector>
#include <string>

namespace automatos{
    /*
        Cada estado transiciona para apenas um outro estado, não há transicoes epsilon.
        Cada estado associa um token opcional da entrada ER (Número do token, nullopt = não há). 
    */
    struct EstadoAFD {
        std::array<std::optional<Estado>, tamanhoAlfabeto> transicoes{};
        std::optional<std::size_t> token;
    };
    
    /*
        Classe de Automato finito determinístico. 
    */
    class AFD {
        public:
        Estado inicio{0};
        std::vector<EstadoAFD> estados;
        
        bool aceita(const std::string& text) const;
        std::optional<std::size_t> reconhece(const std::string& text) const;
        void escreveTabela(std::ostream& output, const std::vector<std::string>& tokenNames = {}) const;
    };
}