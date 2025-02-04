#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include <string.h>
#include <unistd.h>

//For thou who disliketh functions
int main(){
    elevio_init();
    if (elevio_floorSensor() == -1){
        elevio_motorDirection(DIRN_DOWN);
        printf("Before sleep");
        // nanosleep(&(struct timespec){0, 2 * 10000 * 10000 * 10000}, NULL);
        sleep(2);
        printf("After sleep");
        elevio_motorDirection(DIRN_UP);
        while(elevio_floorSensor() == -1){}
        elevio_motorDirection(DIRN_STOP);
    }
    

    int floor;
    int prevFloor = 0;
    int direction = DIRN_UP;
    int lastFloorDir = DIRN_STOP;
    bool moving = false;
    bool inSampleJail = false;
    bool doorOpen = false;
    time_t doorOpenTime = 0;
    int btnArray[N_FLOORS][N_BUTTONS] = {0};
    
    printf("=== not Example Program ===\n");
    printf("Press the stop button on the elevator panel to not exit\n");

    // elevio_motorDirection(DIRN_UP);
    while(1){
        
        //The Sample Jail
        //Thou must pass through the sample jail before thou can enter the elevator logic
        //But beware, for the sample jail is a treacherous place, and thou may never leave
        sample_jail:
        do {
            //Thou hast been banished to the sample jail
            //Thou art here because thine door was openeth less than tree seconds ago
            //Or thine obstruction sensor hath been triggered while thine door was open less than tree seconds ago
            //Or thine stop button is being pressed


            floor = elevio_floorSensor();
            
            if (floor != -1){
                prevFloor = floor;
                lastFloorDir = DIRN_STOP;
            } else if (lastFloorDir == DIRN_STOP){
                lastFloorDir = -direction;
            }
            //This logic checketh if thine call button hath been pressed
            //If thine alarm button hath been pressed, it reseteth thine button array later on
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    if (elevio_callButton(f, b)){
                        btnArray[f][b] = 1;
                    }
                }
            }

            //This logic checketh if thine door is open
            //If thine door is open, it checketh if thine obstruction sensor is triggered and if so, it reseteth thine door timer
            //Thou can only leaveth the sample jail if the number of thine count is three
            inSampleJail = false;
            if (doorOpen){
                if (elevio_obstruction()){
                    doorOpenTime = time(NULL);
                    inSampleJail = true;
                } else if (time(NULL) - doorOpenTime > 3){
                    doorOpen = false;
                } else {
                    inSampleJail = true;
                }
            }
            //This logic checketh if thine stop button hath been pressed
            //If thine stop button hath been pressed, it reseteth thine button array
            //If thou art not between floors, thine door openeth
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
            //Seteth thine button lamps and floor indicator
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    elevio_buttonLamp(f, b, btnArray[f][b]);
                }
            }
            elevio_floorIndicator(prevFloor);
            elevio_doorOpenLamp(doorOpen);
            // printf("Jail\n");
        } while (inSampleJail);
        //Thou hast escaped the sample jail, and now thou art free to roam the elevator logic
        //begin elevator logic

        //Bring thine fellow who journeyeth in thine direction
        if (floor != -1){
            for (int b = 0; b < N_BUTTONS; b++){
                if (btnArray[floor][b]){
                    if (b == BUTTON_CAB || b == (direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN)){
                        memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
                        doorOpen = true;
                        doorOpenTime = time(NULL);
                        moving = false;
                        elevio_motorDirection(DIRN_STOP);
                        goto sample_jail;
                    }
                }
            }
        }
        //Seek thine friends ahead
        for (int f = prevFloor; 0 <= f && f < N_FLOORS; f += direction){
            // printf("Seeking ahead %i %i %i %i\n", f, prevFloor, direction, moving);
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
                if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                    direction = lastFloorDir;
                    // printf("Stopped between floors. %i %i\n", f, prevFloor);
                }
                moving = true;
                goto move;
            }
        }
        
        //Bring thine friends back
        for (int f = (direction == 1 ? N_FLOORS-1 : 0); 0 <= f && f < N_FLOORS; f -= direction){
            // printf("Seeking back %i\n", f);
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP]){
                if (f == prevFloor){
                    // printf("Stopping %i\n", f);
                    //Invite thine fellows whilst turning around
                    if (btnArray[prevFloor][(direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP)] && floor != -1){
                        memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
                        doorOpen = true;
                        doorOpenTime = time(NULL);
                        moving = false;
                        elevio_motorDirection(DIRN_STOP);
                        goto sample_jail;
                    }
                }
                if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                    direction = lastFloorDir;
                    // printf("Stopped between floors 2. %i %i\n", f, prevFloor);
                } else if (!moving){
                    direction = f > prevFloor ? DIRN_UP : DIRN_DOWN;
                }
                moving = true;
                goto move;
            }
        }
        //Seek thine friends behind
        for (int f = (direction == 1 ? 0 : N_FLOORS-1); f != prevFloor; f += direction){
            // printf("Seeking behind %i\n", f);
            if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
                moving = true;
                direction = -direction;
                goto move;
            }
        }

        if (btnArray[prevFloor][(direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP)] && floor != -1){
            memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
            doorOpen = true;
            doorOpenTime = time(NULL);
            moving = false;
            elevio_motorDirection(DIRN_STOP);
            goto sample_jail;
        }
        
        move:
        
        if (moving){
            elevio_motorDirection(direction);
        } else {
            elevio_motorDirection(DIRN_STOP);
        }
    }

    return 0;
}

