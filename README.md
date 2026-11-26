# STM32-Line-Follower · v0.8

Firmware C cho Blue Pill STM32F103C8T6, thanh 8 mắt analog, L298N và hai motor TT 6 V. Chân PWM PB8/PB9 dùng TIM4. Các phiên bản là các bước học được dựng lại cho repo này. Tag Git cho phép lấy trọn bộ mã tại từng bước.

**Mốc này — Tinh chỉnh PD/PID cho các đoạn cua liên tiếp.** PD có lọc D, I tùy chọn có giới hạn, ga theo cua, nhận cụm vạch, khoảng mất vạch hữu hạn, nhận vạch ngang.

## Bắt đầu

- Dùng CubeMX và STM32CubeIDE hoặc GNU Arm theo [hướng dẫn build và bảng chân](Documents/BUILD.md).
- Để thử trên máy tính, cài Python 3 và GCC (Windows có thể dùng MinGW-w64), mở terminal tại gốc repo và chạy `python Tests/run.py`. Bộ thử cấp đầu vào giả cho mã C ứng dụng.
- Mở [CHANGELOG.md](CHANGELOG.md) để xem các mốc có trong tag này. Dùng `git tag --list` để xem những tag đã tải và `git diff v0.6 v0.7 -- Core` để so sánh khi có đủ hai tag.

Các thông số chỉnh sửa nằm trong **Core/Inc/config.h**.

Khi tập với cảm biến, đặt cả dãy lần lượt trên trắng và trên miếng đen cùng vật liệu vạch. Đợi trạng thái READY, kiểm tra số đo bên trái và bên phải trước khi chạy.

## Chọn chế độ và chạy v0.8

- `PIF_LESSON` mặc định là **PIF_TRACK**. Motor đứng yên ở WAIT_WHITE → WAIT_BLACK → READY. Sau hiệu chuẩn, đặt xe trên line hẹp ngay trước vạch ngang xuất phát và nhấn nút lần thứ ba. Nhấn trong lúc chạy để dừng. Sau LOST hoặc FINISHED nhấn để về READY, sau đó đặt lại và nhấn để bắt đầu.
- Chọn `PIF_SENSORS` để chỉ xem ADC thô. Chọn `PIF_WHEELS` để thử từng bánh với nút. Build và nạp lại khi đổi chế độ.
- `KP=13`, `KD=0.12`, `KI=0`; ga nền thẳng/cua 280/185 phần nghìn, trần mỗi bánh 600. Đây là điểm bắt đầu thử cho xe. Muốn thử P, đặt KD và KI bằng 0. Để thử PID, đặt KI dương nhỏ và so sánh sai lệch kéo dài. Mỗi lượt chỉ thay một hệ số.
- Chu kỳ lấy mẫu 2 ms. Dừng LOST sau 15 khung chưa xác nhận lại được tín hiệu. Vạch ngang xác nhận qua 4 khung; hiệu chỉnh nếu sa bàn có ngã tư hoặc vùng đen rộng khác. Xe sử dụng phản hồi vị trí line; PWM là lệnh mỗi bánh, không có encoder đo RPM.
- Các biến xem trong debugger gồm `watch.adc`, `watch.q`, `watch.error_mm`, `watch.pwm_left`, `watch.pwm_right`, `watch.gap_events`, `watch.state`.

| Mô-đun | Việc chính |
|---|---|
| main.c và cấu hình .ioc | Khởi tạo chân, ADC, DMA, các timer và vòng lặp chính |
| motor.c | Xuất hai PWM độc lập và đặt chiều tiến |
| sensor.c | Đọc DMA, hiệu chuẩn, chuẩn hóa, tìm cụm line |
| filter.c | Lấy trung vị ba mẫu cho từng mắt |
| pid.c | Đạo hàm có lọc, điều chỉnh lái và giới hạn tích phân |
| controller.c | Nút, trạng thái xuất phát/đích, giảm ga trong cua, xử lý mất vạch |
| config.h | Chế độ, hệ số điều khiển, thông số cảm biến và giới hạn duty |

Bộ thử PC chạy ba chế độ, hai cực tính cảm biến, lỗi ADC, các tình huống mất vạch và chín lượt mô hình chữ S. Mô hình có ba bộ hình học và độ trễ motor giả định. Các lượt mô hình phục vụ so sánh code; đội cần đo thời gian vòng, độ bám và độ lệch trên bo thật.
