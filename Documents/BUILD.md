# Build và thử trên Blue Pill

## Dùng STM32CubeIDE 2.2 và CubeMX độc lập

1. Giải nén hoặc clone mã nguồn. Nếu đang xem một tag, dùng toàn bộ tệp của cùng tag.
2. Tạo thư mục làm việc **riêng** có tên `PIF_F103` rồi chép `PIF_F103.ioc` vào đó. Mở tệp bằng CubeMX, cài STM32Cube MCU Package for STM32F1 (gói tham chiếu v1.8.5) và chọn Toolchain / IDE → STM32CubeIDE. Nhấn Generate Code. Giữ file startup, linker, Drivers và các file project CubeMX vừa sinh.
3. Chép toàn bộ nội dung `Core/Inc` và `Core/Src` của tag sang hai thư mục tương ứng của project vừa sinh. Thay các file mẫu trùng tên trong **bản làm việc mới**. Khi đổi sang tag cũ hơn, dùng thư mục làm việc sạch để tránh còn sót mô-đun C của tag mới. Các mã IRQ và MSP trong repo nằm đúng vị trí của hai file cùng tên do CubeMX sinh mặc định.
4. Mở CubeIDE → File → Import → General → Existing Projects into Workspace → chọn thư mục có `.project`. Refresh rồi Project → Build Project. Chỉ đưa thư mục Core vào project nhúng. Thư mục Tests chứa HAL giả dành cho máy tính.
5. Kê bánh xe. Ngắt nguồn motor khi nạp hay dừng bằng breakpoint. Nạp qua ST-Link/SWD, nhấn Resume rồi đọc `watch` bằng Live Expressions. Khi hàn mạch, bảo đảm chân EN ở mức tắt trong lúc reset. Dùng `PIF_WHEELS` để thử chiều hai bánh trước khi bám vạch.
6. Nếu sinh lại code, kiểm tra hai dòng trong các vùng USER CODE của main.c. Dòng sau khởi tạo ngoại vi là `Pif_Init(&hadc1, &htim3, &htim4);`. Trong thân `while (1)` là `Pif_Poll();`. Vùng Includes chứa `#include "controller.h"`. Bản main.c đi kèm đã có cả ba.

## Build riêng với GNU Arm

Có thể dùng trực tiếp mã nguồn repo và gói chính thức ở máy, không cần sinh project IDE cho cách này. Cài Python 3, GNU Arm Embedded Toolchain có newlib-nano, và STM32CubeF1. Ví dụ lệnh trên Windows sau khi thêm thư mục chứa `arm-none-eabi-gcc.exe` vào PATH:

```powershell
python Tools/build_firmware.py "C:\STM32\STM32Cube_FW_F1_V1.8.5"
```

Kết quả là `build/line_follower.elf`. Startup và CMSIS/HAL lấy từ gói ST, linker trong repo giới hạn Flash 64 KiB và RAM 20 KiB. Kết quả kiểm thử trên PC chỉ đánh giá logic ứng dụng. Hãy ghi kết quả build ARM, dòng tiêu thụ, điện áp, thời gian vòng và lỗi theo từng lượt khi kiểm chứng trên bo.

## Đối chiếu phần cứng

| Tín hiệu | Chân hoặc cài đặt |
|---|---|
| Dãy 8 ngõ analog S0…S7 | PA0…PA7, S0 bên trái theo chiều tiến, ngõ ADC 0…VDDA |
| Đo pack qua cầu chia | PB0, 33 kΩ phía pin, 10 kΩ xuống GND. 33 kΩ / 8,25 kΩ dùng hệ số 16500 mV. |
| PWM ENA / ENB | PB8 / PB9, TIM4 CH3/CH4, 10 kHz. Tháo jumper ENA/ENB khi cấp PWM ngoài. |
| Chiều trái IN1 / IN2 | PB14 / PB15 |
| Chiều phải IN3 / IN4 | PB13 / PB12 |
| Nút START, LED bo | PB10 kéo lên nội (nhấn xuống GND), PC13 sáng ở mức thấp |
| ST-Link, clock | PA13 SWDIO, PA14 SWCLK, GND chung. Thạch anh 8 MHz, PLL ×9. |

Hai motor TT 6 V DC dùng L298N. Đo đầu ra buck ở 6,0 V trước khi nối nhánh VS của module. Nguồn logic đấu theo đúng jumper của phiên bản L298N. Dây tín hiệu 8 mắt đi cách dây motor. Điện áp đo tại PB0 là điện áp pack sau quy đổi.

[STM32CubeF1 của ST](https://github.com/STMicroelectronics/STM32CubeF1) · [STM32F103C8 datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
