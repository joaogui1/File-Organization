#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utilidades.h"
typedef struct{
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


typedef struct{
  char status;
  long long int topoLista;
  char tags[5];
  char campos[5][40];
} cabecalho;


/*
Funcao que faz o registro b receber o conteúdo do registro a
input: registro* a, origem dos dados
       registro* b, destino dos dados
output: void
*/
void atribute_regs(registro *a, registro *b){
  b->removido = a->removido;
  b->tamRegistro = a->tamRegistro;
  b->encadeamentoLista = a->encadeamentoLista;
  b->idServidor = a->idServidor;
  b->salarioServidor = a->salarioServidor;
  memcpy(b->telefoneServidor, a->telefoneServidor, 14);

  b->tamNomeServidor = a->tamNomeServidor;
  b->nomeServidor = realloc(b->nomeServidor, a->tamNomeServidor*sizeof(char));
  if(a->tamNomeServidor > 0)  strcpy(b->nomeServidor, a->nomeServidor);

  b->tamCargoServidor = a->tamCargoServidor;
  b->cargoServidor = realloc(b->cargoServidor, a->tamCargoServidor*sizeof(char));
  if(a->tamCargoServidor > 0) strcpy(b->cargoServidor, a->cargoServidor);
}

/*
Funcao que imprime no stdout um servidor seguindo a padronizacao
da funcionalidade 3
input: cabecalho cab, registro de cabecalho com a descricao dos
        campos
      registro reg, registro de dados a ser impreso
output: void
*/
void print_servidor(cabecalho cab, registro reg){
  printf("%s: %d\n", cab.campos[0], reg.idServidor);

  printf("%s: ", cab.campos[1]);
  if(reg.salarioServidor != -1) printf("%.2lf\n", reg.salarioServidor);
  else  printf("valor nao declarado\n");

  printf("%s: ", cab.campos[2]);
  if(reg.telefoneServidor[0] != 0) printf("%s\n", reg.telefoneServidor);
  else  printf("valor nao declarado\n");

  printf("%s: ", cab.campos[3]);
  if(reg.tamNomeServidor > 0) printf("%s\n", reg.nomeServidor);
  else printf("valor nao declarado\n");

  printf("%s: ", cab.campos[4]);
  if(reg.tamCargoServidor > 0) printf("%s\n", reg.cargoServidor);
  else printf("valor nao declarado\n");

  printf("\n");
}

/*
Funcao que faz o parsing do id de um servidor, ao fim de sua
execucao pos aponta para a posicao depois da vírgula no csv
input:line, ponteiro para char da linha que faremos
        o parsing para extrair o id
      pos: posicao na linha onde estamos
output: int ret, o id extraido
*/
int parse_id(char *line, int *pos){
  int ret = 0;
  for(*pos = 0; line[*pos] != ','; ++(*pos)){
    ret *= 10;
    ret += (line[*pos] - '0');
  }

  ++(*pos); //pula a virgula
  return ret;
}

/*
Funcao que faz o parsing do salario de um servidor, ao fim de
sua execucao pos aponta para a posicao depois da vírgula no csv
input: line, ponteiro para char da linha que faremos
        o parsing para extrair o salario
      pos: posicao na linha onde estamos
output: double ret, o salario extraido
*/
double parse_salario(char *line, int *pos){
  int beg = *pos;
  double ret = 0;
  char *aux;

  for(; line[*pos] != ','; ++(*pos));
  aux = calloc((*pos) - beg + 1, sizeof(char));
  for(int i = 0; i + beg < (*pos); ++i)  aux[i] = line[beg + i];
  aux[(*pos) - beg] = '\0';

  ret = atof(aux);
  free(aux);
  ++(*pos); //pula a vírgula
  return ret;
}

/*
Funcao que faz o parsing do telefone de um servidor, ao fim de
sua execucao pos aponta para a posicao depois da vírgula no csv
input: line, ponteiro para char da linha que faremos
        o parsing para extrair o telefone
       telefone: array de chars que guardará o telefone extraído
       pos: posicao na linha onde estamos
output: void
*/
void parse_telefone(char *line, char **telefone, int *pos){
  (*telefone)[0] = '\0';
  for(int i = 1; i < 14; ++i) (*telefone)[i] = '@';

  int i = 0;
  for(; line[*pos] != ','; ++(*pos), ++i)
    (*telefone)[i] = line[*pos];
  ++(*pos); //pula a vírgula
  return;
}

/*
Funcao que faz o parsing do nome de um servidor, ao fim de sua
execucao pos aponta para a posicao depois da vírgula no csv
input: line, ponteiro para char da linha que faremos
        o parsing para extrair o nome
       nome: array de chars que guardará o nome extraído
       pos: posicao na linha onde estamos
output: int ret, o id extraido
*/
int parse_nome(char *line, char** nome, int *pos){
  int size = 0;
  *nome = realloc(*nome, 256*sizeof(char));

  for(; line[*pos] != ','; ++(*pos), ++size) (*nome)[size] = line[*pos];
  if(size > 0)  (*nome)[size++] = '\0';
  *nome = realloc(*nome, size*sizeof(char));

  ++(*pos); //pula a vírgula
  return size;
}

/*
Funcao que faz o parsing do cargo de um servidor, ao fim de sua
execucao pos aponta para a posicao depois da vírgula no csv
input: line, ponteiro para char da linha que faremos
        o parsing para extrair o nome
       cargo: array de chars que guardará o cargo extraído
       pos: posicao na linha onde estamos
output: int ret, o id extraido
*/
int parse_cargo(char *line, char** cargo, int *pos){
  int size = 0;
  *cargo = realloc(*cargo, 256*sizeof(char));

  for(; line[*pos] != '\r' && line[*pos] != '\n'; ++(*pos), ++size) (*cargo)[size] = line[*pos];
  if(size > 0)  (*cargo)[size++] = '\0';
  *cargo = realloc(*cargo, size*sizeof(char));

  return size;
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
int write_to_page(char **page, int sz_pg, registro ant_reg){
  char tagc = 'c', tagn = 'n';
  memcpy(*page + sz_pg, &ant_reg.removido, sizeof(ant_reg.removido));
  sz_pg += sizeof(ant_reg.removido);
  memcpy(*page + sz_pg, &ant_reg.tamRegistro, sizeof(ant_reg.tamRegistro));
  sz_pg += sizeof(ant_reg.tamRegistro);
  memcpy(*page + sz_pg, &ant_reg.encadeamentoLista, sizeof(ant_reg.encadeamentoLista));
  sz_pg += sizeof(ant_reg.encadeamentoLista);
  memcpy(*page + sz_pg, &ant_reg.idServidor, sizeof(ant_reg.idServidor));
  sz_pg += sizeof(ant_reg.idServidor);
  memcpy(*page + sz_pg, &ant_reg.salarioServidor, sizeof(ant_reg.salarioServidor));
  sz_pg += sizeof(ant_reg.salarioServidor);
  memcpy(*page + sz_pg, ant_reg.telefoneServidor, 14);
  sz_pg += 14;

  //checa se existe nome
  if(ant_reg.tamNomeServidor > 0){
    ant_reg.tamNomeServidor += 1; //tamanhi da tag
    memcpy(*page + sz_pg, &ant_reg.tamNomeServidor, sizeof(ant_reg.tamNomeServidor));
    ant_reg.tamNomeServidor -= 1;
    sz_pg += sizeof(ant_reg.tamNomeServidor);
    memcpy(*page + sz_pg, &tagn, sizeof(tagn));
    sz_pg += sizeof(tagn);
    memcpy(*page + sz_pg, ant_reg.nomeServidor, ant_reg.tamNomeServidor);
    sz_pg += ant_reg.tamNomeServidor;
  }

  //checa se existe cargp
  if(ant_reg.tamCargoServidor > 0){
    ant_reg.tamCargoServidor += 1; //tamanho da tag
    memcpy(*page + sz_pg, &ant_reg.tamCargoServidor, sizeof(ant_reg.tamCargoServidor));
    ant_reg.tamCargoServidor -= 1;
    sz_pg += sizeof(ant_reg.tamCargoServidor);
    memcpy(*page + sz_pg, &tagc, sizeof(tagc));
    sz_pg += sizeof(tagc);
    memcpy(*page + sz_pg, ant_reg.cargoServidor, ant_reg.tamCargoServidor);
    sz_pg += ant_reg.tamCargoServidor;
  }

  return sz_pg;
}

/*
Funcao que extrai um cabecalho de um csv e
  escreve o registro de cabecalho na primeira
  página de disco
input:  nome: nome do csv onde procuraremos o
          cabecalho
output: void
*/
void write_header(FILE *input, FILE *output){
  int pos = 0;
  cabecalho cab;
  size_t size = 0;
  char page[32000], *linhaVariavel = NULL;
  for(int i = 0; i < 32000; ++i)
    page[i] = '@';

  cab.status = '0';
  cab.topoLista = -1;
  cab.tags[0] = 'i';
  cab.tags[1] = 's';
  cab.tags[2] = 't';
  cab.tags[3] = 'n';
  cab.tags[4] = 'c';

  memcpy(cab.campos[0], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 40);
  memcpy(cab.campos[1], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 40);
  memcpy(cab.campos[2], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 40);
  memcpy(cab.campos[3], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 40);
  memcpy(cab.campos[4], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 40);

  getline(&linhaVariavel, &size, input);

  for(int cnt = 0; cnt < 5; ++cnt){
    int i;
    for(i = 0; linhaVariavel[pos + i] != ',' && linhaVariavel[pos + i] != '\r' && linhaVariavel[pos + i] != '\n'; ++i)
      cab.campos[cnt][i] = linhaVariavel[pos + i];
    cab.campos[cnt][i++] = '\0';
    pos += i;
  }

  fwrite(&cab.status, sizeof(char), sizeof(cab.status), output);
  fwrite(&cab.topoLista, sizeof(char), sizeof(cab.topoLista), output);
  for(int i = 0; i < 5; ++i){
    fwrite(&cab.tags[i], sizeof(char), sizeof(cab.tags[i]), output);
    fwrite(cab.campos[i], sizeof(char), sizeof(cab.campos[i]), output);
  }

  fwrite(page, sizeof(char), 31786, output);
  free(linhaVariavel);
}

/*
Funcao que lê o cabecalho de um arquivo binário
input:  cabecalho *cab, registro de cabecalho que
            guardará o que foi lido
        FILE *input:
            arquivo binário de onde leremos o cabecalho
output: void
*/
void read_header(cabecalho *cab, FILE *input){
  fread(&(cab->status), sizeof(char), 1, input);
  fread(&(cab->topoLista), sizeof(long), 1, input);
  for(int i = 0; i < 5; ++i){
    fread(&(cab->tags[i]), sizeof(char), 1, input);
    fread((cab->campos[i]), sizeof(char), 40, input);
  }
}

/*
Lê um registro de uma página
input: page_offset, posicao que estamos na página
       page: página de disco que estamos lendo
       registro *reg, registro de dados que vai guardar o que foi lido
output: void
*/
void read_register(int page_size, int page_offset, char *page, registro *reg){
  char aux_tag;
  int line_offset = 1;
  reg->tamNomeServidor = -1;
  reg->tamCargoServidor = -1;

  memcpy(&(reg->tamRegistro), page + line_offset + page_offset, 4);
  line_offset += 4 + 8; //topoLista

  memcpy(&(reg->idServidor), page + line_offset + page_offset, 4);
  line_offset += 4;

  memcpy(&(reg->salarioServidor), page + line_offset + page_offset, 8);
  line_offset += 8;

  memcpy(reg->telefoneServidor, page + line_offset + page_offset, 14);
  line_offset += 14;

  if(page_size - line_offset - page_offset > 4 && line_offset - 5 < reg->tamRegistro){ //checa se existem campos variaveis
    memcpy(&aux_tag, page + line_offset + page_offset + 4, 1);
    if(aux_tag == 'n'){ //checa se existe nome
      memcpy(&(reg->tamNomeServidor), page + line_offset + page_offset, 4);
      line_offset += 5; //conta a tag

      memcpy(reg->nomeServidor, page + line_offset + page_offset, reg->tamNomeServidor - 1);
      line_offset += reg->tamNomeServidor - 1;

      if(page_size - line_offset - page_offset > 4 && line_offset - 5 < reg->tamRegistro){ //checa se ainda existe algo no registro
        memcpy(&aux_tag, page + line_offset + page_offset + 4, 1);
        if(aux_tag == 'c'){ //checa se existe cargo
          memcpy(&(reg->tamCargoServidor), page + line_offset + page_offset, 4);
          line_offset += 5;

          memcpy(reg->cargoServidor, page + line_offset + page_offset, reg->tamCargoServidor - 1);
          line_offset += reg->tamCargoServidor - 1;
        }
      }
    }

    else if(aux_tag == 'c'){ //checa se o campo é o cargo
      memcpy(&(reg->tamCargoServidor), page + line_offset + page_offset, 4);
      line_offset += 5;

      memcpy(reg->cargoServidor, page + line_offset + page_offset, reg->tamCargoServidor - 1);
      line_offset += reg->tamCargoServidor - 1;
    }
  }
}

/*
Lê os dados de um CSV e escreve eles em um arquivo binário
input: nome: nome do arquivo a ser lido
output: void
*/
void write_data(FILE *input, FILE *output){
  registro reg, ant_reg;

  int sz_pg = 0;
  char *page;
  page = malloc(32000*sizeof(char));
  for(int i = 0; i < 32000; ++i)
    page[i] = '@';

  long unsigned size;
  int status, cnt = 0, pos, fixed_size = 8 + 4 + 8 + 14;
  char file_status = '1', tagc = 'c', tagn = 'n', *linhaVariavel = NULL;

  ant_reg.removido = '*'; //sinaliza que ainda não foi inicializado
  ant_reg.tamRegistro = 0;
  ant_reg.encadeamentoLista = -1;
  ant_reg.nomeServidor = calloc(128, sizeof(char));
  ant_reg.cargoServidor = calloc(128, sizeof(char));
  ant_reg.telefoneServidor = calloc(14, sizeof(char));

  reg.removido = '-';
  reg.encadeamentoLista = -1;
  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(14, sizeof(char));

  // getline(&linhaVariavel, &size, input); //ignora o cabecalho do csv

  while((status = getline(&linhaVariavel, &size, input)) > 0){

    reg.idServidor = parse_id(linhaVariavel, &pos);
    reg.salarioServidor = parse_salario(linhaVariavel, &pos);

    parse_telefone(linhaVariavel, &(reg.telefoneServidor), &pos);

    reg.tamNomeServidor =  parse_nome(linhaVariavel, &reg.nomeServidor, &pos);

    reg.tamCargoServidor = parse_cargo(linhaVariavel, &reg.cargoServidor, &pos);

    reg.tamRegistro = fixed_size + (reg.tamNomeServidor > 0?5:0) + reg.tamNomeServidor + (reg.tamCargoServidor > 0?5:0) + reg.tamCargoServidor;

    //se ant_reg for o último registro da página modificamos o tamanho dele
    if(sz_pg + ant_reg.tamRegistro + 5 + 5 + reg.tamRegistro > 32000)
      ant_reg.tamRegistro = (32000 - sz_pg - 5);

    //se já atribuímos valor pra ant_reg
    if(ant_reg.removido != '*')
      sz_pg = write_to_page(&page, sz_pg, ant_reg);

    //se escrevemos o último registro da página,
    //escrevemos ela em memória e a resetamos
    if(sz_pg + reg.tamRegistro + 5 > 32000){
      fwrite(page, sizeof(char), 32000, output);
      for(int i = 0; i < 32000; ++i)
        page[i] = '@';
      sz_pg = 0; //reseta o tamanho da página
    }

    atribute_regs(&reg, &ant_reg);
  }

  sz_pg = write_to_page(&page, sz_pg, ant_reg);
  fwrite(page, sizeof(char), sz_pg, output);
  sz_pg = 32000;

  //Diz que o arquivo foi escrito com sucesso
  fseek(output, 0, SEEK_SET);
  fwrite(&file_status, sizeof(char), 1, output);

  free(page);
  free(linhaVariavel);
  free(ant_reg.nomeServidor);
  free(ant_reg.cargoServidor);
  free(ant_reg.telefoneServidor);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);
}

/*
Funcao lê um csv e escreve seu conteúdo (cabecalho e dados)
      em um arquivo binário como detalhado na funcionalidade 1
input:  nome: nome do csv
output: error, 1 se nao achamos o arquivo com o nome dado,
               0 se tudo ocorreu bem
*/
int read_csv(char *nome){
  FILE *input, *output;
  input = fopen(nome, "r");
  if(!input){
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
int read_binary(char *nome){
  FILE *input;
  registro reg;
  cabecalho cab;
  char page[32000], aux_tag;
  int num_pages = 0, page_offset, line_offset;
  int page_size = 32000;

  input = fopen(nome, "rb");
  if(!input){ //arquivo nao existe
    printf("Falha no processamento do arquivo.\n");
    return 1;
  }

  read_header(&cab, input);

  if(cab.status != '1'){ //arquivo corrompido
    printf("Falha no processamento do arquivo.\n");
    return 1;
  }

  fseek(input, 0, SEEK_END);
  if(ftell(input) == 32000){ //arquivo so tem cabecalho
    printf("Registro inexistente.\n");
    return 1;
  }

  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  fseek(input, page_size, SEEK_SET); //pula o lixo do cabecalho
  num_pages += 1;

  while(!feof(input)){
    page_size = fread(page, sizeof(char), 32000, input);
    ++num_pages;
    page_offset = 0;

    while(page_offset < page_size){
      read_register(page_size, page_offset, page, &reg);
      printf("%d ", reg.idServidor);

      if(reg.salarioServidor != -1.00)
        printf("%.2lf ", reg.salarioServidor);
      else
        for(int i = 0; i < 9; ++i)
          printf(" ");

      if(reg.telefoneServidor[0] != 0)
        printf("%s", reg.telefoneServidor);
      else
        for(int i = 0; i < 14; ++i)
          printf(" ");

      if(reg.tamNomeServidor > 0)
        printf(" %d %s", reg.tamNomeServidor - 2, reg.nomeServidor);

      if(reg.tamCargoServidor > 0)
        printf(" %d %s", reg.tamCargoServidor - 2, reg.cargoServidor);

      page_offset += reg.tamRegistro + 5;
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
output: error: 1, se ocorreu algum erro na leitura do arquivo
               0, se tudo ocorreu bem
*/
int search_binary(char *nome, char *campo, char *valor){
  char *page;
  FILE *input;
  registro reg;
  cabecalho cab;
  double val_double = 0;
  int idx, page_size, num_pages = 0;
  int page_offset = 0, val_int = 0, done = 0, print = 0;

  input = fopen(nome, "rb");
  if(!input){ //arquivo inexistente
    printf("Falha no processamento do arquivo.\n");
    return 1;
  }

  read_header(&cab, input);

  if(cab.status != '1'){ //arquivo corrompido
    printf("Falha no processamento do arquivo.\n");
    return 1;
  }

  fseek(input, 0, SEEK_END);
  if(ftell(input) == 32000){ //arquivo sem dados
    printf("Registro inexistente.\n");
    return 1;
  }

  fseek(input, 32000, SEEK_SET); //pula a página de cabecalho
  ++num_pages;
  page = malloc(32000*sizeof(char));

  if(strcmp(campo, "idServidor") == 0){
    idx = 0;
    val_int = atoi(valor);
  }
  else if(strcmp(campo, "salarioServidor") == 0){
    idx = 1;
    val_double = atof(valor);
  }
  else if(strcmp(campo, "telefoneServidor") == 0) idx = 2;
  else if(strcmp(campo, "nomeServidor") == 0)  idx = 3;
  else  idx = 4;

  reg.nomeServidor = calloc(128, sizeof(char));
  reg.cargoServidor = calloc(128, sizeof(char));
  reg.telefoneServidor = calloc(15, sizeof(char));

  while(!feof(input) && !done){
    page_offset = 0;
    page_size = fread(page, sizeof(char), 32000, input);
    ++num_pages;
    while(page_offset < page_size){
      read_register(page_size, page_offset, page, &reg);
      page_offset += reg.tamRegistro + 5;
      if(reg.removido == '*') continue;

      if(idx == 0){
        if(reg.idServidor == val_int){
          print_servidor(cab, reg);
          done = 1;
          ++print;
          break;
        }
      }

      else if(idx == 1){
        if(reg.salarioServidor == val_double)
          print_servidor(cab, reg);
          ++print;
      }

      else if(idx == 2){
        if(strcmp(reg.telefoneServidor, valor) == 0){
          print_servidor(cab, reg);
          ++print;
        }
      }

      else if(idx == 3){
        if(reg.tamNomeServidor > 0 && (strcmp(reg.nomeServidor, valor) == 0)){
          print_servidor(cab, reg);
          ++print;
        }
      }

      else{
        if(reg.tamCargoServidor > 0 && (strcmp(reg.cargoServidor, valor) == 0)){
          print_servidor(cab, reg);
          ++print;
        }
      }
    }

  }

  if(print > 0)
    printf("Número de páginas de disco acessadas: %d\n", num_pages);
  else
    printf("Registro inexistente.\n");
  free(page);
  free(reg.nomeServidor);
  free(reg.cargoServidor);
  free(reg.telefoneServidor);
  fclose(input);
  return 0;
}
