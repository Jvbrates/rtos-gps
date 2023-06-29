#include "stdio.h"
#include "headers/gps_sensor.h"
#include "headers/blocker_tracker.h"



typedef struct {
  double latitude;
  double longitude;
  double ray;
} position_ray;



// NOTE: Função não otimizada
int on_route(char file_path[500], gpgga_t_simplified position){

  FILE * file = fopen(file_path, "r");

  if(!file)
    return 0xF17E;

  position_ray pr_read;
  while ( // Enquanto ainda houver arquivo para ser lido
      fscanf(file, "%lf,%lf,%lf",
             &(pr_read.longitude),&(pr_read.latitude),&(pr_read.ray)) == 3
      ){
    double  distance = haversine_distance(position, (gpgga_t_simplified){pr_read.latitude, pr_read.longitude});

    printf("Distancia %lf\n", distance);

      if(
          haversine_distance(
              position, (gpgga_t_simplified){pr_read.latitude, pr_read.longitude})
          < pr_read.ray){
        return 1; // on_route
      }

  }
  fclose(file);
  return 0; //out of route

}



int create_file(){
  position_ray pr = {
-29.718330,-53.712851,50.0
  };

  printf("%ld", sizeof(position_ray));

  FILE * file = fopen("route.txt", "w");

  if(!file)
      return 0xF17E;


  //fwrite(&pr, sizeof(position_ray), 1, file);
  fprintf(file, "%lf,%lf,%lf\n", pr.longitude,pr.latitude,pr.ray);
  pr.latitude = -29.717874348989632;
  pr.longitude = -53.711210359440464;
  fprintf(file, "%lf,%lf,%lf", pr.longitude,pr.latitude,pr.ray);
  fclose(file);

  return 0;
}
/*

int main(){
  //create_file();

  gpgga_t_simplified p;

  &p.t_lat = -29.717874348989632;
  p.t_long = -53.711210359440464;

  printf("%d", on_route("route.txt", p));
  //printf("%d", on_route("route.txt", p));
  return 0;
}*/
