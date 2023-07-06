#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "headers/gps_sensor.h"
#include "headers/threads.h"
#include "headers/velocimeter.h"
#include "headers/speed_limiter.h"
#include "headers/data_record.h"
#include "headers/blocker_tracker.h"
#include "headers/threads_aux.h"


void *gps_set_thread(void *structure){
  gps_set_thread_arg * arg = (gps_set_thread_arg *)structure;

  while (1) { //Always enable

    int signal_recv; //To linter no complain

    printf("Esperando o sinal\n\n");
    sigwait(arg->expected_signals, &signal_recv);
    char *c = time_now();
    printf("Sinal recebido por gps_set [%d] (%s)\n", signal_recv, c);
    free(c);
    gps_set(&(gps_struct_t){arg->global_pos, arg->mutex_globals_pos});

    //Set and Wake up data_record
    set_enable(arg->record_cond, 1);
    pthread_cond_signal(arg->record_cond.cond);
  }

  pthread_exit(NULL);

}


void *data_record_thread(void *structure){
  data_record_thread_arg  *arg = (data_record_thread_arg * )structure;

  while(arg->enable){
    printf("Data_record: Esperando enable de gps_read \n\n");
    wait_enable_dec(arg->record_cond);
    printf("Data_record: Passou enable de gps_read\n\n");

    // Cria a linha que será escrita
    data_line dl;

    pthread_mutex_lock(arg->gps.mutex);
    dl.position = *(arg->gps.data);
    pthread_mutex_unlock(arg->gps.mutex);

    pthread_mutex_lock(arg->speed_limit.mutex);
    dl.max_speed = *(arg->speed_limit.data);
    pthread_mutex_unlock(arg->speed_limit.mutex);

    pthread_mutex_lock(arg->instant_speed.mutex);
      get_speed((arg->instant_speed.data));
      //Tudo isso para manter INST_SPEED abaixo de max_speed
     dl.instant_speed = (*(arg->instant_speed.data) -
                          (int)(*(arg->instant_speed.data))) +
                         ((int)(*(arg->instant_speed.data)) % (int)dl.max_speed);
    pthread_mutex_unlock(arg->instant_speed.mutex);

    data_record(arg->fs, dl);

    pthread_mutex_lock(arg->fs->mutex);
      arg->fs->line_count++;
    pthread_mutex_unlock(arg->fs->mutex);

  }

  pthread_exit(NULL);

}



void *blocker_tracker_thread(void *structure){
  //decode
  blocker_tracker_thread_arg *arg = (blocker_tracker_thread_arg *)structure;

  //FIXME:O enable também poderiam ser implementados com variaveis condicionais
  while (arg->enable){

    int signal_recv; //To linter no complain
    printf("Esperando o sinal\n");
    sigwait(arg->expected_signals, &signal_recv);
    char *c = time_now();
    printf("Sinal recebido por blocker_tracker [%d] (%s)\n", signal_recv, c);
    free(c);


    pthread_mutex_lock(arg->mutex_globals_pos);
      gpgga_t_simplified local_copy = *(arg->global_pos);
    pthread_mutex_unlock(arg->mutex_globals_pos);

    //Caso esteja fora de rota
    int ver = on_route(arg->file_path, local_copy);
    if ( ver == 1){
      printf("Em rota\n");
    }else if( ver == 0xF17E) {
      printf("Impossível definir rota");
    } else {
      printf("Fora de rota\n");
      own_timer_set(&arg->timer);
    }
  }

  pthread_exit(NULL);
}

void *blocker_thread(void *structure){
  //decode
  blocker_thread_arg *arg = (blocker_thread_arg *)structure;

  //TODO Os enables também poderiam ser implementados com variaveis condicionais
  while (arg->enable){

    int signal_recv; //To linter no complain
    sigwait(arg->expected_signals, &signal_recv);
    char *c = time_now();
    printf("Sinal recebido por blocker [%d] (%s)\n", signal_recv, c);
    free(c);

    pthread_mutex_lock(arg->mutex_globals_pos);
    gpgga_t_simplified local_copy = *(arg->global_pos);
    pthread_mutex_unlock(arg->mutex_globals_pos);

    //Caso esteja fora de rota
    if (on_route(arg->file_path, local_copy) != 1){
      //Desativa verificação blocker_tracker
      //FIXME como valores enables serão lido e escritos por mais de uma thread
      //quem sabe deveriam também ter seus mutexes
      //arg->enable = 0;

      //Ativa  reduce speed
      set_enable(arg->reducer, 1);
      pthread_cond_broadcast(arg->reducer.cond);
    }

  }

  pthread_exit(NULL);
}


void *reducer_thread(void *structure) {
  //decode
  reducer_thread_arg *arg = (reducer_thread_arg *)structure;

  while (1) {

    wait_enable_dec(arg->control_enable); // Só precisa ser ativado uma vez

    while (1) {

      own_timer_set(&(arg->timer));
      int sig;
      sigwait(arg->expected_signals, &sig);
      char *c = time_now();
      printf("Sinal recebido por reducer [%d] (%s)\n", sig, c);
      free(c);

      // Executa o procedimento

      get_speed_limit(&(arg->speed_limit));

      if (*(arg->speed_limit.data) >= 0) {
        speed new_limit = *(arg->speed_limit.data) - arg->reduction;
        speed_struct_t speedStruct = {&new_limit, arg->speed_limit.mutex

        };
        set_speed_limit(speedStruct);
      } else {
        // Disable self
        set_enable(arg->control_enable, 0);
        break;
      }
    }
  }
}