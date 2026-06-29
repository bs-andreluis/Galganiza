CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude

LIB_SOURCES = src/automatos/afd.cpp src/automatos/afnd.cpp src/automatos/algoritmos.cpp \
              src/regex/compilador_regex.cpp src/regex/parser.cpp \
              src/lexico/gerador_lexico.cpp \
			  src/parser/grammar.cpp src/parser/parser.cpp src/parser/tabelaSimbolos.cpp

.PHONY: all test clean

all: parser

parser: $(LIB_SOURCES) src/execute_parser.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

parser_testes: $(LIB_SOURCES) testes/testes.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

test: parser_testes
	./parser_testes

clean:
	rm -f parser parser_testes
	rm -rf build
