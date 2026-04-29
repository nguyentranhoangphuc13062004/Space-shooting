#include "scr_game_setting.h"
#include "app_dbg.h"

#define SIG_SETTING_ANIM 100 

ar_game_setting_t settingdata;
static uint8_t setting_location_chosse;

/* === KHAI BÁO CÁC BIẾN HIỆU ỨNG ĐỘNG CỰC CHÁY === */
#define NUM_STARS 12
static int8_t star_x[NUM_STARS];
static int8_t star_y[NUM_STARS];
static int8_t star_speed[NUM_STARS];

// Tàu vũ trụ lượn lờ
static int8_t deco_ship_y = 20;
static int8_t deco_ship_dir = 1;

// Thiên thạch bay xẹt ngang
static int16_t deco_met_x = 150;
static int8_t deco_met_y = 10;
static uint8_t deco_met_frame = 1;

static void view_scr_game_setting();

view_dynamic_t dyn_view_item_game_setting = {
	{ .item_type = ITEM_TYPE_DYNAMIC, },
	view_scr_game_setting
};

view_screen_t scr_game_setting = {
	&dyn_view_item_game_setting, ITEM_NULL, ITEM_NULL, .focus_item = 0,
};

void view_scr_game_setting() {
	view_render.clear(); 
	
	/* --- 1. VẼ MƯA SAO BĂNG --- */
	for(uint8_t i = 0; i < NUM_STARS; i++) {
		if (star_speed[i] > 2) {
			view_render.drawRect(star_x[i], star_y[i], 2, 1, WHITE); 
		} else {
			view_render.drawPixel(star_x[i], star_y[i], WHITE); 
		}
	}

	/* --- 2. VẼ TÀU VŨ TRỤ LƯỢN LỜ BÊN PHẢI --- */
    // Lấy hình ảnh tàu vũ trụ từ game ra trang trí
	view_render.drawBitmap(105, deco_ship_y, bitmap_archery_I, SIZE_BITMAP_ARCHERY_X, SIZE_BITMAP_ARCHERY_Y, WHITE);

    /* --- 3. VẼ THIÊN THẠCH BAY XẸT NGANG --- */
    if (deco_met_x < 128 && deco_met_x > -20) {
        const unsigned char* bmp = (deco_met_frame == 1) ? bitmap_meteoroid_I : ((deco_met_frame == 2) ? bitmap_meteoroid_II : bitmap_meteoroid_III);
        view_render.drawBitmap(deco_met_x, deco_met_y, bmp, SIZE_BITMAP_METEOROIDS_X, SIZE_BITMAP_METEOROIDS_Y, WHITE);
    }
	
	/* --- 4. VẼ GIAO DIỆN CHÍNH LÊN TRÊN CÙNG (CÓ VIỀN ĐEN ĐỂ KHÔNG BỊ RỐI) --- */
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	
	view_render.drawBitmap(0, setting_location_chosse - AR_GAME_SETTING_CHOSSE_ICON_AXIS_Y, chosse_icon, AR_GAME_SETTING_CHOSSE_ICON_SIZE_W, AR_GAME_SETTING_CHOSSE_ICON_SIZE_H, WHITE);
	if (settingdata.silent == 0) {
		view_render.drawBitmap(109, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*3-12, speaker_1, 7, 7, WHITE);
	} else {
		view_render.drawBitmap(109, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*3-12, speaker_2, 7, 7, WHITE);
	}
	
	view_render.fillRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, BLACK);
	view_render.drawRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, WHITE);
	view_render.fillRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, BLACK);
	view_render.drawRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, WHITE);
	view_render.fillRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*2, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, BLACK);
	view_render.drawRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*2, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, WHITE);
	view_render.fillRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*3, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, BLACK);
	view_render.drawRoundRect(AR_GAME_SETTING_FRAMES_AXIS_X, AR_GAME_SETTING_FRAMES_AXIS_Y_1 + AR_GAME_SETTING_FRAMES_STEP*3, AR_GAME_SETTING_FRAMES_SIZE_W, AR_GAME_SETTING_FRAMES_SIZE_H, AR_GAME_SETTING_FRAMES_SIZE_R, WHITE);
	
	view_render.setCursor(AR_GAME_SETTING_TEXT_AXIS_X, 5);
	view_render.print(" Lasers       ( ) ");
	view_render.setCursor(AR_GAME_SETTING_NUMBER_AXIS_X, 5);
	view_render.print(settingdata.num_arrow);    
	view_render.setCursor(AR_GAME_SETTING_TEXT_AXIS_X, 20);
	view_render.print(" Lives        ( ) ");	
	view_render.setCursor(AR_GAME_SETTING_NUMBER_AXIS_X, 20);
	view_render.print(settingdata.meteoroid_speed); 
	view_render.setCursor(AR_GAME_SETTING_TEXT_AXIS_X, 35);
	view_render.print(" Silent           ");
	view_render.setCursor(AR_GAME_SETTING_TEXT_AXIS_X + 32, 50);
	view_render.print(" EXIT ") ;
	view_render.update();
}

void scr_game_setting_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		setting_location_chosse = SETTING_ITEM_ARRDESS_1;
		eeprom_read(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingdata, sizeof(settingdata));
		if(settingdata.meteoroid_speed < 3 || settingdata.meteoroid_speed > 5) settingdata.meteoroid_speed = 3;

		/* Khởi tạo tọa độ cho các hiệu ứng */
		for(uint8_t i = 0; i < NUM_STARS; i++) {
			star_x[i] = (rand() % 120) + 10;
			star_y[i] = rand() % 64;
			star_speed[i] = (rand() % 3) + 1; 
		}
		deco_ship_y = 20; deco_ship_dir = 1;
		deco_met_x = (rand() % 100) + 150; deco_met_y = rand() % 40;
		
		timer_set(AC_TASK_DISPLAY_ID, SIG_SETTING_ANIM, 250, TIMER_PERIODIC); // Đổi thành 100ms cho mượt mà không bị lag nút nhấn
	} break;

	case SIG_SETTING_ANIM: {
		/* Di chuyển sao băng */
		for(uint8_t i = 0; i < NUM_STARS; i++) {
			star_x[i] -= star_speed[i];
			if (star_x[i] < 0) { star_x[i] = 128; star_y[i] = rand() % 64; }
		}
        /* Di chuyển tàu vũ trụ lượn lên lượn xuống */
        deco_ship_y += deco_ship_dir;
        if (deco_ship_y > 35 || deco_ship_y < 5) deco_ship_dir = -deco_ship_dir;

        /* Di chuyển thiên thạch xoay vòng bay ngang */
        deco_met_x -= 5; 
        if (++deco_met_frame > 3) deco_met_frame = 1;
        if (deco_met_x < -30) {
            deco_met_x = (rand() % 150) + 150; // Random thời gian xuất hiện lại
            deco_met_y = rand() % 40;
        }

		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE); 
	} break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		switch (setting_location_chosse) {
		case SETTING_ITEM_ARRDESS_1: {
			settingdata.num_arrow++;
			if (settingdata.num_arrow > 5) settingdata.num_arrow = 1;
		} break;
		case SETTING_ITEM_ARRDESS_2: {
			settingdata.meteoroid_speed++;
			if (settingdata.meteoroid_speed > 5) settingdata.meteoroid_speed = 3;
		} break;
		case SETTING_ITEM_ARRDESS_3: {
			settingdata.silent = !settingdata.silent;
			BUZZER_Sleep(settingdata.silent);
		} break;
		case SETTING_ITEM_ARRDESS_4: {
			settingdata.arrow_speed = 5;
			eeprom_write(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingdata, sizeof(settingdata));
			timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_SETTING_ANIM); // Dừng timer
			SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
			BUZZER_PlayTones(tones_startup);
		} break;
		default: break;
		}
	} BUZZER_PlayTones(tones_cc); break;
	
	case AC_DISPLAY_BUTTON_UP_LONG_PRESSED: {
		settingdata.num_arrow = 5; settingdata.meteoroid_speed = 5; settingdata.silent = 0;
	} BUZZER_Sleep(settingdata.silent); BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		setting_location_chosse -= STEP_SETTING_CHOSSE;
		if (setting_location_chosse == SETTING_ITEM_ARRDESS_0) setting_location_chosse = SETTING_ITEM_ARRDESS_4;
	} BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED: {
		settingdata.num_arrow = 1; settingdata.meteoroid_speed = 3; settingdata.silent = 1;
	} BUZZER_Sleep(settingdata.silent); BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		setting_location_chosse += STEP_SETTING_CHOSSE;
		if (setting_location_chosse > SETTING_ITEM_ARRDESS_4) setting_location_chosse = SETTING_ITEM_ARRDESS_1;
	} BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_SHOW_IDLE: { } break;
	default: break;
	}
}
