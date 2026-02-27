#pragma once

#include "driver/elevio.h"

void add_order(int floor, ButtonType button);
void remove_order(int floor, ButtonType button);
void clear_floor_orders(int floor);
void clear_orders(void);

int has_order_at_floor(int floor);
int should_stop_at_floor(int floor, MotorDirection dir);
int has_order(int floor, ButtonType button);
int next_floor_in_direction(int current_floor, MotorDirection dir);
int has_any_orders(void);

int calculate_direction(int current_floor, int destination_floor);