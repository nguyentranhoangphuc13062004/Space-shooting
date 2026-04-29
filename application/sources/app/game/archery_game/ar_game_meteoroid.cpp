#include "ar_game_meteoroid.h"
#include "ar_game_arrow.h"
#include "ar_game_bang.h"
#include "ar_game_border.h"
#include "scr_archery_game.h"

extern bool is_kame_active;
extern uint8_t ar_lives;
extern uint8_t current_level;
extern uint16_t lightning_timer; 

ar_game_meteoroid_t meteoroid[NUM_METEOROIDS];

static uint8_t move_tick = 0;

/* === BIẾN ĐẾM THỜI GIAN CHỜ (COOLDOWN) CỦA TIA SÉT === */
// 200 tick x 50ms = 10 giây.
static uint16_t lightning_cooldown = 500; 

#define SLOW_FACTOR (3)

static uint8_t get_level() {
    if (ar_game_score >= 1000) return METEOROID_LV3; 
    if (ar_game_score >= 100) return METEOROID_LV2;  
    return METEOROID_LV1;
}

static void spawn_one(uint8_t i) {
    uint8_t type_chance = rand() % 100;
    uint8_t lv = get_level();
    
    // Tỉ lệ rớt trái tim: Giảm xuống còn 8% cho đỡ bị lạm phát mạng
    if (type_chance < 8) {
        lv = 4;
    }
    // Tia sét CHỈ XUẤT HIỆN khi đồng hồ đếm ngược (cooldown) đã về 0
    else if (lightning_cooldown == 0 && type_chance < 50) { 
        // Khi đã đến giờ, cho 50% cơ hội rơi ra ngay lập tức
        lv = 5;
        // Xuất hiện xong thì khóa lại 10 giây (200 tick) + ngẫu nhiên thêm 0-5 giây nữa
        lightning_cooldown = 200 + (rand() % 100); 
    }

    meteoroid[i].x = (rand() % 60) + 140;
    
    uint8_t lane = rand() % 4; 
    meteoroid[i].y = 4 + (lane * 13);

    meteoroid[i].action_image = rand() % 3 + 1;
    meteoroid[i].level = lv;
    meteoroid[i].hp = (lv == 4 || lv == 5) ? 1 : lv; 
    meteoroid[i].visible = WHITE;
}

static void do_setup() {
    lightning_cooldown = 200; // Vào game hoặc chơi lại là khóa sét 10s đầu tiên
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) spawn_one(i);
    move_tick = 0;
}

static void do_run() {
    /* MỖI 50MS, ĐỒNG HỒ SÉT SẼ ĐẾM LÙI 1 NHỊP */
    if (lightning_cooldown > 0) {
        lightning_cooldown--;
    }

    move_tick++;
    if (move_tick < SLOW_FACTOR) return;
    move_tick = 0;
    
    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
        if (meteoroid[i].visible != WHITE) continue;
        
        meteoroid[i].x -= settingsetup.meteoroid_speed;
        
        if (current_level == 2) {
            meteoroid[i].y += (meteoroid[i].x % 12 < 6) ? 1 : -1;
        } else if (current_level == 3) {
            meteoroid[i].y += (meteoroid[i].x % 8 < 4) ? 2 : -2;
        }
        
        if (meteoroid[i].y < 2) meteoroid[i].y = 2;
        if (meteoroid[i].y > 47) meteoroid[i].y = 47;

        if (++meteoroid[i].action_image > 3) meteoroid[i].action_image = 1;
        
        if (meteoroid[i].x <= 16) {
            if (meteoroid[i].level == 4) { 
                if (ar_lives < 5) ar_lives++;
                BUZZER_PlayTones(tones_startup);
                spawn_one(i); 
                continue; 
            } 
            else if (meteoroid[i].level == 5) { 
                lightning_timer = 100; // Xả đạn 5s
                BUZZER_PlayTones(tones_startup);
                spawn_one(i); 
                continue;
            }

            if (ar_shield_active) {
                ar_shield_active = false; 
                BUZZER_PlayTones(tones_3beep);
                spawn_one(i); 
                continue; 
            }

            uint32_t penalty = meteoroid[i].level * 10;
            if (ar_game_score >= penalty) ar_game_score -= penalty;
            else {
                ar_game_score = 0; 
                if (ar_lives > 0) ar_lives--; 
            }
            
            BUZZER_PlayTones(tones_3beep); 
            spawn_one(i); 
            
            if (ar_lives == 0) {
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AR_GAME_RESET);
                return; 
            }
        }
    }
}

static void do_detonator() {
    if (is_kame_active) {
        for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
            if (meteoroid[i].visible == WHITE && meteoroid[i].x < 128) {
                if (meteoroid[i].level == 4 && ar_lives < 5) ar_lives++;
                if (meteoroid[i].level == 5) lightning_timer = 100;
                else ar_game_score += meteoroid[i].level * 10;
                spawn_one(i);
            }
        }
        return; 
    }

    for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
        if (meteoroid[i].visible != WHITE) continue;
        for (uint8_t j = 0; j < MAX_NUM_ARROW; j++) {
            if (arrow[j].visible != WHITE) continue;
            if (meteoroid[i].x >= (arrow[j].x + SIZE_BITMAP_ARROW_X)) continue;
            if ((meteoroid[i].y + 8) <= arrow[j].y) continue;
            if ((meteoroid[i].y - 1) >= arrow[j].y) continue;

            arrow[j].visible = BLACK; 
            arrow[j].x = 0;
            
            meteoroid[i].hp--;
            if (meteoroid[i].hp == 0) {
                if (meteoroid[i].level == 4) { 
                    if (ar_lives < 5) ar_lives++;
                    BUZZER_PlayTones(tones_startup);
                } 
                else if (meteoroid[i].level == 5) { 
                    lightning_timer = 100;
                    BUZZER_PlayTones(tones_startup);
                } 
                else { 
                    bang[i % NUM_BANG].visible = WHITE;
                    bang[i % NUM_BANG].x = meteoroid[i].x - 5;
                    bang[i % NUM_BANG].y = meteoroid[i].y + 2;
                    ar_game_score += meteoroid[i].level * 10;
                    settingsetup.num_arrow++;
                    BUZZER_PlayTones(tones_BUM);
                }
                spawn_one(i);
            } else {
                meteoroid[i].x += 10; 
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
