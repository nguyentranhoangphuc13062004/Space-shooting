<div align="center">

# 🛠️ Hướng Dẫn Phát Triển Game trên STM32L151
### Space Shooting Game · AK Embedded Base Kit

> Tài liệu này hướng dẫn bạn từ **setup môi trường** đến **tự tay modify game**,  
> áp dụng kiến trúc **Event-Driven** với Task, Signal, State Machine.

</div>

---

## 📋 Mục lục

- [I. Yêu cầu hệ thống](#i-yêu-cầu-hệ-thống)
- [II. Setup môi trường (một lần duy nhất)](#ii-setup-môi-trường)
- [III. Clone & Build lần đầu](#iii-clone--build-lần-đầu)
- [IV. Hiểu cấu trúc dự án](#iv-hiểu-cấu-trúc-dự-án)
- [V. Quy trình thêm tính năng mới](#v-quy-trình-thêm-tính-năng-mới)
- [VI. Git workflow chuẩn](#vi-git-workflow-chuẩn)
- [VII. Debug & Troubleshooting](#vii-debug--troubleshooting)
- [VIII. Tài liệu tham khảo](#viii-tài-liệu-tham-khảo)

---

## I. Yêu cầu hệ thống

| Thành phần | Yêu cầu |
|---|---|
| **OS** | Ubuntu 20.04+ / Linux (khuyến nghị) |
| **Hardware** | AK Embedded Base Kit (STM32L151) |
| **Kết nối** | ST-Link V2 hoặc USB (ak_flash) |
| **Editor** | VS Code + C/C++ Extension |
| **Toolchain** | ARM GCC (`arm-none-eabi-gcc`) |

> ⚠️ **Tại sao phải dùng Linux?**  
> Embedded firmware build dựa trên `Makefile` + ARM GCC toolchain — môi trường Linux đảm bảo tương thích tốt nhất, tránh lỗi path và encoding trên Windows.

---

## II. Setup môi trường

> Chỉ cần làm **một lần duy nhất**.

### 2.1 Cài ARM GCC Toolchain

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi make git -y

# Kiểm tra cài đặt thành công
arm-none-eabi-gcc --version
```

### 2.2 Cài VS Code

```bash
# Download .deb từ https://code.visualstudio.com/
sudo dpkg -i code_*.deb

# Cài extension C/C++
code --install-extension ms-vscode.cpptools
```

### 2.3 Cài ak_flash (flash qua USB, không cần ST-Link)

```bash
git clone https://github.com/ak-embedded-software/ak-flash.git
cd ak-flash
make
sudo cp ak_flash /usr/local/bin/
```

### 2.4 Tạo cấu trúc thư mục làm việc

```
~/Workspace/
├── Sources/        ← chứa source code các dự án
└── Tools/          ← chứa toolchain, ak-flash, ...
```

```bash
mkdir -p ~/Workspace/Sources ~/Workspace/Tools
```

---

## III. Clone & Build lần đầu

### 3.1 Clone repo về máy

```bash
cd ~/Workspace/Sources
git clone https://github.com/nguyentranhoangphuc13062004/Space-shooting.git
cd Space-shooting
git checkout space-shooting
```

### 3.2 Build firmware

```bash
make all
```

Kết quả thành công:
```
Linking...
arm-none-eabi-size application.elf
   text    data     bss     dec     hex filename
  52480     192    8192   60864    EDA0 application.elf
Build complete.
```

### 3.3 Flash lên kit

**Cách 1 — ST-Link:**
```bash
make flash
```

**Cách 2 — USB (ak_flash):**
```bash
ak_flash /dev/ttyUSB0 application.bin 0x08003000
```

> 💡 Nếu `/dev/ttyUSB0` báo lỗi permission:
> ```bash
> sudo usermod -aG dialout $USER
> # Logout và login lại
> ```

---

## IV. Hiểu cấu trúc dự án

```
Space-shooting/
├── application/
│   └── sources/
│       └── app/
│           ├── screens/            ← màn hình game (logic hiển thị)
│           │   ├── scr_game.cpp    ← màn hình gameplay chính
│           │   ├── scr_menu.cpp    ← màn hình menu
│           │   └── scr_game_over.cpp
│           ├── objects/            ← các đối tượng game
│           │   ├── ss_game_spaceship.cpp
│           │   ├── ss_game_bullet.cpp
│           │   ├── ss_game_enemy.cpp
│           │   ├── ss_game_explosion.cpp
│           │   └── ss_game_border.cpp
│           ├── task_display.cpp    ← điều phối hiển thị
│           └── app.cpp             ← điểm khởi động app
├── boot/                           ← bootloader STM32
├── resources/
│   └── images/                     ← sơ đồ UML, ảnh demo
└── Makefile
```

### Luồng hoạt động Event-Driven

```
┌─────────┐   TIME_TICK(100ms)   ┌────────┐   Signal   ┌────────────┐
│  Timer  │ ──────────────────►  │ Screen │ ─────────► │ Spaceship  │
└─────────┘                      │        │ ─────────► │ Bullet[N]  │
                                  │        │ ─────────► │ Enemy[N]   │
┌─────────┐   Button press        │        │ ─────────► │ Explosion  │
│ Button  │ ──────────────────►  │        │ ─────────► │ Border     │
└─────────┘                      └────────┘            └────────────┘
                                      ▲
                              SS_GAME_RESET
                           (khi Enemy chạm Border)
```

**Nguyên tắc quan trọng:**
- Không object nào gọi thẳng object khác
- Mọi giao tiếp qua Signal/Message
- Screen là "bộ điều phối" trung tâm

---

## V. Quy trình thêm tính năng mới

Ví dụ: **Thêm object mới — Shield (khiên bảo vệ)**

### Bước 1 — Định nghĩa struct và Signal

Tạo file `ss_game_shield.h`:

```c
#ifndef SS_GAME_SHIELD_H
#define SS_GAME_SHIELD_H

#include <stdint.h>
#include <stdbool.h>

// Struct mô tả trạng thái Shield
typedef struct {
    bool visible;
    uint32_t x, y;
    uint8_t durability;     // độ bền: 0 = destroyed
    uint8_t action_image;
} ss_game_shield_t;

extern ss_game_shield_t shield;

// Handler nhận Signal
void ss_game_shield_handle(ak_msg_t* msg);

// Hàm display (gọi từ task_display)
void ss_game_shield_display(void);

#endif
```

### Bước 2 — Thêm Signal vào danh sách

Trong file `ss_game_signal.h`, thêm:

```c
// Signal cho Shield
SS_GAME_SHIELD_SETUP,
SS_GAME_SHIELD_UPDATE,
SS_GAME_SHIELD_HIT,       // khi bị enemy tấn công
SS_GAME_SHIELD_RESET,
```

### Bước 3 — Implement handler

Tạo file `ss_game_shield.cpp`:

```c
#include "ss_game_shield.h"

ss_game_shield_t shield;

#define SHIELD_INITIAL_DURABILITY   3
#define AXIS_X_SHIELD               15
#define AXIS_Y_SHIELD               32

#define SS_GAME_SHIELD_SETUP() \
do { \
    shield.x = AXIS_X_SHIELD; \
    shield.y = AXIS_Y_SHIELD; \
    shield.visible = WHITE; \
    shield.durability = SHIELD_INITIAL_DURABILITY; \
    shield.action_image = 1; \
} while(0)

#define SS_GAME_SHIELD_RESET() \
do { \
    shield.visible = BLACK; \
    shield.durability = 0; \
} while(0)

void ss_game_shield_handle(ak_msg_t* msg) {
    switch (msg->sig) {

    case SS_GAME_SHIELD_SETUP:
        SS_GAME_SHIELD_SETUP();
        break;

    case SS_GAME_SHIELD_HIT:
        if (shield.durability > 0) {
            shield.durability--;
            BUZZER_PlayTones(tones_shield_hit);
        }
        if (shield.durability == 0) {
            shield.visible = BLACK;
        }
        break;

    case SS_GAME_SHIELD_UPDATE:
        // Cập nhật animation
        shield.action_image = (shield.action_image % 2) + 1;
        break;

    case SS_GAME_SHIELD_RESET:
        SS_GAME_SHIELD_RESET();
        break;

    default:
        break;
    }
}
```

### Bước 4 — Gửi Signal từ Screen

Trong `scr_game.cpp`, thêm vào `SCREEN_ENTRY`:

```c
case SCREEN_ENTRY: {
    // ... các setup cũ ...
    task_post_pure_msg(SS_GAME_SHIELD_TASK, SS_GAME_SHIELD_SETUP);  // ← thêm dòng này
    break;
}
```

Và trong TIME_TICK handler:

```c
case SS_GAME_TIME_TICK: {
    // ... các update cũ ...
    task_post_pure_msg(SS_GAME_SHIELD_TASK, SS_GAME_SHIELD_UPDATE); // ← thêm dòng này
    break;
}
```

### Bước 5 — Thêm vào Makefile

Trong `Makefile.mk`:

```makefile
OBJECTS += ss_game_shield.o
```

### Bước 6 — Thêm display

Trong `task_display.cpp`:

```c
void view_scr_space_shooting_game() {
    if (ss_game_status == GAME_ON) {
        ss_game_frame_display();
        ss_game_spaceship_display();
        ss_game_shield_display();       // ← thêm vào đây
        ss_game_bullet_display();
        ss_game_enemy_display();
        ss_game_explosion_display();
        ss_game_border_display();
    }
}
```

> ✅ **Đây là sức mạnh của Event-Driven:** thêm object mới không cần đụng logic cũ, chỉ cần thêm Signal và kết nối vào Screen.

---

## VI. Git Workflow chuẩn

### Quy tắc commit message

```
<type>: <mô tả ngắn gọn>

type:
  feat     → thêm tính năng mới
  fix      → sửa bug
  docs     → cập nhật tài liệu
  refactor → cải thiện code, không thêm tính năng
  chore    → cấu hình, Makefile, ...
```

**Ví dụ thực tế:**
```bash
# ❌ Không nên
git commit -m "update"
git commit -m "fix bug"
git commit -m "abc"

# ✅ Nên làm
git commit -m "feat: add shield object with 3-hit durability"
git commit -m "fix: enemy spawn position out of screen boundary"
git commit -m "docs: add shield implementation guide to GUIDE.md"
git commit -m "refactor: extract collision detection to separate function"
```

### Push code lên GitHub

```bash
# Kiểm tra thay đổi trước khi commit
git status
git diff

# Stage và commit
git add .
git commit -m "feat: add shield object"

# Push lên branch của bạn
git push origin space-shooting
```

### Làm việc với branch (khi thêm tính năng lớn)

```bash
# Tạo branch mới cho tính năng
git checkout -b feature/shield-object

# ... code, test ...

# Merge vào branch chính khi hoàn thành
git checkout space-shooting
git merge feature/shield-object
git push origin space-shooting
```

---

## VII. Debug & Troubleshooting

### Lỗi thường gặp

| Lỗi | Nguyên nhân | Cách fix |
|---|---|---|
| `arm-none-eabi-gcc: not found` | Chưa cài toolchain | `sudo apt install gcc-arm-none-eabi` |
| `Permission denied /dev/ttyUSB0` | Chưa add user vào group | `sudo usermod -aG dialout $USER` rồi logout/login |
| `fatal: Invalid username or token` | Dùng password thay vì PAT | Tạo Personal Access Token tại GitHub Settings |
| Object không hiển thị | `visible` chưa set `WHITE` | Kiểm tra lại hàm SETUP |
| Game treo | Signal loop vô tận | Kiểm tra lại logic gửi Signal trong handler |

### Debug bằng UART

```c
// In log qua UART để debug (không ảnh hưởng game)
APP_DBG_SIG("SS_GAME_SHIELD_HIT - durability: %d\n", shield.durability);
```

### Kiểm tra memory

```bash
# Xem memory usage sau khi build
arm-none-eabi-size application.elf
```

---

## VIII. Tài liệu tham khảo

| Chủ đề | Link |
|---|---|
| Getting Started | [AK Embedded Base Kit - Getting Started](https://epcb.vn/blogs/ak-embedded-software/ak-embedded-base-kit-stm32l151-getting-started) |
| Event Driven: Task & Signal | [epcb.vn/blogs/ak-embedded-software](https://epcb.vn/blogs/ak-embedded-software/ak-embedded-base-kit-stm32l151-event-driven-task-signal) |
| Base kit firmware | [ak-base-kit-stm32l151](https://github.com/ak-embedded-software/ak-base-kit-stm32l151) |
| ak-flash tool | [ak-embedded-software/ak-flash](https://github.com/ak-embedded-software/ak-flash) |
| Archery Game (base project) | [ak-embedded-software/archery-game](https://github.com/ak-embedded-software/archery-game) |
| Mua AK Embedded Base Kit | [epcb.vn](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu) |

---

<div align="center">

**Được phát triển bởi [Hoàng Phúc](https://github.com/nguyentranhoangphuc13062004)**  
Electronics & Telecom @ HCMUS · Embedded Systems Engineer @ FPT Telecom

[![GitHub](https://img.shields.io/badge/GitHub-nguyentranhoangphuc13062004-181717?style=flat-square&logo=github)](https://github.com/nguyentranhoangphuc13062004)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-phuchoang1306-0A66C2?style=flat-square&logo=linkedin)](https://www.linkedin.com/in/phuchoang1306/)

*Chúc bạn lập trình vui vẻ và tạo ra những tựa game thật thú vị! 🚀*

</div>

