//
// Created by jvbrates on 6/21/23.
//

#include "headers/speed_limiter.h"
#include <pthread.h>
#include <stdio.h>

int get_speed_limit(speed_struct_t *s){
  if(s->mutex!=NULL)
    pthread_mutex_lock(s->mutex);
  FILE * file_decr;

  if((file_decr = fopen(FILE_SIM_sl, "r") )== NULL){
    printf("Erro na manipulação de arquivos\n");
    return 0xF17E;
  }

  //-----

  fscanf(file_decr, "%lf", s->data);

  //-----
  fclose(file_decr);
  if(s->mutex!=NULL)
    pthread_mutex_unlock(s->mutex);

  return 0;
}

int set_speed_limit(speed_struct_t s){
  if(s.mutex!=NULL)
    pthread_mutex_lock(s.mutex);

  FILE  * file_decr;

  if((file_decr = fopen(FILE_SIM_sl, "w") )== NULL){
    printf("Erro na manipulação de arquivos\n");
    return 0xF17E;
  }
  //-----

  fprintf(file_decr, "%lf", *(s.data));

  //-----
  fclose(file_decr);

  if(s.mutex!=NULL)
    pthread_mutex_unlock(s.mutex);

  return 0;
}

/*

int main(){
  speed_struct_t s, s0;
  s.data = malloc(sizeof(double ));
  s0.data = malloc(sizeof(double ));

  *(s.data) = 45.7;
  *(s0.data) = 100.7;

  s.mutex = NULL;
  s0.mutex = NULL;

  set_speed_limit(&s);

  scanf("%d", (int *)malloc(sizeof(int )));

  set_speed_limit(&s0);

  get_speed_limit(&s);

  printf("%lf", *(s.data));


}*/
