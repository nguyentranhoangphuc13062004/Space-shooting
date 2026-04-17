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

/* BIẾN TÍNH NĂNG MỚI */
bool is_kame_active = false;
uint8_t kame_timer = 0;
uint8_t ar_lives = 3;

/* BITMAP 3 TRÁI TIM */
static const unsigned char bitmap_heart_8x8[] = { 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00 };

static void view_scr_archery_game();

view_dynamic_t dyn_view_item_archery_game = {
    { .item_type = ITEM_TYPE_DYNAMIC },
    view_scr_archery_game
};

view_screen_t scr_archery_game = {
    &dyn_view_item_archery_game,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
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
        
        if (ar_shield_active) {
            view_render.setCursor(45, 55);
            view_render.print("SH");
        }
        view_render.setCursor(60, 55);
        view_render.print("S:");
        view_render.print(ar_game_score);
        
        if (!ar_revive_used) {
            view_render.setCursor(108, 55);
            view_render.print("R!");
        }
        
        view_render.drawLine(0, 53, 128, 53, WHITE);

        /* VẼ 3 TRÁI TIM (SINH MẠNG) */
        for (uint8_t i = 0; i < ar_lives; i++) {
            view_render.drawBitmap(95 + (i * 10), 2, bitmap_heart_8x8, 8, 8, WHITE);
        }

        view_render.drawRect(0, 0, 128, 64, 1);

        /* Vẽ archery (Tàu Vũ Trụ) */
        if (archery.visible == WHITE) {
            const unsigned char* bmp = (settingsetup.num_arrow != 0) ? bitmap_archery_I : bitmap_archery_II;
            view_render.drawBitmap(archery.x, archery.y - 10, bmp, SIZE_BITMAP_ARCHERY_X, SIZE_BITMAP_ARCHERY_Y, WHITE);
        }

        /* Vẽ khiên vòng tròn */
        if (ar_shield_active) {
            uint8_t cx = archery.x + 7;
            uint8_t cy = archery.y - 3;
            view_render.drawCircle(cx, cy, 14, WHITE);
            view_render.drawCircle(cx, cy, 13, WHITE);
        }

        /* VẼ TIA KAMEHAMEHA */
        if (is_kame_active) {
            uint8_t cx = archery.x + 18;
            uint8_t cy = archery.y - 3;
            view_render.fillCircle(cx, cy, 4, WHITE);
            view_render.fillRect(cx, cy - 2, 110, 5, WHITE);
        }

        /* Vẽ Đạn Thường */
        for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
            if (arrow[i].visible == WHITE) {
                view_render.drawBitmap(arrow[i].x, arrow[i].y,
                    bitmap_arrow, SIZE_BITMAP_ARROW_X, SIZE_BITMAP_ARROW_Y, WHITE);
            }
        }

        /* Vẽ Quái Vật (Chuột, Zombie, Thiên thạch) */
        for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
            if (meteoroid[i].visible == WHITE) {
                const unsigned char* bmp = bitmap_meteoroid_I;
                if (meteoroid[i].level == 2) bmp = bitmap_mouse;
                else if (meteoroid[i].level == 3) bmp = bitmap_zombie;
                else if (meteoroid[i].action_image == 2) bmp = bitmap_meteoroid_II;
                else if (meteoroid[i].action_image == 3) bmp = bitmap_meteoroid_III;
                view_render.drawBitmap(meteoroid[i].x, meteoroid[i].y,
                    bmp, SIZE_BITMAP_METEOROIDS_X, SIZE_BITMAP_METEOROIDS_Y, WHITE);
            }
        }

        /* Vẽ Vụ nổ (Bang) */
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

        /* Vẽ border */
        if (border.visible == WHITE) {
            view_render.drawFastVLine(border.x, AXIS_Y_BORDER_ON, AXIS_Y_BORDER_UNDER, WHITE);
        }

    } else if (ar_game_state == GAME_OVER) {
        view_render.clear();
        view_render.setTextSize(2);
        view_render.setTextColor(WHITE);
        view_render.setCursor(11, 24);
        view_render.print("GOOD LUCK");
    }
}

void ar_game_level_setup() {
    eeprom_read(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingsetup, sizeof(settingsetup));
    if (settingsetup.num_arrow == 0 || settingsetup.num_arrow > 10) settingsetup.num_arrow = 5;
    if (settingsetup.arrow_speed == 0 || settingsetup.arrow_speed > 10) settingsetup.arrow_speed = 3;
    if (settingsetup.meteoroid_speed == 0 || settingsetup.meteoroid_speed > 10) settingsetup.meteoroid_speed = 1;
}

static void ar_mana_regen() {
    ar_mana_tick++;
    if (ar_mana_tick >= 40) { 
        ar_mana_tick = 0;
        if (ar_mana < 5) ar_mana++;
    }
}

void scr_archery_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {

    case SCREEN_ENTRY: {
        ar_game_level_setup();
        ar_mana          = 5; 
        ar_shield_active = false;
        ar_revive_used   = false;
        ar_mana_tick     = 0;
        is_kame_active   = false;
        kame_timer       = 0;
        ar_lives         = 3; // Reset lại 3 tim mỗi đầu game
        
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_SETUP);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_SETUP);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_SETUP);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_SETUP);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_BORDER_SETUP);
        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 50, TIMER_PERIODIC);
        ar_game_state = GAME_PLAY;
    }
        break;

    case AR_GAME_TIME_TICK: {
        ar_mana_regen();
        if (ar_shield_active && ar_mana == 0) ar_shield_active = false;
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_UPDATE);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_RUN);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_RUN);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_DETONATOR);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_UPDATE);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_LEVEL_UP);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_CHECK_GAME_OVER);
    }
        break;

    case AR_GAME_RESET: {
        if (!ar_revive_used && ar_mana >= 5) {
            ar_revive_used   = true;
            ar_mana          = 0;
            ar_shield_active = false;
            ar_lives         = 3; // Hồi sinh lại được 3 mạng
            BUZZER_PlayTones(tones_startup);
            break;
        }
        task_post_pure_msg(AR_GAME_ARCHERY_ID,   AR_GAME_ARCHERY_RESET);
        task_post_pure_msg(AR_GAME_ARROW_ID,     AR_GAME_ARROW_RESET);
        task_post_pure_msg(AR_GAME_METEOROID_ID, AR_GAME_METEOROID_RESET);
        task_post_pure_msg(AR_GAME_BANG_ID,      AR_GAME_BANG_RESET);
        task_post_pure_msg(AR_GAME_BORDER_ID,    AR_GAME_BORDER_RESET);
        eeprom_write(EEPROM_SCORE_PLAY_ADDR, (uint8_t*)&ar_game_score, sizeof(ar_game_score));
        ar_game_score    = 0;
        ar_game_state    = GAME_OVER;
        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_EXIT_GAME, 2000, TIMER_ONE_SHOT); 
        BUZZER_PlayTones(tones_3beep);
    }
        break;

    case AR_GAME_EXIT_GAME: {
        ar_game_state = GAME_OFF;
        SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
    }
        break;

    default:
        break;
    }
}
