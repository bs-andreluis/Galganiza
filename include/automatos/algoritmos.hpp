#pragma once

#include "afnd.hpp"
#include "afd.hpp"


namespace automatos {
    // Minimiza um automato, não modifica a entrada, retorna uma cópia na saída.
    AFD minimizar(const AFD& afd);

    // Une um vetor de automatos determinísticos, resultando em um automato não determinístico.
    AFND unirViaEpsilon(const std::vector<AFD>& automato);

    // Determiniza um automato não determinístico, não modifica a entrada, retorna uma cópia na saída.
    AFD determinizar(const AFND& afnd);
}
