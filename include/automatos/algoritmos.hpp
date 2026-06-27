#pragma once

#include "afnd.hpp"
#include "afd.hpp"


namespace automatos {
    AFD minimizar(const AFD& afd);
    AFND unirViaEpsilon(const std::vector<AFD>& automato);
    AFD determinizar(const AFND& afnd);
}
