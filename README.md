Multiplayer Space Shooter Game - Build on AK Embedded Base Kit
I. Giới thiệu

Multiplayer Space Shooter là một trò chơi chạy trên AK Embedded Base Kit (STM32L151).
Trò chơi được thiết kế nhằm giúp sinh viên thực hành các khái niệm lập trình nhúng theo mô hình Event-driven.

Trong quá trình xây dựng trò chơi, người học sẽ hiểu rõ hơn về:

UML Sequence Diagram

Task

Signal

Message

Timer

State Machine

Giao tiếp không dây RF (NRF24L01+)

Game cho phép 2 thiết bị chơi với nhau theo thời gian thực thông qua module NRF24L01+.

1.1 Phần cứng

AK Embedded Base Kit - STM32L151

Kit bao gồm các thành phần chính:

OLED Display 1.3 inch

3 Buttons

Buzzer

NRF24L01+ RF module

RS485

32MB Flash

Những phần cứng này đủ để xây dựng một game nhúng hoàn chỉnh.

1.2 Mô tả trò chơi
Gameplay

Người chơi điều khiển Spaceship để né thiên thạch và bắn kẻ địch.

Nút điều khiển
Button	Chức năng
UP	Di chuyển lên
DOWN	Di chuyển xuống
MODE	Bắn đạn
1.2.1 Các đối tượng trong game
Đối tượng	Tên	Mô tả
Tàu vũ trụ	Player	Nhân vật điều khiển
Thiên thạch	Asteroid	Chướng ngại vật
Kẻ địch	Enemy	Bắn đạn về phía người chơi
Đạn	Bullet	Đạn của người chơi
Sao	Star	Nền background
1.2.2 Cách chơi

Game sử dụng 2 thiết bị kết nối RF.

Một thiết bị được chọn làm MASTER.

MASTER nhấn MODE để bắt đầu game.

Trong game:

Nhấn UP → tàu bay lên

Nhấn DOWN → tàu bay xuống

Nhấn MODE → bắn đạn

Mục tiêu:

Né thiên thạch

Bắn kẻ địch

Sống càng lâu càng tốt

1.2.3 Cơ chế hoạt động
Cách tính điểm
Hành động	Điểm
Né asteroid	+1
Bắn trúng enemy	+3
Độ khó

Tốc độ ban đầu:

Speed = 30

Mỗi 10 điểm

Speed += 5

Tốc độ tối đa

Max speed = 60
Multiplayer Attack

Khi người chơi bắn hạ Enemy đặc biệt, hệ thống gửi lệnh:

CMD_ATTACK

Máy đối thủ sẽ:

tăng tốc game

màn hình nhấp nháy

phát âm thanh cảnh báo

trong 3 giây

Kết thúc game

Game kết thúc khi:

tàu va chạm Asteroid

bị Enemy bullet bắn trúng

Thiết bị gửi:

CMD_I_DIED

Máy còn lại hiển thị

YOU WIN

Máy thua hiển thị

YOU LOSE
II. Thiết kế hệ thống
Event Driven Architecture

Trong hệ thống:

Thành phần	Chức năng
Task	Nhận message
Signal	Nội dung công việc
Message	Thông điệp giữa các task
Handler	xử lý logic
2.1 Sequence Diagram

Luồng hoạt động chính:

1️⃣ SCREEN_ENTRY
2️⃣ GAME_START
3️⃣ GAME_PLAY
4️⃣ GAME_OVER

SCREEN ENTRY

Khởi tạo hệ thống

rf_init_hardware_kit()
rf_mode_rx()
game_reset()

Khởi tạo timer

10 ms tick
GAME PLAY

Chu kỳ mỗi 10 ms

AR_GAME_TIME_TICK

Hệ thống thực hiện

read_buttons()
rf_receive()
player_update()
object_update()
collision_check()
render_screen()

Sau đó khởi động lại timer

2.2 Thuộc tính đối tượng
Player
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
Áp dụng
player_t player
game_obj_t objects[5]
bg_obj_t stars[3]
2.3 Task
Task	Chức năng
DISPLAY_TASK	vẽ màn hình
GAME_TASK	logic game
RF_TASK	xử lý RF
SOUND_TASK	phát âm thanh
2.4 Message & Signal
Task	Signal	Chức năng
DISPLAY	SCREEN_ENTRY	khởi tạo game
DISPLAY	AR_GAME_TIME_TICK	cập nhật frame
DISPLAY	BUTTON_UP	tàu bay lên
DISPLAY	BUTTON_DOWN	tàu bay xuống
DISPLAY	BUTTON_MODE	bắn đạn
III. Logic Game
3.1 Update Player
void player_update() {

    if(btn_up.state == BUTTON_SW_STATE_PRESSED)
        player.y -= 2;

    if(btn_down.state == BUTTON_SW_STATE_PRESSED)
        player.y += 2;

    if(btn_mode.state == BUTTON_SW_STATE_PRESSED)
        player.shooting = true;
}
3.2 Collision Detection
bool hit_x =
(player_x + player_w > obj_x) &&
(player_x < obj_x + obj_w);

bool hit_y =
(player_y + player_h > obj_y) &&
(player_y < obj_y + obj_h);

Nếu xảy ra va chạm

mp_state = MP_LOSE
rf_send_cmd(CMD_I_DIED)
3.3 RF Communication

Gửi lệnh

void rf_send_cmd(uint8_t cmd) {

uint8_t tx_buf[1] = {cmd};

nRF24_TXMode(...);
nRF24_TXPacket(tx_buf,1);

rf_mode_rx();
}
Nhận lệnh
uint8_t rx_data;

if(nRF24_RXPacket(&rx_data,1)) {

    if(rx_data == CMD_START)
        game_reset();

    if(rx_data == CMD_ATTACK)
        attack_timer = 300;

    if(rx_data == CMD_I_DIED)
        mp_state = MP_WIN;
}
IV. Hiển thị đồ họa
Bitmap
Object	Size
spaceship	16x16
asteroid	8x16
enemy	16x16
bullet	4x4
star	2x2
Render Screen
void view_game_screen() {

view_render.clear();

draw_player();
draw_objects();
draw_score();

view_render.update();
}
V. Âm thanh

Buzzer sử dụng non-blocking sound

Sound	Chức năng
tones_shoot	bắn đạn
tones_hit	trúng địch
tones_attack	bị tấn công
tones_gameover	game over
VI. Kết luận

Dự án Multiplayer Space Shooter giúp sinh viên hiểu và thực hành:

Lập trình event-driven

Thiết kế state machine

Giao tiếp NRF24L01+ RF

Xử lý graphics OLED

Thiết kế game nhúng real-time

Hệ thống có thể mở rộng thêm:

nhiều loại enemy

power-up

lưu high score vào flash