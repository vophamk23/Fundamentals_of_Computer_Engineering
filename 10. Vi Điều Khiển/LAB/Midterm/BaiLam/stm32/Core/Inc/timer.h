/*
 * timer.h
 *
 *  Created on: Nov 6, 2021
 *      Author: DELL
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h" // De define TIM_HandleTypeDef

// Configuration
#define TIMER_CYCLE 10; // Notes: Moi khi cai lai TIM2 trong ioc thi phai set cai nay lai

void timer0_set(int duration);
int timer0_getFlag(void);

void timer1_set(int duration);
int timer1_getFlag(void);

void decrementSoftwareTimer(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

#endif /* INC_TIMER_H_ */
