#!/usr/bin/env bash
#
# Executa o pipeline completo do trabalho:
#
#   análise léxica  (lexico)  ->  tokens.txt  ->  análise sintática (sintatico)
#
# O analisador léxico gera a tabela de tokens a partir das definições regulares
# e do arquivo fonte; essa tabela é então consumida pelo analisador sintático,
# junto com a gramática e as palavras reservadas.
#
# Uso:
#   ./executar_pipeline.sh [definicoes.er] [fonte.txt] [regras.txt] [palavras_reservadas.txt]
#
# Sem argumentos, roda um exemplo padrão.

set -uo pipefail

cd "$(dirname "$0")"

DEFINICOES="${1:-exemplos/pipeline.er}"
FONTE="${2:-exemplos/fonte_pipeline.txt}"
REGRAS="${3:-exemplos_sintatico/regras1.txt}"
PALAVRAS="${4:-exemplos_sintatico/palavras1.txt}"

# Arquivo intermediário: saída do léxico e entrada do sintático.
TOKENS="tokens.txt"

# Compila os executáveis caso ainda não existam.
if [[ ! -x ./lexico || ! -x ./sintatico ]]; then
	echo ">> Compilando executáveis (make lexico sintatico)..."
	make lexico sintatico -j2 || exit 1
fi

echo
echo "==================== ANÁLISE LÉXICA ===================="
echo ">> ./lexico \"$DEFINICOES\" \"$FONTE\" \"$TOKENS\""
echo
if ! ./lexico "$DEFINICOES" "$FONTE" "$TOKENS"; then
	echo ">> Falha na análise léxica." >&2
	exit 1
fi

echo
echo "==================== ANÁLISE SINTÁTICA ===================="
echo ">> ./sintatico \"$REGRAS\" \"$PALAVRAS\" \"$TOKENS\""
echo
if ! ./sintatico "$REGRAS" "$PALAVRAS" "$TOKENS"; then
	echo ">> Falha na análise sintática." >&2
	exit 1
fi

echo
echo ">> Pipeline concluído. Tokens intermediários em: $TOKENS"
