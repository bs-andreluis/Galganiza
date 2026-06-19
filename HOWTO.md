# HowTo rápido

Requisitos: compilador compatível com C++17 e `make` (ou CMake 3.16+).

```bash
make
make test
./gal exemplos/identificadores.er exemplos/fonte_identificadores.txt tokens.txt tabelas
```

Arquivos produzidos:

- `tokens.txt`: pares `<lexema, padrão>` e eventuais `<lexema, erro!>`;
- `tabelas/afd_N_nome.tsv`: AFD minimizado de cada expressão regular;
- `tabelas/tabela_analise_lexica.tsv`: AFD final obtido após união por ε e determinização.

Para criar outro analisador, escreva um arquivo com uma regra por linha:

```text
nome_do_token: expressão regular
```

Os lexemas no arquivo fonte devem ser separados por espaços, tabulações ou quebras de
linha. Consulte o `README.md` para a sintaxe completa e para a compilação com CMake.
