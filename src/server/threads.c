//
// Created by jvbrates on 6/23/23.
//
#include <pthread.h>
#include "headers/gps_sensor.h"
#include "headers/velocimeter.h"
#include "headers/speed_limiter.h"

#include "signal.h"
typedef struct {
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;
  pthread_cond_t *record_cond;
  sigset_t *expected_signals;

} gps_set_thread_arg;



void gps_set_thread(void *structure){
  gps_set_thread_arg * arg = (gps_set_thread_arg *)structure;

  pthread_sigmask(SIG_SETMASK, arg->expected_signals, NULL);

  while (1) { //Maybe add a control variable

    int signal_recv; //To linter no complain
    sigwait(arg->expected_signals, &signal_recv);

    gps_set(&(gps_struct_t){arg->global_pos, arg->mutex});

    // New gps value got, wakeup the record
    pthread_cond_signal(arg->record_cond);
  }
  pthread_exit(NULL); //if gps_set returned something, it would return here

}

typedef struct {
  gpgga_t_simplified *global_position;
  pthread_mutex_t *mutex_globals_pos;
} data_record_thread_arg;


void data_record_thread(void *structure){

  data_record_thread_arg  *arg = (data_record_thread_arg * )structure;

  //  TODO {Ask Isto seria uma boa prática?}
  pthread_mutex_lock(arg->mutex_globals_pos);
    gpgga_t_simplified local_copy = *(arg->global_position);
  pthread_mutex_lock(arg->mutex_globals_pos);
}

