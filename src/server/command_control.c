#include "headers/command_control.h"
#include "headers/timers.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PORT_LISTEN
  #define PORT_LISTEN 50012
#endif

#ifndef BUFFER_SIZE
  #define BUFFER_SIZE 128
#endif

/* Interpretador dos comandos enviados pelo cliente
 * Usarei os comandos específico para socket (netinet) para manter o padrao
 * Isto é, send, recv e shutdown ao inves de write, read e close*/

arg_set parse(char *input){
  arg_set arg = {0};

  char *p1 = strtok(input, ";");

  arg.arguments[0] = strtok(p1, " ");

  for (int i = 1; i < 5; ++i) {
    arg.arguments[i] = strtok(NULL, " ");
  }

  return arg;
}

typedef struct data {
  int end;
  union {
    data_line data;
    char msg[100];
  }data_msg;
} record_data;

void print_cc_control(command_control_arg arg){
  printf("------------------------------------\n");
  printf( "%ld, %ld, %ld\n",arg.gps.gps_timer.t_id,arg.gps.gps_timer.setup->it_interval.tv_sec, arg.gps.gps_timer.setup->it_interval.tv_nsec);
  printf("------------------------------------\n");
}

void connection(void *arg) {
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t clilen;
  int sockfd, errno;

  // Criando socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    printf("Erro abrindo o socket!\n");
    return;
  }

  memset(&serv_addr, 0, sizeof(serv_addr));

  //Configurações da conexão
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons((uint16_t)PORT_LISTEN);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    printf("Erro fazendo bind!\n");
    return;
  }

  listen(sockfd,1);
  printf("Conexão aberta\n");

  int connected_socket;
  connected_socket = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  char buffer[BUFFER_SIZE];
  char ip[INET_ADDRSTRLEN];
  int port_cli = ntohs(cli_addr.sin_port);
  inet_ntop(AF_INET, &(cli_addr.sin_addr), ip, sizeof(ip));

  printf("Conectado com %s:%i\n", ip, port_cli);

  while (1) {

    memset(buffer, 0, BUFFER_SIZE);
    errno = recv(connected_socket, buffer, sizeof(buffer), 0);

    if(!strcmp(buffer, "sair")){
      printf("Conexão fechada por commando \"sair\".");
      shutdown(connected_socket, SHUT_RDWR);

      break;
    }

    if (errno <= 0) {
      printf("Erro recebendo do socket, fechando conexão");
      shutdown(connected_socket, SHUT_RDWR);
      break;
    }
    printf("Recebeu: %s - %lu\n", buffer, strlen(buffer));
    // Aqui o tratamento do comando

    arg_set argumentos = parse(buffer);
    command_control(arg, buffer, argumentos, connected_socket);

    errno = send(connected_socket, buffer, sizeof(buffer), 0);

    if (errno <= 0) {
      printf("Erro enviando do socket, fechando conexão");
      shutdown(connected_socket, SHUT_RDWR);
    }
  }
}

typedef enum {undefined, GPS, load_route, record, locker} opcode;

opcode switch_aux(char* op){
   if(!strcmp("record", op))
    return record;
   if(!strcmp("GPS", op))
    return GPS;
   if(!strcmp("load_route", op))
    return load_route;
   if(!strcmp("locker", op))
    return locker;
   return undefined;
}


void gps_control(struct command_gps c_gps, arg_set parsed, char response[BUFFER_SIZE]){
    if(!strcmp(parsed.arguments[1], "read_interval")){
      c_gps.gps_timer.setup->it_interval.tv_sec = atoi(parsed.arguments[2]);
      own_timer_set(&c_gps.gps_timer);

      char * tmp = malloc(150);

      sprintf(tmp, "GPS read interval set to %ld",
              c_gps.gps_timer.setup->it_interval.tv_sec);

      strcpy(response, tmp);

      free(tmp);

    }else {
      strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | GPS ???");
    }

};

int cond_set(char *str, triple_cond_t cond){
    if(!strcmp(str, "enable")){
      set_enable(cond, 1);
    } else if (!strcmp(str, "disable")) {
      set_enable(cond, 1);
    } else {
      return  -1;
    }

    return 0;
}

int record_send(data_line dl, void *fd){
    record_data send_pack = {
      0,
      dl
    };

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    memcpy(buffer, &send_pack, sizeof(send_pack));

    int n =  (int)send((uintptr_t)fd, buffer, BUFFER_SIZE, 0);

    if(n > 0)
      return 0;
    return n;

}

void record_control(struct command_record c_rec, arg_set parsed, char response[BUFFER_SIZE], uintptr_t fd){
    if(!strcmp(parsed.arguments[1], "set")){
      if(!cond_set(parsed.arguments[2], c_rec.enable)){
        pthread_cond_broadcast(c_rec.enable.cond);
        char * c = (char *)malloc(100);
        sprintf(c, "record set %s", parsed.arguments[2]);
        strcpy(response, c);
        free(c);
      }
      else {
        strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | record set ???");
      }
    }
    else if (!strcmp(parsed.arguments[1], "get")){
      int line_start  = atoi(parsed.arguments[2]);
      int line_end  = atoi(parsed.arguments[3]);

      //----------Send lines of file
      int r = data_iterate_lines(*(c_rec.fs), line_start, line_end, record_send, (void *)fd);

      //----------Indicate end of file read->send
      record_data send_pack = {0};
      send_pack.end = 1;
      char buffer[BUFFER_SIZE];
      memset(buffer, 0, BUFFER_SIZE);
      memcpy(buffer, &send_pack, sizeof(send_pack));
      (int)send((int)fd, buffer, BUFFER_SIZE, 0);

      //-----------End of command procedure
      char *c = malloc(BUFFER_SIZE-10);
      sprintf(c, "Received file operation [%d]", r);
      strcpy(response, c);
      free(c);

    }
    else if (!strcmp(parsed.arguments[1], "snapshot")){
        //Set and Wake up data_record
        set_enable(c_rec.snapshot, 1);
        pthread_cond_broadcast(c_rec.snapshot.cond);
        strcpy(response, "signal sended to snapshot");
    }
    else {
      strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | record ???");
    }
}

void locker_control(struct command_locker c_loc, arg_set parsed, char response[BUFFER_SIZE]){
    if(!strcmp(parsed.arguments[1], "set")){
      if(!cond_set(parsed.arguments[2], c_loc.enable)){
        pthread_cond_broadcast(c_loc.enable.cond);
        char * c = (char *)malloc(100);
        sprintf(c, "locker set %s", parsed.arguments[2]);
        strcpy(response, c);
        free(c);
      } else {
        strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | locker set ???");
      }
    } else if (!strcmp(parsed.arguments[1], "config")){

      set_enable(c_loc.km_reduction, atoi(parsed.arguments[2]));
      c_loc.blocker_timer->setup->it_interval.tv_sec = atoi(parsed.arguments[3]);

      c_loc.tolerance_timer->setup->it_value.tv_sec = atoi(parsed.arguments[4]);
      own_timer_set(c_loc.blocker_timer);


      char * tmp = malloc(150);

      //sprintf(tmp, "GPS read interval set to %ld",
        //      c_gps.gps_timer.setup->it_interval.tv_sec);

      strcpy(response, tmp);

      free(tmp);
    } else {
      strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | locker ???");
    }
}

void load_route_control(struct command_load_route c_lr, arg_set parsed, char response[BUFFER_SIZE]){
    if(!strcmp(parsed.arguments[1], "open")){
      if(*(c_lr.fdesc)){
        strcpy(response, "file already open");
      } else {
        *(c_lr.fdesc) = fopen("tmploadfile.csv", "w");
        char *c = malloc(200);
        sprintf(c, "File opened [%p]", c_lr.fdesc);
        strcpy(response, c);
        free(c);
      }
    }
    else if(!strcmp(parsed.arguments[1], "close")){
      if(*(c_lr.fdesc)){
        int r  = fclose(*(c_lr.fdesc));
        *(c_lr.fdesc) = NULL;
        char *c = malloc(200);
        sprintf(c, "File closed [%i].",r);
        strcpy(response, c);
        free(c);
      } else {
        strcpy(response, "There is not file opened.");
      }
    }
    else if(!strcmp(parsed.arguments[1], "write")){

      char *d;
      int r = fprintf(*(c_lr.fdesc), "%Lf,%Lf,%Lf\n",
                      strtold(parsed.arguments[2], &d),
                      strtold(parsed.arguments[3], &d),
                      strtold(parsed.arguments[4], &d));

      if(r < 0){
        char *c = malloc(200);
        sprintf(c, "Error on write [%d].", r);
        strcpy(response, c);
        free(c);
      } else {
        strcpy(response, "Successfully wrote");
      }



    }
    else if(!strcmp(parsed.arguments[1], "commit")){
      pthread_mutex_lock(c_lr.locker_cond.mutex);
        int en = *(c_lr.locker_cond.enable);
      pthread_mutex_unlock(c_lr.locker_cond.mutex);
      set_enable(c_lr.locker_cond, 0);
        if(*(c_lr.fdesc)){
          strcpy(response, "Close the file before commit.");
        } else {
          int r = rename("tmploadfile.csv", "route.csv");
          if(!r){
            strcpy(response, "Successfully commited.");
          } else {
            strcpy(response, "Failure on rename().");
          }
        }
      set_enable(c_lr.locker_cond, en);

    } else {
      strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED | load_route ???");
    }
}

void command_control(void *arg, char response[BUFFER_SIZE], arg_set parsed, int fd){

   command_control_arg *cc_arg = (command_control_arg *)(arg);

   switch (switch_aux(parsed.arguments[0])) {

   case undefined:
    strcpy(response, "COMMAND SYNTAX ERROR, NOT RECOGNIZED");
    break;
   case GPS:
    gps_control(cc_arg->gps, parsed, response);
    break;
   case load_route:
    load_route_control(cc_arg->load_route, parsed, response);
    break;
   case record:
    record_control(cc_arg->record, parsed, response, fd);
    break;
   case locker:
    locker_control(cc_arg->locker, parsed, response);
    break;
   }

}


int maint(){

   connection(NULL);

   //char a[] = {"locker config 123 433 11"};
   /* Isto é diferente pois aloca o ponteiro na heap (ok), entretanto a string é
 * alocada em outro lugar (não especificado por C99), mas que geralmente seria
 * .static e tratada como uma constate, por isso tentar alterar a string causa
 * SISEGV
    */
   //char *a= {"locker config 123 433 11"};
   //arg_set saida = parse(a);


   return 0;
}
