# GAL — Gerador de Analisadores Léxicos

Implementação em C++17 do trabalho de Linguagens Formais e Compiladores. O programa lê
definições regulares, gera e minimiza um AFD para cada padrão, une os AFDs com
ε-transições (`&` na entrada), determiniza o AFND resultante e usa essa tabela para
classificar os lexemas de um arquivo fonte.

## Compilação e testes

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Alternativamente, sem CMake:

```bash
make
make test
```

## Execução

```bash
./build/gal definicoes.er fonte.txt tokens.txt [diretorio-das-tabelas]
```

Exemplo:

```bash
./build/gal exemplos/identificadores.er \
  exemplos/fonte_identificadores.txt tokens.txt tabelas
```

O resultado tem o formato `<lexema, padrão>`; uma palavra não reconhecida produz
`<lexema, erro!>`. O quarto argumento é opcional e vale `tabelas` por padrão. Nesse
diretório são escritos os AFDs individuais minimizados e a tabela final em TSV, formato
que pode ser aberto diretamente em editores de texto ou planilhas.

## Sintaxe das expressões regulares

- Alternância: `a | b`
- Concatenação implícita: `ab`
- Fecho de Kleene: `a*`
- Fecho positivo: `a+`
- Opcional: `a?`
- Agrupamento: `(a | b)`
- Classes e intervalos: `[a-zA-Z]`, `[0-9]`
- Epsilon: `&`
- Escape: `\*`, `\|`, `\&`, `\n`, `\t`, `\r`

Espaços fora de classes são ignorados, conforme os exemplos do enunciado. As definições
seguem `nome: expressão`, uma por linha. Linhas vazias e linhas iniciadas por `#` são
ignoradas. Quando mais de um padrão reconhece o mesmo lexema, prevalece o primeiro do
arquivo de definições.

### Nota sobre o segundo exemplo do enunciado

As expressões `a?(a|b)+` e `b?(a|b)+` se sobrepõem: ambas reconhecem toda palavra não
vazia sobre `{a,b}`. Por isso, com a regra de prioridade, a primeira vence em todos os
empates. O arquivo `exemplos/inicios.er` usa `a(a|b)*` e `b(a|b)*`, que correspondem à
descrição e à saída pretendida no anexo (palavras iniciadas por `a` ou por `b`).

## Organização

- `regex.cpp`: parser e construção direta de AFD pelo algoritmo de Aho, com
  `nullable`, `firstpos`, `lastpos` e `followpos`.
- `automaton.cpp`: minimização, união por ε-transição, determinização, reconhecimento e
  visualização tabular.
- `lexer_generator.cpp`: fachada de projeto/execução do analisador léxico.
- `tests/` e `exemplos/`: testes independentes e conjuntos variados para avaliação.

O arquivo fonte é tratado como um conjunto de lexemas separados por espaço em branco,
tal como especificado e exemplificado no enunciado.
