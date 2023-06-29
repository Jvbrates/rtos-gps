//
// Created by jvbrates on 6/6/23.
//

#ifndef RTOS_GPS_SKETCH_H
#define RTOS_GPS_SKETCH_H

#include <pthread.h>
typedef enum {high, low} urgency;

typedef struct {
  char text[500];
  urgency log_level;
  int seconds;
} message;

pthread_mutex_t print;

void display_message(void *)


#endif // RTOS_GPS_SKETCH_H
