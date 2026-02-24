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

        clear_orders();
        ignore_orders();
        
        while(elevio_stopButton() == 1){}

        sleep(3);
        emergency_stop();
    }
}