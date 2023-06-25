#include "stdio.h"
#include "headers/gps_sensor.h"


typedef struct {
  char file_path[500];
  unsigned int seek_pointer;

}route_file;

typedef struct {
  double latitude;
  double longitude;
  double ray;
} position_ray;



// NOTE: Função não otimizada
int on_route(route_file r, gpgga_t_simplified position){

  FILE * file = fopen(r.file_path, "r");

  if(!file)
    return 0xF17E;

  position_ray pr_read;
  while ( // Enquanto ainda houver arquivo para ser lido
      fread(&pr_read, sizeof(position_ray), 1, file) ==
      sizeof(position_ray)
      ){

      if(
          haversine_distance(
              position, (gpgga_t_simplified){pr_read.latitude, pr_read.longitude})
          > pr_read.ray){
        return 0; // out_of_route
      }

  }
  fclose(file);
  return 1; //Está em rota

}