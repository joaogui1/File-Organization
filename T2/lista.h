#ifndef LISTA_H
#define LISTA_H
#include "utilidades.h"


typedef struct Node {
  long int pos;
  int size, priority;
  struct Node *next;
} node;

typedef struct {
  node *begin, *end;
} list;
list *create();
void *from_file(list *removed, FILE *input, cabecalho *cab);
void insert(list *l, long int pos, int size, int priority);
void insert_end(list *l, long int pos, int size);
long best_fit(list *l, int size);
void destroy(list *l);
void print_list(list *l);
#endif /* LIST_H */