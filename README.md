# Multiplayer Space Shooting Game - AK Embedded Base Kit

## Overview

Multiplayer Space Shooting Game là một trò chơi được phát triển trên **AK Embedded Base Kit sử dụng vi điều khiển STM32L151**.  


Dự án sử dụng các kỹ thuật sau:

- Event Driven Programming
- Task & Signal Architecture
- Timer Interrupt
- State Machine
- OLED Graphics Rendering
- RF Wireless Communication (NRF24L01+)

Game hỗ trợ **2 người chơi thông qua kết nối RF** giữa hai kit.

---

# Hardware

Phần cứng sử dụng:

| Component | Description |
|-----------|-------------|
| STM32L151 | Main MCU |
| OLED 1.3" | Hiển thị đồ họa game |
| Buttons (3) | Điều khiển game |
| Buzzer | Phát âm thanh |
| NRF24L01+ | Kết nối Multiplayer |
| Flash 32MB | Lưu trữ dữ liệu |

---

# Game Objects

Các đối tượng trong game:

| Object | Name | Description |
|------|------|-------------|
| Player | Spaceship | Tàu vũ trụ của người chơi |
| Enemy | Alien | Kẻ địch bay từ phải sang |
| Obstacle | Asteroid | Thiên thạch cần né |
| Bullet | Laser | Đạn của người chơi |
| Background | Star | Hiệu ứng nền |

---

# Gameplay

Game được chơi trên **2 thiết bị**.

Một thiết bị đóng vai trò **MASTER**.

MASTER nhấn nút **MODE** để bắt đầu game.

### Control Buttons

| Button | Function |
|------|------|
| UP | Di chuyển lên |
| DOWN | Di chuyển xuống |
| MODE | Bắn đạn |

---

# Game Objective

Người chơi cần:

- Né thiên thạch
- Bắn kẻ địch
- Sống càng lâu càng tốt
- Đạt điểm cao nhất

---

# Score System

| Action | Score |
|------|------|
| Destroy enemy | +3 |
| Avoid asteroid | +1 |

---

# Difficulty System

Tốc độ ban đầu:


Speed = 30


Mỗi khi đạt **10 điểm**:


Speed += 5


Giới hạn:


Max Speed = 60


---

# Multiplayer Attack System

Khi người chơi tiêu diệt **Special Enemy**, hệ thống sẽ gửi lệnh RF:


CMD_ATTACK


Thiết bị đối thủ sẽ:

- Tăng tốc game
- Màn hình nhấp nháy
- Phát âm thanh cảnh báo

Hiệu lực trong:


3 seconds


---

# Game Over Condition

Game kết thúc khi:

- Spaceship va chạm Asteroid
- Spaceship bị Enemy bắn trúng

Thiết bị sẽ gửi lệnh:


CMD_I_DIED


Máy còn lại hiển thị:


YOU WIN


Máy thua hiển thị:


YOU LOSE


---

# System Architecture

Game được thiết kế theo **Event Driven Architecture**.

Các thành phần chính:

| Component | Description |
|----------|-------------|
| Task | Thực hiện xử lý logic |
| Signal | Nội dung của message |
| Message | Truyền thông tin giữa các task |
| Handler | Xử lý sự kiện |

---

# Game Loop

Game chạy theo chu kỳ **10ms Timer Tick**

Flow hoạt động:


Timer Tick
↓
Read Buttons
↓
RF Receive
↓
Update Player
↓
Update Objects
↓
Collision Detection
↓
Render OLED
↓
Restart Timer


---

# Data Structures

## Player Structure

```c
typedef struct {
    int16_t y;
    int16_t speed;
    bool shooting;
} player_t;
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
player_t player;
game_obj_t objects[5];
bg_obj_t stars[3];
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
        game_reset();

    if(rx_data == CMD_ATTACK)
        attack_timer = 300;

    if(rx_data == CMD_I_DIED)
        mp_state = MP_WIN;
}
Graphics

Game sử dụng bitmap graphics hiển thị trên OLED.
| Object    | Size  |
| --------- | ----- |
| Spaceship | 16x16 |
| Enemy     | 16x16 |
| Asteroid  | 12x12 |
| Bullet    | 4x4   |
| Star      | 2x2   |

Render Screen
void view_game_screen() {

    view_render.clear();

    draw_background();
    draw_player();
    draw_objects();
    draw_score();

    view_render.update();
}
Sound System

Buzzer sử dụng Non-blocking audio.

Sound	Description
tones_shoot	bắn đạn
tones_hit	bắn trúng địch
tones_attack	bị tấn công
tones_gameover	game over
Project Structure
Space-Shooting-Game
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
│   ├── bitmap_spaceship.h
│   ├── bitmap_enemy.h
│
└── README.md
│
└── README.md
