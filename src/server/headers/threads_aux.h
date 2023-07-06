//
// Created by jvbrates on 6/26/23.
//

#ifndef RTOS_GPS_THREADS_AUX_H
#define RTOS_GPS_THREADS_AUX_H
#include <pthread.h>

typedef struct triple_cond {
  pthread_mutex_t *mutex;
  pthread_cond_t *cond;
  int *enable;
}triple_cond_t;

/*Whait while enable == 0*/
void wait_enable(triple_cond_t control);

void wait_enable_dec(triple_cond_t control);

void set_enable(triple_cond_t control, int setval);

char *time_now();

#endif // RTOS_GPS_THREADS_AUX_H
