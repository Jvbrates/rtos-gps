# rtos-gps
Final Work of the RTOS discipline: Develop an embedded software (server) and a client composed of at least: 4 threads, use of mutex and condition variables, implementation of 5 commands, and GPS.


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