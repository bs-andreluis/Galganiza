#pragma once
#include <cstddef>
#include <string>
#include <sstream>

namespace automatos {
    constexpr std::size_t tamanhoAlfabeto = 256;
    using Estado = std::size_t;


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