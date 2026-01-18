/*
 * input_processing.c
 *
 *  Created on: Oct 7, 2021
 *      Author: DELL
 */


#include "main.h"
#include "input_reading.h"
#include "input_processing.h"
#include "timer.h"

typedef enum
{
    BUTTON_RELEASED,
    BUTTON_PRESSED,
} ButtonState;
#define MODE_START_IDX 0
#define MODE_END_IDX_OFFSET 2
#define CLOCK_START_IDX_OFFSET 1
#define CLOCK_END_IDX_OFFSET 12
#define MODE_TIMEOUT_OFFSET 5

ButtonState button[N0_OF_BUTTONS] = {BUTTON_RELEASED, BUTTON_RELEASED, BUTTON_RELEASED};

int mode = MODE_START_IDX; // 0 - 2
int fsmLed = Fsm_Led_0_Pin;
int clockValue[3] = {7, 11, 1};
int count_time_out = 0;

void fsm_for_input_processing(void)
{
    for (int i = 0; i < N0_OF_BUTTONS; ++i) {
    	switch (button[i]) {
    	    case BUTTON_RELEASED:
    	        if (is_button_pressed(i)) {
    	            button[i] = BUTTON_PRESSED;
    	            fsmLed = Fsm_Led_1_Pin;

    	            // TODO: Xu ly tai thoi diem btn moi press
					// Button 1
					if (i == 0) {
						mode = (mode < MODE_END_IDX_OFFSET) ? mode+1 : MODE_START_IDX;
						count_time_out = 0;
					}

					if (mode > 0) {
						switch (i) {
							case 1:
								clockValue[mode-1] = (clockValue[mode-1] < CLOCK_END_IDX_OFFSET) ? clockValue[mode-1] + 1 : CLOCK_START_IDX_OFFSET;
								break;
							case 2:
								clockValue[mode-1] = (clockValue[mode-1] > CLOCK_START_IDX_OFFSET) ? clockValue[mode-1] - 1 : CLOCK_END_IDX_OFFSET;
								break;
							default:
								break;
						}
					}
    	        } else {
    	        	// TODO: Xu ly tai thoi diem btn dang release (lap lai giong timer)
    	        }
    	        break;
    	    case BUTTON_PRESSED:
    	    	if (!is_button_pressed(i)) {
    	            button[i] = BUTTON_RELEASED;
    	            fsmLed = Fsm_Led_0_Pin;

    	            // TODO: Xu ly tai thoi diem btn moi release
    	    	}
    	    	else
    	        {
    	    		// TODO: Xu ly tai thoi diem btn press chua den 1s (lap lai giong timer)
    	        }
    	        break;
    	    default:
    	    	break;
    	}
	}
}

uint16_t mapLedPin(int num) {
	switch (num) {
		case 1:
			return Led_1_Pin;
		case 2:
			return Led_2_Pin;
		case 3:
			return Led_3_Pin;
		case 4:
			return Led_4_Pin;
		case 5:
			return Led_5_Pin;
		case 6:
			return Led_6_Pin;
		case 7:
			return Led_7_Pin;
		case 8:
			return Led_8_Pin;
		case 9:
			return Led_9_Pin;
		case 10:
			return Led_10_Pin;
		case 11:
			return Led_11_Pin;
		case 12:
			return Led_12_Pin;
		default:
			return Led_3_Pin;
	}
}

void displayClock(void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_All, GPIO_PIN_SET);
	switch (mode) {
		case 0:
			clockValue[2]++;
			if (clockValue[2] >= CLOCK_END_IDX_OFFSET) {
				clockValue[2] = CLOCK_START_IDX_OFFSET;
				clockValue[1] = (clockValue[1] >= CLOCK_END_IDX_OFFSET) ? CLOCK_START_IDX_OFFSET : clockValue[1] + 1;
				clockValue[0] = (clockValue[0] >= CLOCK_END_IDX_OFFSET) ? CLOCK_START_IDX_OFFSET : clockValue[0] + 1;
			}

			HAL_GPIO_WritePin(GPIOA, mapLedPin(clockValue[0]), GPIO_PIN_RESET);
			if (clockValue[1] != clockValue[0])
				HAL_GPIO_WritePin(GPIOA, mapLedPin(clockValue[1]), GPIO_PIN_RESET);
			if (clockValue[2] != clockValue[1] && clockValue[2] != clockValue[0])
				HAL_GPIO_WritePin(GPIOA, mapLedPin(clockValue[2]), GPIO_PIN_RESET);
			break;
		case 1:
		case 2:
			if (count_time_out >= MODE_TIMEOUT_OFFSET) mode = 0;
			else {
				count_time_out++;
				HAL_GPIO_WritePin(GPIOA, mapLedPin(clockValue[mode-1]), GPIO_PIN_RESET);
			}
			break;
		default:
			break;
	}
}
