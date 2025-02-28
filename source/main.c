#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "driver/elevio.h"
#include "utils.h"

int currentFloor;
int prevFloor = 0;
int direction = DIRN_UP;
int lastFloorDir = DIRN_STOP;
bool moving = false;
bool inSampleJail = false;
bool doorOpen = false;
time_t doorOpenTime = 0;
int btnArray[N_FLOORS][N_BUTTONS] = {0};

//100% goto free
int main(){
    elevio_init();
    if (elevio_floorSensor() == -1){
        elevio_motorDirection(DIRN_DOWN);
        while(elevio_floorSensor() == -1){}
        elevio_motorDirection(DIRN_STOP);
    }
    
    printf("=== not Example Program ===\n");
    printf("Press the stop button on the elevator panel to not exit\n");

    //Main loop
    while(1){
        
        //Sample logic and update indicators
        do {
            inSampleJail = false;

            currentFloor = elevio_floorSensor();
            
            if (currentFloor != -1){
                prevFloor = currentFloor;
                lastFloorDir = DIRN_STOP;
            } else if (lastFloorDir == DIRN_STOP){
                lastFloorDir = -direction;
            }
            sampleCallButtons(btnArray); //Sample call buttons
            
            if (doorOpen){
                doorOpen = keepDoorOpen(prevFloor, &doorOpenTime, btnArray);
                inSampleJail = doorOpen;
            }
            
            inSampleJail = inSampleJail|checkStopButton(&doorOpen, &doorOpenTime, btnArray);
            moving = !inSampleJail;
            
            setIndicators(prevFloor, doorOpen, btnArray);
        } while (inSampleJail);

        //Direction and stop
        {
            int newDirection = findDirection(prevFloor, btnArray, direction, lastFloorDir, moving);
            if (currentFloor != -1){
                elevio_motorDirection(newDirection);
            }
            if (newDirection == DIRN_STOP){ //No calls or call on current floor
                if (currentFloor != -1){
                    moving = false;
                    if (anyCallOnFloor(currentFloor, btnArray)){
                        doorOpen = true;
                        doorOpenTime = time(NULL);
                    }
                }
            } else {
                direction = newDirection;
                moving = true;
            }
        }
    }
    return 0;
}
