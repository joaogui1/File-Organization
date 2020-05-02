/*
Autor: João Guilherme Madeira Araújo
NUSP: 9725165
*/
#include <stdio.h>
#include "lista.h"
#include "utilidades.h"
#include "trabalho1.h"
#include "trabalho2.h"
#include "trabalho3.h"
#include "trabalho4.h"

int main(){
  int opc, error, n;
  char name[256], name2[256], name3[256], field[64], value[256];
  scanf("%d %s", &opc, name);

  switch (opc) {
    case 1:
      read_csv(name);
      break;
    case 2:
      read_binary(name);
      break;
    case 3:
      scanf("%s %[^\r\n]", field, value);
      search_binary(name, field, value, 0);
      break;
    case 4:
      scanf("%d", &n);
      remove_binary(name, n);
      break;
    case 5:
      scanf("%d", &n);
      insert_binary(name, n);
      break;
    case 6:
      scanf("%d", &n);
      update_binary(name, n);
      break;
    case 7:
      scanf("%s", name2);
      sort_binary(name, name2);
      break;
    case 8:
      scanf("%s %s", name2, name3);
      merge(name, name2, name3);
      break;
    case 9:
      scanf("%s %s", name2, name3);
      match(name, name2, name3);
      break;
    case 10:
      scanf("%s", name2);
      create_index(name, name2);
      break;
    case 11:
      scanf("%s %s %[^\r\n]", name2, field, value);
      index_search(name, name2, value, 0);
      break;
    case 12:
      scanf("%s %d", name2, &n);
      remove_index(name, name2, n);
      break;
    case 13:
      scanf("%s %d", name2, &n);
      insert_index(name, name2, n);
      break;
    case 14:
      scanf("%s %s %[^\r\n]", name2, field, value);
      search_statistics(name, name2, field, value);
      break;
  }




  return 0;
}
