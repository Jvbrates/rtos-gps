//
// Created by jvbrates on 6/21/23.
//

#ifndef RTOS_GPS_VELOCITY_LIMITER_H
#define RTOS_GPS_VELOCITY_LIMITER_H
#include <pthread.h>
#define FILE_SIM "speed_limit.simulacrum"

typedef double speed;

typedef struct {
  speed * data;
  pthread_mutex_t * mutex;
} speed_struct_t;

int get_speed_limit(speed_struct_t *);

int set_speed_limit(speed_struct_t *);

#endif // RTOS_GPS_VELOCITY_LIMITER_H
