#ifndef SYMTAB_H
#define SYMTAB_H

typedef enum {
    SYM_SCALAR,
    SYM_ARRAY
} SymType;

typedef struct Symbol {
    char *name;
    SymType type;
    struct Symbol *next;
} Symbol;

void insert_symbol(char *name, SymType type);
Symbol* lookup_symbol(char *name);

#endif

