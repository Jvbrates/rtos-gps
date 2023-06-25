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

  FILE * f = fopen("test", "w+");

  int a[5], b[4];
  b[0] = 64;
  a[0] = 0xF;
  a[1] = 0xF;
  a[2] = 0xF;
  a[3] = 0xFFFFFFFF;
  a[4] = 0xF11E;


  fwrite(a, sizeof(char), 16, f);

  fseek(f, 0, SEEK_SET);

  unsigned int t = fread(b, sizeof(int), 16, f);
  unsigned int r = fread(b, sizeof(int), 16, f);


  for (int i = 0; i < 4; ++i) {
    printf("[%d]\n", *(b+i));
  }


  printf("%u\n", r);
  printf("%u", t);
  fclose(f);

  return 0;
}