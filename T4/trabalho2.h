#include "lista.h"
void remove_binary(char *nome, int n);
int insert_register(FILE *input, list *removed, registro reg, cabecalho cab);
void insert_binary(char *nome, int n);
void update_binary(char *nome, int n);

typedef struct {
  enum { ID, SALARIO, TELEFONE, NOME, CARGO } type;
  union {
    int id;
    double salario;
    char telefone[15], nome[128], cargo[128];
  } value;
} query;
