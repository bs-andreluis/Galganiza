#include "automatos/afnd.hpp"

namespace automatos {

    std::set<Estado> AFND::epsilonFecho(std::set<Estado> estados_c) const{
        std::vector<Estado> busca(estados_c.begin(), estados_c.end());
        while (!busca.empty()) {
            const Estado atual = busca.back();
            busca.pop_back();
            for (const Estado prox : this->estados.at(atual).epsilon) {
                if (estados_c.insert(prox).second) busca.push_back(prox);
            }
        }
        return estados_c;
    }


    std::optional<std::size_t> AFND::tokenSelecionado(const std::set<Estado>& estados) const{
        std::optional<std::size_t> selected;
        for (const Estado s : estados) {
            const auto token = this->estados.at(s).token;
            if (token && (!selected || *token < *selected)) selected = token;
        }
        return selected;
    }
}