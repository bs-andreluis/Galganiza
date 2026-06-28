#pragma once

#include "automatos/afd.hpp"

#include <string>

namespace regex {
    // classe do compilador regex; 
    class CompiladorRegex {
        public:
            // Transforma uma expressão de entrada em um automato finito determinístico.
            automatos::AFD compilar(const std::string& expressao) const;
    };
}