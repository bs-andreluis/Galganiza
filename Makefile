CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude

LIB_SOURCES = src/automatos/afd.cpp src/automatos/afnd.cpp src/automatos/algoritmos.cpp \
              src/regex/compilador_regex.cpp src/regex/parser.cpp \
              src/lexico/gerador_lexico.cpp

.PHONY: all test clean

all: galganiza

galganiza: $(LIB_SOURCES) src/main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

galganiza_testes: $(LIB_SOURCES) testes/testes.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

test: galganiza_testes
	./galganiza_testes

clean:
	rm -f galganiza galganiza_testes
	rm -rf build
