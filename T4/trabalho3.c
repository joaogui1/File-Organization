#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SCC0215012019escreverTela2.h"
#include "trabalho3.h"
#include "utilidades.h"

/*
Funcão comparadora para usar com o qsort
Input: const void *a: ponteiro genérico, no caso será um ponteiro para registro
       const void *b: ponteiro genérico, no caso será um ponteiro para registro
output: positivo, se o id de a for maior que o de b
        0, se for igual
        negativo, se for menor
*/
int comp(const void *a, const void *b){
  return (((registro *)a)->idServidor - ((registro *)b)->idServidor);
}

/*
Funcão que ordena um arquivo binário pelos ids dos registros
Salvando em nome_output um arquivo com os mesmos registros que
os contidos em nome_input, porém sem os removidos e dessa vez
ordenados por id
Input: char *nome_input: Nome do arquivo a ser ordenado
       char *nome_output: Nome do arquivo de saída
*/
void sort_binary(char *nome_input, char *nome_output){
    FILE *input, *output;
    cabecalho cab;
    long file_size = 0;
    registro *reg;
    reg = calloc(6400, sizeof(registro));
    int page_size = 32000, cnt = 0;
    int sz_pg = 0, offset = 0;
    char aux_tag, *file, *page;

    input = fopen(nome_input, "rb");
    if (file_error(input, &cab)) return;

    fseek(input, 0, SEEK_END);
    file_size = ftell(input);
    fseek(input, 32000, SEEK_SET);

    file_size -= page_size; //file_size só guarda o tamanho do arquivo que contem dados,
                            // sem conta o cabecalho
    file = calloc(file_size + 1, sizeof(char));
    fread(file, sizeof(char), file_size, input);
    fclose(input);

    reg[0].nomeServidor = calloc(128, sizeof(char));
    reg[0].cargoServidor = calloc(128, sizeof(char));
    reg[0].telefoneServidor = calloc(15, sizeof(char));
    
    //leitura dos registros para a RAM
    while (offset < file_size) {
        read_register(file_size, offset, file, &reg[cnt]);

        offset += reg[cnt].tamRegistro + 5;

        reg[cnt].tamRegistro = 34;
        if (reg[cnt].tamNomeServidor > 1)
          reg[cnt].tamRegistro += 4 + reg[cnt].tamNomeServidor;

        if (reg[cnt].tamCargoServidor > 1)
          reg[cnt].tamRegistro += 4 + reg[cnt].tamCargoServidor;
       
        if(reg[cnt].removido == '-'){ //se o registro não tiver removido, incrementamos o contador
            ++cnt;
            reg[cnt].nomeServidor = calloc(128, sizeof(char));
            reg[cnt].cargoServidor = calloc(128, sizeof(char));
            reg[cnt].telefoneServidor = calloc(15, sizeof(char));
        }
    }
    free(file);
    qsort(reg, cnt, sizeof(registro), comp);
    
    output = fopen(nome_output, "wb");
    page = malloc(page_size*sizeof(char));
    memset(page, '@', page_size);

    //escrita do cabecalho
    cab.topoLista = -1;
    cab.status = '0';
    fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);
    fwrite(&cab.topoLista, sizeof(char), sizeof(cab.topoLista), output);
    
    for (int i = 0; i < 5; ++i) {
        fwrite(&cab.tags[i], sizeof(char), sizeof(cab.tags[i]), output);
        fwrite(cab.campos[i], sizeof(char), sizeof(cab.campos[i]), output);
    }
    fwrite(page, sizeof(char), 31786, output);

    //escreve os registros já ordenados 
    sz_pg = write_to_page(&page, sz_pg, reg[0]);
    for(int i = 1; i < cnt; ++i) 
        sz_pg = new_write_to_page(output, &page, sz_pg, reg[i - 1], reg[i]);
    
    fwrite(page, sizeof(char), sz_pg, output);

    //diz que o arquivo foi escrito com sucesso
    fseek(output, 0, SEEK_SET);
    cab.status = '1';
    fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);


    //libera memória
    for(int i = 0; i <=cnt; ++i){
      free(reg[i].nomeServidor);
      free(reg[i].cargoServidor);
      free(reg[i].telefoneServidor);
    }
    free(reg);
    free(page);
    fclose(output);
    binarioNaTela2(nome_output);
}

void merge(char *nome_input1, char *nome_input2, char *nome_output){
    FILE *input[2], *output;

    cabecalho cab;
    long file_size = 0;
    char aux_tag, *file, *page;
    registro ant_reg, reg[2][6400];
    int idx1 = 0, idx2 = 0;
    int sz_pg = 0, offset = 0, page_offset, size[2] = {0};
    int page_size = 32000, cnt = 0, not_done1 = 1, not_done2 = 1;
    page = malloc(32000);
    memset(page, '@', 32000);

    input[0] = fopen(nome_input1, "rb");
    input[1] = fopen(nome_input2, "rb");
    output = fopen(nome_output, "wb");
    
    if (file_error(input[1], &cab)) return;
    fseek(input[1], 32000, SEEK_SET);
   
    if (file_error(input[0], &cab)) return;
    fseek(input[0], 32000, SEEK_SET);
    
    //escrita do cabecalho
    cab.topoLista = -1;
    cab.status = '0';
    fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);
    fwrite(&cab.topoLista, sizeof(char), sizeof(cab.topoLista), output);

    for (int i = 0; i < 5; ++i) {
      fwrite(&cab.tags[i], sizeof(char), sizeof(cab.tags[i]), output);
      fwrite(cab.campos[i], sizeof(char), sizeof(cab.campos[i]), output);
    }
    fwrite(page, sizeof(char), 31786, output);


    ant_reg.tamRegistro = 0;
    ant_reg.tamNomeServidor = 0;
    ant_reg.tamCargoServidor = 0;
    ant_reg.nomeServidor = calloc(128, sizeof(char));
    ant_reg.cargoServidor = calloc(128, sizeof(char));
    ant_reg.telefoneServidor = calloc(15, sizeof(char));
    
    //leitura dos registros
    for(int i = 0; i < 2; ++i){
      cnt = 0;
      offset = 0;
      fseek(input[i], 0, SEEK_END);
      file_size = ftell(input[i]);
      fseek(input[i], 32000, SEEK_SET);

      file_size -= page_size;
      file = calloc(file_size + 1, sizeof(char));
      fread(file, sizeof(char), file_size, input[i]);
      fclose(input[i]);

      reg[i][0].nomeServidor = calloc(128, sizeof(char));
      reg[i][0].cargoServidor = calloc(128, sizeof(char));
      reg[i][0].telefoneServidor = calloc(15, sizeof(char));

      while (offset < file_size) {
        read_register(file_size, offset, file, &reg[i][cnt]);

        offset += reg[i][cnt].tamRegistro + 5;
        //recalculamos o tamanho dos registros para os casos em que eles estavam no fim de página
        reg[i][cnt].tamRegistro = 34;
        if (reg[i][cnt].tamNomeServidor > 1)
          reg[i][cnt].tamRegistro += 4 + reg[i][cnt].tamNomeServidor;

        if (reg[i][cnt].tamCargoServidor > 1)
          reg[i][cnt].tamRegistro += 4 + reg[i][cnt].tamCargoServidor;

        if (reg[i][cnt].removido == '-') {
          ++cnt;
          reg[i][cnt].nomeServidor = calloc(128, sizeof(char));
          reg[i][cnt].cargoServidor = calloc(128, sizeof(char));
          reg[i][cnt].telefoneServidor = calloc(15, sizeof(char));
        }
      }
      size[i] = cnt;
      free(file);

    }


    //algoritmo de merge como descrito em sala
    while(idx1 < size[0] || idx2 < size[1]){
      if (idx2 >= size[1] || (comp(&(reg[0][idx1]), &(reg[1][idx2])) < 0)){ 
        //se já acabou o segundo arquivo ou o id do primeiro registro é menor que o segundo
        sz_pg = new_write_to_page(output, &page, sz_pg, ant_reg, reg[0][idx1]); 
        atribute_regs(&(reg[0][idx1]), &ant_reg);
        ++idx1;
      }

      else if (idx1 >= size[0] || (comp(&(reg[0][idx1]), &(reg[1][idx2])) > 0)) {
        //se já acabou o primeiro arquivo ou o id do primeiro registro é maior que o segundo
        sz_pg = new_write_to_page(output, &page, sz_pg, ant_reg, reg[1][idx2]); 
        atribute_regs(&(reg[1][idx2]), &ant_reg);
        ++idx2;
      }

      else{
        //se os arquivos não acabaram e os registros são ccomuns aos dois
        sz_pg = new_write_to_page(output, &page, sz_pg, ant_reg,reg[0][idx1]); 
        atribute_regs(&(reg[0][idx1]), &ant_reg);
        ++idx1;
        ++idx2;
      }
    }
    fwrite(page, sizeof(char), sz_pg, output);

    for(int i = 0; i < 2; ++i)
      for(int j = 0; j <= size[i]; ++j){
        free(reg[i][j].nomeServidor);
        free(reg[i][j].cargoServidor);
        free(reg[i][j].telefoneServidor); 
      }

    // diz que o arquivo foi escrito com sucesso
    fseek(output, 0, SEEK_SET);
    cab.status = '1';
    fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);

    free(page);
    free(ant_reg.nomeServidor);
    free(ant_reg.cargoServidor);
    free(ant_reg.telefoneServidor);
    
    fclose(output);

    binarioNaTela2(nome_output);
}

void match(char *nome_input1, char *nome_input2, char *nome_output) {
  FILE *input[2], *output;

  cabecalho cab;
  long file_size = 0;
  char aux_tag, *file, *page;
  registro ant_reg, reg[2][6400];
  int idx1 = 0, idx2 = 0;
  int sz_pg = 0, offset = 0, page_offset, size[2] = {0};
  int page_size = 32000, cnt = 0, not_done1 = 1, not_done2 = 1;
  page = malloc(32000);
  memset(page, '@', 32000);

  input[0] = fopen(nome_input1, "rb");
  input[1] = fopen(nome_input2, "rb");
  output = fopen(nome_output, "wb");

  if (file_error(input[1], &cab)) return;
  fseek(input[1], 32000, SEEK_SET);

  if (file_error(input[0], &cab)) return;
  fseek(input[0], 32000, SEEK_SET);

  // escrita do cabecalho
  cab.topoLista = -1;
  cab.status = '0';
  fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);
  fwrite(&cab.topoLista, sizeof(char), sizeof(cab.topoLista), output);

  for (int i = 0; i < 5; ++i) {
    fwrite(&cab.tags[i], sizeof(char), sizeof(cab.tags[i]), output);
    fwrite(cab.campos[i], sizeof(char), sizeof(cab.campos[i]), output);
  }
  fwrite(page, sizeof(char), 31786, output);

  ant_reg.tamRegistro = 0;
  ant_reg.tamNomeServidor = 0;
  ant_reg.tamCargoServidor = 0;
  ant_reg.nomeServidor = calloc(128, sizeof(char));
  ant_reg.cargoServidor = calloc(128, sizeof(char));
  ant_reg.telefoneServidor = calloc(15, sizeof(char));

  // leitura dos registros
  for (int i = 0; i < 2; ++i) {
    cnt = 0;
    offset = 0;
    fseek(input[i], 0, SEEK_END);
    file_size = ftell(input[i]);
    fseek(input[i], 32000, SEEK_SET);

    file_size -= page_size;
    file = calloc(file_size + 1, sizeof(char));
    fread(file, sizeof(char), file_size, input[i]);
    fclose(input[i]);

    reg[i][0].nomeServidor = calloc(128, sizeof(char));
    reg[i][0].cargoServidor = calloc(128, sizeof(char));
    reg[i][0].telefoneServidor = calloc(15, sizeof(char));

    while (offset < file_size) {
      read_register(file_size, offset, file, &reg[i][cnt]);

      offset += reg[i][cnt].tamRegistro + 5;
      // recalculamos o tamanho dos registros para os casos em que eles estavam
      // no fim de página
      reg[i][cnt].tamRegistro = 34;
      if (reg[i][cnt].tamNomeServidor > 1)
        reg[i][cnt].tamRegistro += 4 + reg[i][cnt].tamNomeServidor;

      if (reg[i][cnt].tamCargoServidor > 1)
        reg[i][cnt].tamRegistro += 4 + reg[i][cnt].tamCargoServidor;

      if (reg[i][cnt].removido == '-') {
        ++cnt;
        reg[i][cnt].nomeServidor = calloc(128, sizeof(char));
        reg[i][cnt].cargoServidor = calloc(128, sizeof(char));
        reg[i][cnt].telefoneServidor = calloc(15, sizeof(char));
      }
    }
    size[i] = cnt;
    free(file);
  }

  // algoritmo de match como descrito em sala
  while (idx1 < size[0] || idx2 < size[1]) {
    if (idx2 >= size[1] || (comp(&(reg[0][idx1]), &(reg[1][idx2])) < 0)) {
      // se já acabou o segundo arquivo ou o id do primeiro registro é menor que
      // o segundo
      ++idx1;
    }

    else if (idx1 >= size[0] || (comp(&(reg[0][idx1]), &(reg[1][idx2])) > 0)) {
      // se já acabou o primeiro arquivo ou o id do primeiro registro é maior
      // que o segundo
      ++idx2;
    }

    else {
      // se os arquivos não acabaram e os registros são ccomuns aos dois
      sz_pg = new_write_to_page(output, &page, sz_pg, ant_reg, reg[0][idx1]);
      atribute_regs(&(reg[0][idx1]), &ant_reg);
      ++idx1;
      ++idx2;
    }
  }
  fwrite(page, sizeof(char), sz_pg, output);

  for (int i = 0; i < 2; ++i)
    for (int j = 0; j <= size[i]; ++j) {
      free(reg[i][j].nomeServidor);
      free(reg[i][j].cargoServidor);
      free(reg[i][j].telefoneServidor);
    }

  // diz que o arquivo foi escrito com sucesso
  fseek(output, 0, SEEK_SET);
  cab.status = '1';
  fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);

  free(page);
  free(ant_reg.nomeServidor);
  free(ant_reg.cargoServidor);
  free(ant_reg.telefoneServidor);

  fclose(output);

  binarioNaTela2(nome_output);
}