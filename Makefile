CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude

LIB_SOURCES = src/automaton.cpp src/regex.cpp src/lexer_generator.cpp

.PHONY: all test clean

all: gal

gal: $(LIB_SOURCES) src/main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

gal_tests: $(LIB_SOURCES) tests/tests.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

test: gal_tests
	./gal_tests

clean:
	rm -f gal gal_tests
