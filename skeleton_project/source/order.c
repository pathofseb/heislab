#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"

#define N_FLOORS 4
#define N_BUTTONS 3

static int current_floor;
static int prev_floor;
static int destination_floor;
static int direction;

// 2D array, order array
static int orders[N_FLOORS][N_BUTTONS];

// Add an order to the order array
void add_order(int floor, ButtonType button) {
    if (floor >= 0 && floor < N_FLOORS && button >= 0 && button < N_BUTTONS) {
        orders[floor][button] = 1;
    }
}


// Remove orders for a specific floor
void remove_order(int floor, ButtonType button) {
    if (floor >= 0 && floor < N_FLOORS && button >= 0 && button < N_BUTTONS) {
        orders[floor][button] = 0;
    }
}

// Clear all orders
void clear_floor_orders(int floor) {
    for (int i = 0; i < N_BUTTONS; i++) {
        orders[floor][i] = 0;
    }
}

// Check if there are any orders at a floor
int has_order_at_floor(int floor) {
    for (int i = 0; i < N_BUTTONS; i++) {
        if (orders[floor][i]) {
            return 1;
        }
    }
    return 0;
}

// Check spesific order
int has_order(int floor, ButtonType button) {
    return orders[floor][button];
}

// Find next floor in direction of movement
int next_floor_in_direction(int current_floor, MotorDirection dir) {
    if (dir == DIRN_UP) {
        for (int i = current_floor + 1; i < N_FLOORS; i++) {
            if (has_order_at_floor(i)) {
                return i;
            }

        }
    } else if (dir == DIRN_DOWN) {
        for (int i = current_floor - 1; i >= 0; i--) {
            if (has_order_at_floor(i)) {
                return i;
            }
        }
    }
    return current_floor;
}

// Check if there are any orders in the system
int has_any_orders(void) {
    for (int i = 0; i <N_FLOORS; i++) {
        if (has_order_at_floor(i)) {
            return 1;
        }
    }
    return 0;
}


// Calculate the direction of movement based on current floor and destination floor
void calculate_direction() {
    if (destination_floor > current_floor) {
        direction = DIRN_UP;
    } else if (destination_floor < current_floor) {
        direction = DIRN_DOWN;
    } else {
        direction = DIRN_STOP;
    }
}

