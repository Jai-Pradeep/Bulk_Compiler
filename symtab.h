#ifndef SYMTAB_H
#define SYMTAB_H


typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR
} BaseType;

typedef enum {
    SYM_SCALAR,
    SYM_ARRAY
} SymType;


typedef struct Symbol {
    char *name;
    SymType type;
    BaseType baseType;
    int size; // for arrays, 0 for scalars
    struct Symbol *next;
} Symbol;

void insert_symbol(char *name, SymType type, BaseType baseType, int size);
Symbol* lookup_symbol(char *name);

#endif

