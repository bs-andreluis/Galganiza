# HowTo rápido

Requisitos: compilador compatível com C++17 e `make` (ou CMake 3.16+).

```bash
make            # compila os dois executáveis: lexico e sintatico
make lexico     # compila apenas o analisador léxico
make sintatico  # compila apenas o analisador sintático
make test
# tokens só no stdout:
./lexico exemplos/identificadores.er exemplos/fonte_identificadores.txt
# tokens no stdout e também salvos em tokens.txt:
./lexico exemplos/identificadores.er exemplos/fonte_identificadores.txt tokens.txt
# análise sintática SLR sobre os exemplos em exemplos_sintatico/:
./sintatico
```

Os tokens são sempre impressos no `stdout`. Arquivos produzidos:

- `tokens.txt` (opcional, só se o terceiro argumento for informado): pares
  `<lexema, padrão>` e eventuais `<lexema, erro!>`;
- `tabelas/afd_N_nome.tsv`: AFD minimizado de cada expressão regular;
- `tabelas/tabela_analise_lexica.tsv`: AFD final obtido após união por ε e determinização.

Para criar outro analisador, escreva um arquivo com uma regra por linha:

```text
nome_do_token: expressão regular
```

Os lexemas no arquivo fonte devem ser separados por espaços, tabulações ou quebras de
linha. Consulte o `README.md` para a sintaxe completa e para a compilação com CMake.
