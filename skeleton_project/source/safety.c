#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "driver/elevio.h"
#include "order.c"

void emergency_stop() {
    if(elevio_stopButton() == 1){
        elevio_motorDirection(DIRN_STOP);
        elevio_stopLamp(1);

        clear_orders();
        ignore_orders();
        
        while(elevio_stopButton() == 1){}

        clock_t start_time = clock();

        while(1){
            if((float)((clock() - start_time)/CLOCKS_PER_SEC) >= 3.0){
                printf("[EMERGENCY STOP] Clock has expired");
                elevio_stopLamp(0);
                break;
            }
            if(elevio_stopButton() == 1){
                start_time = clock();
                printf("[EMERGENCY STOP] Clock has been reset");
            }
        }
    }
}