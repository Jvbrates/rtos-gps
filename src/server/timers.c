//
// Created by jvbrates on 6/27/23.
//

#include "headers/timers.h"
#include "time.h"
#include "signal.h"
#include "stdlib.h"


sigevent_t * own_sigevent_create(int SIGNAL){
  sigevent_t  * sigevent = malloc(sizeof(sigevent_t));
  sigevent->sigev_notify = SIGEV_SIGNAL;
  sigevent->sigev_value.sival_int = 0;
  sigevent->sigev_signo = SIGNAL;

  return sigevent;
}

void own_timer_create(timer_control *tc){
  timer_create(CLOCK_MONOTONIC, tc->sigevent, &(tc->t_id));
}

void own_timer_set(timer_control *tc){
  timer_settime(tc->t_id, 0, (tc->setup), NULL);
}

struct itimerspec * own_itimerspec(int val_sec, int iterv_sec){
  struct itimerspec * it = malloc(sizeof(struct itimerspec));

  it->it_interval.tv_sec = iterv_sec;
  it->it_value.tv_sec = val_sec;

  it->it_value.tv_nsec = 0;
  it->it_interval.tv_nsec = 0;

  return it;
}

