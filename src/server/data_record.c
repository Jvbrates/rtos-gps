//
// Created by jvbrates on 6/23/23.
//

#include "headers/data_record.h"
#include "headers/gps_sensor.h"
#include "headers/velocimeter.h"

#include <stdio.h>
#include <stdlib.h>


/* Nesta implementação o arquivo de gravação é aberto->escrito->fechado toda vez
 * que é necessário escrever (quando uma nova posição é gravada ou
 * comando do client), não seria melhor mante-lo sempre aberto?
 * */


int data_record(file_stat *fs, data_line write_data){
    pthread_mutex_t *mutex = fs->mutex;
    FILE * fpointer = fopen(fs->file_path, "a");

    if(!fpointer)
      return 0xF17E;

    printf("Escrevendooooooo\n");
    int r = fprintf(fpointer, "%lf,%lf,%lf,%d,%d,%lf,%lf\n",
            write_data.position.t_lat,
            write_data.position.t_long,
            write_data.position.height,
            write_data.position.quality,
            write_data.position.satellites,
            write_data.instant_speed,
            write_data.max_speed);

    fclose(fpointer);

    if(r < 0)
      return 0xF17E;


    /* After write increment the number of lines, this prevents race conditions
     * with read function (the read operation is limited by num of lines)*/


    return 0;
};

int data_get_line(FILE* fpointer, data_line *write_data){

    data_line * dl = (data_line *)malloc(sizeof(data_line));



    return fscanf(fpointer, "%lf,%lf,%lf,%d,%d,%lf,%lf\n",
                &(write_data->position.t_lat),
                &(write_data->position.t_long),
                &(write_data->position.height),
                &(write_data->position.quality),
                &(write_data->position.satellites),
                &(write_data->instant_speed),
                &(write_data->max_speed)
                  );
}


int data_iterate_lines(file_stat fs, int line_start, int line_end,
                  int (* func_it)(data_line, void *arg), void *arg){
    FILE *fpoiter;

    fpoiter = fopen(fs.file_path, "r");

    if(!fpoiter || fs.line_count < line_end || line_start > line_end)
      return 0xF17E;

    fseek(fpoiter, (fs.line_size)*line_start, SEEK_SET);

    data_line aux;
    int r_func_it;
    for (int i = line_start; i <= line_end ; ++i) {

      data_get_line(fpoiter, &aux);
      if((r_func_it = func_it(aux, arg) )!= 0) {
        fclose(fpoiter);
        return r_func_it;
      }
    }

    fclose(fpoiter);
    return 0;
}


int test_func_iterate(data_line dl, void *arg){
    data_line  write_data = dl;
    printf("%lf,%lf,%lf,%d,%d,%lf,%lf\n",
            write_data.position.t_lat,
            write_data.position.t_long,
            write_data.position.height,
            write_data.position.quality,
            write_data.position.satellites,
            write_data.instant_speed,
            write_data.max_speed);
    return 0;
}



/*
int main() {
  file_stat fs = {"test_file", 52, 8};


  data_line dl;
  dl.position.satellites = 3;
  dl.position.t_lat = 0;
  dl.position.t_long = 4.3;
  dl.position.quality = 4;
  dl.position.height = 4.643543;

  dl.max_speed = 66;
  dl.instant_speed = 664;

  printf("linhas: %d", fs.line_count);

  data_record(&fs, dl);

  printf("linhas: %d", fs.line_count);

  data_iterate_lines(fs, 0, 7, test_func_iterate, NULL);
  return 0;
}*/
