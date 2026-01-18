/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - Hệ thống quản lý nút bấm với STM32
 *
 * TỔNG QUAN HỆ THỐNG:
 * - Sử dụng Timer2 để tạo ngắt định kỳ 10ms
 * - Trong ngắt timer gọi hàm getKeyInput() để quét nút bấm
 * - Xử lý debouncing và phát hiện long press
 * - Điều khiển LED dựa trên trạng thái nút bấm
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h" // Include thư viện quản lý nút bấm
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;   // Handle cho Timer 2 - tạo ngắt định kỳ
UART_HandleTypeDef huart2; // Handle cho UART2 - có thể dùng cho debug

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void); // Khởi tạo Timer 2

/**
 * HÀM CALLBACK TIMER - ĐƯỢC GỌI KHI TIMER2 OVERFLOW
 * Hàm này được gọi mỗi 10ms
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    timer_isr(); // Gọi hàm quét nút bấm
  }
}
/**
 * Hàm ISR (Interrupt Service Routine) được gọi từ timer callback
 * Nhiệm vụ: Gọi hàm getKeyInput() để quét trạng thái nút bấm
 */
void timer_isr()
{
  getKeyInput(); // Quét nút bấm mỗi 10ms
}
/**
 * HÀM MAIN - ĐIỂM VÀO CHƯƠNG TRÌNH
 */
int main(void)
{
  /* ==================================================================
   * KHỞI TẠO HỆ THỐNG
   * ================================================================== */

  // Khởi tạo HAL và cấu hình hệ thống cơ bản
  HAL_Init();
  SystemClock_Config();

  // Khởi tạo các peripheral
  MX_GPIO_Init();        // Khởi tạo GPIO (nút bấm và LED)
  MX_USART2_UART_Init(); // Khởi tạo UART
  MX_TIM2_Init();        // Khởi tạo Timer2

  getKeyInput(); // Quét nút bấm mỗi 10ms

  /* ==================================================================
   * VÒNG LẶP CHÍNH
   * ================================================================== */
  while (1)
  {
    /* ------ XỬ LÝ NÚT BẤM THÔNG THƯỜNG ------ */
    if (isButton1Pressed())
    { // Kiểm tra nút vừa được nhấn
      // Toggle LED khi nút được nhấn
      HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    }

    /* ------ XỬ LÝ NÚT BẤM DÀI ------ */
    if (isButton1LongPressed())
    { // Kiểm tra nhấn dài
      // Toggle LED khi nút được nhấn dài
      HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    }
  }
}

/**
 * CẤU HÌNH SYSTEM CLOCK
 * Sử dụng HSI (High Speed Internal) 8MHz làm nguồn clock chính
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // Cấu hình HSI 8MHz
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // Không sử dụng PLL

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  // Cấu hình các bus clock
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; // SYSCLK = HSI = 8MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     // AHB = 8MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      // APB1 = 8MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;      // APB2 = 8MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * CẤU HÌNH TIMER2 - TẠO NGẮT ĐỊNH KỲ 10ms
 *
 * TÍNH TOÁN:
 * - System Clock = 8MHz
 * - Prescaler = 800 - 1 = 799
 * - Timer Clock = 8,000,000 / 800 = 10,000 Hz
 * - Period = 100 - 1 = 99
 * - Tần số ngắt = 10,000 / 100 = 100 Hz
 * - Chu kỳ ngắt = 1/100 = 10ms
 */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 800 - 1;              // Chia tần số từ 8MHz xuống 10kHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP; // Đếm lên
  htim2.Init.Period = 100 - 1;                 // Đếm từ 0 đến 99 (100 lần đếm)
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  // Cấu hình nguồn clock nội bộ
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  // Cấu hình Master mode (không sử dụng)
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * CẤU HÌNH UART2 - DÙNG CHO DEBUG (115200 baud)
 */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * CẤU HÌNH GPIO - NÚT BẤM VÀ LED
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Bật clock cho các port GPIO
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Thiết lập trạng thái ban đầu cho các pin output
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);

  /* ------ CẤU HÌNH NÚT BẤM ------ */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Chế độ input
  GPIO_InitStruct.Pull = GPIO_PULLUP;     // Kéo lên nội bộ
  // Với pullup: nút không nhấn = HIGH, nút nhấn = LOW
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* ------ CẤU HÌNH LED ------ */
  GPIO_InitStruct.Pin = GREEN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Push-pull output
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GREEN_LED_GPIO_Port, &GPIO_InitStruct);

  /* ------ CẤU HÌNH PIN KHÁC ------ */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * XỬ LÝ LỖI HỆ THỐNG
 */
void Error_Handler(void)
{
  __disable_irq(); // Tắt tất cả ngắt
  while (1)        // Vòng lặp vô hạn
  {
    // Có thể thêm code debug ở đây
  }
}

/* ==================================================================
 * LUỒNG HOẠT ĐỘNG CỦA HỆ THỐNG:
 *
 * 1. KHỞI TẠO (main function):
 *    - Khởi tạo HAL, clock, GPIO, UART, Timer
 *    - Bắt đầu Timer2 với ngắt mỗi 10ms
 *    - Vào vòng lặp chính
 *
 * 2. NGẮT TIMER (mỗi 10ms):
 *    - HAL_TIM_PeriodElapsedCallback() được gọi
 *    - timer_isr() được thực thi
 *    - getKeyInput() được gọi để quét nút bấm
 *
 * 3. QUÉT NÚT BẤM (trong getKeyInput()):
 *    - Đọc trạng thái GPIO của nút bấm
 *    - Thực hiện debouncing (loại bỏ nhiễu)
 *    - Phát hiện sự kiện nhấn và nhấn dài
 *    - Cập nhật các cờ trạng thái
 *
 * 4. VÒNG LẶP CHÍNH:
 *    - Kiểm tra cờ nút bấm thông thường
 *    - Kiểm tra cờ nút bấm dài
 *    - Toggle LED tương ứng
 *
 * ĐẶC ĐIỂM KỸ THUẬT:
 * - Tần số quét: 100Hz (10ms)
 * - Debouncing: 3 lần đọc liên tiếp phải giống nhau
 * - Long press: giữ nút trong 500 × 10ms = 5 giây
 * - LED toggle mỗi khi có sự kiện nút bấm
 *
 * VẤN ĐỀ CẦN LƯU Ý:
 * - Cả short press và long press đều toggle LED giống nhau
 * - Logic long press chưa được implement đầy đủ
 * - Có thể cần phân biệt rõ hơn giữa các loại sự kiện
 * ================================================================== */