#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <DHT.h>
#include <RTClib.h>

// ==================== KHỞI TẠO CẢM BIẾN ====================
void initSensors();

// ==================== LẤY ĐỐI TƯỢNG CẢM BIẾN ====================
DHT *getDHT();
RTC_DS3231 *getRTC();

#endif // SENSOR_HANDLER_H