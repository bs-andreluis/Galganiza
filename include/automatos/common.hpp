#pragma once
#include <cstddef>
#include <string>
#include <sstream>

namespace automatos {
    // tamanho configurável do alfabeto, consideramos a tabela ASCII onde 1 char = 1 byte = 256 possíveis valores;
    constexpr std::size_t tamanhoAlfabeto = 256;
    // Estados são representados como 'size_t'; são basicamente indices para os vetores das classes de automatos.
    using Estado = std::size_t;

    // resolve o nome do símbolo para exibí-lo na tela; Lida com caracteres especiais do C como \n. 
    inline std::string nomeSimbolo(unsigned char c) {
        switch (c) {
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case ' ': return "espaço";
        default:
            if (c >= 33 && c <= 126) return std::string(1, static_cast<char>(c));
            std::ostringstream output;
            output << "0x" << std::hex << std::uppercase << static_cast<int>(c);
            return output.str();
        }
    }
}