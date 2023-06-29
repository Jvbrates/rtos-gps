//
// Created by jvbrates on 6/21/23.
// gcc main.c speed_limiter.c  velocimeter.c  timers.c  gps_sensor.c data_record.c threads_aux.c blocker_tracker.c threads.c  -o rtos_gps -lm -Wall -Wextranizer"
//

#include "headers/gps_sensor.h"
#include "headers/velocimeter.h"
#include "headers/speed_limiter.h"
#include "headers/threads.h"
#include "headers/threads_aux.h"
#include "headers/timers.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SIGGPS SIGRTMIN   //gps_timer
#define SIGBLK SIGRTMIN+1 //blocker_tracker_timer
#define SIGTOL SIGRTMIN+2 //tolerance_timer
#define SIGRED SIGRTMIN+3 //reduction_timer


//Globals
//Record
triple_cond_t record_cond;
data_record_thread_arg record_struct;
file_stat fs_record;

//GPS
gpgga_t_simplified global_position;
pthread_mutex_t  global_position_mutex;

gps_set_thread_arg gps_set_struct;

//Blocker Tracker
blocker_tracker_thread_arg blocker_tracker_s, blocker_s;

//speed_limit
speed_struct_t speed_limit;
//instant_speed
speed_struct_t instant_speed;

void init(){
  speed_limit.data = malloc(sizeof(double ));
  *(speed_limit.data) = 50;
  speed_limit.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(speed_limit.mutex, NULL);

  instant_speed.data = malloc(sizeof(double ));
  instant_speed.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(instant_speed.mutex, NULL);

  fs_record.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(fs_record.mutex, NULL);
  strcpy(fs_record.file_path, "../../record.csv");
  fs_record.line_count = 0;
  fs_record.line_size = 800;


  // Record
  record_cond.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(record_cond.mutex, NULL);
  record_cond.enable = malloc(sizeof (int));
  *(record_cond.enable) = 0;
  record_cond.cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(record_cond.cond, NULL);
  record_struct.record_cond = record_cond;
  record_struct.enable = malloc(sizeof (int));
  *(record_struct.enable) = 1;
  record_struct.instant_speed = instant_speed;
  record_struct.speed_limit = speed_limit;
  record_struct.fs = &fs_record;
  record_struct.gps.data = &global_position;
  record_struct.gps.mutex = &global_position_mutex;


  //gps
  gps_set_struct.global_pos = &global_position;
  //Não é necessário que seja um pointer, pois os itens da estrutura já são.
  gps_set_struct.record_cond = record_cond;
  pthread_mutex_init(&global_position_mutex, NULL);
  gps_set_struct.mutex_globals_pos = &global_position_mutex;
  gps_set_struct.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(gps_set_struct.expected_signals);
  sigaddset(gps_set_struct.expected_signals, SIGGPS);

  //blocker_tracker
  strcpy(blocker_tracker_s.file_path,"route.txt");
  blocker_tracker_s.enable = 1;
  blocker_tracker_s.mutex_globals_pos = &global_position_mutex;
  blocker_tracker_s.global_pos = &global_position;
  blocker_tracker_s.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(blocker_tracker_s.expected_signals);
  sigaddset(blocker_tracker_s.expected_signals, SIGBLK);

  //blocker
  strcpy(blocker_s.file_path,"route.txt");
  blocker_s.enable = 1;
  blocker_s.mutex_globals_pos = &global_position_mutex;
  blocker_s.global_pos = &global_position;
  blocker_s.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(blocker_s.expected_signals);
  sigaddset(blocker_s.expected_signals, SIGTOL);



}

//Timers
timer_control gps_timer = {0};
timer_control blocker_tracker_timer = {0};
timer_control tolerance_timer = {0};
timer_control reduction_timer = {0};

void init_timers(){

  gps_timer.sigevent = own_sigevent_create(SIGGPS);
  gps_timer.setup = own_itimerspec(1, 10);
  own_timer_create(&gps_timer);
  printf("SETANDO TIMER [%d]\n", SIGGPS);
  own_timer_set(&gps_timer);

  blocker_tracker_timer.sigevent = own_sigevent_create(SIGBLK);
  blocker_tracker_timer.setup = own_itimerspec(15, 15);
  own_timer_create(&blocker_tracker_timer);
  printf("SETANDO TIMER [%d]\n", SIGBLK);
  own_timer_set(&blocker_tracker_timer);

  tolerance_timer.sigevent = own_sigevent_create(SIGTOL);
  tolerance_timer.setup = own_itimerspec(1, 10);
  own_timer_create(&tolerance_timer);
  printf("CRIANDO TIMER [%d], mas NÃO ativando\n", SIGTOL);
  //own_timer_set(&tolerance_timer);

  reduction_timer.sigevent = own_sigevent_create(SIGRED);
  reduction_timer.setup = own_itimerspec(1, 10);
  own_timer_create(&reduction_timer);
  printf("CRIANDO TIMER [%d], mas NÃO ativando\n", SIGRED);
  //own_timer_set(&tolerance_timer);



}


int main(){

  //Define os valores iniciais e estruturas
  init();

  //-----------------------------------//
  //-----------[SIGNAL]----------------//
  // SIG_BLOCK is incremental
  pthread_sigmask(SIG_BLOCK, gps_set_struct.expected_signals, NULL);
  pthread_sigmask(SIG_BLOCK, blocker_tracker_s.expected_signals, NULL);
  pthread_sigmask(SIG_BLOCK, blocker_s.expected_signals, NULL);

  //-----------------------------------//
  //-----------[THREADS]----------------//
  pthread_t gps_set_thread_t, record_thread_t,
  blocker_tracker_thread_t, blocker_thread_t;

  /* É possível que devido ao tempo entre o inicios dos timers e a criação das
   *  threads, alguns sinais sejam perdidos/ignorados.
   */
  init_timers();


  pthread_create(&record_thread_t, NULL, &data_record_thread, &record_struct);

  pthread_create(&gps_set_thread_t, NULL, &gps_set_thread, &gps_set_struct);

  pthread_create(&blocker_tracker_thread_t, NULL, &blocker_tracker_thread, &blocker_tracker_s);

  //pthread_create(&blocker_thread_t, NULL, &blocker_thread, &blocker_s);

  //pthread_create(&reduce_thread_t, NULL, &reduce_thread, &reduce_s);

  //TODO: Implementar
  //pthread_create(&connection_thread_t, NULL, &connection_thread, &connection_s);

  //-----------------------------------//
  //-----------[DEBUG]----------------//
 /* int a;

  sigset_t debug_set;
  sigemptyset(&debug_set);
  //sigaddset(&debug_set, SIGGPS);
  sigaddset(&debug_set, SIGBLK);
  sigaddset(&debug_set, SIGTOL);
  sigaddset(&debug_set, SIGRED);
  sigwait(&debug_set, &a);
  printf("[%d]\n\n", a);

  sigwait(&debug_set, &a);
  printf("[%d]\n\n", a);

  sigwait(&debug_set, &a);
  printf("[%d]\n\n", a);

  sigwait(&debug_set, &a);
  printf("[%d]\n\n", a);*/

  pthread_join(record_thread_t, NULL);
  pthread_join(gps_set_thread_t, NULL);





  return 0;

}