# STM32F103 Register-Level Projects: Snake Game & Calculator

---

## 1. Rắn săn mồi

Dự án mô phỏng trò chơi Snake huyền thoại hiển thị trên Terminal máy tính thông qua giao tiếp UART.

### Các cập nhật và tối ưu hóa đã thực hiện:
Để khắc phục hiện tượng giật lag và nhấp nháy màn hình, các kỹ thuật sau đã được áp dụng:
* **Vẽ qua Buffer (Mảng đệm):** Thay vì gửi từng ký tự đơn lẻ, toàn bộ bản đồ được xây dựng trong một chuỗi `char screen_buf[]` lớn và gửi một lần duy nhất qua UART để tăng tốc độ hiển thị.
* **Sử dụng SysTick Timer:** Loại bỏ vòng lặp `delay` bằng lệnh `for`. Sử dụng ngắt `SysTick` (1ms) để tạo nhịp di chuyển ổn định cho con rắn, giúp nút bấm nhạy hơn và không bị block.
* **ANSI Escape Codes:**
    * `\033[H`: Đưa con trỏ về đầu trang thay vì xóa toàn bộ màn hình, giảm nhấp nháy.
    * `\033[?25l`: Ẩn con trỏ chuột màu xanh nhấp nháy để mang lại trải nghiệm game chuyên nghiệp.
* **Tăng tốc độ truyền:** Khuyến nghị cấu hình Baudrate lên **115200** (thay vì 9600) để xử lý lượng dữ liệu lớn từ khung hình.

### Sơ đồ nối dây (Nút bấm):
| Chân STM32 | Hướng di chuyển | Cấu hình |
| :--- | :--- | :--- |
| **PA0** | **Up** | Input Pull-up (Nối nút nhấn xuống GND) |
| **PA1** | **Down** | Input Pull-up (Nối nút nhấn xuống GND) |
| **PA2** | **Left** | Input Pull-up (Nối nút nhấn xuống GND) |
| **PA3** | **Right** | Input Pull-up (Nối nút nhấn xuống GND) |

---

## 2. Máy tính bỏ túi)

Dự án thực hiện các phép tính cơ bản thông qua Ma trận phím (Keypad 4x4) và hiển thị kết quả.

### Chức năng chính:
* **Quét Ma trận phím:** Sử dụng thuật toán quét hàng/cột để nhận diện phím nhấn từ Keypad 4x4.
* **Xử lý tính toán:** Thực hiện các phép toán cộng, trừ, nhân, chia số nguyên.
* **Hiển thị:** Kết quả được gửi qua UART để theo dõi trên máy tính hoặc hiển thị qua LCD (tùy cấu hình).

