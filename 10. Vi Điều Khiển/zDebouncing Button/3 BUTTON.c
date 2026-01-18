#include "button.h"

/* ==================================================================
 * GIẢI THÍCH CƠ CHẾ HOẠT ĐỘNG:
 *
 * Module này xử lý 3 nút bấm với các tính năng:
 * 1. DEBOUNCING (Chống dội): Loại bỏ nhiễu cơ học của nút
 * 2. SHORT PRESS: Phát hiện nhấn nhanh
 * 3. LONG PRESS: Phát hiện nhấn giữ (sau 500ms)
 *
 * NGUYÊN LÝ DEBOUNCING:
 * - Sử dụng 4 thanh ghi trạng thái: KeyReg0, KeyReg1, KeyReg2, KeyReg3
 * - KeyReg0: Giá trị đọc mới nhất
 * - KeyReg1: Giá trị đọc lần trước
 * - KeyReg2: Giá trị đọc 2 lần trước
 * - KeyReg3: Trạng thái ổn định cuối cùng (sau khi debounce)
 * - Chỉ chấp nhận trạng thái khi 3 lần đọc liên tiếp giống nhau
 *
 * THỜI GIAN:
 * - Hàm getKeyInput() phải được gọi mỗi 1ms trong timer interrupt
 * - Long press = 500 lần gọi = 500ms
 * ================================================================== */

// ==================================================================
// KHAI BÁO BIẾN TOÀN CỤC
// ==================================================================

// Các thanh ghi lưu trạng thái nút qua các lần đọc (cho 3 nút)
int KeyReg0[3] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE}; // Giá trị đọc hiện tại
int KeyReg1[3] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE}; // Giá trị đọc lần trước
int KeyReg2[3] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE}; // Giá trị đọc 2 lần trước
int KeyReg3[3] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE}; // Trạng thái ổn định (sau debounce)

// Bộ đếm thời gian cho long press (đơn vị: số lần gọi hàm)
// 500 lần gọi x 1ms = 500ms
int TimeOutForKeyPress[3] = {500, 500, 500};

// Cờ báo hiệu nút được nhấn (short press)
int button_flag[3] = {0, 0, 0};

// Cờ báo hiệu nút được nhấn giữ (long press)
int button_long_pressed[3] = {0, 0, 0};

/* ==================================================================
 * HÀM KIỂM TRA TRẠNG THÁI NÚT (GỌI TRONG MAIN)
 * ================================================================== */

/**
 * @brief Kiểm tra nút 1 có được nhấn không (short press)
 * @return 1 nếu nút vừa được nhấn, 0 nếu không
 * @note Hàm này chỉ trả về 1 MỘT LẦN cho mỗi lần nhấn (one-shot)
 *       Sau khi trả về 1, cờ sẽ tự động reset về 0
 */
int isButton1Pressed()
{
  if (button_flag[0] == 1) // Kiểm tra cờ nhấn
  {
    button_flag[0] = 0; // Xóa cờ sau khi đọc
    return 1;           // Trả về 1 lần duy nhất
  }
  return 0;
}

int isButton2Pressed()
{
  if (button_flag[1] == 1)
  {
    button_flag[1] = 0;
    return 1;
  }
  return 0;
}

int isButton3Pressed()
{
  if (button_flag[2] == 1)
  {
    button_flag[2] = 0;
    return 1;
  }
  return 0;
}

/**
 * @brief Kiểm tra nút 1 có được nhấn giữ không (long press)
 * @return 1 nếu nút được nhấn giữ > 500ms, 0 nếu không
 * @note Hàm này cũng chỉ trả về 1 MỘT LẦN cho mỗi sự kiện long press
 *       Nếu tiếp tục giữ, sự kiện sẽ lặp lại mỗi 500ms
 */
int isButton1LongPressed()
{
  if (button_long_pressed[0] == 1)
  {
    button_long_pressed[0] = 0;
    return 1;
  }
  return 0;
}

int isButton2LongPressed()
{
  if (button_long_pressed[1] == 1)
  {
    button_long_pressed[1] = 0;
    return 1;
  }
  return 0;
}

int isButton3LongPressed()
{
  if (button_long_pressed[2] == 1)
  {
    button_long_pressed[2] = 0;
    return 1;
  }
  return 0;
}

/* ==================================================================
 * XỬ LÝ SỰ KIỆN NÚT BẤM
 * ================================================================== */

/**
 * @brief Hàm xử lý khi nút được nhấn (được gọi từ getKeyInput)
 * @param index Chỉ số nút (0, 1, hoặc 2)
 * @note Hàm này đặt cờ button_flag khi phát hiện nhấn
 */
void subKeyProcess(int index)
{
  // Đặt cờ báo nút được nhấn
  button_flag[index] = 1;

  // TODO: Có thể thêm xử lý khác tại đây
  // Ví dụ: Bật LED, phát âm thanh, gửi tín hiệu, v.v.
}

/* ==================================================================
 * HÀM CHÍNH - ĐỌC VÀ XỬ LÝ NÚT BẤM (GỌI TRONG TIMER INTERRUPT)
 * ================================================================== */

/**
 * @brief Hàm đọc và xử lý trạng thái nút bấm
 * @note HÀM NÀY PHẢI ĐƯỢC GỌI ĐỊNH KỲ MỖI 1ms TRONG TIMER INTERRUPT
 *
 * Quy trình xử lý:
 * 1. Dịch chuyển các giá trị cũ: Reg2 ← Reg1 ← Reg0
 * 2. Đọc giá trị mới từ GPIO → Reg0
 * 3. Kiểm tra debouncing: Reg0 == Reg1 == Reg2?
 * 4. Nếu ổn định và có thay đổi → Xử lý sự kiện nhấn
 * 5. Nếu giữ nút → Đếm thời gian và phát hiện long press
 */
void getKeyInput()
{
  for (int i = 0; i < 3; i++) // Xử lý lần lượt 3 nút
  {
    // ===== BƯỚC 1: DỊCH CHUYỂN GIÁ TRỊ CŨ =====
    KeyReg2[i] = KeyReg1[i]; // Lưu giá trị 2 lần trước
    KeyReg1[i] = KeyReg0[i]; // Lưu giá trị lần trước

    // ===== BƯỚC 2: ĐỌC GIÁ TRỊ MỚI TỪ GPIO =====
    switch (i)
    {
    case 0: // Nút 1
      KeyReg0[i] = HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin);
      break;
    case 1: // Nút 2
      KeyReg0[i] = HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin);
      break;
    case 2: // Nút 3
      KeyReg0[i] = HAL_GPIO_ReadPin(BUTTON3_GPIO_Port, BUTTON3_Pin);
      break;
    }

    // ===== BƯỚC 3: DEBOUNCING - KIỂM TRA 3 LẦN ĐỌC GIỐNG NHAU =====
    if ((KeyReg1[i] == KeyReg0[i]) && (KeyReg1[i] == KeyReg2[i]))
    {
      // Trạng thái ổn định (không có nhiễu)

      // ===== BƯỚC 4: PHÁT HIỆN THAY ĐỔI TRẠNG THÁI =====
      if (KeyReg3[i] != KeyReg2[i])
      {
        // Trạng thái mới khác trạng thái cũ → Có sự kiện
        KeyReg3[i] = KeyReg2[i]; // Cập nhật trạng thái ổn định

        if (KeyReg3[i] == PRESSED_STATE) // Nút được nhấn xuống
        {
          subKeyProcess(i);            // Gọi xử lý sự kiện nhấn
          TimeOutForKeyPress[i] = 500; // Reset bộ đếm long press
        }
      }
      else
      {
        // ===== BƯỚC 5: XỬ LÝ NHẤN GIỮ (LONG PRESS) =====
        // Trạng thái không đổi → Đếm thời gian
        TimeOutForKeyPress[i]--; // Giảm bộ đếm

        if (TimeOutForKeyPress[i] == 0) // Hết thời gian đếm
        {
          TimeOutForKeyPress[i] = 500; // Reset để đếm lại

          if (KeyReg3[i] == PRESSED_STATE) // Vẫn đang giữ nút
          {
            button_long_pressed[i] = 1; // Đặt cờ long press
                                        // Nếu tiếp tục giữ, sự kiện sẽ lặp lại mỗi 500ms
          }
        }
      }
    }
    // Nếu 3 lần đọc không giống nhau → Bỏ qua (chờ ổn định)
  }
}
