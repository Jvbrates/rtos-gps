//
// Created by jvbrates on 6/21/23.
//

#include "headers/velocimeter.h"
#include "stdio.h"

int get_speed(speed *km_h){
  static long int pointer_count;

  FILE  * file_decr;

  if((file_decr = fopen(FILE_SIM, "r") )== NULL){
    return 0xF17E;
  }
  //-----
  if(pointer_count != 0) {
    fseek(file_decr, pointer_count, SEEK_SET);
    printf("Update seel %ld\n", pointer_count);
  }


  fscanf(file_decr, "%lf", km_h);



  if(feof(file_decr))
    pointer_count = 0;
  else
    pointer_count = ftell(file_decr);


  //-----
  fclose(file_decr);

  return 0;
}
/*


int main(){
  speed val;
  int a;

  do {
    scanf("%d", &a);
    get_speed(&val);
    printf("%lf\n", val);
  } while (a!=0);

  return 0;
}*/
