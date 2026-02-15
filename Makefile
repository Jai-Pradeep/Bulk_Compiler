CC = gcc
FLEX = flex
BISON = bison

TARGET = parser

SRCS = ast.c symtab.c parser.tab.c lex.yy.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

$(TARGET): parser.tab.c lex.yy.c ast.c symtab.c
	$(CC) ast.c symtab.c lex.yy.c parser.tab.c -o $(TARGET)

clean:
	rm -f $(TARGET) *.o lex.yy.c parser.tab.c parser.tab.h

