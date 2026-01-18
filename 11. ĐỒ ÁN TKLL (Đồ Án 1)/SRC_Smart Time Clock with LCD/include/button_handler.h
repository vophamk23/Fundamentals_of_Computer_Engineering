#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

// #include <LedControl.h>
#include "LedControl.h"

// ==================== KHỞI TẠO NÚT NHẤN ====================
void initButtons();

// ==================== XỬ LÝ CÁC NÚT NHẤN ====================
void handleModeButton(LedControl *lc);
void handleSetButton();
void handleIncButton();

// ==================== KIỂM TRA NÚT NHẤN ====================
void checkButtons(LedControl *lc);

#endif // BUTTON_HANDLER_H