#include "ar_game_arrow.h"
#include "ar_game_archery.h"
#include "scr_archery_game.h"

extern bool is_kame_active;
extern uint8_t kame_timer;

ar_game_arrow_t arrow[MAX_NUM_ARROW];

#define AR_GAME_ARROW_SETUP() \
do { \
    for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) { \
        arrow[i].x = 0; arrow[i].y = 0; \
        arrow[i].visible = BLACK; \
        arrow[i].action_image = 1; \
    } \
} while (0);

#define AR_GAME_ARROW_RUN() \
do { \
    if (is_kame_active) { \
        kame_timer++; \
        if (kame_timer >= 100) { \
            is_kame_active = false; \
            kame_timer = 0; \
        } \
    } \
    for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) { \
        if (arrow[i].visible == WHITE) { \
            arrow[i].x += settingsetup.arrow_speed; \
            if (arrow[i].x >= MAX_AXIS_X_ARROW) { \
                arrow[i].visible = BLACK; \
                arrow[i].x = 0; \
            } \
        } \
    } \
} while(0);

#define AR_GAME_ARROW_SHOOT() \
do { \
    if (is_kame_active) break; \
    if (ar_mana < AR_MANA_COST_SHOOT) { \
        BUZZER_PlayTones(tones_3beep); \
        break; \
    } \
    for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) { \
        if (arrow[i].visible == BLACK) { \
            ar_mana -= AR_MANA_COST_SHOOT; \
            arrow[i].visible = WHITE; \
            arrow[i].y = archery.y - 5; \
            BUZZER_PlayTones(tones_cc); \
            break; \
        } \
    } \
} while(0);

#define AR_GAME_ARROW_RESET() \
do { \
    for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) { \
        arrow[i].x = 0; arrow[i].y = 0; \
        arrow[i].visible = BLACK; \
        arrow[i].action_image = 1; \
    } \
} while (0);

void ar_game_arrow_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_ARROW_SETUP:    AR_GAME_ARROW_SETUP();  break;
    case AR_GAME_ARROW_RUN:      AR_GAME_ARROW_RUN();    break;
    case AR_GAME_ARROW_SHOOT:    AR_GAME_ARROW_SHOOT();  break;
    case AR_GAME_ARROW_RESET:    AR_GAME_ARROW_RESET();  break;
    default: break;
    }
}
