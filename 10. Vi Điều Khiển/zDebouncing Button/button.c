#include "button.h"

/* ==================================================================
 * KHAI BÁO BIẾN TOÀN CỤC
 * ================================================================== */

// Các thanh ghi lưu trữ trạng thái nút bấm để thực hiện debouncing
int KeyReg0 = NORMAL_STATE; // Trạng thái hiện tại của nút (mới đọc)
int KeyReg1 = NORMAL_STATE; // Trạng thái trước đó 1 lần đọc
int KeyReg2 = NORMAL_STATE; // Trạng thái trước đó 2 lần đọc
int KeyReg3 = NORMAL_STATE; // Trạng thái ổn định cuối cùng

// Bộ đếm thời gian để phát hiện nhấn dài (long press)
int TimeOutForKeyPress = 500; // 500 lần gọi hàm = khoảng 500ms nếu gọi mỗi 1ms

// Các cờ trạng thái nút bấm
int button1_pressed = 0;      // Biến chưa được sử dụng trong code này
int button1_long_pressed = 0; // Cờ báo nút được nhấn dài
int button1_flag = 0;         // Cờ báo nút vừa được nhấn

/* ==================================================================
 * KIỂM TRA TRẠNG THÁI NÚT BẤM
 * ================================================================== */

/**
 * Hàm kiểm tra xem nút có vừa được nhấn hay không
 * @return 1 nếu nút vừa được nhấn, 0 nếu không
 * Lưu ý: Hàm này chỉ trả về 1 một lần duy nhất cho mỗi lần nhấn
 */
int isButton1Pressed()
{
  if (button1_flag == 1)
  {
    button1_flag = 0; // Reset cờ sau khi đã đọc
    return 1;
  }
  return 0;
}

/**
 * Hàm kiểm tra xem nút có được nhấn dài hay không
 * @return 1 nếu nút được nhấn dài, 0 nếu không
 * Lưu ý: Hàm này chỉ trả về 1 một lần duy nhất cho mỗi lần nhấn dài
 */
int isButton1LongPressed()
{
  if (button1_long_pressed == 1)
  {
    button1_long_pressed = 0; // Reset cờ sau khi đã đọc
    return 1;
  }
  return 0;
}

/* ==================================================================
 * XỬ LÝ SỰ KIỆN NÚT BẤM
 * ================================================================== */

/**
 * Hàm xử lý phụ khi nút được nhấn
 * Hiện tại chỉ đặt cờ button1_flag
 */
void subKeyProcess()
{
  // TODO: Có thể thêm các xử lý khác ở đây
  // HAL_GPIO_TogglePin(GPIOA, LED_RED_Pin); // Ví dụ: bật/tắt LED
  button1_flag = 1; // Đặt cờ báo nút được nhấn
}

/* ==================================================================
 * HÀM CHÍNH ĐỌC VÀ XỬ LÝ NÚT BẤM
 * ================================================================== */

/**
 * Hàm đọc trạng thái nút bấm và thực hiện debouncing + phát hiện long press
 * Hàm này cần được gọi định kỳ (ví dụ mỗi 1ms trong timer interrupt)
 */
void getKeyInput()
{
  /* ------ BƯỚC 1: CẬP NHẬT TRẠNG THÁI CÁC THANH GHI ------ */
  KeyReg2 = KeyReg1; // Chuyển giá trị cũ xuống
  KeyReg1 = KeyReg0; // Chuyển giá trị cũ xuống

  // Đọc trạng thái hiện tại của nút từ GPIO
  KeyReg0 = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);

  /* ------ BƯỚC 2: KIỂM TRA TÍNH ỔN ĐỊNH (DEBOUNCING) ------ */
  // Chỉ xử lý khi 3 lần đọc liên tiếp cho cùng một kết quả
  if ((KeyReg1 == KeyReg0) && (KeyReg1 == KeyReg2))
  {

    /* ------ BƯỚC 3: PHÁT HIỆN THAY ĐỔI TRẠNG THÁI ------ */
    if (KeyReg2 != KeyReg3)
    {                    // Có thay đổi trạng thái ổn định
      KeyReg3 = KeyReg2; // Cập nhật trạng thái ổn định mới

      // Nếu nút vừa được nhấn xuống
      if (KeyReg3 == PRESSED_STATE)
      {
        subKeyProcess();          // Gọi hàm xử lý phụ
        TimeOutForKeyPress = 500; // Reset bộ đếm thời gian
      }
    }
    else
    {
      /* ------ BƯỚC 4: XỬ LÝ NHẤN DÀI (LONG PRESS) ------ */
      // Trạng thái không thay đổi, giảm bộ đếm thời gian
      TimeOutForKeyPress--;

      // Nếu hết thời gian đếm và nút vẫn đang được nhấn
      if (TimeOutForKeyPress == 0)
      {
        TimeOutForKeyPress = 500; // Reset bộ đếm để tiếp tục phát hiện long press

        if (KeyReg3 == PRESSED_STATE)
        {
          subKeyProcess(); // Gọi hàm xử lý phụ
        }
      }
    }
  }
  // Nếu 3 lần đọc không giống nhau thì bỏ qua (đang có nhiễu)
}

/* ==================================================================
 * CÁCH THỨC HOẠT ĐỘNG:
 *
 * 1. DEBOUNCING (Chống nhiễu):
 *    - Sử dụng 3 thanh ghi KeyReg0, KeyReg1, KeyReg2
 *    - Chỉ chấp nhận trạng thái khi 3 lần đọc liên tiếp cho cùng kết quả
 *    - Điều này loại bỏ nhiễu cơ học của nút bấm
 *
 * 2. PHÁT HIỆN NHẤN:
 *    - So sánh trạng thái hiện tại (KeyReg2) với trạng thái trước đó (KeyReg3)
 *    - Nếu có thay đổi từ NORMAL_STATE sang PRESSED_STATE → nút được nhấn
 *
 * 3. PHÁT HIỆN NHẤN DÀI:
 *    - Sử dụng bộ đếm TimeOutForKeyPress
 *    - Khi nút được nhấn liên tục trong 500 lần gọi hàm → long press
 *    - Có thể kích hoạt sự kiện lặp lại mỗi 500ms khi giữ nút
 *
 * 4. SỬ DỤNG:
 *    - Gọi getKeyInput() định kỳ trong timer interrupt (ví dụ: 1ms)
 *    - Sử dụng isButton1Pressed() để kiểm tra nhấn thông thường
 *    - Sử dụng isButton1LongPressed() để kiểm tra nhấn dài
 *
 * VẤN ĐỀ CẦN KHẮC PHỤC:
 *    - Biến button1_long_pressed chưa được sử dụng trong logic
 *    - Cần thêm logic để phân biệt short press và long press rõ ràng hơn
 * ================================================================== */

/*
 Bắt đầu
   │
   ▼
Đọc giá trị nút → KeyReg0
   │
   ▼
3 lần đọc liên tiếp có giống nhau?
   │
┌───┴───┐
│       │
Không   Có
│       ▼
│   Trạng thái mới có khác cũ?
│       │
│   ┌───┴───┐
│   │       │
│   Không   Có
│   │       │
│   │    Cập nhật KeyReg3 = trạng thái mới
│   │    Nếu PRESSED_STATE:
│   │       - TimeOut = 500
│   │       - Cờ nhấn = 1
│   │
│   ▼
│ Giảm TimeOutForKeyPress
│   │
│   TimeOut = 0?
│   │
│   ┌───┴───┐
│   │       │
│   Không   Có
│   │       │
│   │    Reset TimeOut = 500
│   │    Nếu vẫn PRESSED_STATE:
│   │       - Cờ nhấn giữ = 1
│   │
▼
Kết thúc
*/
