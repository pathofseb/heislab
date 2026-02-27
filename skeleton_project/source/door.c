#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "driver/elevio.h"
#include "boot.h"
#include "utils.h"
#include "door.h"


static DoorState door_state = CLOSED;
static time_t door_open_time;  // Track when door opened

void open_door(int floor) {
    elevio_doorOpenLamp(1);
    elevio_floorIndicator(floor);
    door_state = OPEN;
    door_open_time = time(NULL);  // Record the time when the door opened
    
}

void close_door(int floor) {
    elevio_doorOpenLamp(0);
    elevio_floorIndicator(floor);
    door_state = CLOSED;
}

// Check if door should close (called in main)
int should_close_door(void) {
    if (door_state != OPEN) {
        return 0; // Door is not open, no need to close
    }

    // Check obstruction
    if (elevio_obstruction()) {
        door_open_time = time(NULL); // Reset timer if there's an obstruction
        return 0; // Don't close the door
    }

    // Check if 3 sec have passed
    if (time(NULL) - door_open_time >= 3) {
        return 1; // Time to close the door
    }

    return 0; // Still waiting
}
