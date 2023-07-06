//
// Created by jvbrates on 6/27/23.
//

#ifndef RTOS_GPS_TIMERS_H
#define RTOS_GPS_TIMERS_H
#include <time.h>
#include <signal.h>

typedef struct timer_control_s {
  sigevent_t *sigevent;
  struct itimerspec *setup;
  timer_t t_id;
} timer_control;


sigevent_t * own_sigevent_create(int SIGNAL);

int own_timer_create(timer_control *tc);

int own_timer_set(timer_control *tc);

struct itimerspec * own_itimerspec(int val_sec, int iterv_sec);

#endif // RTOS_GPS_TIMERS_H
