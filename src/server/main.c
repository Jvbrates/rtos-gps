//
// Created by jvbrates on 6/21/23.
//
#include "headers/gps_sensor.h"
#include "headers/velocimeter.h"
#include "headers/speed_limiter.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*The data get by gps antenna will be recorded in a variable(??? obvious),
 * this var also will be read. Then need a mutex*/
pthread_mutex_t gps_data_sync;

/* Also from above*/
pthread_mutex_t instant_speed_sync;

/* Despite I no think that it is needed, is more early remove than add*/
pthread_mutex_t limit_speed_sync;

int main(){

  struct {int * a;}aa;

  aa.a = malloc(sizeof(int));

  *((&aa)->a) = 2;
  (*((&aa)->a))++;

  printf("%d", *(aa.a));
  return 0;
}