#include "automatos/algoritmos.hpp"
#include <algorithm>
#include <map>
#include <queue>

namespace automatos {
    AFD minimizar(const AFD& afd) {
        if (afd.estados.empty()) return afd;

        
        // Inicia a tabela para minimização
        const Estado morto = afd.estados.size();
        const std::size_t count = afd.estados.size() + 1;
        std::vector<std::array<Estado, tamanhoAlfabeto>> transicao(count);
        for (Estado s = 0; s < afd.estados.size(); ++s) {
            for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                transicao[s][c] = afd.estados[s].transicoes[c].value_or(morto);
            }
        }
        transicao[morto].fill(morto);

        std::vector<std::optional<std::size_t>> tokens(count);
        for (Estado s = 0; s < afd.estados.size(); ++s) tokens[s] = afd.estados[s].token;

        // cria os grupos
        std::map<std::optional<std::size_t>, std::size_t> gruposIniciais;
        std::vector<std::size_t> particao(count);
        for (Estado s = 0; s < count; ++s) {
            const auto [it, inserted] = gruposIniciais.emplace(tokens[s], gruposIniciais.size());
            (void)inserted;
            particao[s] = it->second; // agrupa por token
        }

        // computa as classes de equivalência; Continua até que não haja mudanças na iteraçao.
        while (true) {
            std::map<std::vector<std::size_t>, std::size_t> grupos;
            std::vector<std::size_t> nova_particao(count);
            for (Estado s = 0; s < count; ++s) {
                std::vector<std::size_t> assinatura;
                assinatura.reserve(tamanhoAlfabeto + 1);
                assinatura.push_back(particao[s]);
                for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                    assinatura.push_back(particao[transicao[s][c]]);
                }
                const auto [it, inserted] = grupos.emplace(std::move(assinatura), grupos.size());
                nova_particao[s] = it->second;
                (void)inserted;
            }
            if (nova_particao == particao) break;
            particao = std::move(nova_particao);
        }

        const auto qntGrupos = *std::max_element(particao.begin(), particao.end()) + 1;
        const auto grupoMorto = particao[morto];
        if (particao[afd.inicio] == grupoMorto) {
            AFD linguagemVazia;
            linguagemVazia.estados.emplace_back();
            return linguagemVazia;
        }
        
        // mapeia os grupos para seus novos IDs
        std::vector<std::optional<Estado>> remap(qntGrupos);
        Estado prox = 0;
        for (std::size_t group = 0; group < qntGrupos; ++group) {
            if (group != grupoMorto) remap[group] = prox++;
        }

        // gera o novo automato minimizado
        AFD resultado;
        resultado.estados.resize(prox);
        resultado.inicio = *remap[particao[afd.inicio]];
        std::vector<Estado> lider(qntGrupos, morto);
        for (Estado s = 0; s < count; ++s) lider[particao[s]] = s;
        for (std::size_t group = 0; group < qntGrupos; ++group) {
            if (group == grupoMorto) continue;
            const Estado s = lider[group];
            auto& estadoMinimizado = resultado.estados[*remap[group]]; // modifica resultado por referência
            estadoMinimizado.token = tokens[s];
            for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                const auto grupoAlvo = particao[transicao[s][c]];
                if (grupoAlvo != grupoMorto) estadoMinimizado.transicoes[c] = *remap[grupoAlvo];
            }
        }
        return resultado;
    }

    AFND unirViaEpsilon(const std::vector<AFD>& automatos) {
        AFND resultado;
        resultado.estados.emplace_back();
        resultado.inicio = 0;
        for (const auto& afd : automatos) {
            if (afd.estados.empty()) continue;
            const Estado offset = resultado.estados.size();
            resultado.estados.resize(offset + afd.estados.size());

            // cria a transição epsilon no inicio do AFND para todos os automatos no vetor de entrada.
            resultado.estados[resultado.inicio].epsilon.insert(offset + afd.inicio);
            for (Estado s = 0; s < afd.estados.size(); ++s) {
                resultado.estados[offset + s].token = afd.estados[s].token;
                for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                    if (const auto next = afd.estados[s].transicoes[c]) {
                        resultado.estados[offset + s].transicoes[c].insert(offset + *next);
                    }
                }
            }
        }
        return resultado;
    }

    AFD determinizar(const AFND& afnd) {
        if (afnd.estados.empty()) return {};
        AFD resultado;
        std::map<std::set<Estado>, Estado> indices;
        std::queue<std::set<Estado>> busca;

        const auto fechoInicial = afnd.epsilonFecho({afnd.inicio});
        indices.emplace(fechoInicial, 0);
        busca.push(fechoInicial);
        resultado.estados.emplace_back();
        resultado.estados[0].token = afnd.tokenSelecionado(fechoInicial);

        while (!busca.empty()) {
            const auto atual = busca.front();
            busca.pop();
            const Estado id = indices.at(atual);
            for (std::size_t c = 0; c < tamanhoAlfabeto; ++c) {
                std::set<Estado> reached;
                for (const Estado state : atual) {
                    const auto& algos = afnd.estados[state].transicoes[c];
                    reached.insert(algos.begin(), algos.end());
                }
                if (reached.empty()) continue;
                reached = afnd.epsilonFecho(std::move(reached));
                auto [it, inserted] = indices.emplace(reached, indices.size());
                if (inserted) {
                    resultado.estados.emplace_back();
                    resultado.estados[it->second].token = afnd.tokenSelecionado(reached);
                    busca.push(reached);
                }
                resultado.estados[id].transicoes[c] = it->second;
            }
        }
        return resultado;
    }

}