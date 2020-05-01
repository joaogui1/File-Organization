#ifndef UTILS_H
#define UTILS_H

typedef struct {
  char removido;
  int tamRegistro;
  long int encadeamentoLista;
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
int write_to_page(char **page, int sz_pg, registro ant_reg);
void read_register_from_binary(FILE *input, registro *reg);
void write_data(FILE *input, FILE *output);
int new_write_to_page(FILE *output, char **page, int sz_pg, registro ant_reg, registro reg);

#endif /* UTILS_H */
