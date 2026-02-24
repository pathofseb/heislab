#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "driver/elevio.h"
#include "utils.h"

void find_position_on_boot() {
    if(elevio_floorSensor() == -1) {
        elevio_motorDirection(DIRN_UP);
        while(1){
            if (elevio_floorSensor() != -1){
                CURRENT_FLOOR = elevio_floorSensor();
                elevio_motorDirection(DIRN_STOP);
                printf("[BOOT] Set current floor with movement");
            }
        }
    }else{
    CURRENT_FLOOR = elevio_floorSensor();
    if(CURRENT_FLOOR == -1){
        printf("[ERROR] CURRENT_FLOOR is set to -1!");
    }
    printf("[BOOT] Set current floor without movement");
    }
}
