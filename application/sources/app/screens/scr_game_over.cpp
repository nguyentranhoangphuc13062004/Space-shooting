#include "scr_game_over.h"
#include "app_dbg.h"
#include "scr_archery_game.h" // Để xài ké hình ảnh phi thuyền

#define SIG_GAME_OVER_ANIM 105

/*****************************************************************************/
/* Variable Declaration - game over */
/*****************************************************************************/
static ar_game_score_t gamescore;
static uint8_t anim_tick = 0;

/* Biến cho hiệu ứng khói (Thua) */
static int8_t smoke_y[3] = {25, 20, 15};
static int8_t smoke_x[3] = {100, 105, 95};

/* Biến cho hiệu ứng pháo hoa (Rank S) */
static uint8_t fw_radius = 0;

/*****************************************************************************/
/* View - game over */
/*****************************************************************************/
static void view_scr_game_over();

view_dynamic_t dyn_view_item_game_over = {
	{ .item_type = ITEM_TYPE_DYNAMIC, },
	view_scr_game_over
};

view_screen_t scr_game_over = {
	&dyn_view_item_game_over, ITEM_NULL, ITEM_NULL, .focus_item = 0,
};

void view_scr_game_over() {
	view_render.clear(); // Nền đen vũ trụ

	/* --- 1. CHỮ GAME OVER NHẤP NHÁY --- */
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	if (anim_tick % 10 < 8) { // Chớp tắt: Sáng 8 nhịp, tắt 2 nhịp
		view_render.setCursor(11, 2);
		view_render.print("GAME OVER");
	}

	/* --- 2. HIỂN THỊ ĐIỂM SỐ --- */
	view_render.setTextSize(1);
	view_render.setCursor(5, 22);
	view_render.print("Score: ");
	view_render.print(gamescore.score_now);

	/* --- 3. HỆ THỐNG XẾP HẠNG & ANIMATION --- */
	view_render.setCursor(5, 34);
	view_render.print("Rank : ");
	
	if (gamescore.score_now >= 3000) {
		// --- RANK S : PHÁO HOA RỰC RỠ ---
		view_render.print("S - LEGEND");
		
		// Vẽ pháo hoa bung tỏa (Vòng tròn to dần)
		view_render.drawCircle(100, 25, fw_radius, WHITE);
		view_render.drawCircle(100, 25, fw_radius > 4 ? fw_radius - 4 : 0, WHITE);
		// Chớp sao nhỏ xung quanh pháo hoa
		if (anim_tick % 2 == 0) {
			view_render.drawPixel(100 - fw_radius - 2, 25 - fw_radius, WHITE);
			view_render.drawPixel(100 + fw_radius + 2, 25 + fw_radius, WHITE);
		}
	} 
	else {
		// --- RANK A, B, C : PHI THUYỀN BỐC KHÓI ---
		if (gamescore.score_now >= 2000)      view_render.print("A - SNIPER");
		else if (gamescore.score_now >= 1000) view_render.print("B - GOOD");
		else                                  view_render.print("C - NOOB");

		// Vẽ phi thuyền góc phải
		view_render.drawBitmap(95, 20, bitmap_archery_I, 15, 15, WHITE);
		// Vẽ vết nứt đứt ngang phi thuyền
		view_render.drawLine(92, 17, 112, 37, WHITE);
		
		// Vẽ hiệu ứng khói bay lên (dấu chấm bay từ đuôi tàu lên trên)
		for(uint8_t i = 0; i < 3; i++) {
			view_render.fillCircle(smoke_x[i], smoke_y[i], 1, WHITE);
		}
	}

	/* --- 4. 3 ICON CHỨC NĂNG Ở ĐÁY MÀN HÌNH --- */
	// Chuyển màu thành WHITE (1) để nổi trên nền đen
	view_render.drawBitmap(10, 	48,	icon_restart,	15,	15,	WHITE);
	view_render.drawBitmap(55, 	50,	icon_charts,	17,	15,	WHITE);
	view_render.drawBitmap(100,	48,	icon_go_home,	16,	16,	WHITE);
    
    // Gợi ý nhỏ ở đáy
    view_render.setCursor(27, 52);
    view_render.print("<<");
    view_render.setCursor(85, 52);
    view_render.print(">>");
}

/*****************************************************************************/
/* Handle - game over */
/*****************************************************************************/
void rank_ranking() {
	if (gamescore.score_now > gamescore.score_1st) {
		gamescore.score_3rd = gamescore.score_2nd;
		gamescore.score_2nd = gamescore.score_1st;
		gamescore.score_1st = gamescore.score_now;
	}
	else if (gamescore.score_now > gamescore.score_2nd) {
		gamescore.score_3rd = gamescore.score_2nd;
		gamescore.score_2nd = gamescore.score_now;
	}
	else if (gamescore.score_now > gamescore.score_3rd) {
		gamescore.score_3rd = gamescore.score_now;
	}
}

void scr_game_over_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		view_render.initialize();
		view_render_display_on();
		
		eeprom_read(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore, sizeof(gamescore));
		eeprom_read(EEPROM_SCORE_PLAY_ADDR, (uint8_t*)&gamescore.score_now, sizeof(gamescore.score_now));
		rank_ranking();

		/* KHỞI TẠO BIẾN ANIMATION VÀ TIMER BẬT MỖI 100ms */
		anim_tick = 0; fw_radius = 0;
		smoke_y[0] = 25; smoke_y[1] = 20; smoke_y[2] = 15;
		timer_set(AC_TASK_DISPLAY_ID, SIG_GAME_OVER_ANIM, 100, TIMER_PERIODIC);
	} break;

	case SIG_GAME_OVER_ANIM: {
		anim_tick++;
		
		// Cập nhật khói (bay lên)
		for(uint8_t i = 0; i < 3; i++) {
			smoke_y[i] -= 1;
			if (smoke_y[i] < 5) { // Khói bay cao quá thì reset về đuôi tàu
				smoke_y[i] = 25; 
				smoke_x[i] = (rand() % 10) + 95; // Lệch X một chút cho tự nhiên
			}
		}

		// Cập nhật pháo hoa
		fw_radius += 2;
		if (fw_radius > 20) fw_radius = 0; // Tỏa to rồi thu lại nổ tiếp

		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE); // Kích hoạt vẽ lại
	} break;

	case AC_DISPLAY_SHOW_IDLE: { } break;

	/* --- NÚT BẤM (GIỮ NGUYÊN 100% LOGIC CŨ, CHỈ THÊM LỆNH XÓA TIMER) --- */
	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore, sizeof(gamescore));
		timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_GAME_OVER_ANIM); // DỪNG TIMER
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
	} BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
		eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore, sizeof(gamescore));
		timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_GAME_OVER_ANIM); // DỪNG TIMER
		SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game );
	} BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
		eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore, sizeof(gamescore));
		timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_GAME_OVER_ANIM); // DỪNG TIMER
		SCREEN_TRAN(scr_archery_game_handle, &scr_archery_game );
	} BUZZER_PlayTones(tones_cc); break;

	default: break;
	}
}
