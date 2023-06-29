cmake_minimum_required(VERSION 3.25)
project(rtos_gps C)

set(CMAKE_C_STANDARD 11)

include_directories(src/server)
include_directories(src/server/headers)

add_executable(rtos_gps
        src/server/headers/blocker_tracker.h
        src/server/headers/cmd_parser.h
        src/server/headers/data_record.h
        src/server/headers/gps_sensor.h
        src/server/headers/sketch.h
        src/server/headers/speed_limiter.h
        src/server/headers/threads.h
        src/server/headers/threads_aux.h
        src/server/headers/velocimeter.h
        src/server/blocker_tracker.c
        src/server/cmd_parser.c
        src/server/data_record.c
        src/server/gps_sensor.c
        src/server/main.c
        src/server/speed_limiter.c
        src/server/threads.c
        src/server/threads_aux.c
        src/server/timers.c
        src/server/headers/timers.h
        src/server/velocimeter.c)

target_link_libraries(rtos_gps m)
