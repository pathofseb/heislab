#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "driver/elevio.h"
#include "boot.h"
#include "utils.h"
#include "order.h"
#include "door.h"
#include "safety.h"

// Elevator state
typedef enum {
    STATE_IDLE,
    STATE_MOVING,
    STATE_DOOR_OPEN
} ElevatorState;

static ElevatorState elevator_state = STATE_IDLE;
static MotorDirection current_direction = DIRN_STOP;

// Function prototypes
void poll_buttons(void);
void handle_floor_arrival(void);
int determine_next_direction(void);
void update_floor_indicator(void);

int main() {
    printf("========================================\n");
    printf("    Elevator Control System v1.0\n");
    printf("========================================\n");

    // Initialize hardware
    elevio_init();
    printf("[INIT] Elevator hardware initialized\n");

    // Find initial position
    find_position_on_boot();
    printf("[INIT] Current floor: %d\n", CURRENT_FLOOR);
    elevio_floorIndicator(CURRENT_FLOOR);

    // Clear all orders and lamps on startup
    clear_orders();
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            elevio_buttonLamp(f, b, 0);
        }
    }

    printf("[INIT] Elevator ready. Waiting for orders...\n");
    printf("========================================\n");

    // Main control loop
    while (1) {
        // Priority 1: Emergency stop button
        if (elevio_stopButton()) {
            printf("[SAFETY] Emergency stop activated!\n");
            emergency_stop();
            elevator_state = STATE_IDLE;
            current_direction = DIRN_STOP;
            continue;
        }

        // Poll all buttons and update orders
        poll_buttons();

        // Update floor position
        int floor = elevio_floorSensor();
        if (floor != -1) {
            CURRENT_FLOOR = floor;
            update_floor_indicator();
        }

        // State machine
        switch (elevator_state) {
            case STATE_IDLE:
                // Check if there are any orders
                if (has_any_orders()) {
                    printf("[IDLE] New order detected, determining direction...\n");

                    // Determine which direction to go
                    current_direction = determine_next_direction();

                    // Check if we're already at an ordered floor
                    if (CURRENT_FLOOR != -1 && has_order_at_floor(CURRENT_FLOOR)) {
                        printf("[IDLE] Already at ordered floor %d, opening door\n", CURRENT_FLOOR);
                        elevator_state = STATE_DOOR_OPEN;
                        open_door(CURRENT_FLOOR);
                        clear_floor_orders(CURRENT_FLOOR);

                        // Turn off button lamps for this floor
                        for (int b = 0; b < N_BUTTONS; b++) {
                            elevio_buttonLamp(CURRENT_FLOOR, b, 0);
                        }
                    } else {
                        // Start moving
                        printf("[IDLE] Moving %s\n",
                            current_direction == DIRN_UP ? "UP" : "DOWN");
                        elevio_motorDirection(current_direction);
                        elevator_state = STATE_MOVING;
                    }
                } else {
                    // Stay idle
                    elevio_motorDirection(DIRN_STOP);
                }
                break;

            case STATE_MOVING:
                // Check if we've arrived at a floor
                if (CURRENT_FLOOR != -1 && should_stop_at_floor(CURRENT_FLOOR, current_direction)) {
                    printf("[MOVING] Arrived at floor %d with orders\n", CURRENT_FLOOR);

                    // Stop the motor
                    elevio_motorDirection(DIRN_STOP);

                    // Open door and transition to DOOR_OPEN state
                    elevator_state = STATE_DOOR_OPEN;
                    open_door(CURRENT_FLOOR);
                    clear_floor_orders(CURRENT_FLOOR);

                    // Turn off button lamps for this floor
                    for (int b = 0; b < N_BUTTONS; b++) {
                        elevio_buttonLamp(CURRENT_FLOOR, b, 0);
                    }
                    // CRITICAL: break here to prevent direction change logic from running
                    break;
                }

                // Only check direction changes if we didn't stop at a floor
                int next_in_direction = next_floor_in_direction(CURRENT_FLOOR, current_direction);
                if (next_in_direction == CURRENT_FLOOR && !has_order_at_floor(CURRENT_FLOOR)) {
                    // No more orders in current direction
                    if (has_any_orders()) {
                        // Orders in opposite direction - change direction
                        printf("[MOVING] No orders ahead, checking other direction\n");
                        current_direction = determine_next_direction();
                        elevio_motorDirection(current_direction);
                    } else {
                        // No orders at all - go idle
                        printf("[MOVING] No more orders, going idle\n");
                        elevio_motorDirection(DIRN_STOP);
                        elevator_state = STATE_IDLE;
                    }
                }
                break;

            case STATE_DOOR_OPEN:
                // CRITICAL: Don't allow movement while obstruction is active
                if (elevio_obstruction()) {
                    elevio_motorDirection(DIRN_STOP);
                }

                // Check if door should close
                if (should_close_door()) {
                    printf("[DOOR] Closing door at floor %d\n", CURRENT_FLOOR);
                    close_door(CURRENT_FLOOR);

                    // Determine next action
                    if (has_any_orders()) {
                        current_direction = determine_next_direction();

                        // Check if there's an order at current floor (button pressed while door was open)
                        if (has_order_at_floor(CURRENT_FLOOR)) {
                            printf("[DOOR] New order at current floor, reopening door\n");
                            open_door(CURRENT_FLOOR);
                            clear_floor_orders(CURRENT_FLOOR);
                            for (int b = 0; b < N_BUTTONS; b++) {
                                elevio_buttonLamp(CURRENT_FLOOR, b, 0);
                            }
                        } else {
                            // Start moving to next order
                            printf("[DOOR] Moving %s to next order\n",
                                current_direction == DIRN_UP ? "UP" : "DOWN");
                            elevio_motorDirection(current_direction);
                            elevator_state = STATE_MOVING;
                        }
                    } else {
                        // No more orders
                        printf("[DOOR] No more orders, going idle\n");
                        elevator_state = STATE_IDLE;
                        current_direction = DIRN_STOP;
                    }
                }
                break;
        }

        // Small delay to prevent CPU spinning
        nanosleep(&(struct timespec){0, 25*1000*1000}, NULL);  // 25ms
    }

    return 0;
}

// Poll all buttons and add orders
void poll_buttons(void) {
    for (int floor = 0; floor < N_FLOORS; floor++) {
        for (int button = 0; button < N_BUTTONS; button++) {
            // Skip invalid button combinations
            if ((floor == 0 && button == BUTTON_HALL_DOWN) ||
                (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP)) {
                continue;
            }

            if (elevio_callButton(floor, button)) {
                // Check if this is a new order
                if (!has_order(floor, button)) {
                    printf("[ORDER] New order: Floor %d, Button %d\n", floor, button);
                    add_order(floor, button);
                    elevio_buttonLamp(floor, button, 1);
                }
            }
        }
    }
}

// Determine which direction to move next
int determine_next_direction(void) {
    if (CURRENT_FLOOR == -1) {
        // Between floors, keep current direction or go up by default
        return (current_direction != DIRN_STOP) ? current_direction : DIRN_UP;
    }

    // Check if there are orders in current direction
    if (current_direction == DIRN_UP) {
        int next = next_floor_in_direction(CURRENT_FLOOR, DIRN_UP);
        if (next != CURRENT_FLOOR) {
            return DIRN_UP;  // Continue up
        }
    } else if (current_direction == DIRN_DOWN) {
        int next = next_floor_in_direction(CURRENT_FLOOR, DIRN_DOWN);
        if (next != CURRENT_FLOOR) {
            return DIRN_DOWN;  // Continue down
        }
    }

    // No orders in current direction, check opposite direction
    if (current_direction != DIRN_DOWN) {
        int next = next_floor_in_direction(CURRENT_FLOOR, DIRN_DOWN);
        if (next != CURRENT_FLOOR) {
            return DIRN_DOWN;
        }
    }

    if (current_direction != DIRN_UP) {
        int next = next_floor_in_direction(CURRENT_FLOOR, DIRN_UP);
        if (next != CURRENT_FLOOR) {
            return DIRN_UP;
        }
    }

    // No orders anywhere, stop
    return DIRN_STOP;
}

// Update floor indicator light
void update_floor_indicator(void) {
    if (CURRENT_FLOOR >= 0 && CURRENT_FLOOR < N_FLOORS) {
        elevio_floorIndicator(CURRENT_FLOOR);
    }
}
