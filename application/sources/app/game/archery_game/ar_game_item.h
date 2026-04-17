#ifndef __AR_GAME_ITEM_H__
#define __AR_GAME_ITEM_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "scr_archery_game.h"

#define NUM_ITEMS           (2)
#define ITEM_TYPE_HEART     (1)
#define ITEM_TYPE_THUNDER   (2)
#define SIZE_BITMAP_HEART_X (10)
#define SIZE_BITMAP_HEART_Y (9)
#define SIZE_BITMAP_THUNDER_X (8)
#define SIZE_BITMAP_THUNDER_Y (12)
#define ITEM_SPAWN_TICK     (50)   /* spawn item mỗi 50 tick (~5 giây) */

typedef struct {
    bool     visible;
    uint32_t x, y;
    uint8_t  type;   /* ITEM_TYPE_HEART hoặc ITEM_TYPE_THUNDER */
} ar_game_item_t;

extern ar_game_item_t item[NUM_ITEMS];
extern void ar_game_item_handle(ak_msg_t* msg);

#endif
