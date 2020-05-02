#include "index.h"

/*
Funcao que escreve um registro de indice em uma página de disco
input:  page, ponteiro para array de chars que representa
          nossa página de disco
        sz_pg, posicao que estamos na página antes da
          funcao comecar a executar
        idx_registro reg: registro que vamos gravar na
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
Funcao que escreve um vetor de índices em um arquivo binário
input:  FILE *output: arquivo onde escreveremos nosso índice
        page, ponteiro para array de chars que representa
          nossa página de disco
        int cnt: número de índices a serem escritos
        idx_registro *index: vetor de índices que guarda
                            os registros de índice que serão 
                            escritos
output: void
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

/*
Funcao que carrega um vetor de índices de um arquivo binário
input:  FILE *index_file: arquivo de onde carregaremos nosso índice
        idx_registro **index: ponteiro para vetor de índices 
                              onde carregaremos os dados
                              nosso arquivo de índice
        int *index_pages: ponteiro para um inteiro que guarda
                          o número de páginas que acessamos para
                          carregar o índice
output: número de registros no índice
*/
int load_idx(FILE *index_file, idx_registro **index, int *index_pages) {
  if(!index_file){
    return -1;
  }
  idx_cabecalho cab;
  fread(&cab.status, sizeof(char), 1, index_file);

  if(cab.status != '1'){
    return -1;
  }
  fread(&cab.nroRegistros, sizeof(int), 1, index_file);
  fseek(index_file, 31995, SEEK_CUR);
  

  *index = calloc(cab.nroRegistros, sizeof(idx_registro));

  for(int i = 0; i < cab.nroRegistros; ++i){
    (*index)[i].chaveBusca = malloc(120);
    fread((*index)[i].chaveBusca, sizeof(char), 120, index_file);
    fread(&((*index)[i].byteOffset), sizeof(long), 1, index_file);
  }
  *index_pages = (31999 + ftell(index_file))/32000;
  return cab.nroRegistros;
}

/*
Funcao que faz busca binária em um vetor de índices
input:  idx_registro *index: vetor de índices que guarda
                            os registros de índice que serão
                            escritos
        int size: número de registros de índice no vetor index
        char *nomeServidor: nome a ser buscado
output: retorna a posicão em index do último registro de índice
        com nome lexicograficamente menor que nome servidor (lower_bound)
*/
int binary_search(idx_registro *index, int size, char *nomeServidor){
  int ini = 0, fim = size, m, cmp;
  while(fim > ini + 1){
    m = (ini + fim)/2;  
    cmp = strcmp(index[m].chaveBusca, nomeServidor);
    if(cmp >= 0) fim = m;
    else if(cmp < 0)  ini = m;
  }
  return ini;
}