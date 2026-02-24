#pragma once

typedef enum { 
    CLOSED,
    OPEN,
} DoorState;


void open_door(int floor);
void close_door(int floor);
int should_close_door(void);
