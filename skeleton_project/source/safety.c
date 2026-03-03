#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "driver/elevio.h"
#include "order.h"
#include "door.h"
#include "utils.h"

void emergency_stop(void) {
    if(elevio_stopButton() == 1){
        elevio_motorDirection(DIRN_STOP);
        elevio_stopLamp(1);

        clear_orders();
        turn_off_all_lamps();
        while(elevio_stopButton() == 1){
            if (elevio_floorSensor() != -1) {
                elevio_floorIndicator(elevio_floorSensor());
                CURRENT_FLOOR = elevio_floorSensor();  // Update current floor while stopped
                open_door(CURRENT_FLOOR); // Open door if we're at a floor

            }
        }

        clock_t start_time = clock();

        while(1){
            if((float)((clock() - start_time)/CLOCKS_PER_SEC) >= (3.0/2.0)){
                elevio_stopLamp(0);
                close_door(CURRENT_FLOOR);
                break;
            }
            if(elevio_stopButton() == 1){
                start_time = clock();
            }
        }
    }
}