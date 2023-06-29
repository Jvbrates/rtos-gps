//
// Created by jvbrates on 6/21/23.
//

#ifndef RTOS_GPS_GPS_SENSOR_H
#define RTOS_GPS_GPS_SENSOR_H
#include <pthread.h>

//NMEA - START

typedef struct gpgga {
  // Latitude eg: 4124.8963 (XXYY.ZZKK.. DEG, MIN, SEC.SS)
  double latitude;
  // Latitude eg: N
  char lat;
  // Longitude eg: 08151.6838 (XXXYY.ZZKK.. DEG, MIN, SEC.SS)
  double longitude;
  // Longitude eg: W
  char lon;
  // Quality 0, 1, 2
  int quality;
  // Number of satellites: 1,2,3,4,5...
  int satellites;
  // Altitude eg: 280.2 (Meters above mean sea level)
  double altitude;
} gpgga_t;

typedef struct gprmc {
  double latitude;
  char lat;
  double longitude;
  char lon;
  double speed;
  double course;
} gprmc_t;

void nmea_parse_gpgga(char *nmea, gpgga_t *loc);
void nmea_parse_gprmc(char *nmea, gprmc_t *loc);
int nmea_valid_checksum(const char *message);
int nmea_get_message_type(const char *message);

//NMEA - END

typedef struct {
  double t_lat;
  double t_long;
  int satellites;
  int quality;
  double height;
} gpgga_t_simplified;

typedef struct {
  gpgga_t_simplified *data;
  pthread_mutex_t *mutex;
} gps_struct_t;

void gps_set(gps_struct_t *write_in);
double haversine_distance(gpgga_t_simplified A, gpgga_t_simplified B);
#endif // RTOS_GPS_GPS_SENSOR_H
