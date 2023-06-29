//
// Created by jvbrates on 6/23/23.
//

#ifndef RTOS_GPS_DATA_RECORD_H
#define RTOS_GPS_DATA_RECORD_H
#include "gps_sensor.h"
#include "velocimeter.h"
#include <stdio.h>
typedef struct {
  char file_path[50];
  int line_size;
  int line_count;
  pthread_mutex_t *mutex;
} file_stat;


typedef struct {
  gpgga_t_simplified position;
  speed instant_speed;
  speed max_speed;
} data_line;



int data_record(file_stat *fs, data_line write_data);

int data_get_line(FILE* fpointer, data_line *write_data);

int data_iterate_lines(file_stat fs, int line_start, int line_end,
                       int (* func_it)(data_line, void *arg), void *arg);

int test_func_iterate(data_line dl, void *arg);

#endif // RTOS_GPS_DATA_RECORD_H
