/*
 * timer.c
 *
 *  Created on: Nov 6, 2021
 *      Author: DELL
 */

#include "timer.h"
#include "input_reading.h"

// Private variables
int timer0_counter = 0;
int timer1_counter = 0;
int timer0_flag = 1;
int timer1_flag = 1;

void timer0_set(int duration){
	timer0_counter = duration / TIMER_CYCLE ;
	timer0_flag = 0;
}
int timer0_getFlag(void) {
	return timer0_flag;
}

void decrementSoftwareTimer(void){
	if(timer0_counter > 0){
		timer0_counter--;
		if(timer0_counter == 0) timer0_flag = 1;
	}

	if(timer1_counter > 0){
		timer1_counter--;
		if(timer1_counter == 0) timer1_flag = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	if(htim->Instance == TIM2){
		//Test: HAL_GPIO_TogglePin(Led_2_GPIO_Port, Led_2_Pin);

		button_reading();
		decrementSoftwareTimer();
	}
}
