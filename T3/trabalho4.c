#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SCC0215012019escreverTela2.h"
#include "trabalho3.h"
#include "utilidades.h"
#include "index.h"
#include "trabalho4.h"

int comp_idx(const void *a, const void *b) {
    int strdiff = strcmp(((idx_registro *)a)->chaveBusca, ((idx_registro *)b)->chaveBusca); 

    if(strdiff != 0)
        return strdiff;

    return (((idx_registro *)a)->byteOffset - ((idx_registro *)b)->byteOffset);
}

void create_index(char *nome_input, char *nome_output){
    FILE *input, *output;
    cabecalho cab;
    registro reg;
    long file_size = 0;
    idx_cabecalho idx_cab;
    idx_registro index[6400];
    int sz_pg = 0, offset = 0, page_offset = 0;
    int num_pages = 1, page_size = 32000, cnt = 0;
    char aux_tag, *page;

    input = fopen(nome_input, "rb");
    if (file_error(input, &cab)) return;
    fseek(input, 32000, SEEK_SET);
    
    page = malloc(32000);
    reg.nomeServidor = calloc(128, sizeof(char));
    reg.cargoServidor = calloc(128, sizeof(char));
    reg.telefoneServidor = calloc(15, sizeof(char));
    
    while (!feof(input)) {
      page_size = fread(page, sizeof(char), 32000, input);
      ++num_pages;
      page_offset = 0;

      while (page_offset < page_size) {
        read_register(page_size, page_offset, page, &reg);
        if(reg.removido == '-' && reg.tamNomeServidor > 0){
            index[cnt].chaveBusca = malloc(120);
            memset(index[cnt].chaveBusca, '@', 120);
                strcpy(index[cnt].chaveBusca, reg.nomeServidor);
            index[cnt].byteOffset = (num_pages - 1)*32000 + page_offset;
            ++cnt;
        }
        page_offset += reg.tamRegistro + 5;
        
      }
    }


    memset(page, '@', 32000);
    qsort(index, cnt, sizeof(idx_registro), comp_idx);

    output = fopen(nome_output, "wb");
    idx_cab.status = '0';
    idx_cab.nroRegistros = cnt;
    fwrite(&idx_cab.status, sizeof(char), 1, output);
    fwrite(&idx_cab.nroRegistros, sizeof(int), 1, output);
    fwrite(page, sizeof(char), 31995, output);

    idx_write(output, &page, cnt, index);

    fseek(output, 0, SEEK_SET);
    idx_cab.status = '1';
    fwrite(&idx_cab.status, sizeof(char), 1, output);

    for(int i = 0; i < cnt; ++i)
        free(index[i].chaveBusca);
    free(page);
    free(reg.nomeServidor);
    free(reg.cargoServidor);
    free(reg.telefoneServidor);
    fclose(input);
    fclose(output);
    binarioNaTela2(nome_output);
}