#include "button.h"
#include "sys_dbg.h"
#include "app.h"
#include "app_bsp.h"
#include "app_dbg.h"
#include "app_if.h"
#include "task_list.h"
#include "scr_archery_game.h"

extern bool is_kame_active;
extern uint8_t kame_timer;
extern uint8_t ar_mana;
extern uint8_t ar_game_state;

button_t btn_mode;
button_t btn_up;
button_t btn_down;

void btn_mode_callback(void* b) {
    button_t* me_b = (button_t*)b;
    switch (me_b->state) {
    case BUTTON_SW_STATE_PRESSED: {
        APP_DBG("[btn_mode_callback] BUTTON_SW_STATE_PRESSED\n");
    }
        break;

    case BUTTON_SW_STATE_LONG_PRESSED: {
        APP_DBG("[btn_mode_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
        /* LOGIC KAMEHAMEHA Ở ĐÂY */
        if (ar_game_state != GAME_OFF && ar_mana >= 5 && !is_kame_active) {
            is_kame_active = true;
            kame_timer = 0;
            ar_mana = 0;
            BUZZER_PlayTones(tones_startup); // Phát âm báo sạc chưởng
        } else {
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_MODE_LONG_PRESSED);
        }
    }
        break;

    case BUTTON_SW_STATE_RELEASED: {
        APP_DBG("[btn_mode_callback] BUTTON_SW_STATE_RELEASED\n");
        if (ar_game_state != GAME_OFF) {
            /* Nếu đang bắn chưởng thì khóa đạn thường */
            if (!is_kame_active) {
                task_post_pure_msg(AR_GAME_ARROW_ID, AR_GAME_ARROW_SHOOT);
            }
        }
        else {
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_MODE_RELEASED);
        }
    }
        break;

    default:
        break;
    }
}

void btn_up_callback(void* b) {
    button_t* me_b = (button_t*)b;
    switch (me_b->state) {
    case BUTTON_SW_STATE_PRESSED: {
        APP_DBG("[btn_up_callback] BUTTON_SW_STATE_PRESSED\n");
    }
        break;

    case BUTTON_SW_STATE_LONG_PRESSED: {
        APP_DBG("[btn_up_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
        task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_LONG_PRESSED);
    }
        break;

    case BUTTON_SW_STATE_RELEASED: {
        APP_DBG("[btn_up_callback] BUTTON_SW_STATE_RELEASED\n");
        if (ar_game_state != GAME_OFF) {
            task_post_pure_msg(AR_GAME_ARCHERY_ID, AR_GAME_ARCHERY_UP);
        }
        else {
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_RELEASED);
        }
    }
        break;

    default:
        break;
    }
}

void btn_down_callback(void* b) {
    button_t* me_b = (button_t*)b;
    switch (me_b->state) {
    case BUTTON_SW_STATE_PRESSED: {
        APP_DBG("[btn_down_callback] BUTTON_SW_STATE_PRESSED\n");
    }
        break;

    case BUTTON_SW_STATE_LONG_PRESSED: {
        APP_DBG("[btn_down_callback] BUTTON_SW_STATE_LONG_PRESSED\n");
        task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED);
    }   
        break;

    case BUTTON_SW_STATE_RELEASED: {
        APP_DBG("[btn_down_callback] BUTTON_SW_STATE_RELEASED\n");
        if (ar_game_state != GAME_OFF) {
            task_post_pure_msg(AR_GAME_ARCHERY_ID, AR_GAME_ARCHERY_DOWN);
        }
        else {
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_DOWN_RELEASED);
        }
    }
        break;

    default:
        break;
    }
}
