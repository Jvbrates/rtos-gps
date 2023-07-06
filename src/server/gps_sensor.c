//
// Modified from RTOS discipline
// TODO {Study devices, specifically serial}

#include "headers/gps_sensor.h"
#include <stdlib.h>
#include <stdio.h>				// cin, cout
#include <time.h>				// time_t
#include <unistd.h>				// sleep (para imprimir todas as info do arquivo)
#include <termios.h>                            // termios
#include <fcntl.h>                              // O_RDWR
#include <math.h>                                // cos, sin, ...
#include <string.h>
#include <signal.h>
#include <pthread.h>


#define PI 	(3.141592653589793)
#define R 	(6371000)		// raio médio terra em metros (6.371Km)

// Baudrate para comunicacao USB
#define BAUDRATE_GPS B19200

// Threashold distancia GPS
#define DISTANCIA_GPS 1

// NMEA - START

#define _EMPTY 0x00
#define NMEA_GPRMC 0x01
#define NMEA_GPRMC_STR "$GPRMC"
#define NMEA_GPGGA 0x02
#define NMEA_GPGGA_STR "GPGGA"
#define NMEA_UNKNOWN 0x00
#define _COMPLETED 0x03

#define NMEA_CHECKSUM_ERR 0x80
#define NMEA_MESSAGE_ERR 0xC0

// NMEA - END



gpgga_t_simplified get_gppga_simplified() {

  // variaveis temporarias para conversao

  gpgga_t t_gpgga, c_gpgga;
  gprmc_t t_gprmc, c_gprmc;
  double t_lat, t_long, t_lat_r, t_long_r = 0;
  double c_lat, c_long, c_lat_r, c_long_r = 0;
  int graus, t_satelites = 0;

  // Valor de longitude/latitude e satelites atuais nao foram setados
  c_gpgga.latitude = c_gprmc.longitude = c_lat = c_long = c_lat_r = c_long_r = 0;
  int satelites = -1;

  // variaveis temporarias para calcular distancia (haversine)
  double lat1, lat2, d_lat, d_long, a, dist;

  // Fazer a leitura do GPS da serial
  char lineMsg[250], c;
  int res = 0, posicao = 0;

  // Opcoes de leitura/escrita serial do radio
  struct termios options;
  int device;

  // ABRIR SERIAL
  device = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_SYNC);
  //	device = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);
  //	device = open(device_name, O_RDWR | O_NOCTTY);

  memset(&options, 0, sizeof(options));

  if (tcgetattr(device, &options) != 0) {
    return (gpgga_t_simplified){0,0,0,0,0,};
  }

  cfsetispeed(&options, BAUDRATE_GPS);
  cfsetospeed(&options, BAUDRATE_GPS);

  options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;

  //      options.c_iflag &= ~IGNBRK;         // disable break processing
  options.c_lflag = 0;                // no signaling chars, no echo,
                           // no canonical processing
  options.c_oflag = 0;                // no remapping, no delays
  options.c_cc[VMIN] = 1;		// does block (disables timeout)
  options.c_cc[VTIME] = 0;            // 0.5 seconds read timeout (blocks indefinitely)

  //	options.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
  options.c_iflag = 0;	// disable all input
  options.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                         // enable reading
  options.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  options.c_cflag |= 0;	// No Parity
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CRTSCTS;

  if (tcsetattr(device, TCSANOW, &options) != 0) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Dispositivo GPS usb nao responde para obter informacoes da serial!\n");
    printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return (gpgga_t_simplified){0,0,0,0,0,};
  }

  // PEGAR DADOS GPS E MOSTRAR DADOS
  while(1) {
    // Fica na espera ate ter uma msg do GPS
    memset(lineMsg, '\0', 250);
    posicao = 0;
    while (1) {
      res = read(device,&c,1);
      if (res == 1) {
        lineMsg[posicao] = c;
        posicao++;
        if (c == '\n') {
          //printf("aa%s\n", lineMsg);
          break;
        }
      } else if (res == 0) {
        continue;
      } else {
        continue;
      }
    }

    if (lineMsg[0] == '$') {
      res = nmea_get_message_type(lineMsg);
      if (res == NMEA_GPGGA) {
        nmea_parse_gpgga(lineMsg, &t_gpgga);

        // valor de satelites
        t_satelites = t_gpgga.satellites;
        printf("Quantidade de satélites: %d\n", t_satelites);

        // calculo latitude atual
        t_lat = (t_gpgga.latitude/100.0);
        graus = (int)t_lat;
        t_lat = (t_lat - (float)graus);
        t_lat = ((t_lat * 100.0) / 60.0) + (float)graus;
        t_lat *= (t_gpgga.lat == 'S') ? -1.0 : +1.0;
        t_lat_r = (t_lat * PI) / 180.0;

        // calculo longitude atual
        t_long = (t_gpgga.longitude/100.0);
        graus = (int)t_long;
        t_long = (t_long - (float)graus);
        t_long = ((t_long * 100.0) / 60.0) + (float)graus;
        t_long *= (t_gpgga.lon == 'W') ? -1.0 : +1.0;
        t_long_r = (t_long * PI) / 180.0;

        gpgga_t_simplified g = {t_lat, t_long, t_gpgga.satellites ,t_gpgga.quality, t_gpgga.altitude};
        return g;

        printf("Distância: %f, Latitude: %f (%f), Longitude: %f (%f), Qualidade: %d, Altitude: %f\n",
               dist, t_lat_r, t_lat, t_long_r, t_long, t_gpgga.quality, t_gpgga.altitude);

        // inicializa valores da latitude e longitude correntes
        if ((c_lat == 0) && (t_lat != 0)) {
          c_gpgga = t_gpgga;
          c_lat = t_lat;
          c_lat_r = t_lat_r;
          c_long = t_long;
          c_long_r = t_long_r;
        } else {
          // a = pow ( sin(d_lat/2) , 2 ) + cos(lat1) * cos(lat2) * pow ( sin(d_long/2) , 2)
          //var R = 6371;
          //var lat1 = lat1.toRad();
          //var lat2 = lat2.toRad();
          lat1 = c_lat_r;
          lat2 = t_lat_r;
          //var dLat = (lat2-lat1).toRad();
          //var dLon = (lon2-lon1).toRad();
          d_lat = ((t_lat - c_lat) * PI) / 180.0;
          d_long = ((t_long - c_long) * PI) / 180.0;
          //var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
          //        Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2);
          a = pow(sin(d_lat/2.0),2) +
              cos(lat1) * cos(lat2) * pow(sin(d_long/2.0),2);
          //var c = 2 * atan2( sqrt(a) , sqrt( ( 1 - a )) )
          c = 2 * atan2(sqrt(a), sqrt((1-a)));
          //var d = R * c
          dist = R * c;

          if (dist > DISTANCIA_GPS) {
            printf("Distância: %f, Latitude: %f (%f), Longitude: %f (%f), Qualidade: %d, Altitude: %f\n",
                   dist, t_lat_r, t_lat, t_long_r, t_long, t_gpgga.quality, t_gpgga.altitude);

            c_gpgga = t_gpgga;
            c_lat = t_lat;
            c_lat_r = t_lat_r;
            c_long = t_long;
            c_long_r = t_long_r;
          }
        }
      }
    }
  }


  return (gpgga_t_simplified){0,0,0,0,0,};
}


// NMEA - start
void nmea_parse_gpgga(char *nmea, gpgga_t *loc)
{
  char *p = nmea;

  p = strchr(p, ',')+1; //skip time

  p = strchr(p, ',')+1;
  loc->latitude = atof(p);

  p = strchr(p, ',')+1;
  switch (p[0]) {
  case 'N':
    loc->lat = 'N';
    break;
  case 'S':
    loc->lat = 'S';
    break;
  case ',':
    loc->lat = '\0';
    break;
  }

  p = strchr(p, ',')+1;
  loc->longitude = atof(p);

  p = strchr(p, ',')+1;
  switch (p[0]) {
  case 'W':
    loc->lon = 'W';
    break;
  case 'E':
    loc->lon = 'E';
    break;
  case ',':
    loc->lon = '\0';
    break;
  }

  p = strchr(p, ',')+1;
  loc->quality = (int)atoi(p);

  p = strchr(p, ',')+1;
  loc->satellites = (int)atoi(p);

  p = strchr(p, ',')+1;

  p = strchr(p, ',')+1;
  loc->altitude = atof(p);
}

void nmea_parse_gprmc(char *nmea, gprmc_t *loc)
{
  char *p = nmea;

  p = strchr(p, ',')+1; //skip time
  p = strchr(p, ',')+1; //skip status

  p = strchr(p, ',')+1;
  loc->latitude = atof(p);

  p = strchr(p, ',')+1;
  switch (p[0]) {
  case 'N':
    loc->lat = 'N';
    break;
  case 'S':
    loc->lat = 'S';
    break;
  case ',':
    loc->lat = '\0';
    break;
  }

  p = strchr(p, ',')+1;
  loc->longitude = atof(p);

  p = strchr(p, ',')+1;
  switch (p[0]) {
  case 'W':
    loc->lon = 'W';
    break;
  case 'E':
    loc->lon = 'E';
    break;
  case ',':
    loc->lon = '\0';
    break;
  }

  p = strchr(p, ',')+1;
  loc->speed = atof(p);

  p = strchr(p, ',')+1;
  loc->course = atof(p);
}

int nmea_valid_checksum(const char *message) {
  int checksum= (int)strtol(strchr(message, '*')+1, NULL, 16);

  char p;
  int sum = 0;
  ++message;
  while ((p = *message++) != '*') {
    sum ^= p;
  }

  if (sum != checksum) {
    return NMEA_CHECKSUM_ERR;
  }

  return _EMPTY;
}

int nmea_get_message_type(const char *message)
{
  int checksum = 0;
  if ((checksum = nmea_valid_checksum(message)) != _EMPTY) {
    return checksum;
  }

  if (strstr(message, NMEA_GPGGA_STR) != NULL) {
    return NMEA_GPGGA;
  }

  if (strstr(message, NMEA_GPRMC_STR) != NULL) {
    return NMEA_GPRMC;
  }

  return NMEA_UNKNOWN;
}

//NMEA - end

void gps_set(gps_struct_t *write_in){


  //Manter fora da seção de bloqueio diminui o tempo em que o mutex estará bloqueado
  gpgga_t_simplified g = get_gppga_simplified();

  if(write_in->mutex!=NULL)
    pthread_mutex_lock(write_in->mutex);

  memcpy(write_in->data, &g, sizeof(gpgga_t_simplified));

  if(write_in->mutex!=NULL)
    pthread_mutex_unlock(write_in->mutex);

}
/*Nunca estudei trigonometria esférica*/
double grautoRad(double angle){
  return (PI/180)*angle;
}

double haversine_distance(gpgga_t_simplified A, gpgga_t_simplified B){
  double A_lat = grautoRad(A.t_lat);
  double A_long = grautoRad(A.t_long);

  double B_lat = grautoRad(B.t_lat);
  double B_long = grautoRad(B.t_long);

  return 2*R*asin(
          sqrt(
             pow(sin(
                  (A_lat-B_lat)/2
                  ), 2)+
              cos(A_lat)* cos(B_lat)*
                  pow(sin(
                      (A_long - B_long)/2
                      ), 2)
              )
          );
}






/*
int main(){
  gpgga_t_simplified A, B;
  A.t_lat = -29.71280345581664;
  A.t_long = -53.71364025771885;

  B.t_lat = -29.711351698885817;
  B.t_long = -53.717921195969655;

  printf("%lf", haversine_distance(A, B));


  return 0;
}

 */