CXX     = g++
CXXFLAGS= -std=c++17 -Wall -Wextra

SRC     = parser.tab.c lex.yy.c main.cpp ast.cpp symtab.cpp ir.cpp depcheck.cpp
OBJ     = $(SRC:.cpp=.o)

all: compiler

compiler: parser.tab.c lex.yy.c
	$(CXX) $(CXXFLAGS) $(SRC) -o compiler

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

# ── Run test cases ────────────────────────────────────────────────────────────
test_for: compiler
	./compiler -o simple_for.ir < simple_for.bc
	@echo ""; cat simple_for.ir

test_bubble: compiler
	./compiler -o bubble_sort.ir < bubble_sort.bc
	@echo ""; cat bubble_sort.ir

clean:
	rm -f compiler parser.tab.c parser.tab.h lex.yy.c *.ir