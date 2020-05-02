#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
  char *chaveBusca;
  long byteOffset;
} idx_registro;

typedef struct {
  char status;
  int nroRegistros;
} idx_cabecalho;

int idx_write_to_page(char **page, int sz_pg, idx_registro reg);
int load_idx(FILE *index_file, idx_registro **index, int *index_pages);
void idx_write(FILE *output, char **page, int cnt, idx_registro *index);
int binary_search(idx_registro *index, int size, char *nomeServidor);