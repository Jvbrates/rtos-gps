//
// Created by jvbrates on 6/26/23.
//

#include "headers/threads_aux.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void wait_enable(triple_cond_t control){
  pthread_mutex_lock(control.mutex);
  while (!(*(control.enable))){
    printf("Indo dormir\n");
    pthread_cond_wait(control.cond, control.mutex);
  }
  pthread_mutex_unlock(control.mutex);
}

void wait_enable_dec(triple_cond_t control){
  pthread_mutex_lock(control.mutex);
    while (!(*(control.enable))){
      printf("Indo dormir\n");
      pthread_cond_wait(control.cond, control.mutex);
    }
    (*(control.enable))--;
  pthread_mutex_unlock(control.mutex);
}

void set_enable(triple_cond_t control, int set_val){
  pthread_mutex_lock(control.mutex);
    *(control.enable) = set_val;
  pthread_mutex_unlock(control.mutex);
}

char *time_now(){
  char *c = (char *)malloc(40);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  printf("Dispositivo GPS usb nao responde para obter informacoes da serial!\n");
  sprintf(c,"now: %d-%02d-%02d %02d:%02d:%02d\0", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return c;
}

int get_value(triple_cond_t tripla){

  pthread_mutex_lock(tripla.mutex);
  int r = *(tripla.enable);
  pthread_mutex_unlock(tripla.mutex);

  return r;
}