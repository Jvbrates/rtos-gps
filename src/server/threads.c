//
// Created by jvbrates on 6/23/23.
//
#include <pthread.h>
#include "headers/gps_sensor.h"
#include "headers/velocimeter.h"
#include "headers/speed_limiter.h"
#include "headers/data_record.h"
#include "headers/blocker_tracker.h"

#include "signal.h"

typedef int cond_control;
typedef int loop_control;

/* TODO Nas gravações de dados no GPS pode-se implementar a mesma ideia de
   leitores-escritor */

typedef struct {
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;
  pthread_mutex_t *record_mut;
  pthread_cond_t *record_cond;
  cond_control *record_req;

  sigset_t *expected_signals;

} gps_set_thread_arg;


void gps_set_thread(void *structure){
  gps_set_thread_arg * arg = (gps_set_thread_arg *)structure;

  pthread_sigmask(SIG_SETMASK, arg->expected_signals, NULL);

  while (1) { //Maybe add a control variable

    int signal_recv; //To linter no complain
    sigwait(arg->expected_signals, &signal_recv);

    gps_set(&(gps_struct_t){arg->global_pos, arg->mutex_globals_pos});

    // New gps value got, wakeup the data_record_thread
    pthread_mutex_lock(arg->record_mut);
      (*(arg->record_req))++;
    pthread_mutex_unlock(arg->record_mut);
    pthread_cond_signal(arg->record_cond);
  }

  pthread_exit(NULL);

}

typedef struct {

  cond_control *record_req;
  pthread_mutex_t *record_mut;
  pthread_cond_t *record_cond;

  file_stat *fs;
  speed_struct_t speed_limit;
  speed_struct_t instant_speed;
  gps_struct_t gps;

  loop_control *enable;

} data_record_thread_arg;

void data_record_thread(void *structure){

  data_record_thread_arg  *arg = (data_record_thread_arg * )structure;

  while(arg->enable){

    //Variável de condição
    pthread_mutex_lock(arg->record_mut);
    while ( *(arg->record_req) <= 0){
      pthread_cond_wait(arg->record_cond, arg->record_mut);
    }
    (*(arg->record_req))--;
    pthread_mutex_unlock(arg->record_mut);


    // Cria a linha que será escrita
    data_line dl;

    pthread_mutex_lock(arg->gps.mutex);
    dl.position = *(arg->gps.data);
    pthread_mutex_unlock(arg->gps.mutex);

    pthread_mutex_lock(arg->speed_limit.mutex);
    dl.max_speed = *(arg->speed_limit.data);
    pthread_mutex_unlock(arg->speed_limit.mutex);

    pthread_mutex_lock(arg->instant_speed.mutex);
    dl.instant_speed = *(arg->instant_speed.data);
    pthread_mutex_unlock(arg->instant_speed.mutex);

    //Aqui tambem ocorre mutex lock/unlock
    data_record(arg->fs, dl);

  }

  pthread_exit(NULL);

}

typedef struct {
  char file_path[250];
  gpgga_t_simplified *global_pos;
  pthread_mutex_t *mutex_globals_pos;
  sigset_t *expected_signals;
  loop_control enable;
} blocker_tracker_thread_arg;

// TODO BLOCK_TRACKER_THREAD  fazer TRACKER com variavel de condição

void blocker_tracker_thread(void *structure){
  //decode
  blocker_tracker_thread_arg *arg = (blocker_tracker_thread_arg *)structure;

  //TODO Os enables também poderiam ser implementados com variaveis condicionais
  while (arg->enable){

    int signal_recv; //To linter no complain
    sigwait(arg->expected_signals, &signal_recv);

    pthread_mutex_lock(arg->mutex_globals_pos);
      gpgga_t_simplified local_copy = *(arg->global_pos);
    pthread_mutex_unlock(arg->mutex_globals_pos);

    //Caso esteja fora de rota
    if (on_route(arg->file_path, local_copy) != 1){
        //Countdown signal 36
    }

  }

  pthread_exit(NULL);
}

void blocker_thread(void *structure){
  //decode
  blocker_tracker_thread_arg *arg = (blocker_tracker_thread_arg *)structure;

  //TODO Os enables também poderiam ser implementados com variaveis condicionais
  while (arg->enable){

    int signal_recv; //To linter no complain
    sigwait(arg->expected_signals, &signal_recv);

    pthread_mutex_lock(arg->mutex_globals_pos);
    gpgga_t_simplified local_copy = *(arg->global_pos);
    pthread_mutex_unlock(arg->mutex_globals_pos);

    //Caso esteja fora de rota
    if (on_route(arg->file_path, local_copy) != 1){
      //Desativa verificação blocker_tracker
      //FIXME como valores enables serão lido e escritos por mais de uma thread
      //quem sabe deveriam também ter seus mutexes
      arg->enable = 0;

      //Ativa countdown para reduce speed
    }

  }

  pthread_exit(NULL);
}