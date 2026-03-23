CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function

GENSRC   = parser.tab.cpp lex.yy.cpp
CPPSRC   = main.cpp ast.cpp symtab.cpp ir.cpp depcheck.cpp optimizer.cpp codegen.cpp
SRC      = $(GENSRC) $(CPPSRC)

all: compiler

compiler: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o compiler

# ── Bison: always produce .c/.h first, then rename to .cpp
#    Works on every bison version (avoids .hpp vs .h ambiguity)
parser.tab.cpp parser.tab.h: parser.y
	bison -d parser.y
	mv parser.tab.c parser.tab.cpp

# ── Flex
lex.yy.cpp: lexer.l parser.tab.h
	flex lexer.l
	mv lex.yy.c lex.yy.cpp

test_for: compiler
	./compiler -o simple_for.ir < simple_for.bc
	@echo ""; cat simple_for.ir

test_bubble: compiler
	./compiler -o bubble_sort.ir < bubble_sort.bc
	@echo ""; cat bubble_sort.ir

clean:
	rm -f compiler parser.tab.cpp parser.tab.h lex.yy.cpp *.ir *.o