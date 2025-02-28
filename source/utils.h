#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include "driver/elevio.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

/*
* @brief: This function finds the direction of the elevator based on the current floor and the button matrix.
* @param: prevFloor[in]: The current floor of the elevator.
* @param: btnArray[in]: The button matrix.
* @param: direction[in]: The current direction of the elevator.
* @param: lastFloorDir[in]: The direction of the elevator when it last stopped.
* @param: moving[in]: A boolean value indicating whether or not the elevator is moving.
* @return: The direction of the elevator.
*/
MotorDirection findDirection(int prevFloor, int btnArray[N_FLOORS][N_BUTTONS], MotorDirection direction, MotorDirection lastFloorDir, bool moving);

/*
* @brief: This function determines whether or not there are any calls on the current floor.
* @param: floor: The current floor of the elevator.
* @param: btnArray: The button matrix.
* @return: A boolean value indicating whether or not there are any calls on the current floor.
*/
bool anyCallOnFloor(int floor, int btnArray[N_FLOORS][N_BUTTONS]);

/*
* @brief: This function samples the call buttons and updates the button matrix.
* @param[out]: btnArray: The button matrix.
*/
void sampleCallButtons(int btnArray[N_FLOORS][N_BUTTONS]);

/*
* @brief: This function determines whether or not the door should be kept open.
* @important: This assumes that the door is already open.
* @param[in]: prevFloor: The current floor of the elevator.
* @param[in, out]: doorOpenTime: The time the door has been open.
* @param[in]: btnArray: The button matrix.
* @return: A boolean value indicating whether or not the door should be open.
*/
bool keepDoorOpen(int prevFloor, time_t * doorOpenTime, int btnArray[N_FLOORS][N_BUTTONS]);

/*
* @brief: This function sets the indicators of the elevator. Does not update the alarm lamp.
* @param: floor: The current floor of the elevator.
* @param: door: A boolean value indicating whether or not the door is open.
* @param: btnArray: The button matrix.
*/
void setIndicators(int floor, bool door, int btnArray[N_FLOORS][N_BUTTONS]);

/*
* @brief: This function checks if the stop button has been pressed.
* @param[in, out]: doorOpen: A boolean value indicating whether or not the door is open.
* @param[in, out]: doorOpenTime: The time the door has been open.
* @param[out]: btnArray: The button matrix. Kill it with fire!!!
* @return: A boolean value indicating whether or not the stop button has been pressed.
*/
bool checkStopButton(bool * doorOpen, time_t * doorOpenTime, int btnArray[N_FLOORS][N_BUTTONS]);


#endif // UTILITIES
