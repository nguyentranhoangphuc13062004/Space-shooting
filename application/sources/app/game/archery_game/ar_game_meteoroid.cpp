#include "ar_game_meteoroid.h"
#include "ar_game_arrow.h"
#include "ar_game_bang.h"
#include "ar_game_border.h"
#include "scr_archery_game.h"

/* GỌI BIẾN NGOÀI */
extern bool is_kame_active;
extern uint8_t ar_lives;

ar_game_meteoroid_t meteoroid[NUM_METEOROIDS];

static uint8_t move_tick = 0;
#define SLOW_FACTOR (3)

static uint8_t get_level() {
    if (ar_game_score >= SCORE_LV3) return METEOROID_LV3;
    if (ar_game_score >= SCORE_LV2) return METEOROID_LV2;
    return METEOROID_LV1;
}

static void spawn_one(uint8_t i) {
    uint8_t lv         = get_level();
    meteoroid[i].x     = (rand() % 60) + 140;
    meteoroid[i].y     = rand() % 45 + 2;
    meteoroid[i].action_image = rand() % 3 + 1;
    meteoroid[i].level = lv;
    meteoroid[i].hp    = lv;
    meteoroid[i].visible = WHITE;
}

static void do_setup() {
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) spawn_one(i);
    move_tick = 0;
}

static void do_run() {
    move_tick++;
    if (move_tick < SLOW_FACTOR) return;
    move_tick = 0;
    
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
        if (meteoroid[i].visible != WHITE) continue;
        
        meteoroid[i].x -= settingsetup.meteoroid_speed;
        if (++meteoroid[i].action_image > 3) meteoroid[i].action_image = 1;
        
        /* 1. QUÁI VẬT TÔNG VÀO TÀU (TỌA ĐỘ x <= 16) */
        if (meteoroid[i].x <= 16) {
            
            /* --- CỨU LẠI LOGIC KHIÊN CHẮN Ở ĐÂY --- */
            if (ar_shield_active) {
                ar_shield_active = false; // Vỡ khiên
                BUZZER_PlayTones(tones_3beep);
                spawn_one(i); // Quái nổ tung
                continue; // Có khiên bảo vệ thì BỎ QUA việc trừ điểm bên dưới!
            }
            /* -------------------------------------- */

            uint32_t penalty = meteoroid[i].level * 10;
            
            // Phạt điểm
            if (ar_game_score >= penalty) {
                ar_game_score -= penalty;
            } else {
                ar_game_score = 0; // Tránh điểm âm
                if (ar_lives > 0) ar_lives--; // Trừ 1 trái tim
            }
            
            BUZZER_PlayTones(tones_3beep); // Báo hiệu mất máu
            spawn_one(i); // Đẻ quái mới
            
            // Nếu hết 3 tim -> Game Over
            if (ar_lives == 0) {
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AR_GAME_RESET);
                return; // Kết thúc ngay vòng lặp để tránh lỗi
            }
        }
    }
}

static void do_detonator() {
    /* 2. KAMEHAMEHA QUÉT SẠCH QUÁI */
    if (is_kame_active) {
        for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
            if (meteoroid[i].visible == WHITE && meteoroid[i].x < 128) {
                ar_game_score += meteoroid[i].level * 10;
                spawn_one(i);
            }
        }
        return; // Thoát ngay, không xét đạn thường nữa
    }

    /* 3. VA CHẠM ĐẠN THƯỜNG */
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
        if (meteoroid[i].visible != WHITE) continue;

        for (uint8_t j = 0; j < MAX_NUM_ARROW; j++) {
            if (arrow[j].visible != WHITE) continue;
            if (meteoroid[i].x >= (arrow[j].x + SIZE_BITMAP_ARROW_X)) continue;
            if ((meteoroid[i].y + 8) <= arrow[j].y) continue;
            if ((meteoroid[i].y - 1) >= arrow[j].y) continue;

            // Xóa đạn
            arrow[j].visible = BLACK; 
            arrow[j].x = 0;
            
            // Quái mất máu
            meteoroid[i].hp--;
            if (meteoroid[i].hp == 0) {
                bang[i % NUM_BANG].visible = WHITE;
                bang[i % NUM_BANG].x = meteoroid[i].x - 5;
                bang[i % NUM_BANG].y = meteoroid[i].y + 2;
                ar_game_score += meteoroid[i].level * 10;
                settingsetup.num_arrow++;
                BUZZER_PlayTones(tones_BUM);
                spawn_one(i);
            } else {
                meteoroid[i].x += 10; // Đẩy lùi quái
                BUZZER_PlayTones(tones_cc);
            }
            break;
        }
    }
}

static void do_reset() {
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) meteoroid[i].visible = BLACK;
    move_tick = 0;
}

void ar_game_meteoroid_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_METEOROID_SETUP:      do_setup();      break;
    case AR_GAME_METEOROID_RUN:        do_run();        break;
    case AR_GAME_METEOROID_DETONATOR:  do_detonator();  break;
    case AR_GAME_METEOROID_RESET:      do_reset();      break;
    default: break;
    }
}
