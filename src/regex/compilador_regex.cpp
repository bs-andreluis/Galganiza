#include "regex/compilador_regex.hpp"
#include "regex/parser.hpp"

#include <set>
#include <vector>
#include <map>
#include <queue>

namespace regex {
    namespace {
        std::set<std::size_t> setUnion(std::set<std::size_t> left, const std::set<std::size_t>& right) {
            left.insert(right.begin(), right.end());
            return left;
        }

        void anota(Node& node, std::vector<std::set<std::size_t>>& follow) {
            if (node.left) anota(*node.left, follow);
            if (node.right) anota(*node.right, follow);

            switch (node.tipo) {
                case Tipo::Simbolo:
                case Tipo::MarcadorFim:
                    node.nullable = false;
                    node.first = node.last = {node.posicao};
                    break;
                case Tipo::Epsilon:
                    node.nullable = true;
                    break;
                case Tipo::Uniao:
                    node.nullable = node.left->nullable || node.right->nullable;
                    node.first = setUnion(node.left->first, node.right->first);
                    node.last = setUnion(node.left->last, node.right->last);
                    break;
                case Tipo::Concatenacao:
                    node.nullable = node.left->nullable && node.right->nullable;
                    node.first = node.left->nullable ? setUnion(node.left->first, node.right->first) : node.left->first;
                    node.last = node.right->nullable ? setUnion(node.left->last, node.right->last) : node.right->last;
                    for (const auto position : node.left->last) {
                        follow[position].insert(node.right->first.begin(), node.right->first.end());
                    }
                    break;
                case Tipo::Fecho:
                case Tipo::Plus:
                    node.nullable = node.tipo == Tipo::Fecho || node.left->nullable;
                    node.first = node.left->first;
                    node.last = node.left->last;
                    for (const auto position : node.last) {
                        follow[position].insert(node.first.begin(), node.first.end());
                    }
                    break;
                case Tipo::Opcional:
                    node.nullable = true;
                    node.first = node.left->first;
                    node.last = node.left->last;
                    break;
                }
        }
    } // namespace

    automatos::AFD CompiladorRegex::compilar(const std::string& expressao) const {
        std::vector<Posicao> posicoes;
        Parser parser(expressao, posicoes);
        auto raizExpressao = parser.parse();

        auto marker = std::make_unique<Node>();
        marker->tipo = Tipo::MarcadorFim;
        marker->posicao = posicoes.size();
        const std::size_t posicaoMarcador = posicoes.size();
        posicoes.push_back({{}, true});

        auto raiz = std::make_unique<Node>();
        raiz->tipo = Tipo::Concatenacao;
        raiz->left = std::move(raizExpressao);
        raiz->right = std::move(marker);

        std::vector<std::set<std::size_t>> follow(posicoes.size());
        anota(*raiz, follow);

        std::set<unsigned char> alfabeto;
        for (const auto& position : posicoes) {
            alfabeto.insert(position.simbolos.begin(), position.simbolos.end());
        }

        automatos::AFD afd;
        std::map<std::set<std::size_t>, automatos::Estado> indices;
        std::queue<std::set<std::size_t>> busca;
        indices.emplace(raiz->first, 0);
        busca.push(raiz->first);
        afd.estados.emplace_back();
        if (raiz->first.count(posicaoMarcador)) afd.estados[0].token = 0;

        while (!busca.empty()) {
            const auto atual = busca.front();
            busca.pop();
            const automatos::Estado idAtual = indices.at(atual);
            for (const unsigned char c : alfabeto) {
                std::set<std::size_t> alvo;
                for (const auto posicao : atual) {
                    if (posicoes[posicao].simbolos.count(c)) {
                        alvo.insert(follow[posicao].begin(), follow[posicao].end());
                    }
                }
                if (alvo.empty()) continue;
                auto [it, inserted] = indices.emplace(alvo, indices.size());
                if (inserted) {
                    afd.estados.emplace_back();
                    if (alvo.count(posicaoMarcador)) afd.estados[it->second].token = 0;
                    busca.push(alvo);
                }
                afd.estados[idAtual].transicoes[c] = it->second;
            }
        }
        return afd;
    }

} // namespace regex
