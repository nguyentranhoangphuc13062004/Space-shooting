# Multiplayer Dino Game - AK Embedded Base Kit

## Overview

Multiplayer Dino Game là một trò chơi được xây dựng trên **AK Embedded Base Kit sử dụng vi điều khiển STM32L151**.  
Dự án được thiết kế nhằm giúp sinh viên thực hành các khái niệm quan trọng trong **lập trình hệ thống nhúng theo mô hình Event-Driven**.

Trong quá trình phát triển game, các kỹ thuật sau được áp dụng:

- Event Driven Programming
- Task & Message Handling
- Timer Interrupt
- State Machine
- RF Wireless Communication (NRF24L01+)
- OLED Graphics Rendering

Game hỗ trợ **2 thiết bị chơi với nhau theo thời gian thực** thông qua giao tiếp **RF NRF24L01+**.

---

# Hardware

Project sử dụng **AK Embedded Base Kit (STM32L151)**.

Các thành phần phần cứng chính:

| Component | Description |
|----------|-------------|
| STM32L151 | Main MCU |
| OLED 1.3" | Hiển thị game |
| Buttons (3) | Điều khiển game |
| Buzzer | Phát âm thanh |
| NRF24L01+ | Giao tiếp RF Multiplayer |
| Flash 32MB | Lưu trữ dữ liệu |

---

# Game Objects

Trong trò chơi có các đối tượng sau:

| Object | Name | Description |
|------|------|-------------|
| Player | Dino | Nhân vật chính |
| Obstacle | Cactus | Chướng ngại vật trên mặt đất |
| Enemy | Bird | Chướng ngại vật trên không |
| Item | Gift | Vật phẩm tấn công đối thủ |
| Background | Cloud | Đám mây nền tạo hiệu ứng |

---

# Gameplay

Game được chơi trên **2 thiết bị**.

Một thiết bị đóng vai trò **MASTER** để bắt đầu game.

### Control Buttons

| Button | Action |
|------|------|
| UP | Nhảy |
| DOWN | Cúi xuống |
| MODE | Bắt đầu game |

### Game Objective

- Né chướng ngại vật
- Thu thập Gift
- Sống càng lâu càng tốt
- Đạt điểm cao nhất

---

# Game Mechanism

## Score System

| Action | Score |
|------|------|
| Pass Cactus | +1 |
| Pass Bird | +1 |
| Collect Gift | +5 |

---

## Difficulty System

Tốc độ ban đầu:
Speed = 35


Mỗi **15 điểm**:


Speed += 5


Giới hạn:


Max Speed = 60


---

# Multiplayer Attack

Khi người chơi ăn **Gift**, thiết bị sẽ gửi lệnh RF:
CMD_ATTACK


Thiết bị đối thủ sẽ:

- Tăng tốc độ game
- Màn hình nhấp nháy
- Phát âm thanh cảnh báo

Thời gian hiệu lực:


3 seconds


---

# Game Over Condition

Game kết thúc khi Dino va chạm:

- Cactus
- Bird

Thiết bị sẽ gửi lệnh:


CMD_I_DIED


Thiết bị còn lại hiển thị:


YOU WIN


Thiết bị thua hiển thị:


YOU LOSE


---

# System Architecture

Game được xây dựng theo **Event Driven Architecture**.

Các thành phần chính:

| Component | Role |
|----------|------|
| Task | Nhận và xử lý message |
| Signal | Nội dung message |
| Message | Gửi công việc giữa các task |
| Handler | Xử lý logic |

---

# Game Loop

Game chạy theo chu kỳ **10ms Timer Tick**

Flow:
Timer Tick
↓
Read Buttons
↓
RF Receive
↓
Update Game Logic
↓
Collision Detection
↓
Render OLED
↓
Restart Timer

---

# Data Structures

## Dino Object

```c
typedef struct {
    int16_t y;
    int16_t v_y;
    bool is_jumping;
    bool is_ducking;
} dino_t;

Game Object
typedef struct {
    int32_t x;
    int16_t y;
    uint8_t w;
    uint8_t h;
    uint8_t type;
    bool active;
} game_obj_t;

Game Variables
dino_t dino;
game_obj_t objects[4];
bg_obj_t bgs[1];
RF Communication
Send Command
void rf_send_cmd(uint8_t cmd) {
    uint8_t tx_buf[1] = {cmd};

    nRF24_TXMode(...);
    nRF24_TXPacket(tx_buf,1);

    rf_mode_rx();
}
Receive Command
uint8_t rx_data;

if(nRF24_RXPacket(&rx_data,1)) {

    if(rx_data == CMD_START)
        dino_reset();

    if(rx_data == CMD_I_DIED)
        mp_state = MP_WIN;

    if(rx_data == CMD_ATTACK)
        attack_timer = 300;
}

Graphics

Game sử dụng bitmap graphics hiển thị trên OLED.

| Bitmap | Size  |
| ------ | ----- |
| Dino   | 16x16 |
| Cactus | 8x16  |
| Bird   | 16x8  |
| Gift   | 8x8   |
| Cloud  | 16x8  |


Render Screen
void view_scr_dino_game() {

    view_render.clear();

    draw_background();
    draw_objects();
    draw_dino();
    draw_score();

    view_render.update();
}

Sound System

Buzzer hoạt động theo chế độ Non-blocking.
| Sound         | Description     |
| ------------- | --------------- |
| tones_cc      | Khi ăn Gift     |
| tones_startup | Khi bị tấn công |
| tones_3beep   | Game Over       |


Project Structure

Multiplayer-Dino-Game
│
├── src
│   ├── main.c
│   ├── game.c
│   ├── rf.c
│   ├── display.c
│
├── inc
│   ├── game.h
│   ├── rf.h
│   ├── display.h
│
├── assets
│   ├── bitmap_dino.h
│   ├── bitmap_cactus.h
│
└── README.md