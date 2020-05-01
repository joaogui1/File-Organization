#include "index.h"

/*
Funcao que escreve um registro em uma página de disco
input:  page, ponteiro para array de chars que representa
          nossa página de disco
        sz_pg, posicao que estamos na página antes da
          funcao comecar a executar
        registro ant_reg: registro que vamos gravar na
          página de disco
output: sz_pg, posicao que estamos na página após a
          execucao da funcao
*/
int idx_write_to_page(char **page, int sz_pg, idx_registro reg) {
  memcpy(*page + sz_pg, reg.chaveBusca, 120);
  sz_pg +=120;
  memcpy(*page + sz_pg, &reg.byteOffset, sizeof(reg.byteOffset));
  sz_pg += sizeof(reg.byteOffset);
 
  return sz_pg;
}

/*
Funcao que escreve um registro em uma página de disco
input:  page, ponteiro para array de chars que representa
          nossa página de disco
        sz_pg, posicao que estamos na página antes da
          funcao comecar a executar
        registro ant_reg: registro que vamos gravar na
          página de disco
output: sz_pg, posicao que estamos na página após a
          execucao da funcao
*/
void idx_write(FILE *output, char **page, int cnt, idx_registro *index) {
    int sz_pg = 0;
    for(int i = 0; i < cnt; i += 250){
        for(int j = 0; j < 250 && (j + i < cnt); ++j)
            sz_pg = idx_write_to_page(page, sz_pg, index[i + j]);
        fwrite(*page, sizeof(char), sz_pg, output);
        memset(*page, '@', 32000);
        sz_pg = 0;
    }

}