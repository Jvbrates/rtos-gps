//
// Created by jvbrates on 6/21/23.
//

#ifndef RTOS_GPS_VELOCIMETER_H
#define RTOS_GPS_VELOCIMETER_H
#define FILE_SIM "velocimeter.simulacrum"
#define SIZE_SIM 50 //Número de velocidades diferentes

typedef double speed;

//Record the speed in *km_h, if can't connect to velocimeter (can't open the
// file) return != 0 value;
int get_speed(speed *km_h);

#endif // RTOS_GPS_VELOCIMETER_H
