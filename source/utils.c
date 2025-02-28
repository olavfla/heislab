
#include "utils.h"

MotorDirection findDirection(int prevFloor, int btnArray[N_FLOORS][N_BUTTONS], MotorDirection direction, MotorDirection lastFloorDir, bool moving){
    // printf("Searching direction %i\n", direction);
    int floor = elevio_floorSensor();
    //Search forwards
    for (int f = prevFloor; 0 <= f && f < N_FLOORS; f += direction){
        printf("Search floors 1 - %i %i %i\n", f, prevFloor, direction);
        if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
            printf("Found floor 1 - %i %i\n", f, prevFloor);
            if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                return lastFloorDir;
            }
            
            return f == prevFloor ? DIRN_STOP : direction;
        }
    }
    //Search backwards
    for (int f = (direction == 1 ? N_FLOORS-1 : 0); 0 <= f && f < N_FLOORS; f -= direction){
        printf("Search floors 2 - %i %i\n", f, prevFloor);
        if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_DOWN : BUTTON_HALL_UP]){
            printf("Found floor 2 - %i %i\n", f, prevFloor);
            if (!moving && floor == -1 && f == prevFloor){ //Thou hath stopped between floors
                printf("Stopped between floors 2. %i %i\n", f, prevFloor);
                return lastFloorDir;
            } else if (f != prevFloor){ //If matched on current floor, check next loop to 
                return f > prevFloor ? DIRN_UP : DIRN_DOWN;
            }
        }
    }
    printf("Search behind\n");
    //Search behind (in the same direction)
    for (int f = (direction == 1 ? 0 : N_FLOORS-1); f != prevFloor + direction; f += direction){
        printf("Search floors 3 - %i %i\n", f, prevFloor);
        if (btnArray[f][BUTTON_CAB] || btnArray[f][direction == DIRN_UP ? BUTTON_HALL_UP : BUTTON_HALL_DOWN]){
            printf("Found floor 3 - %i %i\n", f, prevFloor);
            return -direction;
        }
    }
    return DIRN_STOP;
}

bool anyCallOnFloor(int floor, int btnArray[N_FLOORS][N_BUTTONS]){
    if (floor < 0 || floor >= N_FLOORS){
        return false;
    }
    for (int b = 0; b < N_BUTTONS; b++){
        if (btnArray[floor][b]){
            return true;
        }
    }
    return false;
}

void sampleCallButtons(int btnArray[N_FLOORS][N_BUTTONS]){
    for (int f = 0; f < N_FLOORS; f++){
        for (int b = 0; b < N_BUTTONS; b++){
            btnArray[f][b] = elevio_callButton(f, b)|btnArray[f][b]; //Bitwise OR to keep previous calls
        }
    }
}

bool keepDoorOpen(int prevFloor, time_t * doorOpenTime, int btnArray[N_FLOORS][N_BUTTONS]){
    if (elevio_obstruction() || anyCallOnFloor(prevFloor, btnArray)){
        memset(btnArray[prevFloor], 0, sizeof(btnArray[prevFloor]));
        *doorOpenTime = time(NULL);
        return true;
    } else if (time(NULL) - *doorOpenTime > 3){
        return false;
    } else {
        return true;
    }
}

void setIndicators(int prevFloor, bool door, int btnArray[N_FLOORS][N_BUTTONS]){
    elevio_floorIndicator(prevFloor);
    elevio_doorOpenLamp(door);
    for (int f = 0; f < N_FLOORS; f++){
        for (int b = 0; b < N_BUTTONS; b++){
            elevio_buttonLamp(f, b, btnArray[f][b]);
        }
    }
}

bool checkStopButton(bool * doorOpen, time_t * doorOpenTime, int btnArray[N_FLOORS][N_BUTTONS]){
    if (elevio_stopButton()){
        elevio_motorDirection(DIRN_STOP);
        elevio_stopLamp(1);
        if(elevio_floorSensor() != -1){
            *doorOpen = true;
            *doorOpenTime = time(NULL);
        }
        memset(btnArray, 0, N_BUTTONS*N_FLOORS*sizeof(int)); //Compiler bad >:(
        return true;
    }
    elevio_stopLamp(0);
    return false;
}