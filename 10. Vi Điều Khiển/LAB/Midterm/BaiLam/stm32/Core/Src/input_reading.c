/*
 * input_reading.c
 *
 *  Created on: Oct 6, 2021
 *      Author: DELL
 */

#include "main.h"
#include "input_reading.h"

// we aim to work with more than one buttons

#define BUTTON_IS_PRESSED GPIO_PIN_RESET
#define BUTTON_IS_RELEASED GPIO_PIN_SET

// we define two buffers for debouncing
static GPIO_PinState debounceButtonBuffer1[ N0_OF_BUTTONS ];
static GPIO_PinState debounceButtonBuffer2[ N0_OF_BUTTONS ];

// the buffer that the final result is stored after debouncing
static GPIO_PinState buttonBuffer[ N0_OF_BUTTONS ];

// This function is used in timer.c
void button_reading(void){
	for (int i = 0; i < N0_OF_BUTTONS ; i++) {
		debounceButtonBuffer2[i] = debounceButtonBuffer1[i];
		switch (i) {
			case 0:
				debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(Btn_1_GPIO_Port, Btn_1_Pin);
				break;
			case 1:
				debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(Btn_2_GPIO_Port, Btn_2_Pin);
				break;
			case 2:
				debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(Btn_3_GPIO_Port, Btn_3_Pin);
				break;
			default:
				break;
		}

		// Confirm it's a valid button
		if(debounceButtonBuffer1[i] == debounceButtonBuffer2[i])
			buttonBuffer[i] = debounceButtonBuffer1[i];
	}
}

// Export function
unsigned char is_button_pressed( uint8_t index ){
	if( index >= N0_OF_BUTTONS ) return 0;

	return ( buttonBuffer[index] == BUTTON_IS_PRESSED );
}


