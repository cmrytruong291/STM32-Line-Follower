# Các mốc firmware

Các mốc học tập được dựng lại từ firmware xe; phạm vi kiểm tra được ghi theo từng tag.

## v0.8 · Tinh chỉnh PD/PID cho các đoạn cua liên tiếp

PD có lọc D, I tùy chọn có giới hạn, ga theo cua, nhận cụm vạch, khoảng mất vạch hữu hạn, nhận vạch ngang.

## v0.7 · TIM3, ADC và DMA ở chu kỳ 2 ms

TIM3 kích ADC mỗi 2 ms. DMA vòng hai nửa x 9 mẫu. Mất khung quá 20 ms trong vòng giám sát gây dừng.

## v0.6 · Điều khiển P và PD cơ bản

Đặt xe lên vạch hẹp và nhấn sau khi READY. PD cơ bản chạy bằng lịch polling. Đặt KD = 0 để thử P.

## v0.5 · Tính vị trí đường line

Tính trọng tâm độ đen theo đơn vị mm. Vạch bên phải cho error_mm dương.

## v0.4 · Lọc nhiễu và chuẩn hóa

Trung vị 3 mẫu theo kênh, chuẩn hóa trắng/đen về 0…1000. Xem watch.q.

## v0.3 · Hiệu chuẩn trắng và đen

Nhấn trên nền trắng rồi trên tấm đen phủ cả dãy, mỗi lần giữ yên 0,2 s. Kiểm tra bad_mask và READY.

## v0.2 · Đọc tám cảm biến ADC

Đọc PA0…PA7 và PB0 bằng ADC polling; xem watch.adc, motor đứng yên.

## v0.1 · Điều khiển độc lập hai động cơ

Nhấn nút để thử một bánh trong khoảng 0,5 s; nhấn tiếp để thử bánh kia.

## v0.0 · Khởi tạo STM32 và kiểm tra LED

LED PC13 đổi mức mỗi 250 ms; theo dõi watch.frames.
