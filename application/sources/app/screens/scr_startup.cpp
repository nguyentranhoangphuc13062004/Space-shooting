#include "scr_startup.h"
#include "screens_bitmap.h" // Thư viện chứa hình ảnh phi thuyền, thiên thạch
#include <stdlib.h>         // Thư viện hỗ trợ hàm rand() tạo sao ngẫu nhiên

static void view_scr_startup();

view_dynamic_t dyn_view_startup = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_startup
};

view_screen_t scr_startup = {
	&dyn_view_startup,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void view_scr_startup() {
	BUZZER_PlayTones(tones_startup);
	view_render.clear();
	
	/* --- BACKGROUND VŨ TRỤ --- */
	// 1. Vẽ 30 ngôi sao lấp lánh ngẫu nhiên
	for (int i = 0; i < 30; i++) {
		view_render.drawPixel(rand() % 128, rand() % 64, WHITE);
	}

	// 2. Vẽ 2 phi thuyền (UFO) đậu 2 bên chữ SPACE
	view_render.drawBitmap(8, 16, bitmap_archery_I, 16, 16, WHITE);
	view_render.drawBitmap(104, 16, bitmap_archery_I, 16, 16, WHITE);
	
	// 3. Vẽ vài viên thiên thạch trôi nổi ở góc
	view_render.drawBitmap(25, 45, bitmap_meteoroid_I, 16, 16, WHITE);
	view_render.drawBitmap(85, 5, bitmap_meteoroid_I, 16, 16, WHITE);
	/* ------------------------- */

	// In chữ SPACE to (size 2) ở giữa
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	view_render.setCursor(35, 20);
	view_render.print("SPACE");
	
	// In chữ SHOOTING nhỏ (size 1) ở dưới
	view_render.setTextSize(1);
	view_render.setCursor(40, 40);
	view_render.print("SHOOTING");
	
	view_render.update();
}

void scr_startup_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DISPLAY_INITIAL: {
		APP_DBG_SIG("AC_DISPLAY_INITIAL\n");
		view_render.initialize();
		view_render_display_on();
		// Đứng hình khoe logo 2 giây (2000ms) rồi tự động vào Menu
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_LOGO, \
					2000, \
					TIMER_ONE_SHOT);
		
		eeprom_read(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingdata, sizeof(settingdata));
		BUZZER_Sleep(settingdata.silent);
	}
		break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
	}
		break;

	case AC_DISPLAY_SHOW_LOGO: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_LOGO\n");
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
	}
		break;

	default:
		break;
	}
}
