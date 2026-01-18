/*
 * input_reading.h
 *
 *  Created on: Oct 6, 2021
 *      Author: DELL
 */

#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

// This function is used in timer.c
void button_reading( void );

// Export function
unsigned char is_button_pressed( unsigned char index );

// Export define
#define N0_OF_BUTTONS 3

#endif /* INC_INPUT_READING_H_ */
