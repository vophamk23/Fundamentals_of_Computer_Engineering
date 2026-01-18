#include "software_timer.h"

#define MAX_COUNTER 10 // Số lượng timer phần mềm tối đa (ở đây cho phép dùng 10 timer)

static int timer_counter[MAX_COUNTER]; // Mảng lưu giá trị đếm còn lại cho từng timer
static int timer_flag[MAX_COUNTER];    // Mảng cờ báo hiệu khi timer hết hạn

//--------------------------------------------------------
// Hàm cài đặt timer
//--------------------------------------------------------
void setTimer(int index, int value)
{
    if (index < MAX_COUNTER)
    {                                 // Kiểm tra chỉ số có hợp lệ không
        timer_counter[index] = value; // Nạp giá trị đếm (thường đơn vị ms hoặc 10ms)
        timer_flag[index] = 0;        // Reset cờ hết hạn về 0
    }
}

//--------------------------------------------------------
// Hàm kiểm tra timer có hết hạn không
//--------------------------------------------------------
int isTimerExpired(int index)
{
    if (index < MAX_COUNTER)
    { // Kiểm tra chỉ số có hợp lệ không
        if (timer_flag[index] == 1)
        {                          // Nếu cờ báo hết hạn
            timer_flag[index] = 0; // Reset lại cờ để tránh báo liên tục
            return 1;              // Trả về 1 nghĩa là timer đã hết hạn
        }
    }
    return 0; // Nếu chưa hết hạn hoặc index sai thì trả về 0
}

//--------------------------------------------------------
// Hàm cập nhật trạng thái các timer
// Hàm này phải được gọi định kỳ trong ngắt Timer cứng
// Ví dụ: nếu Timer cứng gọi mỗi 1ms -> timerRun giảm giá trị counter mỗi 1ms
//--------------------------------------------------------
void timerRun()
{
    for (int i = 0; i < MAX_COUNTER; i++)
    { // Quét qua tất cả các timer
        if (timer_counter[i] > 0)
        {                       // Nếu timer đang chạy
            timer_counter[i]--; // Giảm giá trị đếm đi 1
            if (timer_counter[i] <= 0)
            {                      // Nếu đếm về 0
                timer_flag[i] = 1; // Đặt cờ báo hết hạn
            }
        }
    }
}
