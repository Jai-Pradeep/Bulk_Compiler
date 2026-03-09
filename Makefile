all:
	bison -d parser.y
	flex lexer.l
	g++ parser.tab.c lex.yy.c main.cpp ast.cpp symtab.cpp ir.cpp -o compiler

clean:
	rm -f compiler parser.tab.c parser.tab.h lex.yy.c