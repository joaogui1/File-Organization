#include <stdlib.h>
#include <stdio.h>
#include "lista.h"

/*
Cria uma lista encadeada ordenada com nós sentinelas
onde o begin é o menor de todos e o end o maior
input:
output: list *l, a lista criada
*/
list *create(){
    list *l;
    l = calloc(1, sizeof(list));
    l->begin = calloc(1, sizeof(node));
    l->end = calloc(1, sizeof(node));

    l->begin->pos = -1;
    l->begin->size = -1;

    l->end->pos = -1; 
    l->end->size = (1 << 30);

    l->begin->next = l->end;
    return l;
}

/*
Cria Insere a lista de removidos de um arquivo binário
na lista removed
input:  list *removed: Lista onde vamos inserir os removidos
                      do arquivo binário
        FILE *input:   Arquivo binário que será processado
        cabecalho *cab: Ponteiro para registro de cabecalho
                        do arquivo input
output: void
*/
void *from_file(list *removed, FILE* input, cabecalho *cab){
  int size;
  char is_removed;
  long int pos = cab->topoLista;
  while (pos != -1) {
    fseek(input, pos, SEEK_SET);
    fread(&is_removed, sizeof(char), 1, input);
    fread(&size, sizeof(int), 1, input);
    insert_end(removed, pos, size);
    fread(&pos, sizeof(long int), 1, input);
  }
  fseek(input, 32000, SEEK_SET);
}

/*
Insere um elemento na lista de removidos de forma a mantê-la em ordem
crescente pelo tamanho dos elementos
input: list *l, lista encadeada ordenada
       long int pos: posicão na memória do registro removido que está sendo
                      adicionado a lista
        int size: tamanho do registro removido que está sendo adicionado a 
                    lista
output: void
*/
void insert(list *l, long int pos, int size, int priority){
    node *next_node, *new_node;

    new_node = calloc(1, sizeof(node));
    new_node->pos = pos;
    new_node->size = size;
    new_node->priority = priority;
   
    for(node *n = l->begin; n != l->end; n = n->next){
        next_node = n->next;
        if (next_node->size > size) {
          new_node->next = next_node;
          n->next = new_node;
          return;
        } else if (next_node->size == size && next_node->priority <= priority) {
          new_node->next = next_node;
          n->next = new_node;
          return;
        }

    }

    l->end->next = new_node;
    l->end = new_node;
}

/*
Insere um elemento na lista de removidos logo
antes do nó sentinela
input: list *l, lista encadeada ordenada
       long int pos: posicão na memória do registro removido que está sendo
                      adicionado a lista
        int size: tamanho do registro removido que está sendo adicionado a
                    lista
output: void
*/
void insert_end(list *l, long int pos, int size){
    node *new_node, *n;

    new_node = calloc(1, sizeof(node));
    new_node->pos = pos;
    new_node->size = size;
    new_node->priority = 0;
    for (n = l->begin; n->next != l->end; n = n->next){
      if (pos == n->pos) return;
    }

    n->next = new_node;
    new_node->next = l->end;
}

/*
Imprime a lista encadeada
input: list *l, lista encadeada ordenada
output: void
*/
void print_list(list *l){
  printf("imprimindo lista\n");
  for (node *n = l->begin; n != NULL; n = n->next) {
    printf("lista %d %ld\n", n->size, n->pos);
  }
}

/*
Retorna o best fit dado o tamanho size
input: list *l, lista encadeada ordenada
        int size: tamanho do registro que será inserido no arquivo
output: long ret: posicão no arquivo binário usado para criar a lista
                  do best fit do tamanho dado, -1 caso ele seja grande 
                  demais
*/
long best_fit(list *l, int size) { 
  long int ret;
  node *aux;
  for (node *n = l->begin; n != l->end; n = n->next){
    aux = n->next;
    if(aux  == l->end)  break;
    if(aux->size >= size){
      n->next = aux->next;
      ret = aux->pos;
      free(aux);
      return ret;
    }
  } 
  return -1;
}

/*
Libera toda a memória da lista encadeada l
input: list *l, lista encadeada ordenada
output: void
*/
void destroy(list *l){
    node *aux;
    for (node* n = l->begin; n != NULL;){
        aux = n->next;
        free(n);
        n = aux;
    }
    free(l);
}