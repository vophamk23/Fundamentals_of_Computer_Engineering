/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - With LED Matrix Shifting Effect
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "software_timer.h" // Thư viện timer phần mềm tự tạo

/* Khai báo hàm ------------------------------------------------------------------*/
void SystemClock_Config(void);  // Cấu hình xung clock hệ thống
static void MX_GPIO_Init(void); // Khởi tạo các chân GPIO
static void MX_TIM2_Init(void); // Khởi tạo Timer 2

TIM_HandleTypeDef htim2; // Biến xử lý Timer 2

/* ========================================================================== */
/* ===== PHẦN 1: BIẾN TOÀN CỤC CHO ĐỒNG HỒ LED 7 ĐOẠN ===== */
/* ========================================================================== */
const int MAX_LED = 4;                  // Số lượng LED 7 đoạn (4 LED)
int index_led = 0;                      // Chỉ số LED đang được quét (0-3)
int hour = 15, minute = 8, second = 50; // Thời gian ban đầu: 15:08:50
int led_buffer[4] = {0, 0, 0, 0};       // Mảng lưu giá trị hiển thị cho 4 LED 7 đoạn

/* ========================================================================== */
/* ===== PHẦN 2: BIẾN TOÀN CỤC CHO LED MATRIX 8x8 VỚI HIỆU ỨNG CHẠY ===== */
/* ========================================================================== */
const int MAX_LED_MATRIX = 8; // Số lượng hàng của LED Matrix (8 hàng)
int index_led_matrix = 0;     // Chỉ số hàng đang được quét (0-7)
int shiftVar = 0;             // Biến điều khiển độ dịch chuyển (0-8, chu kỳ 9)

/**
 * Mảng chứa mẫu hiển thị chữ "A" trên LED Matrix
 * - Mỗi phần tử là 1 hàng của LED Matrix (8 hàng x 8 cột)
 * - Sử dụng 16 bit nhưng chỉ dùng 8 bit cao (bit 8-15)
 * - Chu kỳ lặp: 9 bit (8 bit chữ A + 1 bit trống để tạo khoảng cách)
 *
 * Cấu trúc bit: 0001 1000 0000 0000
 *               ^^^^ ^^^^           <-- 8 bit này chứa hình dạng chữ A
 *                         ^^^^ ^^^^ <-- 8 bit này không dùng
 */
uint16_t matrix_buffer[8] = {
    0x1800, // 0001 1000 - Hàng 0: Đỉnh chữ A (2 LED giữa sáng)
    0x2400, // 0010 0100 - Hàng 1: Hai bên chữ A mở rộng
    0x4200, // 0100 0010 - Hàng 2: Hai bên chữ A rộng hơn
    0x4200, // 0100 0010 - Hàng 3: Hai bên chữ A (giống hàng 2)
    0x7E00, // 0111 1110 - Hàng 4: Thanh ngang chữ A (6 LED sáng)
    0x4200, // 0100 0010 - Hàng 5: Hai bên chữ A
    0x4200, // 0100 0010 - Hàng 6: Hai bên chữ A
    0x4200  // 0100 0010 - Hàng 7: Chân chữ A
};

/* ========================================================================== */
/* ===== PHẦN 3: HÀM XỬ LÝ LED MATRIX VỚI HIỆU ỨNG CHẠY ===== */
/* ========================================================================== */

/**
 * @brief  Chuyển đổi bit từ giá trị hexa sang trạng thái GPIO với hiệu ứng dịch chuyển
 * @param  hexa: Giá trị 16-bit chứa pattern của 1 hàng LED Matrix
 * @param  index: Vị trí cột LED cần lấy giá trị (0-7)
 * @retval GPIO_PIN_SET nếu bit = 1 (LED sáng), GPIO_PIN_RESET nếu bit = 0 (LED tắt)
 *
 * @note   Thuật toán:
 *         1. Tính vị trí thực tế của bit sau khi dịch chuyển theo chu kỳ 9
 *         2. Nếu vị trí = 8 (bit thứ 9): trả về 0 (cột trống)
 *         3. Nếu vị trí < 8: lấy bit từ pattern chữ A
 *
 * @example
 *         - Khi shiftVar = 0: Hiển thị bit 0-7 của chữ A (chữ A đầy đủ)
 *         - Khi shiftVar = 1: Hiển thị bit 1-7 của chữ A + 1 cột trống
 *         - Khi shiftVar = 8: Hiển thị 7 cột trống + bit 0 của chữ A mới
 */
GPIO_PinState convertToBit(uint16_t hexa, int index)
{
  // BƯỚC 1: Tính vị trí bit thực tế sau khi dịch chuyển
  // Công thức: (vị trí hiện tại + độ dịch) % chu kỳ
  // Chu kỳ = 9 (8 bit chữ A + 1 bit trống)
  int actualIndex = (index + shiftVar) % 9;

  int bitValue; // Giá trị bit cần lấy (0 hoặc 1)

  // BƯỚC 2: Kiểm tra xem đây có phải cột trống không
  if (actualIndex == 8)
  {
    // Bit thứ 8 trong chu kỳ luôn là 0 (tạo khoảng cách giữa các chữ A)
    bitValue = 0;
  }
  else
  {
    // BƯỚC 3: Lấy bit từ pattern chữ A
    // Dịch phải (15 - actualIndex) vị trí để đưa bit cần lấy về vị trí cuối cùng
    // Sau đó AND với 0x01 để chỉ lấy bit cuối cùng
    //
    // Ví dụ: hexa = 0x1800 = 0001 1000 0000 0000
    //        actualIndex = 0 → dịch phải 15 vị trí → bit cuối = 0
    //        actualIndex = 3 → dịch phải 12 vị trí → bit cuối = 1
    bitValue = (hexa >> (15 - actualIndex)) & 0x01;
  }

  // BƯỚC 4: Chuyển đổi sang trạng thái GPIO
  return (bitValue == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

/**
 * @brief  Cài đặt dữ liệu cho 1 hàng của LED Matrix
 * @param  index: Chỉ số hàng cần cài đặt (0-7)
 * @retval None
 *
 * @note   Hàm này sẽ:
 *         1. Lấy dữ liệu từ matrix_buffer[index]
 *         2. Chuyển đổi từng bit sang trạng thái GPIO (có tính toán shift)
 *         3. Xuất ra 8 chân ENM0-ENM7 để điều khiển 8 cột LED
 *
 * @example
 *         updateLEDMatrix(0) → Cài đặt dữ liệu cho hàng 0 (hàng trên cùng)
 */
void updateLEDMatrix(int index)
{
  switch (index)
  {
  case 0: // Cài đặt hàng 0 (hàng trên cùng của LED Matrix)
    // Lấy 8 bit từ matrix_buffer[0] và chuyển đổi sang trạng thái GPIO
    // Mỗi bit điều khiển 1 cột LED (ENM0 = cột 0, ENM1 = cột 1, ...)
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[0], 0)); // Cột 0
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[0], 1)); // Cột 1
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[0], 2)); // Cột 2
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[0], 3)); // Cột 3
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[0], 4)); // Cột 4
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[0], 5)); // Cột 5
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[0], 6)); // Cột 6
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[0], 7)); // Cột 7
    break;

  case 1: // Cài đặt hàng 1
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[1], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[1], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[1], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[1], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[1], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[1], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[1], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[1], 7));
    break;

  case 2: // Cài đặt hàng 2
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[2], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[2], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[2], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[2], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[2], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[2], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[2], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[2], 7));
    break;

  case 3: // Cài đặt hàng 3
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[3], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[3], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[3], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[3], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[3], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[3], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[3], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[3], 7));
    break;

  case 4: // Cài đặt hàng 4 (hàng có thanh ngang của chữ A)
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[4], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[4], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[4], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[4], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[4], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[4], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[4], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[4], 7));
    break;

  case 5: // Cài đặt hàng 5
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[5], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[5], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[5], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[5], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[5], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[5], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[5], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[5], 7));
    break;

  case 6: // Cài đặt hàng 6
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[6], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[6], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[6], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[6], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[6], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[6], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[6], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[6], 7));
    break;

  case 7: // Cài đặt hàng 7 (hàng dưới cùng của LED Matrix)
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin, convertToBit(matrix_buffer[7], 0));
    HAL_GPIO_WritePin(GPIOA, ENM1_Pin, convertToBit(matrix_buffer[7], 1));
    HAL_GPIO_WritePin(GPIOA, ENM2_Pin, convertToBit(matrix_buffer[7], 2));
    HAL_GPIO_WritePin(GPIOA, ENM3_Pin, convertToBit(matrix_buffer[7], 3));
    HAL_GPIO_WritePin(GPIOA, ENM4_Pin, convertToBit(matrix_buffer[7], 4));
    HAL_GPIO_WritePin(GPIOA, ENM5_Pin, convertToBit(matrix_buffer[7], 5));
    HAL_GPIO_WritePin(GPIOA, ENM6_Pin, convertToBit(matrix_buffer[7], 6));
    HAL_GPIO_WritePin(GPIOA, ENM7_Pin, convertToBit(matrix_buffer[7], 7));
    break;

  default:
    break;
  }
}

/**
 * @brief  Quét LED Matrix với hiệu ứng dịch chuyển
 * @param  None
 * @retval None
 *
 * @note   Nguyên lý hoạt động:
 *         - LED Matrix sử dụng kỹ thuật quét (multiplexing)
 *         - Mỗi lần chỉ bật 1 hàng, sau đó nhanh chóng chuyển sang hàng khác
 *         - Nhờ tính trễ của mắt người (POV), ta thấy cả 8 hàng sáng cùng lúc
 *         - Sau khi quét hết 8 hàng, tăng biến shiftVar để tạo hiệu ứng chạy
 *
 * @example Chu trình quét:
 *         Lần gọi 1: Bật hàng 0, tắt hàng 7
 *         Lần gọi 2: Bật hàng 1, tắt hàng 0
 *         ...
 *         Lần gọi 8: Bật hàng 7, tắt hàng 6
 *         Lần gọi 9: Bật hàng 0, tắt hàng 7, shiftVar++
 */
void scanLEDMatrix()
{
  // ===== BƯỚC 1: CHỌN HÀNG SẼ ĐƯỢC BẬT =====
  // Chỉ 1 hàng được bật tại 1 thời điểm
  // Hàng trước đó phải tắt trước khi bật hàng mới
  switch (index_led_matrix)
  {
  case 0:                                               // Bật hàng 0 (hàng trên cùng)
    HAL_GPIO_WritePin(GPIOB, ROW7_Pin, GPIO_PIN_RESET); // Tắt hàng 7 (hàng trước đó)
    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_SET);   // Bật hàng 0
    break;

  case 1:                                               // Bật hàng 1
    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_RESET); // Tắt hàng 0
    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_SET);   // Bật hàng 1
    break;

  case 2:                                               // Bật hàng 2
    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_RESET); // Tắt hàng 1
    HAL_GPIO_WritePin(GPIOB, ROW2_Pin, GPIO_PIN_SET);   // Bật hàng 2
    break;

  case 3:                                               // Bật hàng 3
    HAL_GPIO_WritePin(GPIOB, ROW2_Pin, GPIO_PIN_RESET); // Tắt hàng 2
    HAL_GPIO_WritePin(GPIOB, ROW3_Pin, GPIO_PIN_SET);   // Bật hàng 3
    break;

  case 4:                                               // Bật hàng 4 (hàng giữa)
    HAL_GPIO_WritePin(GPIOB, ROW3_Pin, GPIO_PIN_RESET); // Tắt hàng 3
    HAL_GPIO_WritePin(GPIOB, ROW4_Pin, GPIO_PIN_SET);   // Bật hàng 4
    break;

  case 5:                                               // Bật hàng 5
    HAL_GPIO_WritePin(GPIOB, ROW4_Pin, GPIO_PIN_RESET); // Tắt hàng 4
    HAL_GPIO_WritePin(GPIOB, ROW5_Pin, GPIO_PIN_SET);   // Bật hàng 5
    break;

  case 6:                                               // Bật hàng 6
    HAL_GPIO_WritePin(GPIOB, ROW5_Pin, GPIO_PIN_RESET); // Tắt hàng 5
    HAL_GPIO_WritePin(GPIOB, ROW6_Pin, GPIO_PIN_SET);   // Bật hàng 6
    break;

  case 7:                                               // Bật hàng 7 (hàng dưới cùng)
    HAL_GPIO_WritePin(GPIOB, ROW6_Pin, GPIO_PIN_RESET); // Tắt hàng 6
    HAL_GPIO_WritePin(GPIOB, ROW7_Pin, GPIO_PIN_SET);   // Bật hàng 7
    break;

  default:
    break;
  }

  // ===== BƯỚC 2: CÀI ĐẶT DỮ LIỆU CHO HÀNG ĐÃ CHỌN =====
  // Xuất dữ liệu ra 8 cột (ENM0-ENM7) cho hàng đang được bật
  updateLEDMatrix(index_led_matrix);

  // ===== BƯỚC 3: CHUYỂN SANG HÀNG TIẾP THEO =====
  index_led_matrix++;

  // Kiểm tra nếu đã quét hết 8 hàng
  if (index_led_matrix >= MAX_LED_MATRIX)
  {
    index_led_matrix = 0; // Quay lại hàng 0

    // Tăng biến shift để tạo hiệu ứng chữ A chạy từ phải sang trái
    // Chu kỳ 9: 0→1→2→...→8→0 (8 vị trí chữ A + 1 vị trí trống)
    shiftVar = (shiftVar + 1) % 9;
  }
}

/* ========================================================================== */
/* ===== PHẦN 4: CÁC HÀM XỬ LÝ ĐỒNG HỒ LED 7 ĐOẠN ===== */
/* ========================================================================== */
/**
 * ===== HÀM CÀI ĐẶT BUFFER HIỂN THỊ ĐỒNG HỒ =====
 * Mục đích: Chuyển đổi thời gian (hour:minute) thành 4 số riêng biệt để hiển thị
 * Ví dụ: 15:08 → led_buffer[0]=1, led_buffer[1]=5, led_buffer[2]=0, led_buffer[3]=8
 */
void updateClockBuffer(void)
{
  led_buffer[0] = hour / 10;   // Chữ số hàng chục của giờ (1 từ 15:xx)
  led_buffer[1] = hour % 10;   // Chữ số hàng đơn vị của giờ (5 từ 15:xx)
  led_buffer[2] = minute / 10; // Chữ số hàng chục của phút (0 từ xx:08)
  led_buffer[3] = minute % 10; // Chữ số hàng đơn vị của phút (8 từ xx:08)
}

/**
 * ===== HÀM HIỂN THỊ 1 SỐ TRÊN LED 7 ĐOẠN =====
 * Mục đích: Điều khiển 7 đoạn LED để hiển thị số từ 0-9
 * Input: num = số cần hiển thị (0-9)
 * Lưu ý: Logic nghịch đảo - RESET = bật LED, SET = tắt LED
 */
void display7SEG(int num)
{
  // Tắt tất cả 7 đoạn LED trước khi bật các đoạn cần thiết
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_SET);

  switch (num)
  {
  case 0: // Hiển thị số 0: bật 6 đoạn (a,b,c,d,e,f)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin, GPIO_PIN_RESET);
    break;
  case 1: // Hiển thị số 1: chỉ bật 2 đoạn (b,c)
    HAL_GPIO_WritePin(GPIOB, SEG1_Pin | SEG2_Pin, GPIO_PIN_RESET);
    break;
  case 2: // Hiển thị số 2: bật 5 đoạn (a,b,g,e,d)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG3_Pin | SEG4_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 3: // Hiển thị số 3: bật 5 đoạn (a,b,g,c,d)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 4: // Hiển thị số 4: bật 4 đoạn (f,g,b,c)
    HAL_GPIO_WritePin(GPIOB, SEG1_Pin | SEG2_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 5: // Hiển thị số 5: bật 5 đoạn (a,f,g,c,d)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG2_Pin | SEG3_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 6: // Hiển thị số 6: bật 6 đoạn (a,f,g,e,d,c)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 7: // Hiển thị số 7: bật 3 đoạn (a,b,c)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin, GPIO_PIN_RESET);
    break;
  case 8: // Hiển thị số 8: bật tất cả 7 đoạn
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 9: // Hiển thị số 9: bật 6 đoạn (a,b,c,d,f,g)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  default: // Số không hợp lệ: chỉ hiển thị dấu gạch ngang
    HAL_GPIO_WritePin(GPIOB, SEG6_Pin, GPIO_PIN_RESET);
    break;
  }
}

/**
 * ===== HÀM BẬT 1 LED 7 ĐOẠN CỤ THỂ =====
 * Mục đích: Chọn LED 7-đoạn nào sẽ được bật (trong 4 LED)
 * Input: index = LED cần bật (0, 1, 2, 3)
 * Hoạt động: Tắt tất cả LED, chỉ bật LED được chọn
 */
void enableLED(int index)
{
  // Tắt tất cả 4 LED 7-đoạn trước
  HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);

  // Chỉ bật LED được chọn
  switch (index)
  {
  case 0: // Bật LED đầu tiên (hàng chục giờ)
    HAL_GPIO_WritePin(GPIOA, EN0_Pin, GPIO_PIN_RESET);
    break;
  case 1: // Bật LED thứ hai (hàng đơn vị giờ)
    HAL_GPIO_WritePin(GPIOA, EN1_Pin, GPIO_PIN_RESET);
    break;
  case 2: // Bật LED thứ ba (hàng chục phút)
    HAL_GPIO_WritePin(GPIOA, EN2_Pin, GPIO_PIN_RESET);
    break;
  case 3: // Bật LED thứ tư (hàng đơn vị phút)
    HAL_GPIO_WritePin(GPIOA, EN3_Pin, GPIO_PIN_RESET);
    break;
  default:
    break;
  }
}

/**
 * ===== HÀM CÀI ĐẶT HIỂN THỊ CHO 1 LED 7 ĐOẠN =====
 * Mục đích: Bật LED cụ thể và hiển thị số tương ứng từ buffer
 */
void update7SEG(int index)
{
  switch (index)
  {
  case 0:
    enableLED(0);               // Bật LED đầu tiên
    display7SEG(led_buffer[0]); // Hiển thị số từ buffer[0]
    break;
  case 1:
    enableLED(1);               // Bật LED thứ hai
    display7SEG(led_buffer[1]); // Hiển thị số từ buffer[1]
    break;
  case 2:
    enableLED(2);               // Bật LED thứ ba
    display7SEG(led_buffer[2]); // Hiển thị số từ buffer[2]
    break;
  case 3:
    enableLED(3);               // Bật LED thứ tư
    display7SEG(led_buffer[3]); // Hiển thị số từ buffer[3]
    break;
  default:
    // Tắt tất cả LED nếu index không hợp lệ
    HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);
    break;
  }
}

/**
 * ===== HÀM QUÉT LED 7 ĐOẠN (MULTIPLEXING) =====
 * Mục đích: Quét qua 4 LED 7-đoạn để hiển thị đồng hồ HH:MM
 * Nguyên lý: Giống như LED Matrix, chỉ bật 1 LED tại 1 thời điểm
 *           Quét nhanh (25ms/LED) để tạo cảm giác tất cả LED đều sáng
 */
void scanLED()
{
  update7SEG(index_led); // Cập nhật LED hiện tại

  // Chuyển sang LED tiếp theo
  index_led++;
  if (index_led >= MAX_LED)
  {
    index_led = 0; // Quay lại LED đầu tiên
  }
}

/**
 * ===== HÀM XỬ LÝ NGẮT TIMER =====
 * Mục đích: Được gọi mỗi 10ms để chạy các timer phần mềm
 * Chức năng: Cập nhật trạng thái timer và quét LED Matrix
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    timerRun();
    // Quét LED 7-đoạn
    if (isTimerExpired(0))
    {
      setTimer(0, 25);
      scanLED();
    }
    // Nhấp nháy LED chấm
    if (isTimerExpired(1))
    {
      setTimer(1, 75);
      HAL_GPIO_TogglePin(GPIOA, DOT_Pin);
    }

    // Nhấp nháy LED đỏ
    if (isTimerExpired(2))
    {
      setTimer(2, 50);
      HAL_GPIO_TogglePin(GPIOA, LED_RED_Pin);
    }
    // Cập nhật thời gian
    if (isTimerExpired(3))
    {
      second++;
      if (second >= 60)
      {
        second = 0;
        minute++;
      }
      if (minute >= 60)
      {
        minute = 0;
        hour++;
      }
      if (hour >= 24)
      {
        hour = 0;
      }
      updateClockBuffer();
      setTimer(3, 100);
    }

    // Timer 4: Quét LED Matrix mỗi 10ms (1 × 10ms)
    if (isTimerExpired(4))
    {
      setTimer(4, 10);
      scanLEDMatrix();
    }
  }
}

/**
 * @brief Chương trình chính
 */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM2_Init();

  /* ===== KHỞI TẠO CÁC TIMER PHẦN MỀM ===== */
  // Timer 0: Quét LED 7-đoạn mỗi 250ms (25 × 10ms)
  setTimer(0, 25);
  // Timer 1: Nhấp nháy LED hai chấm mỗi 750ms (75 × 10ms)
  setTimer(1, 75);
  // Timer 2: Nhấp nháy LED đỏ mỗi 500ms (50 × 10ms)
  setTimer(2, 50);
  // Timer 3: Cập nhật đồng hồ mỗi 1 giây (100 × 10ms)
  setTimer(3, 100);
  // Timer 4: Quét LED Matrix mỗi 100ms (10 × 10ms)
  setTimer(4, 10);
  /* Cập nhật buffer hiển thị đồng hồ ban đầu */
  updateClockBuffer();

  HAL_TIM_Base_Start_IT(&htim2);

  while (1)
  {
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, ENM0_Pin | ENM1_Pin | DOT_Pin | LED_RED_Pin | EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin | ENM2_Pin | ENM3_Pin | ENM4_Pin | ENM5_Pin | ENM6_Pin | ENM7_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | ROW2_Pin | ROW3_Pin | ROW4_Pin | ROW5_Pin | ROW6_Pin | ROW7_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin | ROW0_Pin | ROW1_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = ENM0_Pin | ENM1_Pin | DOT_Pin | LED_RED_Pin | EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin | ENM2_Pin | ENM3_Pin | ENM4_Pin | ENM5_Pin | ENM6_Pin | ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG0_Pin | SEG1_Pin | SEG2_Pin | ROW2_Pin | ROW3_Pin | ROW4_Pin | ROW5_Pin | ROW6_Pin | ROW7_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin | ROW0_Pin | ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
