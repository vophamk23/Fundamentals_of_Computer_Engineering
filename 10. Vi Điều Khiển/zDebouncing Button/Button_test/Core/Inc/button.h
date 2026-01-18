/*
 * button.h
 *  Created on: Oct 5, 2023
 *      Author: KAI
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "main.h"

// Định nghĩa trạng thái nút (Pull-up mode: nút active-low)
#define NORMAL_STATE GPIO_PIN_SET
#define PRESSED_STATE GPIO_PIN_RESET

// Biến cờ cho 3 nút [0]=Button1, [1]=Button2, [2]=Button3
extern int button_flag[3];        // Cờ nhấn thường
extern int button_long_pressed[3]; // Cờ nhấn giữ (>500ms)

// Kiểm tra nhấn thường (trả về 1 một lần duy nhất)
int isButton1Pressed(void);
int isButton2Pressed(void);
int isButton3Pressed(void);

// Kiểm tra nhấn giữ (trả về 1 một lần, lặp lại mỗi 500ms nếu giữ)
int isButton1LongPressed(void);
int isButton2LongPressed(void);
int isButton3LongPressed(void);

// Hàm xử lý nút - GỌI TRONG TIMER INTERRUPT MỖI 1ms
void getKeyInput(void);

/*
 * CÁCH DÙNG:
 * 1. Gọi getKeyInput() trong Timer Interrupt mỗi 1ms
 * 2. Trong main loop:
 *    if (isButton1Pressed()) { ... }      // Nhấn thường
 *    if (isButton1LongPressed()) { ... }  // Nhấn giữ
 */

#endif /* INC_BUTTON_H_ */
