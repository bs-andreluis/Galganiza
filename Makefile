CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude

LIB_SOURCES = src/automatos/afd.cpp src/automatos/afnd.cpp src/automatos/algoritmos.cpp \
              src/regex/compilador_regex.cpp src/regex/parser.cpp \
              src/lexico/gerador_lexico.cpp \
              src/sintatico/grammar.cpp src/sintatico/parser.cpp src/sintatico/tabelaSimbolos.cpp

.PHONY: all lexico sintatico test clean

all: lexico sintatico

lexico: $(LIB_SOURCES) src/main_lexico.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

sintatico: $(LIB_SOURCES) src/main_sintatico.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

galganiza_testes: $(LIB_SOURCES) testes/testes.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

test: galganiza_testes
	./galganiza_testes

clean:
	rm -f lexico sintatico galganiza_testes
	rm -rf build
