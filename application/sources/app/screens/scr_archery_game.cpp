#include <stdint.h>
#include "scr_archery_game.h"
#include "screens.h"
#include "app_dbg.h"
#include "app_if.h"

/* === BIẾN TOÀN CỤC === */
uint8_t  ar_game_state = GAME_OFF;
uint8_t  ar_mana       = 5;
bool     ar_shield_active = false;
bool     ar_revive_used   = false;
uint8_t  ar_mana_tick     = 0;
ar_game_setting_t settingsetup;

/* BIẾN TÍNH NĂNG MỚI & ITEMS */
bool is_kame_active = false;
uint8_t kame_timer = 0;
uint8_t ar_lives = 3; 
uint16_t lightning_timer = 0; // Bộ đếm 5 giây cho tia sét

/* BIẾN LEVEL & HIỆU ỨNG */
uint8_t current_level = 1;
bool is_level_animating = false;
uint8_t anim_tick = 0;
uint8_t game_over_tick = 0; 

/* BITMAP 3 TRÁI TIM */
static const unsigned char bitmap_heart_8x8[] = { 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00 };

static void view_scr_archery_game();

view_dynamic_t dyn_view_item_archery_game = {
    { .item_type = ITEM_TYPE_DYNAMIC },
    view_scr_archery_game
};

view_screen_t scr_archery_game = {
    &dyn_view_item_archery_game, ITEM_NULL, ITEM_NULL, .focus_item = 0,
};

void view_scr_archery_game() {
    if (ar_game_state == GAME_PLAY) {
        /* HUD - Thanh trạng thái */
        view_render.setTextSize(1);
        view_render.setTextColor(WHITE);
        view_render.setCursor(2, 55);
        view_render.print("MP:");
        uint8_t fw = (uint8_t)((ar_mana * 18) / 5); 
        view_render.drawRect(22, 56, 20, 6, WHITE);
        if (fw > 0) view_render.fillRect(23, 57, fw > 18 ? 18 : fw, 4, WHITE);
        
        if (ar_shield_active) { view_render.setCursor(45, 55); view_render.print("SH"); }
        view_render.setCursor(60, 55);
        view_render.print("S:"); view_render.print(ar_game_score);
        
        if (!ar_revive_used) { view_render.setCursor(108, 55); view_render.print("R!"); }
        view_render.drawLine(0, 53, 128, 53, WHITE);

        /* VẼ TRÁI TIM (LIVES) */
        for (uint8_t i = 0; i < ar_lives; i++) {
            view_render.drawBitmap(95 + (i * 10), 2, bitmap_heart_8x8, 8, 8, WHITE);
        }
        view_render.drawRect(0, 0, 128, 64, 1);

        /* VẼ ĐƯỜNG PHÂN LÀN (LANE) */
        for (uint8_t y_line = 14; y_line <= 40; y_line += 13) {
            for (uint8_t x_dot = 20; x_dot < 128; x_dot += 4) {
                view_render.drawPixel(x_dot, y_line, WHITE);
            }
        }

        /* Vẽ Tàu & Khiên */
        if (archery.visible == WHITE) {
            const unsigned char* bmp = (settingsetup.num_arrow != 0) ? bitmap_archery_I : bitmap_archery_II;
            view_render.drawBitmap(archery.x, archery.y - 10, bmp, SIZE_BITMAP_ARCHERY_X, SIZE_BITMAP_ARCHERY_Y, WHITE);
        }
        if (ar_shield_active) {
            uint8_t cx = archery.x + 7; uint8_t cy = archery.y - 3;
            view_render.drawCircle(cx, cy, 14, WHITE);
            view_render.drawCircle(cx, cy, 13, WHITE);
        }

        /* VẼ TIA KAMEHAMEHA */
        if (is_kame_active) {
            uint8_t cx = archery.x + 18; uint8_t cy = archery.y - 3;
            view_render.fillCircle(cx, cy, 4, WHITE);
            view_render.fillRect(cx, cy - 2, 110, 5, WHITE);
        }

        /* Vẽ Đạn Thường */
        for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
            if (arrow[i].visible == WHITE) {
                view_render.drawBitmap(arrow[i].x, arrow[i].y, bitmap_arrow, SIZE_BITMAP_ARROW_X, SIZE_BITMAP_ARROW_Y, WHITE);
            }
        }

        /* VẼ QUÁI VẬT & ITEMS */
        for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
            if (meteoroid[i].visible == WHITE) {
                if (meteoroid[i].level == 4) {
                    // Item: Trái Tim
                    view_render.drawBitmap(meteoroid[i].x, meteoroid[i].y, bitmap_heart_8x8, 8, 8, WHITE);
                } else if (meteoroid[i].level == 5) {
                    // Item: Tia Sét (Tự vẽ bằng đường thẳng)
                    uint8_t lx = meteoroid[i].x + 4; uint8_t ly = meteoroid[i].y;
                    view_render.drawLine(lx, ly, lx-3, ly+4, WHITE);
                    view_render.drawLine(lx-3, ly+4, lx+2, ly+4, WHITE);
                    view_render.drawLine(lx+2, ly+4, lx-2, ly+8, WHITE);
                } else {
                    // Quái vật bình thường
                    const unsigned char* bmp = bitmap_meteoroid_I;
                    if (meteoroid[i].level == 2) bmp = bitmap_mouse;
                    else if (meteoroid[i].level == 3) bmp = bitmap_zombie;
                    else if (meteoroid[i].action_image == 2) bmp = bitmap_meteoroid_II;
                    else if (meteoroid[i].action_image == 3) bmp = bitmap_meteoroid_III;
                    view_render.drawBitmap(meteoroid[i].x, meteoroid[i].y, bmp, SIZE_BITMAP_METEOROIDS_X, SIZE_BITMAP_METEOROIDS_Y, WHITE);
                }
            }
        }

        /* Vẽ Vụ nổ & Biên giới */
        for (uint8_t i = 0; i < NUM_BANG; i++) {
            if (bang[i].visible == WHITE) {
                const unsigned char* bmp = bitmap_bang_I;
                if (bang[i].action_image == 2) bmp = bitmap_bang_II;
                else if (bang[i].action_image == 3) bmp = bitmap_bang_III;
                int bx = bang[i].x + (bang[i].action_image == 3 ? 2 : 0);
                int by = bang[i].y + (bang[i].action_image == 3 ? -1 : 0);
                int bw = (bang[i].action_image == 3) ? SIZE_BITMAP_BANG_II_X : SIZE_BITMAP_BANG_I_X;
                int bh = (bang[i].action_image == 3) ? SIZE_BITMAP_BANG_II_Y : SIZE_BITMAP_BANG_I_Y;
                view_render.drawBitmap(bx, by, bmp, bw, bh, WHITE);
            }
        }
        if (border.visible == WHITE) view_render.drawFastVLine(border.x, AXIS_Y_BORDER_ON, AXIS_Y_BORDER_UNDER, WHITE);

        /* HIỆU ỨNG LEVEL NHẢY MÚA */
        if (is_level_animating) {
            int8_t jump_y = (anim_tick % 10 < 5) ? 20 : 25; 
            view_render.setTextSize(2);
            view_render.setTextColor(WHITE);
            view_render.setCursor(40, jump_y);
            view_render.print("LV ");
            view_render.print(current_level);
        }

    } else if (ar_game_state == GAME_OVER) {
        view_render.clear();
        view_render.setTextColor(WHITE);
        
        if (ar_game_score >= 3000) {
            view_render.setTextSize(2);
            view_render.setCursor(20, 20);
            view_render.print("FINISH!");
            view_render.setTextSize(1);
            view_render.setCursor(25, 45);
            view_render.print("Total: ");
            view_render.print(ar_game_score);
        } else {
            view_render.setTextSize(2);
            view_render.setCursor(11, 10);
            view_render.print("GOOD LUCK");

            uint8_t ax = 85; 
            uint8_t ay = 45; 
            view_render.drawCircle(ax, ay, 8, WHITE); 
            view_render.fillCircle(ax-3, ay-2, 2, WHITE); 
            view_render.fillCircle(ax+3, ay-2, 2, WHITE); 
            view_render.drawLine(ax, ay-8, ax, ay-12, WHITE); 
            view_render.drawCircle(ax, ay-14, 2, WHITE); 
            view_render.drawRect(ax-3, ay+4, 6, 2, WHITE); 
            
            if (game_over_tick % 4 < 2) view_render.drawLine(ax-9, ay+2, ax-15, ay-5, WHITE); 
            else                        view_render.drawLine(ax-9, ay+2, ax-15, ay+2, WHITE); 

            uint8_t hy = 35 - (game_over_tick % 25); 
            uint8_t hx = ax - 30; 
            view_render.drawBitmap(hx, hy, bitmap_heart_8x8, 8, 8, WHITE);

            view_render.setTextSize(1);
            view_render.setCursor(10, 45);
            if (game_over_tick % 6 < 3) view_render.print("Try again!");
        }
    }
}

void ar_game_level_setup() {
    eeprom_read(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingsetup, sizeof(settingsetup));
    if (settingsetup.num_arrow == 0 || settingsetup.num_arrow > 10) settingsetup.num_arrow = 5;
    if (settingsetup.arrow_speed == 0 || settingsetup.arrow_speed > 10) settingsetup.arrow_speed = 3;
}

static void ar_mana_regen() {
    ar_mana_tick++;
    if (ar_mana_tick >= 40) { ar_mana_tick = 0; if (ar_mana < 5) ar_mana++; }
}

void scr_archery_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY: {
        ar_game_level_setup();
        if(settingsetup.meteoroid_speed >= 3 && settingsetup.meteoroid_speed <= 5) ar_lives = settingsetup.meteoroid_speed;
        else ar_lives = 3;
        settingsetup.meteoroid_speed = 1; 

        ar_mana = 5; ar_shield_active = false; ar_revive_used = false; ar_mana_tick = 0;
        is_kame_active = false; kame_timer = 0; lightning_timer = 0;
        current_level = 1; is_level_animating = false; anim_tick = 0; 
        
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_SETUP);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_SETUP);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_SETUP);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_SETUP);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_BORDER_SETUP);
        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 50, TIMER_PERIODIC);
        ar_game_state = GAME_PLAY;
    } break;

    case AR_GAME_TIME_TICK: {
        if (ar_game_state == GAME_OVER) {
            game_over_tick++;
            if (game_over_tick > 30) { 
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AR_GAME_EXIT_GAME);
                timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
            }
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE); 
            break; 
        }

        /* === LOGIC XẢ ĐẠN 4 LÀN KHI ĂN SÉT (5 GIÂY) === */
        if (lightning_timer > 0) {
            lightning_timer--;
            // Mỗi 10 tick (0.5 giây) xả 1 loạt 4 viên đạn
            if (lightning_timer % 10 == 0) {
                for (uint8_t lane = 0; lane < 4; lane++) {
                    for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
                        if (arrow[i].visible == BLACK) { // Tìm viên đạn rảnh
                            arrow[i].visible = WHITE;
                            arrow[i].x = archery.x + 10;
                            arrow[i].y = 4 + (lane * 13) + 4; // Bắn ra giữa làn
                            break; 
                        }
                    }
                }
            }
        }
        /* ============================================== */

        if (is_level_animating) {
            anim_tick++;
            if (anim_tick > 40) { is_level_animating = false; anim_tick = 0; } 
        }

        if (ar_game_score >= 3000) {
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AR_GAME_RESET);
            break;
        } else if (ar_game_score >= 1000 && current_level < 3) {
            current_level = 3; is_level_animating = true; anim_tick = 0;
            settingsetup.meteoroid_speed = 4; 
            BUZZER_PlayTones(tones_startup);
        } else if (ar_game_score >= 100 && current_level < 2) {
            current_level = 2; is_level_animating = true; anim_tick = 0;
            settingsetup.meteoroid_speed = 2; 
            BUZZER_PlayTones(tones_startup);
        }

        ar_mana_regen();
        if (ar_shield_active && ar_mana == 0) ar_shield_active = false;
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_UPDATE);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_RUN);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_RUN);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_DETONATOR);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_UPDATE);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_CHECK_GAME_OVER);
    } break;

    case AR_GAME_RESET: {
        if (ar_game_score < 3000 && !ar_revive_used && ar_mana >= 5) {
            ar_revive_used = true; ar_mana = 0; ar_shield_active = false; ar_lives = 3; 
            BUZZER_PlayTones(tones_startup);
            break;
        }
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_RESET);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_RESET);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_RESET);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_RESET);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_BORDER_RESET);
        eeprom_write(EEPROM_SCORE_PLAY_ADDR, (uint8_t*)&ar_game_score, sizeof(ar_game_score));
        
        ar_game_state = GAME_OVER;
        game_over_tick = 0; 
        
        if (ar_game_score < 3000) { 
            ar_game_score = 0; BUZZER_PlayTones(tones_3beep); 
            timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 100, TIMER_PERIODIC);
        } else { 
            BUZZER_PlayTones(tones_SMB); 
            timer_set(AC_TASK_DISPLAY_ID, AR_GAME_EXIT_GAME, 3000, TIMER_ONE_SHOT);
        } 
    } break;

    case AC_DISPLAY_SHOW_IDLE: { } break;

    case AR_GAME_EXIT_GAME: {
        ar_game_state = GAME_OFF;
        SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
    } break;

    default: break;
    }
}
