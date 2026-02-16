#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static Symbol *table = NULL;

void insert_symbol(char *name, SymType type, BaseType baseType, int size) {
    if (lookup_symbol(name)) {
        printf("Error: redeclaration of %s\n", name);
        exit(1);
    }

    Symbol *s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->type = type;
    s->baseType = baseType;
    s->size = size;
    s->next = table;
    table = s;
}

Symbol* lookup_symbol(char *name) {
    Symbol *curr = table;
    while (curr) {
        if (strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

