//
// Created by jvbrates on 6/26/23.
//

#include "headers/threads_aux.h"
#include <pthread.h>
#include <stdio.h>

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