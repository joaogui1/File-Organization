#ifndef UTILS_H
#define UTILS_H

typedef struct {
  char removido;
  int tamRegistro;
  long long int encadeamentoLista;
  int idServidor;
  double salarioServidor;
  char *telefoneServidor;
  int tamNomeServidor;
  char *nomeServidor;
  int tamCargoServidor;
  char *cargoServidor;
} registro;

typedef struct {
  char status;
  long long int topoLista;
  char tags[5];
  char campos[5][40];
} cabecalho;

void atribute_regs(registro *a, registro *b);
void print_servidor(cabecalho cab, registro reg);
void write_header(FILE *input, FILE *output);
void read_header(cabecalho *cab, FILE *input);
int file_error(FILE *input, cabecalho *cab);
void read_register(int page_size, int page_offset, char *page, registro *reg);
void write_data(FILE *input, FILE *output);

#endif /* UTILS_H */
