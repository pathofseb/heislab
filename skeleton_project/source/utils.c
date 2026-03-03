#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"

int CURRENT_FLOOR;
int CURRENT_DIRECTION;

void turn_off_all_lamps(void) {
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            elevio_buttonLamp(f, b, 0);
        }
    }
}