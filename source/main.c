#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include <string.h>
#include <unistd.h>
void main(){
    elevio_init();
    while (elevio_floorSensor() == -1){
        elevio_motorDirection(DIRN_UP);
    }
    elevio_motorDirection(DIRN_STOP);
    int floor, prevFloor = 0, direction = DIRN_UP, lastFloorDir = DIRN_STOP;
    bool moving = false, inSampleJail = false, doorOpen = false;
    time_t doorOpenTime = 0;
    int btnArray[N_FLOORS][N_BUTTONS] = {0};
    while(1){
        sample_jail:
        do {
            floor = elevio_floorSensor();
            if (floor != -1){
                prevFloor = floor;
                lastFloorDir = DIRN_STOP;
            } else if (lastFloorDir == DIRN_STOP){
                lastFloorDir = -direction;
            }
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    if (elevio_callButton(f, b)){
                        btnArray[f][b] = 1;
                    }
                }
            }
            inSampleJail = false;
            if (doorOpen){
                memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
                moving = false;
                if (elevio_obstruction()){
                    doorOpenTime = time(NULL);
                    inSampleJail = true;
                } else if (time(NULL) - doorOpenTime > 3){
                    doorOpen = false;
                } else {
                    inSampleJail = true;
                }
            }
            if (elevio_stopButton()){
                moving = false;
                elevio_motorDirection(DIRN_STOP);
                elevio_stopLamp(1);
                inSampleJail = true;
                if (floor != -1){
                    doorOpen = true;
                    doorOpenTime = time(NULL);
                }
                memset(btnArray, 0, sizeof(btnArray)); //Cleanse thine button array
            } else {
                elevio_stopLamp(0);
            }
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    elevio_buttonLamp(f, b, btnArray[f][b]);
                }
            }
            elevio_floorIndicator(prevFloor);
            elevio_doorOpenLamp(doorOpen);
        } while (inSampleJail);
        if (floor != -1){
            for (int b = 0; b < N_BUTTONS; b++){
                if (btnArray[floor][b]){
                    if (b == BUTTON_CAB || b == (direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN)){
                        memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
                        doorOpen = true;
                        doorOpenTime = time(NULL);
                        elevio_motorDirection(DIRN_STOP);
                        goto sample_jail;
                    }
                }
            }
        }
        for (int f = prevFloor; 0 <= f && f < N_FLOORS; f += direction){
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
                if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                    direction = lastFloorDir;
                }
                moving = true;
                goto move;
            }
        }
        for (int f = (direction == 1 ? N_FLOORS-1 : 0); 0 <= f && f < N_FLOORS; f -= direction){
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP]){
                if (f == prevFloor){
                    if (btnArray[prevFloor][(direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP)] && floor != -1){
                        doorOpen = true;
                        doorOpenTime = time(NULL);
                        elevio_motorDirection(DIRN_STOP);
                        goto sample_jail;
                    }
                }
                if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                    direction = lastFloorDir;
                } else if (!moving){
                    direction = f > prevFloor ? DIRN_UP : DIRN_DOWN;
                }
                moving = true;
                goto move;
            }
        }
        for (int f = (direction == 1 ? 0 : N_FLOORS-1); f != prevFloor; f += direction){
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
                moving = true;
                direction = -direction;
                goto move;
            }
        }
        if (btnArray[prevFloor][(direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP)] && floor != -1){
            doorOpen = true;
            doorOpenTime = time(NULL);
            elevio_motorDirection(DIRN_STOP);
            goto sample_jail;
        }
        move:
        moving ? elevio_motorDirection(direction) : elevio_motorDirection(DIRN_STOP);
    }
}