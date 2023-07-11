//
// Created by jvbrates on 7/7/23.
//

#ifndef RTOS_GPS_CLIENT_H
#define RTOS_GPS_CLIENT_H

typedef double speed;

typedef struct {
  double t_lat;
  double t_long;
  int satellites;
  int quality;
  double height;
} gpgga_t_simplified;

typedef struct {
  gpgga_t_simplified position;
  speed instant_speed;
  speed max_speed;
} data_line;

typedef struct data {
  int end;
  union {
    data_line data;
    char msg[100];
  }data_msg;
} record_data;

typedef struct arg_set{char* arguments[5];}  arg_set;

typedef struct {
  double latitude;
  double longitude;
  double ray;
} position_ray;

#endif // RTOS_GPS_CLIENT_H
