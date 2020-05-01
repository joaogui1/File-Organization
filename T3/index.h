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
void idx_write(FILE *output, char **page, int cnt, idx_registro *index);