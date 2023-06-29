//
// Created by jvbrates on 6/25/23.
//

#ifndef RTOS_GPS_BLOCKER_TRACKER_H
#define RTOS_GPS_BLOCKER_TRACKER_H
#include "gps_sensor.h"

int on_route(char file_path[250], gpgga_t_simplified position);

#endif // RTOS_GPS_BLOCKER_TRACKER_H
