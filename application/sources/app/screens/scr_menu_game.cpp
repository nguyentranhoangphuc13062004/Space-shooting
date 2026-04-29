#include "scr_menu_game.h"
#include "app_dbg.h"

/* --- KHAI BÁO BIẾN ANIMATION --- */
#define SIG_MENU_ANIM 101
#define SIG_MENU_REDRAW 102
#define NUM_STARS 15
static int8_t star_x[NUM_STARS];
static int8_t star_y[NUM_STARS];
static uint8_t anim_tick = 0;

/*****************************************************************************/
/* Variable and Struct Declaration - Menu game (GIỮ NGUYÊN GỐC) */
/*****************************************************************************/
#define STEP_MENU_CHOSSE				(22)
#define NUMBER_MENU_ITEMS				(4)
#define	SCREEN_MENU_H					(64)

#define MENU_ITEMS_ICON_COLOR() \
do { \
	menu_items_icon_color[0]	= !menu_chosse.items.is_item_1; \
	menu_items_icon_color[1]	= !menu_chosse.items.is_item_2; \
	menu_items_icon_color[2]	= !menu_chosse.items.is_item_3; \
	menu_items_icon_color[3]	= !menu_chosse.items.is_item_4; \
} while(0);

struct menu_items{
	unsigned int is_item_1 : 1;
	unsigned int is_item_2 : 1;
	unsigned int is_item_3 : 1;
	unsigned int is_item_4 : 1;
};

static char menu_items_name[NUMBER_MENU_ITEMS][20] = {
	" Space Shooting ",
	"   Setting      ",
	"   Charts       ",
	"   Exit         ",
};

static const uint8_t *menu_items_icon[NUMBER_MENU_ITEMS] = {
	bitmap_archery_I, setting_icon, chart_icon, exit_icon,
};

uint8_t menu_items_icon_size_w[NUMBER_MENU_ITEMS] = { 15, 16, 16, 15 };
uint8_t menu_items_icon_size_h[NUMBER_MENU_ITEMS] = { 15, 16, 16, 15 };
uint8_t menu_items_icon_color[NUMBER_MENU_ITEMS];

typedef struct { int screen; int location; } screen_t;

union scr_menu_t {
	uint32_t _id = 1;
	menu_items items;
};

typedef struct {
	uint8_t axis_x = 126; uint8_t axis_y = 0;
	uint8_t size_W = 3; uint8_t size_h = SCREEN_MENU_H / NUMBER_MENU_ITEMS;
} scr_menu_scroll_bar_t;

typedef struct {
	uint8_t axis_x = 0; uint8_t axis_y = 0;
	uint8_t size_w = 123; uint8_t size_h = 20; uint8_t size_r = 3;
} scr_menu_frames_t;

screen_t screen_menu;
scr_menu_t menu_chosse;
scr_menu_scroll_bar_t scroll_bar;
scr_menu_frames_t frame_white;
scr_menu_frames_t frame[3];

static void view_scr_menu_game();

view_dynamic_t dyn_view_menu = { { .item_type = ITEM_TYPE_DYNAMIC, }, view_scr_menu_game };
view_screen_t scr_menu_game = { &dyn_view_menu, ITEM_NULL, ITEM_NULL, .focus_item = 0, };

/*****************************************************************************/
/* View - Menu game (LỘT XÁC GIAO DIỆN) */
/*****************************************************************************/
void view_scr_menu_game() {
	view_render.clear(); // Xóa nền cũ

	/* 1. VẼ MƯA SAO BĂNG LÀM NỀN */
	for(uint8_t i = 0; i < NUM_STARS; i++) {
		view_render.drawPixel(star_x[i], star_y[i], WHITE);
	}

	/* 2. TITLE SIÊU TO Ở TRÊN CÙNG */
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	view_render.setCursor(35, 2);
	view_render.print("SPACE");

	/* 3. VẼ SCROLL BAR BÊN PHẢI */
	view_render.drawFastVLine(scroll_bar.axis_x + 1, 19, 45, WHITE); 
	view_render.fillRect(scroll_bar.axis_x, scroll_bar.axis_y, scroll_bar.size_W, scroll_bar.size_h, WHITE);

	/* 4. VẼ DANH SÁCH MENU MỚI */
	for (uint8_t i = 0; i < 3; i++) {
		uint8_t current_y = frame[i].axis_y;
		bool is_selected = (frame_white.axis_y == current_y);

		if (is_selected) {
			/* --- MỤC ĐANG ĐƯỢC CHỌN --- */
			// Hiệu ứng phi thuyền đẩy tới đẩy lui (Animation)
			uint8_t thrust = (anim_tick % 4 < 2) ? 1 : 0;
			view_render.drawBitmap(2 + thrust, current_y - 2, bitmap_archery_I, 15, 15, WHITE);
			
			// Highlight chữ (Chữ đen trên nền trắng)
			view_render.setTextSize(1);
			view_render.setTextColor(BLACK, WHITE);
			view_render.setCursor(22, current_y + 2);
			view_render.print(menu_items_name[screen_menu.screen + i]);
			view_render.setTextColor(WHITE, BLACK); // Trả lại màu gốc
		} else {
			/* --- MỤC BÌNH THƯỜNG --- */
			view_render.drawBitmap(4, current_y, menu_items_icon[screen_menu.screen + i], menu_items_icon_size_w[screen_menu.screen + i], menu_items_icon_size_h[screen_menu.screen + i], WHITE);
			view_render.setTextSize(1);
			view_render.setTextColor(WHITE);
			view_render.setCursor(22, current_y + 2);
			view_render.print(menu_items_name[screen_menu.screen + i]);
		}
	}
}

/*****************************************************************************/
/* Handle - Menu game */
/*****************************************************************************/
void update_menu_screen_chosse() {
	// Dịch toàn bộ menu xuống dưới 19 pixel để nhường chỗ cho Title siêu to
	frame[0].axis_y = 19;
	frame[1].axis_y = 34;
	frame[2].axis_y = 49;
	frame_white.axis_y = frame[screen_menu.location-screen_menu.screen].axis_y;
	
	menu_chosse._id = 1<<screen_menu.location;
	MENU_ITEMS_ICON_COLOR();
	
	// Cập nhật lại thanh cuộn cho khớp với khu vực mới
	scroll_bar.size_h = 45 / NUMBER_MENU_ITEMS;
	scroll_bar.axis_y = 19 + (45 * screen_menu.location / NUMBER_MENU_ITEMS);
}

void screen_tran_menu() {
	// Dừng hiệu ứng sao băng trước khi chuyển màn hình
	timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_MENU_ANIM);
	switch (screen_menu.location) {
	case 0: SCREEN_TRAN(scr_archery_game_handle, &scr_archery_game); break;
	case 1: SCREEN_TRAN(scr_game_setting_handle, &scr_game_setting); break;
	case 2: SCREEN_TRAN(scr_charts_game_handle,  &scr_charts_game); break;
	case 3: SCREEN_TRAN(scr_idle_handle,         &scr_idle); break;
	default: break;
	}
}

void scr_menu_game_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		view_render.initialize();
		view_render_display_on();
		update_menu_screen_chosse();
		
		/* Khởi tạo sao băng và Animation Timer */
		for(uint8_t i = 0; i < NUM_STARS; i++) {
			star_x[i] = rand() % 128; star_y[i] = rand() % 64;
		}
		anim_tick = 0;
		timer_set(AC_TASK_DISPLAY_ID, SIG_MENU_ANIM, 100, TIMER_PERIODIC);
		
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, AC_DISPLAY_IDLE_INTERVAL, TIMER_ONE_SHOT);
	} break;

	case SIG_MENU_ANIM: {
		anim_tick++;
		for(uint8_t i = 0; i < NUM_STARS; i++) {
			star_x[i] -= (i % 2) + 1; // Sao bay tốc độ ngẫu nhiên
			if (star_x[i] < 0) { star_x[i] = 128; star_y[i] = rand() % 64; }
		}
		task_post_pure_msg(AC_TASK_DISPLAY_ID, SIG_MENU_REDRAW);
	} break;

	case SIG_MENU_REDRAW: { } break; // Dummy trigger để vẽ lại màn hình

	case AC_DISPLAY_SHOW_IDLE: {
		timer_remove_attr(AC_TASK_DISPLAY_ID, SIG_MENU_ANIM);
		SCREEN_TRAN(scr_idle_handle,&scr_idle);
	} break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		screen_tran_menu();
	} break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
		if (screen_menu.location > 0) screen_menu.location--;
		if (frame_white.axis_y == frame[0].axis_y) {
			if (screen_menu.screen > 0) screen_menu.screen--;
		}
		else if (frame_white.axis_y == frame[1].axis_y) frame_white.axis_y = frame[0].axis_y;
		else if (frame_white.axis_y == frame[2].axis_y) frame_white.axis_y = frame[1].axis_y;
		
		update_menu_screen_chosse();
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, AC_DISPLAY_IDLE_INTERVAL, TIMER_ONE_SHOT);
	} BUZZER_PlayTones(tones_cc); break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
		if (screen_menu.location < NUMBER_MENU_ITEMS-1) screen_menu.location++;
		if (frame_white.axis_y == frame[0].axis_y) frame_white.axis_y = frame[1].axis_y;
		else if (frame_white.axis_y == frame[1].axis_y) frame_white.axis_y = frame[2].axis_y;
		else if (frame_white.axis_y == frame[2].axis_y) {
			if (screen_menu.screen < NUMBER_MENU_ITEMS-3) screen_menu.screen++;
		}
		
		update_menu_screen_chosse();
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, AC_DISPLAY_IDLE_INTERVAL, TIMER_ONE_SHOT);
	} BUZZER_PlayTones(tones_cc); break;

	default: break;
	}
}
