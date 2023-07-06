//
// Created by jvbrates on 7/6/23.
//

#ifndef RTOS_GPS_COMMAND_CONTROL_H
#define RTOS_GPS_COMMAND_CONTROL_H
typedef struct arg_set{char* arguments[5];}  arg_set;

typedef struct {
  struct command_gps{
    timer_control gps_timer;
  }gps;
} command_control_arg;


arg_set parse(char *input);

void print_cc_control(command_control_arg arg);

void command_control(void *arg, char response[], arg_set parsed);

void connection(void *arg);

#endif // RTOS_GPS_COMMAND_CONTROL_H
