/*
 * scheduler.cpp
 * ESP32 Hardware Timer Implementation
 */

#include "scheduler.h"

/* ==================== BIẾN TOÀN CỤC ==================== */
sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t Error_code_G = 0;
uint8_t MARKING[SCH_MAX_TASKS];
uint32_t task_count = 0;
uint32_t elapsed_time = 0;

/* ==================== ESP32 TIMER ==================== */
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/* ==================== HÀM PRIVATE ==================== */
static void SCH_Update_Marking(void);

/*
 * ============================================================================
 * TIMER INTERRUPT SERVICE ROUTINE (ISR) - ESP32
 * ============================================================================
 * Mô tả: Timer interrupt tạo tick 10ms cho scheduler
 *
 * KIẾN TRÚC:
 *   ESP32 Clock (80MHz)
 *        ↓
 *   Prescaler (÷80)
 *        ↓
 *   Timer Clock (1MHz = 1μs per tick)
 *        ↓
 *   Alarm (10000 ticks = 10ms)
 *        ↓
 *   ISR Trigger → onTimer()
 *        ↓
 *   SCH_Update() - O(1) execution
 * ============================================================================
 */

/* ==================== TIMER ISR CALLBACK ==================== */
void IRAM_ATTR onTimer()
{
    // Tắt tất cả interrupt khác để đảm bảo atomic operation
    // timerMux: Mutex dùng riêng cho timer (tránh conflict với FreeRTOS)
    portENTER_CRITICAL_ISR(&timerMux);

    // ========================================================================
    // SCHEDULER UPDATE - CORE LOGIC
    // ========================================================================
    // Gọi hàm cập nhật scheduler - O(1) complexity!
    // CHỈ giảm delay của task đầu tiên (task sớm nhất)
    // Các task khác sẽ được cập nhật trong Dispatch (không phải ISR)
    SCH_Update(); // Gọi hàm cập nhật O(1)

    // Bật lại interrupt khác
    portEXIT_CRITICAL_ISR(&timerMux);
}

/* ==================== TIMER INITIALIZATION ==================== */
void SCH_Init_Timer(void)
{
    // Tạo timer với prescaler 80 (80MHz / 80 = 1MHz = 1μs per tick)
    timer = timerBegin(0, 80, true);

    // Gắn ISR callback
    timerAttachInterrupt(timer, &onTimer, true);

    // Cài đặt alarm: TIMER_TICK_MS * 1000 = microseconds
    // VD: 10ms = 10000μs
    timerAlarmWrite(timer, TIMER_TICK_MS * 1000, true);

    // Bật timer
    timerAlarmEnable(timer);

    Serial.println("Timer initialized: 10ms tick");
}

/* ==================== IMPLEMENTATION ==================== */
void SCH_Init(void)
{
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++)
    {
        SCH_tasks_G[i].pTask = 0x0000;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = 0;
        MARKING[i] = 0;
    }
    task_count = 0;
    elapsed_time = 0;
    Error_code_G = 0;
}

uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD)
{
    if (task_count == 0)
    {
        SCH_tasks_G[0].pTask = pFunction;
        SCH_tasks_G[0].Delay = DELAY;
        SCH_tasks_G[0].Period = PERIOD;
        SCH_tasks_G[0].RunMe = 0;
        SCH_tasks_G[0].TaskID = 0;
        MARKING[0] = 1;
        elapsed_time = 0;
        task_count++;
        return 0;
    }

    if (task_count >= SCH_MAX_TASKS)
    {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS;
    }

    uint32_t insert_index = 0;
    for (uint32_t i = 0; i < task_count; i++)
    {
        if (DELAY >= SCH_tasks_G[i].Delay)
        {
            insert_index = i + 1;
        }
        else
        {
            break;
        }
    }

    for (int32_t j = task_count; j > insert_index; j--)
    {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1];
    }

    SCH_tasks_G[insert_index].pTask = pFunction;
    SCH_tasks_G[insert_index].Delay = DELAY;
    SCH_tasks_G[insert_index].Period = PERIOD;
    SCH_tasks_G[insert_index].RunMe = 0;
    SCH_tasks_G[insert_index].TaskID = insert_index;

    task_count++;
    SCH_Update_Marking();

    return insert_index;
}

uint8_t SCH_Delete_Task(const uint32_t TASK_INDEX)
{
    if (TASK_INDEX >= task_count || task_count == 0)
    {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return RETURN_ERROR;
    }

    if (task_count == 1)
    {
        SCH_tasks_G[0].pTask = 0x0000;
        SCH_tasks_G[0].Delay = 0;
        SCH_tasks_G[0].Period = 0;
        SCH_tasks_G[0].RunMe = 0;
        SCH_tasks_G[0].TaskID = 0;
        MARKING[0] = 0;
        task_count = 0;
    }
    else
    {
        for (uint32_t k = TASK_INDEX; k < task_count - 1; k++)
        {
            SCH_tasks_G[k] = SCH_tasks_G[k + 1];
        }

        uint32_t last_pos = task_count - 1;
        SCH_tasks_G[last_pos].pTask = 0x0000;
        SCH_tasks_G[last_pos].Delay = 0;
        SCH_tasks_G[last_pos].Period = 0;
        SCH_tasks_G[last_pos].RunMe = 0;
        SCH_tasks_G[last_pos].TaskID = 0;
        MARKING[last_pos] = 0;

        task_count--;

        if (task_count > 0)
        {
            SCH_Update_Marking();
        }
    }

    return RETURN_NORMAL;
}

static void SCH_Update_Marking(void)
{
    if (task_count == 0)
        return;

    uint32_t first_delay = SCH_tasks_G[0].Delay;

    for (uint32_t n = 0; n < task_count; n++)
    {
        if (SCH_tasks_G[n].Delay == first_delay)
        {
            MARKING[n] = 1;
        }
        else
        {
            MARKING[n] = 0;
        }
    }
}

void SCH_Update(void)
{
    if (task_count > 0)
    {
        if (SCH_tasks_G[0].Delay > 0)
        {
            SCH_tasks_G[0].Delay--;
        }

        elapsed_time++;

        if (SCH_tasks_G[0].Delay == 0)
        {
            SCH_tasks_G[0].RunMe++;
        }
    }
}

void SCH_Dispatch_Tasks(void)
{
    if (task_count > 0 && SCH_tasks_G[0].RunMe > 0)
    {

        // BƯỚC 1: Cập nhật delay cho tất cả task
        for (uint32_t m = 0; m < task_count; m++)
        {
            if (MARKING[m] == 0)
            {
                if (SCH_tasks_G[m].Delay >= elapsed_time)
                {
                    SCH_tasks_G[m].Delay -= elapsed_time;
                }
                else
                {
                    SCH_tasks_G[m].Delay = 0;
                }
            }
            else
            {
                SCH_tasks_G[m].Delay = 0;
                SCH_tasks_G[m].RunMe = 1;
            }
        }

        // BƯỚC 2: Thực thi các task sẵn sàng
        while (task_count > 0 && SCH_tasks_G[0].RunMe > 0)
        {

            if (SCH_tasks_G[0].pTask != 0x0000)
            {
                (*SCH_tasks_G[0].pTask)();
            }

            SCH_tasks_G[0].RunMe--;

            if (SCH_tasks_G[0].Period == 0)
            {
                SCH_Delete_Task(0);
            }
            else
            {
                void (*temp_func)(void) = SCH_tasks_G[0].pTask;
                uint32_t temp_period = SCH_tasks_G[0].Period;

                SCH_Delete_Task(0);
                SCH_Add_Task(temp_func, temp_period, temp_period);
            }
        }

        elapsed_time = 0;
    }
}

uint32_t SCH_Get_Current_Size(void)
{
    return task_count;
}