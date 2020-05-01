/*
Autor: João Guilherme Madeira Araújo
NUSP: 9725165
*/
#include <stdio.h>
#include "lista.h"
#include "utilidades.h"
#include "trabalho1.h"
#include "trabalho2.h"

int main(){
  int opc, error, n;
  char name[256], field[64], value[256];
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
      search_binary(name, field, value);
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
  }




  return 0;
}
