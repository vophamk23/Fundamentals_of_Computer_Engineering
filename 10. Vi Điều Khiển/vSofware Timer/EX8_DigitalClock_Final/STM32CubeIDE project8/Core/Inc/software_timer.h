/*
 * software_time.h
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */

#ifndef INC_SOFTWARE_TIME_H_
#define INC_SOFTWARE_TIME_H_

void setTimer(int index, int value);
int isTimerExpired(int index);
void timerRun();

#endif /* INC_SOFTWARE_TIME_H_ */
