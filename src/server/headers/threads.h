//
// Created by jvbrates on 6/27/23.
//

#ifndef RTOS_GPS_THREADS_H
#define RTOS_GPS_THREADS_H
#include <threads.h>
#include <signal.h>

#include "timers.h"
#include "gps_sensor.h"
#include "speed_limiter.h"
#include "threads_aux.h"
#include "data_record.h"

typedef int cond_control;
typedef int loop_control;

typedef struct {
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;

  triple_cond_t record_cond;

  sigset_t *expected_signals;

  timer_control control;

} gps_set_thread_arg;

typedef struct {
  triple_cond_t record_cond;

  file_stat *fs;
  speed_struct_t speed_limit;
  speed_struct_t instant_speed;
  gps_struct_t gps;

  loop_control *enable;

} data_record_thread_arg;

typedef struct {
  char file_path[250];
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;
  sigset_t *expected_signals;
  loop_control enable;
  timer_control timer;

} blocker_tracker_thread_arg;


typedef struct {
  char file_path[250];
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;
  sigset_t *expected_signals;
  loop_control enable;
  struct triple_cond reducer;
  timer_control timer;

} blocker_thread_arg;


typedef struct {

  sigset_t *expected_signals;
  triple_cond_t control_enable;
  speed_struct_t speed_limit;
  speed reduction;
  timer_control timer;

} reducer_thread_arg;



void *gps_set_thread(void *structure);

void *data_record_thread(void *structure);

void *blocker_tracker_thread(void *structure);

void *blocker_thread(void *structure);

void *reducer_thread(void *structure);


#endif // RTOS_GPS_THREADS_H
