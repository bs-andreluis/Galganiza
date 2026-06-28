#include "lexico/gerador_lexico.hpp"
#include "automatos/algoritmos.hpp"
#include "regex/compilador_regex.hpp"
#include "regex/regex.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>

namespace lexico {
    namespace {

        std::string trim(const std::string& val) {
            const auto first = val.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return {};
            const auto last = val.find_last_not_of(" \t\r\n");
            return val.substr(first, last - first + 1);
        }

        std::string filenameSeguro(std::string nome) {
            for (char& c : nome) {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') c = '_';
            }
            return nome.empty() ? "token" : nome;
        }

    } // namespace

    void GeradorLexico::carregarDefinicoes(std::istream& input) {
        definicoes_.clear();
        individual_.clear();
        analise_ = {};
        construido_ = false;

        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(input, line)) {
            ++lineNumber;
            const auto clean = trim(line);
            if (clean.empty() || clean.front() == '#') continue;
            const auto separator = clean.find(':');
            if (separator == std::string::npos) {
                throw std::runtime_error("linha " + std::to_string(lineNumber) + ": ':' ausente");
            }
            lexico::DefinicaoDoToken definition{trim(clean.substr(0, separator)), trim(clean.substr(separator + 1))};
            if (definition.nome.empty()) {
                throw std::runtime_error("linha " + std::to_string(lineNumber) + ": nome do padrão vazio");
            }
            if (definition.expressao.empty()) {
                throw std::runtime_error("linha " + std::to_string(lineNumber) + ": expressão regular vazia");
            }
            for (const auto& existing : definicoes_) {
                if (existing.nome == definition.nome) {
                    throw std::runtime_error("linha " + std::to_string(lineNumber) +
                                            ": padrão duplicado '" + definition.nome + "'");
                }
            }
            definicoes_.push_back(std::move(definition));
        }
        if (definicoes_.empty()) throw std::runtime_error("nenhuma definição regular encontrada");
    }

    void GeradorLexico::construir() {
        if (definicoes_.empty()) throw std::logic_error("carregue as definições antes de gerar o analisador");
        regex::CompiladorRegex compilador;
        individual_.clear();
        individual_.reserve(definicoes_.size());
        for (std::size_t token = 0; token < definicoes_.size(); ++token) {
            try {
                auto afd = automatos::minimizar(compilador.compilar(definicoes_[token].expressao));
                for (auto& s : afd.estados) {
                    if (s.token) s.token = token;
                }
                individual_.push_back(std::move(afd));
            } catch (const regex::ErroRegex& error) {
                throw regex::ErroRegex("padrão '" + definicoes_[token].nome + "': " + error.what());
            }
        }
        analise_ = automatos::determinizar(automatos::unirViaEpsilon(individual_));
        construido_ = true;
    }

    void GeradorLexico::analise(std::istream& fonte, std::ostream& output) const {
        if (!construido_) throw std::logic_error("o analisador léxico ainda não foi gerado");
        std::string lexema;
        while (fonte >> lexema) {
            const auto token = analise_.reconhece(lexema);
            if (token) output << '<' << lexema << ", " << definicoes_.at(*token).nome << ">\n";
            else output << '<' << lexema << ", erro!>\n";
        }
    }

    void GeradorLexico::exportarTabelas(const std::filesystem::path& diretorio) const {
        if (!construido_) throw std::logic_error("o analisador léxico ainda não foi gerado");
        std::filesystem::create_directories(diretorio);
        std::vector<std::string> nomes;
        for (const auto& definition : definicoes_) nomes.push_back(definition.nome);

        for (std::size_t index = 0; index < individual_.size(); ++index) {
            const auto path = diretorio / ("afd_" + std::to_string(index + 1) + "_" +
                                        filenameSeguro(definicoes_[index].nome) + ".tsv");
            std::ofstream output(path);
            if (!output) throw std::runtime_error("não foi possível criar '" + path.string() + "'");
            individual_[index].escreveTabela(output, nomes);
        }
        const auto path = diretorio / "tabela_analise_lexica.tsv";
        std::ofstream output(path);
        if (!output) throw std::runtime_error("não foi possível criar '" + path.string() + "'");
        analise_.escreveTabela(output, nomes);
    }

    const std::vector<lexico::DefinicaoDoToken>& GeradorLexico::definicoes() const noexcept { return definicoes_; }
    const std::vector<automatos::AFD>& GeradorLexico::automatoIndividual() const noexcept { return individual_; }
    const automatos::AFD& GeradorLexico::tabelaDeAnalise() const {
        if (!construido_) throw std::logic_error("o analisador léxico ainda não foi gerado");
        return analise_;
    }

} // namespace lexico

