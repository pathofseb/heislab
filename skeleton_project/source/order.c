#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"

#include "order.h"

#define N_FLOORS 4
#define N_BUTTONS 3

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

// Clear all orders at a specific floor
void clear_floor_orders(int floor) {
    for (int i = 0; i < N_BUTTONS; i++) {
        orders[floor][i] = 0;
    }
}

void clear_orders(void) {
    for (int i = 0; i < N_FLOORS; i++) {
        clear_floor_orders(i);
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

// Check if elevator should stop at a floor given its current direction
int should_stop_at_floor(int floor, MotorDirection dir) {
    // Always stop for inside (cab) orders
    if (orders[floor][BUTTON_CAB]) {
        return 1;
    }

    // Check if there are any orders ahead in the current direction
    int orders_ahead = 0;
    if (dir == DIRN_UP) {
        for (int i = floor + 1; i < N_FLOORS; i++) {
            if (has_order_at_floor(i)) {
                orders_ahead = 1;
                break;
            }
        }
    } else if (dir == DIRN_DOWN) {
        for (int i = floor - 1; i >= 0; i--) {
            if (has_order_at_floor(i)) {
                orders_ahead = 1;
                break;
            }
        }
    }

    // If no orders ahead, stop for any order at this floor
    if (!orders_ahead) {
        return has_order_at_floor(floor);
    }

    // Otherwise, only stop for orders in the current direction
    if (dir == DIRN_UP) {
        return orders[floor][BUTTON_HALL_UP];
    } else if (dir == DIRN_DOWN) {
        return orders[floor][BUTTON_HALL_DOWN];
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
int calculate_direction(int current_floor, int destination_floor) {
    if (destination_floor > current_floor) {
        direction = DIRN_UP;
        return DIRN_UP;
    } else if (destination_floor < current_floor) {
        direction = DIRN_DOWN;
        return DIRN_DOWN;
    } else {
        direction = DIRN_STOP;
        return DIRN_STOP;
    }
}



