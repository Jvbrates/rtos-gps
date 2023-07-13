//
// Created by jvbrates on 6/30/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>		// inet_aton
#include <errno.h>
#include <time.h>
#include "client.h"

#define BUFFER_SIZE 128

int sockfd;


char *time_now_filename(){
  char *c = (char *)malloc(40);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(c,"%d-%02d-%02d_%02d:%02d:%02d.csv", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return c;
}

int get_file(int fd_socket){
  char buffer[BUFFER_SIZE];
  int n;
  record_data r_data;
  FILE *file;

  char *filename = time_now_filename();
  file = fopen(filename,"w");
  if(!file){
    printf("Erro abrindo arquivo\n");
    return -1;
  }


  do {
    n = (int)recv(fd_socket, buffer, sizeof(buffer), 0);

    if (n <= 0) {
      fclose(file);
      return -1;
    }

    memcpy(&r_data, buffer, sizeof(r_data));
    if(r_data.end){
      break ;
    }
    
    fprintf(file, "%lf,%lf,%lf,%d,%d,%lf,%lf\n",
            r_data.data_msg.data.position.t_lat,
            r_data.data_msg.data.position.t_long,
            r_data.data_msg.data.position.height,
            r_data.data_msg.data.position.quality,
            r_data.data_msg.data.position.satellites,
            r_data.data_msg.data.instant_speed,
            r_data.data_msg.data.max_speed);


  } while (1);


  char msg[101];
  memcpy(msg, r_data.data_msg.msg, sizeof(char[100]));
  msg[100] = '\0';
  printf("%s", msg);
  fclose(file);


  return 0;
}

int simple_send_print(int fd_socket, char buffer[BUFFER_SIZE]){
  char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
  memset(buffer1, 0, sizeof(char[BUFFER_SIZE]));
  memcpy(buffer1, buffer, sizeof(char[BUFFER_SIZE]));

  int n = send(fd_socket, buffer1, sizeof(char[BUFFER_SIZE]), 0);

  if(n == -1)
    return -1;

  n = recv(fd_socket, buffer2, sizeof(char[BUFFER_SIZE]), 0);
  if(n <= 0) {
    printf("Erro lendo do socket! %d\n", n);
    return -1;
  }
  printf("%s\n",buffer2);

  return 0;
}


void add_end(char *buffer){
  for (int i = 0; i < BUFFER_SIZE; ++i) {
    if(buffer[i] == '\0') {
      buffer[i] = ';';
      break ;
    }
  }
}

arg_set parse(char *input){
  arg_set arg = {0};

  char *p1 = strtok(input, ";");

  arg.arguments[0] = strtok(p1, " ");

  for (int i = 1; i < 5; ++i) {
    arg.arguments[i] = strtok(NULL, " ");
  }

  return arg;
}

int load_route(char *file_path, int fd_socket){

  // ----- Abrir arquivo local
  FILE *file = NULL;
  file = fopen(file_path, "r");
  if(!file){
    printf("Arquivo não encontrado\n");
    return 0;
  }

  // ---- Abrir arquivo remoto
  char buffer[BUFFER_SIZE];
  memcpy(buffer, "load_route open;", 17);
  int n = simple_send_print(fd_socket, buffer);
  if(n != 0)
    return -1;
  // ---- Enviando linhas dos arquivos
  position_ray pr_read;
  int linha = 0;
  do {
    linha++;
    if ((n = fscanf(file, "%lf,%lf,%lf",
           &(pr_read.longitude),
           &(pr_read.latitude),
           &(pr_read.ray))) != 3){

      printf("Falha na leitura ou fim [linha %d]\n", linha);
      //fclose(file);
      break;
    }

    sprintf(buffer, "load_route write %lf %lf %lf;",
            pr_read.longitude,
            pr_read.latitude,
            pr_read.ray);

    n = simple_send_print(fd_socket, buffer);
    if (n < 0){
      fclose(file);
      return -1;
    }

  } while (!feof(file));
  // ---- Fechando arquivo remoto
  memcpy(buffer, "load_route close;", 18);
  n = simple_send_print(fd_socket, buffer);
  if(n != 0)
    return -1;
  // ---- Commitando arquivo remoto
  memcpy(buffer, "load_route commit;", 19);
  n = simple_send_print(fd_socket, buffer);
  if(n != 0)
    return -1;
  // ---- Fechando arquivo local
  n = fclose(file);
  if (n != 0) {
    printf("Falha ao fechar arquivo\n");
  } else {
    printf("Processo finalizado\n");
  }
  return 0;


}

int main(int argc, char *argv[]) {
  int portno, n;
  struct sockaddr_in serv_addr;

  pthread_t t;


  if (argc < 3) {
    fprintf(stderr,"Uso: %s nomehost porta\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    printf("Erro criando socket!\n");
    return -1;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  //    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  inet_aton(argv[1], &serv_addr.sin_addr);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    printf("Erro conectando [%d]!\n", errno);
    return -1;
  }

  //Connected

  do {
    char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE];


    // Send
    bzero(buffer,sizeof(buffer));
    printf("Digite a mensagem (ou sair):");
    fgets(buffer,sizeof(buffer),stdin);
    add_end(buffer);

    if(!strncmp(buffer, "load_route", 10)){

      arg_set arg = parse(buffer);
      n = load_route(arg.arguments[1], sockfd);

      if(n == 0)
        continue ;

    } else {
      n = send(sockfd,buffer,sizeof(buffer),0);
    }

    if (n == -1) {
      printf("Erro escrevendo no socket [%d]! \n", errno);
      shutdown(sockfd, SHUT_RDWR);
      return -1;
    }

    if (strcmp(buffer,"sair\n") == 0) {
      break;
    }

    if(!strncmp(buffer, "record get", 10)){
      get_file(sockfd);
    }



    // Receive
    bzero(buffer2,sizeof(buffer2));

    n =(int)recv(sockfd,buffer2, sizeof(buffer2),0);

    if (n <= 0) {
      printf("Erro lendo do socket! %d\n", n);
      shutdown(sockfd, SHUT_RDWR);
      return -1;
    }
    printf("MSG: %s\n",buffer2);


  } while (1);
  close(sockfd);
  return 0;
}