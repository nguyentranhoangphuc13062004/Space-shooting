#include "ar_game_item.h"
#include "ar_game_arrow.h"
#include "scr_archery_game.h"

ar_game_item_t item[NUM_ITEMS];
static uint8_t item_spawn_tick = 0;

static const uint8_t item_y_pos[NUM_ITEMS] = {15, 35};

#define AR_GAME_ITEM_SETUP() \
do { \
    for (uint8_t i = 0; i < NUM_ITEMS; i++) { \
        item[i].visible = BLACK; \
        item[i].x = 200; \
        item[i].y = item_y_pos[i]; \
        item[i].type = (i % 2 == 0) ? ITEM_TYPE_HEART : ITEM_TYPE_THUNDER; \
    } \
    item_spawn_tick = 0; \
} while (0);

/* Di chuyển item + spawn theo chu kỳ + check va chạm với arrow */
#define AR_GAME_ITEM_RUN() \
do { \
    /* Di chuyển item đang hiển thị */ \
    for (uint8_t i = 0; i < NUM_ITEMS; i++) { \
        if (item[i].visible == WHITE) { \
            item[i].x -= 1; \
            /* Bay ra ngoài màn hình → ẩn đi */ \
            if (item[i].x <= 16) { \
                item[i].visible = BLACK; \
                item[i].x = 200; \
            } \
            /* Check va chạm với arrow */ \
            for (uint8_t j = 0; j < MAX_NUM_ARROW; j++) { \
                if (arrow[j].visible == WHITE) { \
                    if (arrow[j].x >= item[i].x && \
                        arrow[j].x <= item[i].x + 10 && \
                        arrow[j].y >= item[i].y && \
                        arrow[j].y <= item[i].y + 12) { \
                        /* Ăn item! */ \
                        item[i].visible = BLACK; \
                        arrow[j].visible = BLACK; \
                        arrow[j].x = 0; \
                        item[i].x = 200; \
                        if (item[i].type == ITEM_TYPE_HEART) { \
                            /* Heart: hồi MP đầy */ \
                            ar_mana = AR_MANA_MAX; \
                            BUZZER_PlayTones(tones_startup); \
                        } else { \
                            /* Thunder: +20 điểm thay vì +10 */ \
                            ar_game_score += 20; \
                            BUZZER_PlayTones(tones_BUM); \
                        } \
                        break; \
                    } \
                } \
            } \
        } \
    } \
    /* Spawn item theo chu kỳ */ \
    item_spawn_tick++; \
    if (item_spawn_tick >= ITEM_SPAWN_TICK) { \
        item_spawn_tick = 0; \
        /* Spawn 1 item ngẫu nhiên */ \
        uint8_t idx = rand() % NUM_ITEMS; \
        if (item[idx].visible == BLACK) { \
            item[idx].visible = WHITE; \
            item[idx].x = 120; \
            item[idx].y = item_y_pos[idx]; \
            item[idx].type = (rand() % 2 == 0) ? ITEM_TYPE_HEART : ITEM_TYPE_THUNDER; \
        } \
    } \
} while(0);

#define AR_GAME_ITEM_RESET() \
do { \
    for (uint8_t i = 0; i < NUM_ITEMS; i++) { \
        item[i].visible = BLACK; \
        item[i].x = 200; \
    } \
    item_spawn_tick = 0; \
} while (0);

void ar_game_item_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_ITEM_SETUP: {
        APP_DBG_SIG("AR_GAME_ITEM_SETUP\n");
        AR_GAME_ITEM_SETUP();
    }
        break;
    case AR_GAME_ITEM_RUN: {
        APP_DBG_SIG("AR_GAME_ITEM_RUN\n");
        AR_GAME_ITEM_RUN();
    }
        break;
    case AR_GAME_ITEM_RESET: {
        APP_DBG_SIG("AR_GAME_ITEM_RESET\n");
        AR_GAME_ITEM_RESET();
    }
        break;
    default:
        break;
    }
}
