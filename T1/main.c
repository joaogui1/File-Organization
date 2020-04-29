/*
Autor: João Guilherme Madeira Araújo
NUSP: 9725165
*/
#include <stdio.h>
#include "utilidades.h"

int main(){
  int opc, error;
  char name[256], field[64], value[256];
  scanf("%d %s", &opc, name);

  switch (opc) {
    case 1:
      read_csv(name);
      break;
    case 2:
      read_binary(name);
      break;

    default:
      scanf("%s %[^\r\n]", field, value);
      search_binary(name, field, value);
      break;
  }




  return 0;
}
