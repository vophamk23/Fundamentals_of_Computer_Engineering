//* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - Fixed for Proteus
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "software_timer.h"


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

TIM_HandleTypeDef htim2;

/* ========================================================================== */
/* ========================================================================== */
const int MAX_LED = 4;                  // Số lượng LED 7 đoạn (4 LED)
int index_led = 0;                      // Chỉ số LED đang được quét (0-3)
int hour = 15, minute = 8, second = 50; // Thời gian ban đầu: 15:08:50
int led_buffer[4] = {0, 0, 0, 0};       // Mảng lưu giá trị hiển thị cho 4 LED 7 đoạn

/* Định nghĩa hàm ------------------------------------------------------------*/

/**
 * @brief Cập nhật buffer đồng hồ với thời gian hiện tại
 */
void updateClockBuffer(void) {
    led_buffer[0] = hour / 10;      // Hàng chục của giờ (1 từ 15)
    led_buffer[1] = hour % 10;      // Hàng đơn vị của giờ (5 từ 15)
    led_buffer[2] = minute / 10;    // Hàng chục của phút (0 từ 08)
    led_buffer[3] = minute % 10;    // Hàng đơn vị của phút (8 từ 08)
    // Kết quả: hiển thị "1508" trên 4 LED 7-đoạn
}

/* USER CODE BEGIN PFP */
/**
 * @brief Hiển thị số trên LED 7-đoạn
 * @param num: Số cần hiển thị (0-9)
 */
void display7SEG(int num)
{
  // Tắt tất cả các đoạn LED trước (SET = tắt do logic nghịch đảo)
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_SET);

  switch (num)
  {
  case 0: // Hiển thị số 0 - bật 6 đoạn (a,b,c,d,e,f)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin, GPIO_PIN_RESET);
    break;
  case 1: // Hiển thị số 1 - chỉ bật 2 đoạn (b,c)
    HAL_GPIO_WritePin(GPIOB, SEG1_Pin | SEG2_Pin, GPIO_PIN_RESET);
    break;
  case 2: // Hiển thị số 2 - bật 5 đoạn (a,b,g,e,d)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG3_Pin | SEG4_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 3: // Hiển thị số 3 - bật 5 đoạn (a,b,c,d,g)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 4: // Hiển thị số 4 - bật 4 đoạn (f,g,b,c)
    HAL_GPIO_WritePin(GPIOB, SEG1_Pin | SEG2_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 5: // Hiển thị số 5 - bật 5 đoạn (a,f,g,c,d)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG2_Pin | SEG3_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 6: // Hiển thị số 6 - bật 6 đoạn (a,f,g,e,d,c)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 7: // Hiển thị số 7 - bật 3 đoạn (a,b,c)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin, GPIO_PIN_RESET);
    break;
  case 8: // Hiển thị số 8 - bật tất cả 7 đoạn
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  case 9: // Hiển thị số 9 - bật 6 đoạn (a,b,c,d,f,g)
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);
    break;
  default:
    // Hiển thị gạch ngang (đoạn g) cho số không hợp lệ
    HAL_GPIO_WritePin(GPIOB, SEG6_Pin, GPIO_PIN_RESET);
    break;
  }
}

/**
 * @brief Bật một LED 7-đoạn cụ thể
 * @param index: Chỉ số LED (0-3)
 */
void enableLED(int index)
{
  // Tắt tất cả LED trước (SET = tắt do transistor PNP)
  HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);

  switch (index)
  {
  case 0:
    HAL_GPIO_WritePin(GPIOA, EN0_Pin, GPIO_PIN_RESET); // Bật LED thứ 0 (hiển thị hàng chục giờ)
    break;
  case 1:
    HAL_GPIO_WritePin(GPIOA, EN1_Pin, GPIO_PIN_RESET); // Bật LED thứ 1 (hiển thị hàng đơn vị giờ)
    break;
  case 2:
    HAL_GPIO_WritePin(GPIOA, EN2_Pin, GPIO_PIN_RESET); // Bật LED thứ 2 (hiển thị hàng chục phút)
    break;
  case 3:
    HAL_GPIO_WritePin(GPIOA, EN3_Pin, GPIO_PIN_RESET); // Bật LED thứ 3 (hiển thị hàng đơn vị phút)
    break;
  default:
    break;
  }
}

/**
 * @brief Cập nhật hiển thị LED 7-đoạn tại vị trí cụ thể
 * @param index: Chỉ số LED (0-3)
 */
void update7SEG(int index)
{
  switch (index)
  {
  case 0:
    // Hiển thị LED đầu tiên với giá trị led_buffer[0] (hàng chục giờ)
    enableLED(0);
    display7SEG(led_buffer[0]);
    break;
  case 1:
    // Hiển thị LED thứ hai với giá trị led_buffer[1] (hàng đơn vị giờ)
    enableLED(1);
    display7SEG(led_buffer[1]);
    break;
  case 2:
    // Hiển thị LED thứ ba với giá trị led_buffer[2] (hàng chục phút)
    enableLED(2);
    display7SEG(led_buffer[2]);
    break;
  case 3:
    // Hiển thị LED thứ tư với giá trị led_buffer[3] (hàng đơn vị phút)
    enableLED(3);
    display7SEG(led_buffer[3]);
    break;
  default:
    // Tắt tất cả LED nếu chỉ số không hợp lệ
    HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);
    break;
  }
}

/**
 * @brief Quét tuần tự tất cả LED 7-đoạn (multiplexing)
 */
void scanLED()
{
  update7SEG(index_led);  // Cập nhật LED tại vị trí hiện tại

  // Chuyển sang LED tiếp theo
  index_led++;
  if (index_led >= MAX_LED)
  {
    index_led = 0;  // Quay lại LED đầu tiên
  }
}
/* USER CODE END PFP */

/**
 * @brief Hàm callback ngắt timer - PHIÊN BẢN SOFTWARE TIMER
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    // Chạy các timer phần mềm mỗi 10ms
    timerRun();
  }
}

/**
 * @brief Chương trình chính - ĐỒNG HỒ VỚI SOFTWARE TIMER
 */
int main(void)
{
  /* Cấu hình MCU */
  HAL_Init();             // Khởi tạo HAL
  SystemClock_Config();   // Cấu hình đồng hồ hệ thống

  /* Khởi tạo ngoại vi */
  MX_GPIO_Init();         // Khởi tạo GPIO
  MX_TIM2_Init();         // Khởi tạo Timer 2

  /* USER CODE BEGIN 2 */
  // Khởi tạo các timer phần mềm TRƯỚC KHI bật timer ngắt
  setTimer(0, 25);        // Timer 0: 25 × 10ms = 250ms cho quét LED
  setTimer(1, 75);       // Timer 1: 75 × 10ms = 750ms cho LED chấm
  setTimer(2, 50);        // Timer 2: 50 × 10ms = 500ms cho LED đỏ
  setTimer(3, 25);       // Timer 3: 100 × 10ms = 1000ms cho đồng hồ

  /* Cập nhật buffer đồng hồ ban đầu */
  updateClockBuffer();    // Chuyển đổi thời gian thành dữ liệu hiển thị

  // Bật ngắt timer SAU KHI đã khởi tạo
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Vòng lặp vô hạn - LOGIC CHÍNH VỚI SOFTWARE TIMER */
  while (1) {

	    // -------- Timer 0: Quét LED 7-đoạn mỗi 250ms (hiển thị mượt) --------
	    if(isTimerExpired(0))
	    {
	      setTimer(0, 25);    // 25 × 10ms = 250ms - quét nhanh để tránh nhấp nháy
	      scanLED();          // Quét qua tất cả LED 7-đoạn
	    }

	    // -------- Timer 1: Nhấp nháy LED chấm mỗi 1 giây (trong main loop) --------
		if (isTimerExpired(1))
		{
		  setTimer(1, 50);   // 50 × 10ms = 500ms = 0.5s
		  HAL_GPIO_TogglePin(GPIOA, DOT_Pin);  // Đảo trạng thái LED chấm
		}

		// -------- Timer 2: Nhấp nháy LED đỏ mỗi 500ms --------
		if (isTimerExpired(2))
		{
		setTimer(2, 25);    // 50 × 10ms = 500ms
		HAL_GPIO_TogglePin(GPIOA, LED_RED_Pin);  // Đảo trạng thái LED đỏ
		}

		// -------- Timer 3: Cập nhật đồng hồ mỗi 1 giây (trong main loop) --------
		if (isTimerExpired(3)){
		  // Tăng giây và xử lý tràn thời gian
		  second++;
		  if (second >= 60) {   // Khi giây = 60
			second = 0;         // Reset giây về 0
			minute++;           // Tăng phút
		  }
		  if (minute >= 60) {   // Khi phút = 60
			minute = 0;         // Reset phút về 0
			hour++;             // Tăng giờ
		  }
		  if (hour >= 24) {     // Khi giờ = 24
			hour = 0;           // Reset giờ về 0 (chuyển ngày mới)
		  }

		  updateClockBuffer();  // Cập nhật dữ liệu vào led_buffer
		  setTimer(3, 100);     // Reset timer 3 cho chu kỳ tiếp theo
		}
	  }
}

/**
 * @brief System Clock Configuration
 */
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

/**
 * @brief TIM2 Initialization Function
 */
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

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, DOT_Pin | LED_RED_Pin | EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = DOT_Pin | LED_RED_Pin | EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG0_Pin | SEG1_Pin | SEG2_Pin | SEG3_Pin | SEG4_Pin | SEG5_Pin | SEG6_Pin;
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
  /* User implementation */
}
#endif
