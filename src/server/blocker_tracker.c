#include "stdio.h"
#include "headers/gps_sensor.h"
#include "headers/blocker_tracker.h"



typedef struct {
  double latitude;
  double longitude;
  double ray;
} position_ray;



/* Eu poderia implementar de forma a usar
 * múltiplas threads para realizar o calculo de haversine,
 * mas exigiria mais tempo.
 * */
// NOTE: Função não otimizada
int on_route(char file_path[500], gpgga_t_simplified position){

  FILE * file = fopen(file_path, "r");

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