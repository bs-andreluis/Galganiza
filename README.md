# GAL — Gerador de Analisadores Léxicos

Implementação em C++17 do trabalho de Linguagens Formais e Compiladores. O programa lê
definições regulares, gera e minimiza um AFD para cada padrão, une os AFDs com
ε-transicoes (`&` na entrada), determiniza o AFND resultante e usa essa tabela para
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

A compilação gera dois executáveis: `galganiza_lexico` (analisador léxico) e
`galganiza_sintatico` (analisador sintático). Pelo `make`, os binários se chamam
`lexico` e `sintatico`.

## Execução

```bash
./build/galganiza_lexico definicoes.er fonte.txt [tokens.txt]
```

Exemplo:

```bash
./build/galganiza_lexico exemplos/identificadores.er \
  exemplos/fonte_identificadores.txt tokens.txt
```

O analisador sintático lê as regras, palavras reservadas e tokens de
`exemplos_sintatico/` e executa a análise SLR:

```bash
./build/galganiza_sintatico
```

O resultado é sempre impresso na saída padrão (`stdout`) no formato `<lexema, padrão>`;
uma palavra não reconhecida produz `<lexema, erro!>`. O terceiro argumento `tokens.txt`
é opcional: quando informado, a mesma saída também é gravada nesse arquivo; quando
omitido, os tokens aparecem apenas no `stdout`. As tabelas são sempre escritas no
diretório `tabelas/`, onde ficam os AFDs individuais minimizados e a tabela final em
formato TSV.

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

- namespace `regex`: parser e construção direta de AFD pelo algoritmo de Aho, com
  `nullable`, `firstpos`, `lastpos` e `followpos`.
- namespace `automatos`: minimização, união por ε-transição, determinização, reconhecimento e
  visualização da tabela.
- namespace `lexico`: analisador léxico.
- namespace `sintatico`: analisador sintático SLR (gramática, parser e tabela de símbolos).
- `testes/` e `exemplos/`: testes independentes e conjuntos variados para avaliação.

O arquivo fonte é tratado como um conjunto de lexemas separados por espaço em branco,
tal como especificado e exemplificado no enunciado.
