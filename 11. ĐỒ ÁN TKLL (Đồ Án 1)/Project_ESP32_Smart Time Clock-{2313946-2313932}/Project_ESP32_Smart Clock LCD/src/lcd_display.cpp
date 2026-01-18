#include "lcd_display.h"
#include "config.h"
#include "global_vars.h"
#include <Arduino.h>

// ============================================================================
// KHỞI TẠO ĐỐI TƯỢNG LCD I2C 16x2
// ============================================================================
// Tạo đối tượng LCD với địa chỉ I2C, số cột và số hàng từ file config
static LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Icon đồng hồ (5x8)
byte clockIcon[8] = {
    0b00000,
    0b01110,
    0b10101,
    0b10111,
    0b10001,
    0b01110,
    0b00000,
    0b00000};

// ============================================================================
// HÀM KHỞI TẠO LCD
// ============================================================================
// Chức năng: Khởi tạo màn hình LCD, bật đèn nền và hiển thị thông báo khởi động
void initLCD()
{
    lcd.begin();     // Khởi tạo LCD (không tham số)
    lcd.backlight(); // Bật đèn nền của LCD để có thể nhìn thấy
    lcd.clear();     // Xóa toàn bộ nội dung trên màn hình

    // ---- Tạo icon đồng hồ vào vị trí số 0 ----
    lcd.createChar(0, clockIcon);

    // Hiển thị thông báo chào mừng khi khởi động
    lcd.setCursor(0, 0);      // Di chuyển con trỏ đến cột 0, hàng 0 (hàng đầu tiên)
    lcd.print("Smart Clock"); // In chữ "Smart Clock"

    lcd.write(byte(0)); // In icon đồng hồ sau chữ "Smart Clock"

    lcd.setCursor(0, 1);          // Di chuyển con trỏ đến cột 0, hàng 1 (hàng thứ hai)
    lcd.print("Initializing..."); // In chữ "Initializing..."
    delay(3000);                  // Đợi 2 giây để người dùng đọc thông báo
    lcd.clear();                  // Xóa màn hình để chuẩn bị hiển thị nội dung chính

    Serial.println("LCD initialized"); // Gửi thông báo qua Serial Monitor
}

// ============================================================================
// HÀM TRẢ VỀ CON TRỎ ĐỐI TƯỢNG LCD
// ============================================================================
// Chức năng: Cho phép các file khác truy cập đối tượng LCD
// Trả về: Con trỏ tới đối tượng LCD
LiquidCrystal_I2C *getLCD()
{
    return &lcd; // Trả về địa chỉ của đối tượng lcd
}

// ============================================================================
// HIỂN THỊ NHIỆT ĐỘ VÀ ĐỘ ẨM
// ============================================================================
// Chức năng: Đọc và hiển thị nhiệt độ, độ ẩm từ cảm biến DHT
// Tham số: dht - con trỏ tới đối tượng cảm biến DHT
// Định dạng hiển thị:
// Dòng 1: Temp: 25.3°C
// Dòng 2: Humi: 67.2%
// ============================================================================
void displayLCD_TempHumidity(DHT *dht)
{
    // Đọc nhiệt độ (°C) và độ ẩm (%) từ cảm biến
    float temp = dht->readTemperature();
    float humi = dht->readHumidity();

    // Kiểm tra xem việc đọc có thành công không
    // isnan() = is Not a Number - kiểm tra giá trị không hợp lệ
    if (isnan(temp) || isnan(humi))
    {
        // Nếu đọc thất bại, hiển thị thông báo lỗi
        lcd.setCursor(0, 0);
        lcd.print("DHT Error!      "); // Khoảng trắng để xóa ký tự cũ
        lcd.setCursor(0, 1);
        lcd.print("Check Sensor    ");
        return; // Thoát khỏi hàm
    }

    // Hiển thị nhiệt độ trên dòng 1
    lcd.setCursor(0, 0);  // Đặt con trỏ ở đầu dòng 1
    lcd.print("TEMP: ");  // In nhãn "Temp: "
    lcd.print(temp, 1);   // In giá trị nhiệt độ với 1 chữ số thập phân
    lcd.print((char)223); // In ký tự độ ° (mã ASCII 223)
    lcd.print("C  ");     // In "C" và khoảng trắng để xóa ký tự thừa

    // Hiển thị độ ẩm trên dòng 2
    lcd.setCursor(0, 1); // Đặt con trỏ ở đầu dòng 2
    lcd.print("HUMI: "); // In nhãn "Humi: "
    lcd.print(humi, 1);  // In giá trị độ ẩm với 1 chữ số thập phân
    lcd.print("%    ");  // In "%" và khoảng trắng để xóa ký tự thừa
}

// ============================================================================
// HIỂN THỊ NGÀY GIỜ (CHẾ ĐỘ ĐỒNG HỒ)
// ============================================================================
// Chức năng: Hiển thị thời gian và ngày tháng hiện tại
// Tham số: rtc - con trỏ tới đối tượng RTC DS3231
// Định dạng hiển thị:
// Dòng 1: Time: 14:35:42 (Giờ:Phút:Giây)
// Dòng 2: Date: 25/12/2024 (Ngày/Tháng/Năm)
// ============================================================================
void displayLCD_DateTime(RTC_DS3231 *rtc)
{
    // Lấy thời gian hiện tại từ module RTC
    DateTime now = rtc->now();

    // ========================================================================
    // HIỂN THỊ THỜI GIAN (DÒNG 1)
    // ========================================================================
    lcd.setCursor(0, 0); // Đặt con trỏ ở đầu dòng 1
    lcd.print("Time: "); // In nhãn "Time: "

    // In giờ (thêm số 0 phía trước nếu < 10)
    if (now.hour() < 10)
        lcd.print("0");    // In "0" để có định dạng 2 chữ số (VD: 09)
    lcd.print(now.hour()); // In giá trị giờ
    lcd.print(":");        // In dấu ":"

    // In phút (thêm số 0 phía trước nếu < 10)
    if (now.minute() < 10)
        lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");

    // In giây (thêm số 0 phía trước nếu < 10)
    if (now.second() < 10)
        lcd.print("0");
    lcd.print(now.second());

    // ========================================================================
    // HIỂN THỊ NGÀY THÁNG (DÒNG 2)
    // ========================================================================
    lcd.setCursor(0, 1); // Đặt con trỏ ở đầu dòng 2
    lcd.print("Date: "); // In nhãn "Date: "

    // In ngày (thêm số 0 phía trước nếu < 10)
    if (now.day() < 10)
        lcd.print("0");
    lcd.print(now.day());
    lcd.print("/"); // In dấu "/"

    // In tháng (thêm số 0 phía trước nếu < 10)
    if (now.month() < 10)
        lcd.print("0");
    lcd.print(now.month());
    lcd.print("/");

    // In năm (4 chữ số, không cần thêm số 0)
    lcd.print(now.year());
}

// ============================================================================
// HIỂN THỊ BÁO THỨC
// ============================================================================
// Chức năng: Hiển thị thời gian hiện tại và giờ báo thức đã cài đặt
// Tham số: rtc - con trỏ tới đối tượng RTC DS3231
// Định dạng hiển thị:
// Dòng 1: Now: 14:35:42 (thời gian hiện tại)
// Dòng 2: Alarm: 05:21 (giờ báo thức, nháy khi đang chỉnh sửa)
// ============================================================================
void displayLCD_Alarm(RTC_DS3231 *rtc)
{
    // Lấy thời gian hiện tại từ module RTC
    DateTime now = rtc->now();

    // ========================================================================
    // HIỂN THỊ THỜI GIAN HIỆN TẠI (DÒNG 1)
    // ========================================================================
    lcd.setCursor(0, 0);
    lcd.print("Now: ");

    // In giờ hiện tại
    if (now.hour() < 10)
        lcd.print("0");
    lcd.print(now.hour());
    lcd.print(":");

    // In phút hiện tại
    if (now.minute() < 10)
        lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");

    // In giây hiện tại
    if (now.second() < 10)
        lcd.print("0");
    lcd.print(now.second());
    lcd.print(" "); // Khoảng trắng để xóa ký tự thừa

    // ========================================================================
    // HIỂN THỊ GIỜ BÁO THỨC (DÒNG 2)
    // ========================================================================
    lcd.setCursor(0, 1);

    // Kiểm tra nếu báo thức đang kêu
    if (alarmTriggered)
    {
        lcd.print("ALARM RINGING!  "); // Hiển thị thông báo báo thức đang kêu
    }
    else
    {
        lcd.print("Alarm: ");

        // ====================================================================
        // HIỆU ỨNG NHÁY KHI CHỈNH SỬA
        // ====================================================================
        // Tính toán trạng thái nháy: đổi trạng thái mỗi 500ms
        bool blinkOn = ((millis() / 500) % 2) == 0;
        // millis() / 500: số lần 500ms đã trôi qua
        // % 2: lấy phần dư khi chia 2 (kết quả 0 hoặc 1)
        // == 0: true khi phần dư = 0, false khi phần dư = 1

        // ====================================================================
        // HIỂN THỊ GIỜ BÁO THỨC
        // ====================================================================
        // Nếu đang chỉnh giờ VÀ đang ở trạng thái tắt của nháy
        if (alarmEditHour && !blinkOn)
        {
            lcd.print("  "); // In khoảng trắng (ẩn giờ)
        }
        else
        {
            // Hiển thị giờ bình thường
            if (alarmHour < 10)
                lcd.print("0");
            lcd.print(alarmHour);
        }

        lcd.print(":"); // In dấu ":" giữa giờ và phút

        // ====================================================================
        // HIỂN THỊ PHÚT BÁO THỨC
        // ====================================================================
        // Nếu đang chỉnh phút VÀ đang ở trạng thái tắt của nháy
        if (!alarmEditHour && !blinkOn)
        {
            lcd.print("  "); // In khoảng trắng (ẩn phút)
        }
        else
        {
            // Hiển thị phút bình thường
            if (alarmMinute < 10)
                lcd.print("0");
            lcd.print(alarmMinute);
        }

        lcd.print("    "); // Khoảng trắng để xóa ký tự thừa
    }
}

// ============================================================================
// HIỂN THỊ ĐỒNG HỒ BẤM GIỜ (STOPWATCH)
// ============================================================================
// Chức năng: Hiển thị bộ đếm thời gian stopwatch
// Định dạng hiển thị:
// Dòng 1: Stopwatch RUN/STOP (tiêu đề + trạng thái)
// Dòng 2: 01:23:45.67 (giờ:phút:giây.centisecond) + số lap
// ============================================================================
void displayLCD_Stopwatch()
{
    // ========================================================================
    // CẬP NHẬT THỜI GIAN NẾU ĐANG CHẠY
    // ========================================================================
    if (isTimerRunning)
    {
        // Tính thời gian đã trôi qua = thời điểm hiện tại - thời điểm bắt đầu
        timerCurrentTime = millis() - timerStartTime;
    }
    // Nếu không chạy, giữ nguyên giá trị timerCurrentTime (dừng ở thời điểm pause)

    // ========================================================================
    // CHUYỂN ĐỔI THỜI GIAN TỪ MILLISECONDS SANG ĐỊNH DẠNG HIỂN THỊ
    // ========================================================================
    // Tính giờ (giới hạn tối đa 99 giờ)
    unsigned long hours = (timerCurrentTime / 3600000) % 100;
    // 3600000 ms = 1 giờ, % 100 để giới hạn hiển thị 2 chữ số

    // Tính phút (0-59)
    unsigned long minutes = (timerCurrentTime / 60000) % 60;
    // 60000 ms = 1 phút, % 60 để lấy phần dư khi chia cho 60

    // Tính giây (0-59)
    unsigned long seconds = (timerCurrentTime / 1000) % 60;
    // 1000 ms = 1 giây, % 60 để lấy phần dư

    // Tính centiseconds - phần trăm giây (0-99)
    unsigned long centiseconds = (timerCurrentTime / 10) % 100;
    // 10 ms = 1 centisecond, % 100 để lấy 2 chữ số

    // ========================================================================
    // HIỂN THỊ TIÊU ĐỀ VÀ TRẠNG THÁI (DÒNG 1)
    // ========================================================================
    lcd.setCursor(0, 0);
    lcd.print("Stopwatch ");

    // Hiển thị trạng thái (đang chạy hoặc dừng)
    if (isTimerRunning)
    {
        lcd.print("RUN...."); // Hiển thị "RUN" khi đang chạy
    }
    else
    {
        lcd.print("STOP !"); // Hiển thị "STOP" khi dừng
    }

    // ========================================================================
    // HIỂN THỊ THỜI GIAN (DÒNG 2)
    // ========================================================================
    lcd.setCursor(0, 1);

    // In giờ (2 chữ số)
    if (hours < 10)
        lcd.print("0");
    lcd.print(hours);
    lcd.print(":");

    // In phút (2 chữ số)
    if (minutes < 10)
        lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");

    // In giây (2 chữ số)
    if (seconds < 10)
        lcd.print("0");
    lcd.print(seconds);
    lcd.print(".");

    // In centiseconds (2 chữ số)
    if (centiseconds < 10)
        lcd.print("0");
    lcd.print(centiseconds);

    // ========================================================================
    // HIỂN THỊ SỐ LAP ĐÃ LƯU (GÓC PHẢI DÒNG 2)
    // ========================================================================
    lcd.setCursor(14, 1); // Di chuyển con trỏ đến cột 14 (gần cuối dòng)
    lcd.print("L");       // In chữ "L" (viết tắt của Lap)
    lcd.print(lapCount);  // In số lượng lap đã lưu
}

// ============================================================================
// HIỂN THỊ ĐẾM NGƯỢC (COUNTDOWN)
// ============================================================================
// Chức năng: Hiển thị bộ đếm ngược với 3 trạng thái khác nhau
// Định dạng hiển thị:
// Dòng 1: Countdown EDIT/RUN/STOP (trạng thái)
// Dòng 2: 00:03:00.00 (giờ:phút:giây.centisecond)
// ============================================================================
void displayLCD_Countdown()
{
    // ========================================================================
    // TRẠNG THÁI 1: ĐANG CHỈNH SỬA THỜI GIAN ĐẾM NGƯỢC
    // ========================================================================
    if (countdownEditing)
    {
        // Hiển thị tiêu đề trạng thái chỉnh sửa
        lcd.setCursor(0, 0);
        lcd.print("Countdown EDIT  ");

        // ====================================================================
        // HIỆU ỨNG NHÁY CHO TRƯỜNG ĐANG CHỈNH SỬA
        // ====================================================================
        bool blinkOn = ((millis() / 500) % 2) == 0;
        // Đổi trạng thái mỗi 500ms để tạo hiệu ứng nháy

        lcd.setCursor(0, 1);

        // ====================================================================
        // HIỂN THỊ GIỜ (nháy nếu countdownEditField == 0)
        // ====================================================================
        if (countdownEditField == 0 && !blinkOn)
        {
            lcd.print("  "); // Ẩn giờ khi nháy
        }
        else
        {
            if (c_hours < 10)
                lcd.print("0");
            lcd.print(c_hours);
        }
        lcd.print(":");

        // ====================================================================
        // HIỂN THỊ PHÚT (nháy nếu countdownEditField == 1)
        // ====================================================================
        if (countdownEditField == 1 && !blinkOn)
        {
            lcd.print("  "); // Ẩn phút khi nháy
        }
        else
        {
            if (c_minutes < 10)
                lcd.print("0");
            lcd.print(c_minutes);
        }
        lcd.print(":");

        // ====================================================================
        // HIỂN THỊ GIÂY (nháy nếu countdownEditField == 2)
        // ====================================================================
        if (countdownEditField == 2 && !blinkOn)
        {
            lcd.print("  "); // Ẩn giây khi nháy
        }
        else
        {
            if (c_seconds < 10)
                lcd.print("0");
            lcd.print(c_seconds);
        }
        lcd.print(".00    "); // Luôn hiển thị .00 cho centiseconds khi chỉnh sửa
    }

    // ========================================================================
    // TRẠNG THÁI 2: ĐANG CHẠY ĐẾM NGƯỢC
    // ========================================================================
    else if (isCountdownRunning)
    {
        // Tính thời gian đã trôi qua kể từ khi bắt đầu
        unsigned long elapsed = millis() - countdownStartTime;

        // ====================================================================
        // KIỂM TRA XEM ĐÃ HẾT THỜI GIAN CHƯA
        // ====================================================================
        if (elapsed >= countdownDuration)
        {
            // Đã hết thời gian
            countdownRemaining = 0;         // Đặt thời gian còn lại = 0
            isCountdownRunning = false;     // Dừng đếm ngược
            countdownTriggered = true;      // Đánh dấu đã hết giờ
            digitalWrite(BUZZER_PIN, HIGH); // Bật buzzer để báo
        }
        else
        {
            // Còn thời gian, tính thời gian còn lại
            countdownRemaining = countdownDuration - elapsed;
        }

        // ====================================================================
        // CHUYỂN ĐỔI THỜI GIAN CÒN LẠI SANG ĐỊNH DẠNG HIỂN THỊ
        // ====================================================================
        unsigned long t = countdownRemaining;
        unsigned long hours = (t / 3600000) % 100;   // Giờ
        unsigned long minutes = (t / 60000) % 60;    // Phút
        unsigned long seconds = (t / 1000) % 60;     // Giây
        unsigned long centiseconds = (t / 10) % 100; // Centiseconds

        // Hiển thị tiêu đề trạng thái đang chạy
        lcd.setCursor(0, 0);
        lcd.print("Countdown RUN   ");

        // Hiển thị thời gian còn lại
        lcd.setCursor(0, 1);
        if (hours < 10)
            lcd.print("0");
        lcd.print(hours);
        lcd.print(":");
        if (minutes < 10)
            lcd.print("0");
        lcd.print(minutes);
        lcd.print(":");
        if (seconds < 10)
            lcd.print("0");
        lcd.print(seconds);
        lcd.print(".");
        if (centiseconds < 10)
            lcd.print("0");
        lcd.print(centiseconds);
        lcd.print("  ");
    }

    // ========================================================================
    // TRẠNG THÁI 3: DỪNG HOẶC ĐÃ HẾT GIỜ
    // ========================================================================
    else
    {
        // ====================================================================
        // NẾU ĐÃ HẾT GIỜ (COUNTDOWN TRIGGERED)
        // ====================================================================
        if (countdownTriggered)
        {
            lcd.setCursor(0, 0);
            lcd.print("TIME'S UP!      "); // Hiển thị thông báo hết giờ
            lcd.setCursor(0, 1);
            lcd.print("00:00:00.00     "); // Hiển thị thời gian = 0
        }
        // ====================================================================
        // NẾU ĐANG DỪNG (CHƯA BẮT ĐẦU HOẶC BỊ PAUSE)
        // ====================================================================
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("Countdown STOP  ");

            // Hiển thị thời gian ban đầu đã cài đặt
            lcd.setCursor(0, 1);
            if (c_hours_initial < 10)
                lcd.print("0");
            lcd.print(c_hours_initial);
            lcd.print(":");
            if (c_minutes_initial < 10)
                lcd.print("0");
            lcd.print(c_minutes_initial);
            lcd.print(":");
            if (c_seconds_initial < 10)
                lcd.print("0");
            lcd.print(c_seconds_initial);
            lcd.print(".00    ");
        }
    }
}

// ============================================================================
// HIỂN THỊ GIÁ TRỊ LAP (dùng khi xem lại các lap đã lưu)
// ============================================================================
// Chức năng: Hiển thị thời gian của một lap cụ thể
// Tham số: t - thời gian lap tính bằng milliseconds
// Định dạng hiển thị:
// Dòng 1: Lap Time:
// Dòng 2: 01:23:45.67
// ============================================================================
void displayLCD_LapValue(unsigned long t)
{
    // ========================================================================
    // CHUYỂN ĐỔI THỜI GIAN TỪ MILLISECONDS SANG ĐỊNH DẠNG HIỂN THỊ
    // ========================================================================
    unsigned long hours = (t / 3600000) % 100;   // Giờ (tối đa 99)
    unsigned long minutes = (t / 60000) % 60;    // Phút (0-59)
    unsigned long seconds = (t / 1000) % 60;     // Giây (0-59)
    unsigned long centiseconds = (t / 10) % 100; // Centiseconds (0-99)

    // ========================================================================
    // HIỂN THỊ THÔNG TIN LAP
    // ========================================================================
    lcd.clear(); // Xóa màn hình để hiển thị rõ ràng

    // Dòng 1: Tiêu đề
    lcd.setCursor(0, 0);
    lcd.print("Lap Time:");

    // Dòng 2: Thời gian lap
    lcd.setCursor(0, 1);

    // In giờ
    if (hours < 10)
        lcd.print("0");
    lcd.print(hours);
    lcd.print(":");

    // In phút
    if (minutes < 10)
        lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");

    // In giây
    if (seconds < 10)
        lcd.print("0");
    lcd.print(seconds);
    lcd.print(".");

    // In centiseconds
    if (centiseconds < 10)
        lcd.print("0");
    lcd.print(centiseconds);
}