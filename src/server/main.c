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
#include "headers/command_control.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SIGGPS SIGRTMIN   //gps_timer
#define SIGBLK (SIGRTMIN+1) //blocker_tracker_timer
#define SIGTOL (SIGRTMIN+2) //tolerance_timer
#define SIGRED (SIGRTMIN+3) //reduction_timer


//Globals
//Record
triple_cond_t record_cond;
triple_cond_t record_enable;
data_record_thread_arg record_struct;
file_stat fs_record;

//GPS
gpgga_t_simplified GPS_DATA;
pthread_mutex_t  global_position_mutex;

gps_set_thread_arg gps_set_struct;

//Blocker Tracker
triple_cond_t  blocker_enable;
blocker_tracker_thread_arg blocker_tracker_s;
blocker_thread_arg  blocker_s;

//Reducer
reducer_thread_arg reducer_s;

//SPEED_LIMIT
speed_struct_t SPEED_LIMIT;
//INST_SPEED
speed_struct_t INST_SPEED;

void init(){

  //Speed Limit
  SPEED_LIMIT.data = malloc(sizeof(double ));
  *(SPEED_LIMIT.data) = 50;
  SPEED_LIMIT.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(SPEED_LIMIT.mutex, NULL);

  //Instant Speed
  INST_SPEED.data = malloc(sizeof(double ));
  INST_SPEED.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(INST_SPEED.mutex, NULL);


  //file record
  fs_record.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(fs_record.mutex, NULL);
  strcpy(fs_record.file_path, "record.csv");
  fs_record.line_count = count_line(fs_record.file_path);
  fs_record.line_size = 52;


  // Record
  // record_cond
  record_cond.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(record_cond.mutex, NULL);
  record_cond.enable = malloc(sizeof (int));
  *(record_cond.enable) = 0;
  record_cond.cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(record_cond.cond, NULL);
  //record_enable
  record_enable.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(record_enable.mutex, NULL);
  record_enable.enable = malloc(sizeof (int));
  *(record_enable.enable) = 0; //Default Enable | FIXME
  record_enable.cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(record_enable.cond, NULL);

  record_struct.enable_cond = record_enable;
  record_struct.record_cond = record_cond;

  record_struct.enable = malloc(sizeof (int));
  *(record_struct.enable) = 1;
  record_struct.instant_speed = INST_SPEED;
  record_struct.speed_limit = SPEED_LIMIT;
  record_struct.fs = &fs_record;
  record_struct.gps.data = &GPS_DATA;
  record_struct.gps.mutex = &global_position_mutex;


  //gps
  gps_set_struct.global_pos = &GPS_DATA;
  //Não é necessário que seja um pointer, pois os itens da estrutura já são.
  gps_set_struct.record_cond = record_cond;
  pthread_mutex_init(&global_position_mutex, NULL);
  gps_set_struct.mutex_globals_pos = &global_position_mutex;
  gps_set_struct.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(gps_set_struct.expected_signals);
  sigaddset(gps_set_struct.expected_signals, SIGGPS);

  //blocker_tracker
  strcpy(blocker_tracker_s.file_path,"route.csv");
  blocker_tracker_s.enable = 1;
  blocker_tracker_s.mutex_globals_pos = &global_position_mutex;
  blocker_tracker_s.global_pos = &GPS_DATA;
  blocker_tracker_s.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(blocker_tracker_s.expected_signals);
  sigaddset(blocker_tracker_s.expected_signals, SIGBLK);
  //blocker tracer enable
  //record_enable
  blocker_enable.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(blocker_enable.mutex, NULL);
  blocker_enable.enable = malloc(sizeof (int));
  *(blocker_enable.enable) = 0; //Default disable | FIXME
  blocker_enable.cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(blocker_enable.cond, NULL);
  blocker_tracker_s.enable_cond = blocker_enable;

  //blocker
  strcpy(blocker_s.file_path,"route.csv");
  blocker_s.enable = 1;
  blocker_s.enable_cond = blocker_enable;
  blocker_s.mutex_globals_pos = &global_position_mutex;
  blocker_s.global_pos = &GPS_DATA;
  blocker_s.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(blocker_s.expected_signals);
  sigaddset(blocker_s.expected_signals, SIGTOL);


  //reducer
  reducer_s.speed_limit = SPEED_LIMIT;
  reducer_s.control_enable.enable = malloc(sizeof(int));
  *(reducer_s.control_enable.enable) = 0; //Inicia-se desativado
  reducer_s.control_enable.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(reducer_s.control_enable.mutex, NULL);
  reducer_s.control_enable.cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(reducer_s.control_enable.cond, NULL);
  reducer_s.expected_signals = malloc(sizeof(sigset_t));
  sigemptyset(reducer_s.expected_signals);
  sigaddset(reducer_s.expected_signals, SIGRED);
  reducer_s.km_reduction.enable = malloc(sizeof(int));
  *(reducer_s.km_reduction.enable) = 10; //Inicia-se desativado
  reducer_s.km_reduction.mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(reducer_s.km_reduction.mutex, NULL);



  blocker_s.reducer = reducer_s.control_enable;

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
  tolerance_timer.setup = own_itimerspec(10, 0); //CountDown
  own_timer_create(&tolerance_timer);
  printf("CRIANDO TIMER [%d], mas NÃO ativando\n", SIGTOL);
  blocker_tracker_s.timer = &tolerance_timer;
  //own_timer_set(&tolerance_timer);

  reduction_timer.sigevent = own_sigevent_create(SIGRED);
  reduction_timer.setup = own_itimerspec(10, 0); //CountDown
  own_timer_create(&reduction_timer);

  reducer_s.timer = &reduction_timer;


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
  pthread_sigmask(SIG_BLOCK, reducer_s.expected_signals, NULL);

  //-----------------------------------//
  //-----------[THREADS]----------------//
  pthread_t gps_set_thread_t, record_thread_t,
  blocker_tracker_thread_t, blocker_thread_t, reducer_thread_t;

  /* É possível que devido ao tempo entre o inicios dos timers e a criação das
   *  threads, alguns sinais sejam perdidos/ignorados.
   */
  init_timers();


  pthread_create(&record_thread_t, NULL, &data_record_thread, &record_struct);

  pthread_create(&gps_set_thread_t, NULL, &gps_set_thread, &gps_set_struct);

  pthread_create(&blocker_tracker_thread_t, NULL, &blocker_tracker_thread, &blocker_tracker_s);

  pthread_create(&blocker_thread_t, NULL, &blocker_thread, &blocker_s);

  pthread_create(&reducer_thread_t, NULL, &reducer_thread, &reducer_s);

  command_control_arg cc_debug;

  cc_debug.gps.gps_timer = gps_timer;
  cc_debug.record.enable = record_enable;
  cc_debug.locker.enable = blocker_enable;
  cc_debug.locker.km_reduction = reducer_s.km_reduction;
  cc_debug.locker.tolerance_timer = &tolerance_timer;
  cc_debug.locker.blocker_timer = &blocker_tracker_timer;
  cc_debug.record.snapshot = record_cond;
  cc_debug.record.fs = &fs_record;
  cc_debug.load_route.locker_cond = blocker_enable;

  cc_debug.load_route.fdesc = malloc(sizeof(FILE *));

  print_cc_control(cc_debug);

  connection(&cc_debug);

  print_cc_control(cc_debug);

  pthread_join(record_thread_t, NULL);
  pthread_join(gps_set_thread_t, NULL);

  return 0;
}