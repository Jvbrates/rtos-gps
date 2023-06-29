# rtos-gps \[Rascunhos\] 



### Mutextes (Para que eu não me esqueça)  
- Na leitura e escrita do arquivo (line number, não está implementado na função então, criar uma "cópia local" antes de passar o argumento)
- escrita em GPS_DATA

### Sincronização
- GPS_DATA
- 4 timers
  - gps_timer = 34
  - blocker_tracker_timer = 35
  - tolerance_timer(countdown) = 36
  - reduce_timer() = 37


### Haversine
φ = Latitudes
ƛ = Longitudes
![img.png](img.png)

### Coisas que podiam ser melhor
- Podia haver apenas uma thread para lidar com sinais que comunicasse as demais threads com variaveis de condição
- Função blocker_tracker e blocker são muito semelhantes, quem sabe fosse interessante uni-lás de alguma forma;
- 