#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SCC0215012019escreverTela2.h"
#include "index.h"
#include "lista.h"
#include "trabalho1.h"
#include "trabalho2.h"
#include "trabalho3.h"
#include "trabalho4.h"
#include "utilidades.h"

/*
Funcão que compara dois registros de índices
Input: const void *a: ponteiro genérico, no caso será um ponteiro para registro
de índice const void *b: ponteiro genérico, no caso será um ponteiro para
registro de índice output: positivo, se a deve vir depois de b na ordenacão 0,
se for igual negativo, se a deve vir antes de b na ordenacão
*/
int comp_idx(const void *a, const void *b) {
    int strdiff = strcmp(((idx_registro *)a)->chaveBusca, ((idx_registro *)b)->chaveBusca); 

    if(strdiff != 0)
        return strdiff;

    return (((idx_registro *)a)->byteOffset - ((idx_registro *)b)->byteOffset);
}

/*
Funcão que cria um arquivo de índice a partir de um arquivo de dados
Input: char *nome_input: nome do arquivo de dados
       char *nome_output: nome do arquivo de índice gerado
output: void
*/
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

/*
Faz a busca usando o arquivo de índice a partir de um nome dado
e imprime na tela todos os registros com o nome que estão no arquivo,
além de o número de páginas de disco acessadas
input:  char *nome_input: nome do arquivo binario
        char *nome_idx: nome do arquivo de índice
        char *nomeServidor: nome buscado
        int print: Diz se deve printar o número de páginas de disco
                      independentemente de ter achado o registro (se != 0)
                      ou se apenas se achar o registo (se == 0)
output: -1, se ocorreu algum erro na leitura do arquivo
        número de páginas acessadas, se tudo ocorreu bem
*/
int index_search(char *nome_input, char *nome_idx,  char* nomeServidor, int print){
    registro reg;
    cabecalho cab;
    long file_size = 0;
    
    idx_registro *index;
    idx_cabecalho idx_cab;
    FILE *input, *index_file;

    int index_size, pos = 0, index_pages = 0;
    int num_pages = 1, last_pos = 1, found = 0;
    int sz_pg = 0, offset = 0, page_offset = 0;

    input = fopen(nome_input, "rb");
    if (file_error(input, &cab)) return -1;
    fseek(input, 32000, SEEK_SET);

    index_file = fopen(nome_idx, "rb");
    index_size = load_idx(index_file, &index, &index_pages);
    if(index_size <= 0){
        fclose(input);
        fclose(index_file);
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    reg.nomeServidor = calloc(128, sizeof(char));
    reg.cargoServidor = calloc(128, sizeof(char));
    reg.telefoneServidor = calloc(15, sizeof(char));

    pos = binary_search(index, index_size, nomeServidor);
    for(int i = pos + 1; (i < index_size) && strcmp(index[i].chaveBusca, nomeServidor) == 0; ++i){
        found = 1;
        fseek(input, index[i].byteOffset, SEEK_SET);
        read_register_from_binary(input, &reg);
        print_servidor(cab, reg);

        if(ftell(input)/32000 != last_pos){
            last_pos = ftell(input)/32000;
            ++num_pages;
        }
    }
    if(!found)
        printf("Registro inexistente.\n");
    if (found || print) {
      printf("Número de páginas de disco para carregar o arquivo de índice: %d\n", index_pages);
      printf("Número de páginas de disco para acessar o arquivo de dados: %d\n", num_pages);
    }
    for(int i = 0; i < index_size; ++i)
        free(index[i].chaveBusca);

    free(index);
    free(reg.nomeServidor);
    free(reg.cargoServidor);
    free(reg.telefoneServidor);
    fclose(input);
    fclose(index_file);
    return num_pages;
}
/*
Realiza n remocões de um arquivo binário e seu respectivo arquivo de índice
dados os nomes dos usuários a serem removidos
input: char* nome: nome do arquivo binário
       char *nome_idx: nome do arquivo de índice
       int n: número de nomes que serão usados para remocão
output: void
*/
void remove_index(char *nome_input, char *nome_idx, int n){
    idx_registro *index;
    FILE *input, *index_file;
    char *nome_removido, *to_write, field[32];

    char *page;
    registro reg;
    cabecalho cab;

    double val_double = 0;
    char is_removed, file_status;
    int index_size, index_pages = 0;
    int page_offset = 0, val_int = 0, cnt = 0;
    int idx, page_size, num_pages = 0, pos, new_size = 0, size = 0;

    input = fopen(nome_input, "rb+");
    if (file_error(input, &cab)) return;

    index_file = fopen(nome_idx, "rb");
    index_size = load_idx(index_file, &index, &index_pages);
    if (index_size <= 0) {
      fclose(input);
      fclose(index_file);
      printf("Falha no processamento do arquivo.\n");
      return;
    }
    fclose(index_file);
    index_file = fopen(nome_idx, "wb");

    page = malloc(32000 * sizeof(char));
    memset(page, '@', 32000);

    // Diz que o arquivo está sendo escrito
    file_status = '0';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, index_file);
    fwrite(page, sizeof(char), 31999, index_file);

    nome_removido = calloc(256, sizeof(1));
    to_write = calloc(512, sizeof(1));

    list *removed = create();

    // Diz que o arquivo está sendo escrito
    file_status = '0';
    fseek(input, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, input);

    fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho

    from_file(removed, input, &cab);

    num_pages = 0;

    reg.nomeServidor = calloc(128, sizeof(char));
    reg.cargoServidor = calloc(128, sizeof(char));
    reg.telefoneServidor = calloc(15, sizeof(char));

    new_size = index_size;
    for(int i = 0; i < n; ++i){
        scanf("%s", field);
        scan_quote_string(nome_removido);
        pos = binary_search(index, index_size, nome_removido);
      
        for (int i = pos + 1; (i < index_size) && strcmp(index[i].chaveBusca, nome_removido) == 0; ++i) {
            fseek(input, index[i].byteOffset, SEEK_SET);
            read_register_from_binary(input, &reg);
            insert(removed, index[i].byteOffset, reg.tamRegistro, 1);
            index[i].byteOffset = -1;
            --new_size;
        }
    }

    fseek(input, 1, SEEK_SET);
    fwrite(&(removed->begin->next->pos), sizeof(long int), 1, input);
    is_removed = '*';
    node *aux;

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

    fseek(index_file, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, index_file);
    fwrite(&new_size, sizeof(int), 1, index_file);
    fseek(index_file, 31995, SEEK_CUR);

    for(int i = 0; i < index_size; ++i){
        if(index[i].byteOffset != -1){
            if(i != cnt){
                index[cnt].byteOffset = index[i].byteOffset;
                memset(index[cnt].chaveBusca, '@', 120);
                strcpy(index[cnt].chaveBusca, index[i].chaveBusca);
            }
            ++cnt;
        }
    }

    memset(page, '@', 32000);
    idx_write(index_file, &page, cnt, index);

    // Diz que o arquivo foi escrito com sucesso
    file_status ='1';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, index_file);

    fseek(input, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, input);

    for (int i = 0; i < index_size; ++i) free(index[i].chaveBusca);

    destroy(removed);
    free(index);
    free(page);
    free(to_write);
    free(nome_removido);
    free(reg.nomeServidor);
    free(reg.cargoServidor);
    free(reg.telefoneServidor);


    
    fclose(input);
    fclose(index_file);
    binarioNaTela2(nome_idx);
    return;
}

/*
Realiza n insercões de registros em um arquivo binário e seu respectivo arquivo
de índice 
input: char* nome: nome do arquivo binário int n: número de registros
                    a serem inseridos 
output: void                    
*/
void insert_index(char *nome, char* nome_idx, int n) {
    idx_registro *index;
    FILE *input, *index_file;
    char *campo, *valor, *to_write;

    char *page;
    registro reg;
    cabecalho cab;

    long int pos = 0;

    double val_double = 0;
    char file_status, is_removed;
    int to_index = 0, index_size, index_pages = 0;
    int page_offset = 0, val_int = 0, pos_written;
    int idx, page_size, num_pages = 0, size = 0;

    input = fopen(nome, "rb+");
    if (file_error(input, &cab)) return;

    index_file = fopen(nome_idx, "rb+");
    index_size = load_idx(index_file, &index, &index_pages);
    if (index_size <= 0) {
        fclose(input);
        fclose(index_file);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Diz que o arquivo está sendo escrito
    file_status = '0';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, index_file);
    fseek(index_file, 32000, SEEK_SET);

    fseek(input, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, input);
    fseek(input, 32000, SEEK_SET);  // pula a página de cabecalho

    campo = calloc(256, sizeof(1));
    valor = calloc(256, sizeof(1));
    to_write = calloc(512, sizeof(1));
    page = malloc(32000 * sizeof(char));
    reg.nomeServidor = calloc(128, sizeof(char));
    reg.cargoServidor = calloc(128, sizeof(char));
    reg.telefoneServidor = calloc(15, sizeof(char));

    list *removed = create();
    from_file(removed, input, &cab);

    reg.removido = '-';
    reg.encadeamentoLista = -1;
    while (n--> 0) {
        fseek(input, 32000, SEEK_SET);
        num_pages = 0;
        to_index = 0;

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
        } 
        else {
            ++to_index;
            reg.tamNomeServidor = strlen(valor) + 2;
            strcpy(reg.nomeServidor, valor);
        }

        scan_quote_string(valor);
        if (strcmp(valor, "") == 0) {
            reg.tamCargoServidor = 0;
        } 
        else {
            reg.tamCargoServidor = strlen(valor) + 2;
            strcpy(reg.cargoServidor, valor);
        }

        reg.tamRegistro = 34;
        if (reg.tamNomeServidor > 0) reg.tamRegistro += 4 + reg.tamNomeServidor;

        if (reg.tamCargoServidor > 0)   reg.tamRegistro += 4 + reg.tamCargoServidor;

        pos_written = insert_register(input, removed, reg, cab);
        if(to_index){
            index = realloc(index, (index_size + 1)*sizeof(idx_registro));
            index[index_size].byteOffset = pos_written;
                
            index[index_size].chaveBusca = malloc(120*sizeof(char));
            memset(index[index_size].chaveBusca, '@', 120);
            strcpy(index[index_size].chaveBusca, reg.nomeServidor);
            ++index_size;
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
        fread(&size, sizeof(int), 1, input);
        fwrite(&(aux->pos), sizeof(long int), 1, input);
        memset(to_write, '@', size - 8);
        fwrite(to_write, sizeof(char), size - 8, input);
    }

    memset(page, '@', 32000);
    qsort(index, index_size, sizeof(idx_registro), comp_idx);
    idx_write(index_file, &page, index_size, index);

    // Diz que o arquivo foi escrito com sucesso
    file_status = '1';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, index_file);
    fwrite(&index_size, sizeof(int), 1, index_file);

    fseek(input, 0, SEEK_SET);
    fwrite(&file_status, sizeof(char), 1, input);

    for (int i = 0; i < index_size; ++i) free(index[i].chaveBusca);

    destroy(removed);
    free(page);
    free(index);
    free(campo);
    free(valor);
    free(to_write);
    free(reg.nomeServidor);
    free(reg.cargoServidor);
    free(reg.telefoneServidor);

    fclose(input);
    fclose(index_file);
    binarioNaTela2(nome_idx);
    return;
}

/*
Faz a busca em um arquivo binário e compara a sua
eficiência, em número de páginas de disco acessadas,
 quando se usa e quando não se usa índice
input:  char *nome_input: nome do arquivo binario
        char *nome_idx: nome do arquivo de índice
        char *campo: nome do campo que será usado para 
                     as buscas (será sempre nome servidor)
        char *valor: nome buscado
output: -1, se ocorreu algum erro na leitura do arquivo
        número de páginas acessadas, se tudo ocorreu bem
*/
void search_statistics(char *nome_input, char *nome_idx, char *campo, char *valor) {
    cabecalho cab;
    FILE *input, *index_file;
    int diff_pg = 0;
    
    input = fopen(nome_input, "rb");
    if (file_error(input, &cab)) return;
    fclose(input);

    printf("*** Realizando a busca sem o auxílio de índice\n");
    diff_pg = search_binary(nome_input, campo, valor, 1);
    printf("*** Realizando a busca com o auxílio de um índice secundário fortemente ligado\n");
    diff_pg -= index_search(nome_input, nome_idx, valor, 1);
    printf("\nA diferença no número de páginas de disco acessadas: %d",diff_pg);
    return;
}