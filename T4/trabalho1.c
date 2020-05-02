#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SCC0215012019escreverTela2.h"
#include "utilidades.h"

/*
Funcao lê um csv e escreve seu conteúdo (cabecalho e dados)
      em um arquivo binário como detalhado na funcionalidade 1
input:  nome: nome do csv
output: error, 1 se nao achamos o arquivo com o nome dado,
               0 se tudo ocorreu bem
*/
int read_csv(char *nome) {
  FILE *input, *output;
  input = fopen(nome, "r");
  if (!input) {
    printf("Falha no carregamento do arquivo.\n");
    return 1;
  }
  output = fopen("arquivoTrab1.bin", "w+b");
  write_header(input, output);
  write_data(input, output);
  printf("arquivoTrab1.bin");

  fclose(input);
  fclose(output);
  return 0;
}

/*
Lê um arquivo binario e imprime seus conteúdos na saída padrao
input:  nome: O nome do arquivo binario a ser lido
output: error: 1, se ocorreu algum erro na leitura do arquivo
               0, se tudo ocorreu bem
*/
int read_binary(char *nome) {
  FILE *input;
  registro reg;
  cabecalho cab;
  char page[32000], aux_tag;
  int num_pages = 0, page_offset, line_offset;
  int page_size = 32000;

  input = fopen(nome, "rb");
  if (file_error(input, &cab)) return 1;

  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  fseek(input, page_size, SEEK_SET);  // pula o lixo do cabecalho
  num_pages += 1;

  while (!feof(input)) {
    page_size = fread(page, sizeof(char), 32000, input);
    ++num_pages;
    page_offset = 0;

    while (page_offset < page_size) {
      read_register(page_size, page_offset, page, &reg);
      page_offset += reg.tamRegistro + 5;
      if (reg.removido == '*') continue;
      printf("%d ", reg.idServidor);

      if (reg.salarioServidor != -1.00)
        printf("%.2lf ", reg.salarioServidor);
      else
        for (int i = 0; i < 9; ++i) printf(" ");

      if (reg.telefoneServidor[0] != 0)
        printf("%s", reg.telefoneServidor);
      else
        for (int i = 0; i < 14; ++i) printf(" ");

      if (reg.tamNomeServidor > 0)
        printf(" %d %s", reg.tamNomeServidor - 2, reg.nomeServidor);

      if (reg.tamCargoServidor > 0)
        printf(" %d %s", reg.tamCargoServidor - 2, reg.cargoServidor);

      printf("\n");
    }
  }

  printf("Número de páginas de disco acessadas: %d\n", num_pages);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);
  fclose(input);
  return 0;
}

/*
Faz a busca sequencial no arquivo binario
  de registros que tem campo igual ao valor dado
input:  nome: nome do arquivo binario
        campo: o nome do campo que queremos comparar
        valor: o valor que esse campo deve ter para o registro
              ser considerado um acerto da busca
        always_print: Diz se deve printar o número de páginas de disco 
                      independentemente de ter achado o registro (se != 0)
                      ou se apenas se achar o registo (se == 0)
output: -1, se ocorreu algum erro na leitura do arquivo
        número de páginas acessadas, se tudo ocorreu bem
*/
int search_binary(char *nome, char *campo, char *valor, int always_print) {
  char *page;
  FILE *input;
  registro reg;
  cabecalho cab;
  double val_double = 0;
  int idx, page_size, num_pages = 0;
  int page_offset = 0, val_int = 0, done = 0, print = 0;

  input = fopen(nome, "rb");
  if (file_error(input, &cab)) return -1;

  fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho
  ++num_pages;
  page = malloc(32000 * sizeof(char));

  if (strcmp(campo, "idServidor") == 0) {
    idx = 0;
    val_int = atoi(valor);
  } else if (strcmp(campo, "salarioServidor") == 0) {
    idx = 1;
    val_double = atof(valor);
  } else if (strcmp(campo, "telefoneServidor") == 0)
    idx = 2;
  else if (strcmp(campo, "nomeServidor") == 0)
    idx = 3;
  else
    idx = 4;

  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  while (!feof(input) && !done) {
    page_offset = 0;
    page_size = fread(page, sizeof(char), 32000, input);
    ++num_pages;
    while (page_offset < page_size) {
      read_register(page_size, page_offset, page, &reg);
      page_offset += reg.tamRegistro + 5;
      if (reg.removido == '*') continue;

      if (idx == 0) {
        if (reg.idServidor == val_int) {
          print_servidor(cab, reg);
          done = 1;
          ++print;
          break;
        }
      }

      else if (idx == 1) {
        if (reg.salarioServidor == val_double) {
          print_servidor(cab, reg);
          ++print;
        }
      }

      else if (idx == 2) {
        if (strcmp(reg.telefoneServidor, valor) == 0) {
          print_servidor(cab, reg);
          ++print;
        }
      }

      else if (idx == 3) {
        if (reg.tamNomeServidor > 0 && (strcmp(reg.nomeServidor, valor) == 0)) {
          print_servidor(cab, reg);
          ++print;
        }
      }

      else {
        if (reg.tamCargoServidor > 0 &&
            (strcmp(reg.cargoServidor, valor) == 0)) {
          print_servidor(cab, reg);
          ++print;
        }
      }
    }
  }

  if(print == 0)
    printf("Registro inexistente.\n");
  if ((always_print) || print > 0)
    printf("Número de páginas de disco acessadas: %d\n", num_pages);
  free(page);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);
  fclose(input);
  return num_pages;
}