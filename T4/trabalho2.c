#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SCC0215012019escreverTela2.h"
#include "utilidades.h"
#include "trabalho2.h"

/*
Retprna a posicão do último registro não removido do arquivo input
input: FILE* input: ponteiro para arquivo aberto
output: int: posicão do último registro não removido
*/
int last_register(FILE *input) {
  int pos = 0, offset = 0;
  registro reg;
  const int size = 32000;
  char *page = calloc(32000, sizeof(char));

  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  fseek(input, -32000, SEEK_END);
  fread(page, sizeof(char), 32000, input);

  while (offset < size) {
    read_register(size, offset, page, &reg);
    if (offset + reg.tamRegistro + 5 < size && reg.removido != '*')
      pos = offset;
    offset += reg.tamRegistro + 5;
  }
  fseek(input, -32000, SEEK_END);
  return pos;
}

/*
Realiza remocões de um arquivo binário de acordo com n critérios
input: char* nome: nome do arquivo binário
       int n: número de critérios no formato "campo valor" que serão usados
              para remocão
output: void
*/
void remove_binary(char *nome, int n) {
  FILE *input;
  char *campo, *valor, *to_write;
  campo = calloc(256, sizeof(1));
  valor = calloc(256, sizeof(1));
  to_write = calloc(512, sizeof(1));

  char *page;
  registro reg;
  cabecalho cab;

  list *removed = create();
  long int pos = 0;

  query *q;
  q = calloc(n + 1, sizeof(query));
  double val_double = 0;
  char is_removed, file_status;
  int page_offset = 0, val_int = 0;
  int idx, page_size, num_pages = 0, size = 0;

  input = fopen(nome, "rb+");
  if(file_error(input, &cab)) return;

  // Diz que o arquivo está sendo escrito
  file_status = '0';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);

  fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho

  page = malloc(32000 * sizeof(char));

  from_file(removed, input, &cab);
  
  num_pages = 0;

  for(int i = 1; i <= n; ++i) {
    scanf(" %s", campo);
    scan_quote_string(valor);
    if (strcmp(campo, "idServidor") == 0) {
      q[i].type = ID;
      q[i].value.id = atoi(valor);
    } 
    else if (strcmp(campo, "salarioServidor") == 0) {
      q[i].type = SALARIO;
      q[i].value.salario = atof(valor);
    } 
    else if (strcmp(campo, "telefoneServidor") == 0){
      idx = 2;
      q[i].type = TELEFONE;
      strcpy(q[i].value.telefone, valor);
    }
    else if (strcmp(campo, "nomeServidor") == 0){
      q[i].type = NOME;
      strcpy(q[i].value.nome, valor);
    }
    else{
      q[i].type = CARGO;
      strcpy(q[i].value.cargo, valor);
    }
  }
  
  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));
  while (!feof(input)) {
    page_offset = 0;
    page_size = fread(page, sizeof(char), 32000, input);
    ++num_pages;
    while (page_offset < page_size) {
      pos = num_pages * 32000 + page_offset;
      read_register(page_size, page_offset, page, &reg);
      page_offset += reg.tamRegistro + 5;
      if (reg.removido == '*') continue;
      for(int i = 1; i <= n; ++i){
        if (q[i].type == ID ){
          if (reg.idServidor == q[i].value.id){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
        }

        else if (q[i].type == SALARIO){
          if (reg.salarioServidor == q[i].value.salario){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
        }

        else if (q[i].type == TELEFONE) {
          if (strcmp(reg.telefoneServidor, q[i].value.telefone) == 0){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
        }

        else if (q[i].type == NOME) {
          if (reg.tamNomeServidor > 0 && (strcmp(reg.nomeServidor, q[i].value.nome) == 0)){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
          else if (strcmp("", q[i].value.nome) == 0 && reg.tamNomeServidor == -1) {
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
        }

        else {
          if (reg.tamCargoServidor > 0 &&  (strcmp(reg.cargoServidor, q[i].value.cargo) == 0)){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }

          else if (strcmp("", q[i].value.cargo) == 0 && reg.tamCargoServidor == -1){
            insert(removed, pos, reg.tamRegistro, i);
            break;
          }
        }
      }
    }
  }
  

  fseek(input, 1, SEEK_SET);
  fwrite(&(removed->begin->next->pos), sizeof(long int), 1, input);
  is_removed = '*';
  node *aux;

  int cnt = 0;
  for (node *n = removed->begin->next; n != removed->end; n = n->next) {
    aux = n->next;
    if (aux == removed->end) aux->pos = -1;
    fseek(input, n->pos, SEEK_SET);
    fwrite(&(is_removed), sizeof(char), 1, input);
    fread(&size, sizeof(int), 1, input);
    fwrite(&(aux->pos), sizeof(long int), 1, input);
    memset(to_write, '@', size - 8);
    fwrite(to_write, sizeof(char), size - 8, input);
  }

  destroy(removed);
  free(q);
  free(page);
  free(campo);
  free(valor);
  free(to_write);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);

  // Diz que o arquivo foi escrito com sucesso
  file_status = '1';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);

  fclose(input);
  binarioNaTela2(nome);
  return;
}

/*
Realiza insercão de um registro em um arquivo binário usando best-fit
input: FILE* input: arquivo binário onde será inserido o registro
       list* removed: lista encadeada ordenada de registros removidos do arquivo
       registro reg: registro a ser inserigo
       cabecalho cab: registro de cabecalho do arquivo 
output: void
*/
int insert_register(FILE *input, list *removed, registro reg, cabecalho cab) {
  int old_size;
  long int pos, new_size;
  char *garbage;

  garbage = calloc(256, sizeof(char));
  memset(garbage, '@', 256);

  pos = best_fit(removed, reg.tamRegistro);
  if (pos != -1) {
    //insere o registro em algum lugar onde existe um registro removido
    fseek(input, pos, SEEK_SET);
    fwrite(&(reg.removido), sizeof(char), 1, input);
    fread(&old_size, sizeof(int), 1, input);
    fwrite(&(reg.encadeamentoLista), sizeof(long), 1, input);
    fwrite(&(reg.idServidor), sizeof(int), 1, input);
    fwrite(&(reg.salarioServidor), sizeof(double), 1, input);
    fwrite(reg.telefoneServidor, sizeof(char), 14, input);

    if (reg.tamNomeServidor > 0) {
      fwrite(&(reg.tamNomeServidor), sizeof(int), 1, input);
      fwrite(&(cab.tags[3]), sizeof(char), 1, input);
      fwrite(reg.nomeServidor, sizeof(char), reg.tamNomeServidor - 1, input);
    }

    if (reg.tamCargoServidor > 0) {
      fwrite(&(reg.tamCargoServidor), sizeof(int), 1, input);
      fwrite(&(cab.tags[4]), sizeof(char), 1, input);
      fwrite(reg.cargoServidor, sizeof(char), reg.tamCargoServidor - 1, input);
    }
    fwrite(garbage, sizeof(char), old_size - reg.tamRegistro, input);
  }

  else {
    //insere o registro no fim do arquivo
    fseek(input, 0, SEEK_END);
    pos = ftell(input);
    pos %= 32000;
    if (pos + reg.tamRegistro > 32000) {  // caso o registro não caiba na página última
      garbage = realloc(garbage, 32000 - pos);
      memset(garbage, '@', 32000 - pos);
      fwrite(garbage, sizeof(char), 32000 - pos, input);  // preenche o fim da página com lixo

      pos = last_register(input);
      fseek(input, pos + 1, SEEK_CUR);
      new_size = 32000 - pos - 5; //tamanho do último registro da página
      fwrite(&new_size, sizeof(int), 1, input);
      fseek(input, 0, SEEK_END);
    }

    pos = ftell(input);
    fwrite(&(reg.removido), sizeof(char), 1, input);
    fwrite(&(reg.tamRegistro), sizeof(int), 1, input);
    fwrite(&(reg.encadeamentoLista), sizeof(long), 1, input);
    fwrite(&(reg.idServidor), sizeof(int), 1, input);
    fwrite(&(reg.salarioServidor), sizeof(double), 1, input);
    fwrite(reg.telefoneServidor, sizeof(char), 14, input);

    if (reg.tamNomeServidor > 0) {
      fwrite(&(reg.tamNomeServidor), sizeof(int), 1, input);
      fwrite(&(cab.tags[3]), sizeof(char), 1, input);
      fwrite(reg.nomeServidor, sizeof(char), reg.tamNomeServidor - 1, input);
    }

    if (reg.tamCargoServidor > 0) {
      fwrite(&(reg.tamCargoServidor), sizeof(int), 1, input);
      fwrite(&(cab.tags[4]), sizeof(char), 1, input);
      fwrite(reg.cargoServidor, sizeof(char), reg.tamCargoServidor - 1, input);
    }
  }

  free(garbage);
  return pos;
}

/*
Realiza n insercões de registros em um arquivo binário
input: char* nome: nome do arquivo binário
       int n: número de registros a serem inseridos
output: void
*/
void insert_binary(char *nome, int n) {
  FILE *input;
  char *campo, *valor, *to_write;
  campo = calloc(256, sizeof(1));
  valor = calloc(256, sizeof(1));
  to_write = calloc(512, sizeof(1));

  char *page;
  registro reg;
  cabecalho cab;

  list *removed = create();
  long int pos = 0;

  double val_double = 0;
  char file_status, is_removed;
  int page_offset = 0, val_int = 0;
  int idx, page_size, num_pages = 0, size = 0;

  input = fopen(nome, "rb+");
  if (file_error(input, &cab)) return;

  // Diz que o arquivo está sendo escrito
  file_status = '0';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);

  fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho

  page = malloc(32000 * sizeof(char));
  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  from_file(removed, input, &cab);

  reg.removido = '-';
  reg.encadeamentoLista = -1;
  while (n--> 0) {
    fseek(input, 32000, SEEK_SET);
    num_pages = 0;

    memset(reg.nomeServidor, 0, 128);
    memset(reg.cargoServidor, 0, 128);
    memset(reg.telefoneServidor, 0, 15);

    scanf("%d", &(reg.idServidor));

    scan_quote_string(valor);
    if (strcmp(valor, "") == 0)
      reg.salarioServidor = -1.0;
    else
      reg.salarioServidor = atof(valor);

    scan_quote_string(valor);
    if (strcmp(valor, "") == 0)
      memcpy(reg.telefoneServidor, "\0@@@@@@@@@@@@@", 14);
    else
      strcpy(reg.telefoneServidor, valor);

    scan_quote_string(valor);
    if (strcmp(valor, "") == 0) {
      reg.tamNomeServidor = 0;
    } else {
      reg.tamNomeServidor = strlen(valor) + 2;
      strcpy(reg.nomeServidor, valor);
    }

    scan_quote_string(valor);
    if (strcmp(valor, "") == 0) {
      reg.tamCargoServidor = 0;
    } else {
      reg.tamCargoServidor = strlen(valor) + 2;
      strcpy(reg.cargoServidor, valor);
    }

    reg.tamRegistro = 34;
    if (reg.tamNomeServidor > 0) reg.tamRegistro += 4 + reg.tamNomeServidor;

    if (reg.tamCargoServidor > 0) reg.tamRegistro += 4 + reg.tamCargoServidor;

    insert_register(input, removed, reg, cab);
  }

  fseek(input, 1, SEEK_SET);
  fwrite(&(removed->begin->next->pos), sizeof(long int), 1, input);
  is_removed = '*';
  node *aux;

  int cnt = 0;
  for (node *n = removed->begin->next; n != removed->end; n = n->next) {
    aux = n->next;
    fseek(input, n->pos, SEEK_SET);
    fwrite(&(is_removed), sizeof(char), 1, input);
    fread(&size, sizeof(int), 1, input);
    fwrite(&(aux->pos), sizeof(long int), 1, input);
    memset(to_write, '@', size - 8);
    fwrite(to_write, sizeof(char), size - 8, input);
  }

  destroy(removed);
  free(page);
  free(campo);
  free(valor);
  free(to_write);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);

  // Diz que o arquivo foi escrito com sucesso
  file_status = '1';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);

  fclose(input);
  binarioNaTela2(nome);
  return;
}

/*
Realiza atualizacões em um arquivo binário de acordo com n critérios
input: char* nome: nome do arquivo binário
       int n: número de critérios no formato "campo_original valor_original" que serão usados
              para buscar os registros que serão atualizados e "campo_novo valor_novo", que 
              serão usados para atualizar os registros
output: void
*/
void update_binary(char *nome, int n) {
  FILE *input;
  char *campo_original, *valor_original, *to_write;
  char *campo_novo, *valor_novo;
  campo_original = calloc(256, sizeof(1));
  valor_original = calloc(256, sizeof(1));
  campo_novo = calloc(256, sizeof(1));
  valor_novo = calloc(256, sizeof(1));
  to_write = calloc(512, sizeof(1));

  char *page;
  registro reg;
  cabecalho cab;

  list *removed = create();
  long int pos = 0, read_pos;

  char file_status, is_removed;
  double val_double = 0, val_double_novo = 0;
  int change, page_offset = 0, val_int = 0, val_int_novo = 0;
  int old_size, idx_original, idx_novo, page_size, num_pages = 1, size = 0;

  input = fopen(nome, "rb+");
  if (file_error(input, &cab)) return;

  // Diz que o arquivo está sendo escrito
  file_status = '0';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);
  fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho

  page = malloc(32000 * sizeof(char));
  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  from_file(removed, input, &cab);

  while (n-->0) {
    fseek(input, 32000, SEEK_SET);
    num_pages = 0;
    scanf(" %s", campo_original);
    if (strcmp(campo_original, "idServidor") == 0)
      scanf("%s", valor_original);
    else
      scan_quote_string(valor_original);

    scanf(" %s", campo_novo);
    if (strcmp(campo_novo, "idServidor") == 0)
      scanf("%s", valor_novo);
    else
      scan_quote_string(valor_novo);

    if (strcmp(campo_original, "idServidor") == 0) {
      idx_original = 0;
      val_int = atoi(valor_original);
    } else if (strcmp(campo_original, "salarioServidor") == 0) {
      idx_original = 1;
      val_double = atof(valor_original);
    } else if (strcmp(campo_original, "telefoneServidor") == 0)
      idx_original = 2;
    else if (strcmp(campo_original, "nomeServidor") == 0)
      idx_original = 3;
    else
      idx_original = 4;

    if (strcmp(campo_novo, "idServidor") == 0) {
      idx_novo = 0;
      val_int_novo = atoi(valor_novo);
    } 
    else if (strcmp(campo_novo, "salarioServidor") == 0) {
      idx_novo = 1;
      val_double_novo = atof(valor_novo);
    } 
    else if (strcmp(campo_novo, "telefoneServidor") == 0)
      idx_novo = 2;
    else if (strcmp(campo_novo, "nomeServidor") == 0)
      idx_novo = 3;
    else
      idx_novo = 4;

    memset(reg.nomeServidor, 0, 128);
    memset(reg.cargoServidor, 0, 128);
    memset(reg.telefoneServidor, 0, 15);

    while (!feof(input)) {
      page_offset = 0;
      page_size = fread(page, sizeof(char), 32000, input);
      read_pos = ftell(input);
      ++num_pages;
      while (page_offset < page_size) {
        pos = num_pages * 32000 + page_offset;
        read_register(page_size, page_offset, page, &reg);
        page_offset += reg.tamRegistro + 5;
        if (reg.removido == '*') continue;
        change = 0;

        if ((idx_original == 0 && reg.idServidor == val_int) ||
            (idx_original == 1 && reg.salarioServidor == val_double))
          change = 1;

        else if (idx_original == 2) {
          if (strcmp(reg.telefoneServidor, valor_original) == 0) change = 1;

        }

        else if (idx_original == 3) {
          if (reg.tamNomeServidor > 0 &&
              (strcmp(reg.nomeServidor, valor_original) == 0))
            change = 1;
          else if (strcmp("", valor_original) == 0 &&
                   reg.tamNomeServidor == -1) {
            change = 1;
          }
        }

        else {
          if (reg.tamCargoServidor > 0 &&
              (strcmp(reg.cargoServidor, valor_original) == 0))
            change = 1;

          else if (strcmp("", valor_original) == 0 &&
                   reg.tamCargoServidor == -1)
            change = 1;
        }

        if (change == 1) {
          switch (idx_novo) {
            case 0:
              reg.idServidor = val_int_novo;
              break;

            case 1:
              reg.salarioServidor = val_double_novo;
              break;

            case 2:
              strcpy(reg.telefoneServidor, valor_novo);
              break;

            case 3:
              strcpy(reg.nomeServidor, valor_novo);
              reg.tamNomeServidor = strlen(reg.nomeServidor) + 2;
              break;

            default:
              strcpy(reg.cargoServidor, valor_novo);
              reg.tamCargoServidor = strlen(reg.cargoServidor) + 2;
              break;
          }

          old_size = reg.tamRegistro;

          reg.tamRegistro = 34;
          if (reg.tamNomeServidor > 1)
            reg.tamRegistro += 4 + reg.tamNomeServidor;

          if (reg.tamCargoServidor > 1)
            reg.tamRegistro += 4 + reg.tamCargoServidor;

          if (old_size >= reg.tamRegistro) {
            fseek(input, pos, SEEK_SET);
            fwrite(&(reg.removido), sizeof(char), 1, input);
            fseek(input, 4, SEEK_CUR);
            fwrite(&(reg.encadeamentoLista), sizeof(long), 1, input);
            fwrite(&(reg.idServidor), sizeof(int), 1, input);
            fwrite(&(reg.salarioServidor), sizeof(double), 1, input);
            fwrite(reg.telefoneServidor, sizeof(char), 14, input);

            if (reg.tamNomeServidor > 0) {
              fwrite(&(reg.tamNomeServidor), sizeof(int), 1, input);
              fwrite(&(cab.tags[3]), sizeof(char), 1, input);
              fwrite(reg.nomeServidor, sizeof(char), reg.tamNomeServidor - 1,
                     input);
            }

            if (reg.tamCargoServidor > 0) {
              fwrite(&(reg.tamCargoServidor), sizeof(int), 1, input);
              fwrite(&(cab.tags[4]), sizeof(char), 1, input);
              fwrite(reg.cargoServidor, sizeof(char), reg.tamCargoServidor - 1,
                     input);
            }
          } 
          else {
            fseek(input, pos, SEEK_SET);
            reg.removido = '*';
            fwrite(&(reg.removido), sizeof(char), 1, input);
            reg.removido = '-';

            insert(removed, pos, old_size, 1);
            insert_register(input, removed, reg, cab);
          }

          fseek(input, read_pos, SEEK_SET);
        }
      }
    }
  }

  fseek(input, 1, SEEK_SET);
  fwrite(&(removed->begin->next->pos), sizeof(long int), 1, input);
  is_removed = '*';
  node *aux;

  int cnt = 0;
  for (node *n = removed->begin->next; n != removed->end; n = n->next) {
    aux = n->next;
    fseek(input, n->pos, SEEK_SET);
    fwrite(&(is_removed), sizeof(char), 1, input);
    fread(&(n->size), sizeof(int), 1, input);
    fwrite(&(aux->pos), sizeof(long int), 1, input);
    memset(to_write, '@', n->size - 8);
    fwrite(to_write, sizeof(char), n->size - 8, input);
  }

  destroy(removed);
  free(page);
  free(campo_original);
  free(valor_original);
  free(campo_novo);
  free(valor_novo);
  free(to_write);

  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);

  // Diz que o arquivo foi escrito com sucesso
  file_status = '1';
  fseek(input, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, input);

  fclose(input);
  binarioNaTela2(nome);
  return;
}
