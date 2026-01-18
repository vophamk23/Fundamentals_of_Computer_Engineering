/*
 * scheduler.h
 * Cooperative Scheduler cho ESP32 với Hardware Timer
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

/* ==================== CONFIGURATION ==================== */
#define SCH_MAX_TASKS 20 // Giảm xuống 20 cho ESP32
#define NO_TASK_ID 0
#define TIMER_TICK_MS 10 // 10ms timer tick

/* ==================== ERROR CODES ==================== */
#define ERROR_SCH_TOO_MANY_TASKS 1
#define ERROR_SCH_CANNOT_DELETE_TASK 2

/* ==================== RETURN CODES ==================== */
#define RETURN_ERROR 0
#define RETURN_NORMAL 1

/* ==================== TASK STRUCTURE ==================== */
typedef struct
{
    void (*pTask)(void); // Con trỏ tới hàm task
    uint32_t Delay;      // Delay (ticks) cho đến khi chạy
    uint32_t Period;     // Khoảng thời gian (ticks) giữa các lần chạy
    uint8_t RunMe;       // Cờ: tăng lên khi task cần chạy
    uint32_t TaskID;     // ID của task
} sTask;

/* ==================== GLOBAL VARIABLES ==================== */
extern sTask SCH_tasks_G[SCH_MAX_TASKS];
extern uint8_t Error_code_G;
extern uint8_t MARKING[SCH_MAX_TASKS];
extern uint32_t task_count;
extern uint32_t elapsed_time;

/* ==================== FUNCTION PROTOTYPES ==================== */

/**
 * @brief Khởi tạo scheduler
 * Đặt tất cả task về trạng thái ban đầu
 */
void SCH_Init(void);

/**
 * @brief Khởi tạo Timer Interrupt cho ESP32
 * Tạo timer với chu kỳ TIMER_TICK_MS
 */
void SCH_Init_Timer(void);

/**
 * @brief Hàm cập nhật - được gọi từ Timer ISR mỗi TIMER_TICK_MS
 * Độ phức tạp: O(1) - CHỈ cập nhật task đầu tiên!
 */
void SCH_Update(void);

/**
 * @brief Thực thi các task đã sẵn sàng
 * Nên được gọi trong main loop
 * Độ phức tạp: O(n²) trong trường hợp xấu nhất
 */
void SCH_Dispatch_Tasks(void);

/**
 * @brief Thêm task vào scheduler
 * @param pFunction: Con trỏ tới hàm task
 * @param DELAY: Delay ban đầu (ticks)
 * @param PERIOD: Chu kỳ lặp lại (0 cho one-shot)
 * @return Index của task trong mảng, hoặc SCH_MAX_TASKS nếu thất bại
 *
 * VD: SCH_Add_Task(Task_LED1, 0, 50); // Chạy mỗi 500ms
 */
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD);

/**
 * @brief Xóa task khỏi scheduler
 * @param TASK_INDEX: Index của task cần xóa
 * @return RETURN_NORMAL hoặc RETURN_ERROR
 */
uint8_t SCH_Delete_Task(const uint32_t TASK_INDEX);

/**
 * @brief Lấy số lượng task hiện tại
 */
uint32_t SCH_Get_Current_Size(void);

#endif // SCHEDULER_H