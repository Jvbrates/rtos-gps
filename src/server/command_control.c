#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
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

typedef struct arg_set{char* arguments[5]}  arg_set;

arg_set parse(char *input){
  arg_set arg = {0};

  char *p1 = strtok(input, ";");

  arg.arguments[0] = strtok(p1, " ");

  for (int i = 1; i < 5; ++i) {
    arg.arguments[i] = strtok(NULL, " ");
  }

  return arg;
}

void command_control(void *arg, char response[BUFFER_SIZE]);

_Noreturn void connection(void *arg) {
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t clilen;
  int sockfd, errno;
  long i;

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

    errno = recv(connected_socket, buffer, sizeof(buffer), 0);

    if (errno <= 0) {
      printf("Erro recebendo do socket, fechando conexão");
      shutdown(connected_socket, SHUT_RDWR);
    }
    printf("Recebeu: %s - %lu\n", buffer, strlen(buffer));
    // Aqui o tratamento do comando

    arg_set argumentos = parse(buffer);
    //command_control(arg, argumentos);

    errno = send(connected_socket, buffer, sizeof(buffer), 0);

    if (errno <= 0) {
      printf("Erro enviando do socket, fechando conexão");
      shutdown(connected_socket, SHUT_RDWR);
    }
  }
}


int main(){

   //connection(NULL);

  char a[] = {"locker config 123 433 11"};
 /* Isto é diferente pois aloca o ponteiro na heap (ok), entretanto a string é
 * alocada em outro lugar (não especificado por C99), mas que geralmente seria
 * .static e tratada como uma constate, por isso tentar alterar a string causa
 * SISEGV
 */
  //char *a= {"locker config 123 433 11"};
  arg_set saida = parse(a);


   return 0;
}