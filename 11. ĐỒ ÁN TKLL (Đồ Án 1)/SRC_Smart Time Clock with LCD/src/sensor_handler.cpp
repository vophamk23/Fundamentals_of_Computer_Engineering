#include "sensor_handler.h" // ✓ Đã dùng dấu ""
#include "config.h"         // ✓ Đã dùng dấu ""
#include <Arduino.h>        // ✓ Arduino.h dùng <> là đúng

// Khởi tạo đối tượng cảm biến DHT11 với chân DHT_PIN và loại DHT11
static DHT dht(DHT_PIN, DHT11);

// Khởi tạo đối tượng RTC DS3231 (module đồng hồ thời gian thực)
static RTC_DS3231 rtc;

// Hàm khởi tạo các cảm biến
void initSensors()
{
    // Khởi động cảm biến DHT11
    dht.begin();
    Serial.println("DHT initialized");

    // Khởi động module RTC DS3231
    if (!rtc.begin())
    {
        // Nếu không tìm thấy RTC, in thông báo lỗi và dừng chương trình
        Serial.println("Không tìm thấy RTC");
        while (1)
            ; // Vòng lặp vô hạn - dừng chương trình tại đây
    }
    Serial.println("RTC found");

    // Cài đặt thời gian cho RTC khi cần thiết
    // Chỉ bỏ comment dòng này khi muốn đồng bộ thời gian RTC với thời gian compile
    // Thời gian sẽ được lấy từ thời điểm biên dịch code (__DATE__ và __TIME__)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

// Hàm trả về con trỏ tới đối tượng DHT
// Dùng để truy cập cảm biến DHT từ các file khác
DHT *getDHT()
{
    return &dht;
}

// Hàm trả về con trỏ tới đối tượng RTC
// Dùng để truy cập module RTC từ các file khác
RTC_DS3231 *getRTC()
{
    return &rtc;
}