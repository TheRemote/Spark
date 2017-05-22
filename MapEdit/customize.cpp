#include "main.h"
#include "extern.h"


#define CUSTOMIZE_WALL		0
#define CUSTOMIZE_BRIDGE_LR	1
#define CUSTOMIZE_FLAG		2
#define CUSTOMIZE_BUNKER	3
#define CUSTOMIZE_POLE		4
#define CUSTOMIZE_SPAWN		5
#define CUSTOMIZE_BRIDGE_UD	6
#define CUSTOMIZE_SWITCH	7
#define CUSTOMIZE_CONV_LR	8
#define CUSTOMIZE_CONV_UD	9

#define CURRENT_VERSION		((2<<16)|(1<<8)|(7))

struct draw_dimensions {
	int width;
	int height;
} ddim[] = {
	{ 4, 4 },
	{ 3, 5 },
	{ 3, 3 },
	{ 4, 4 },
	{ 3, 3 },
	{ 3, 3 },
	{ 5, 3 },
	{ 3, 3 },
	{ 4, 2 },
	{ 2, 4 }
};

HWND dlgSettings = NULL;
HWND dlgCustomize = NULL;


int hCustomizeMode = CUSTOMIZE_WALL;
int hCustomizeEntry = -1;
int hCustomizeBacks[7];
int MAX_WALL_TYPES = 0;
int MAX_BUNKER_TYPES = 0;
int MAX_BRIDGE_LR_TYPES = 0;
int MAX_BRIDGE_UD_TYPES	= 0;
int MAX_POLE_TYPES = 0;
int MAX_FLAG_TYPES = 0;
int MAX_HOLDING_PEN_TYPES = 0;
int MAX_SPAWN_TYPES = 0;
int MAX_SWITCH_TYPES = 0;
int MAX_CONV_LR_TYPES = 0;
int MAX_CONV_UD_TYPES = 0;

int **wall_data = NULL, **custom_wall_data = NULL;
int **flag_data = NULL, **custom_flag_data = NULL;
int **bunker_data = NULL, **custom_bunker_data = NULL;
int **pole_data = NULL, **custom_pole_data = NULL;
int **bridge_lr_data = NULL, **custom_bridge_lr_data = NULL;
int **bridge_ud_data = NULL, **custom_bridge_ud_data = NULL;
int **holding_pen_data = NULL, **custom_holding_pen_data = NULL;
int **spawn_data = NULL, **custom_spawn_data = NULL;
int **switch_data = NULL, **custom_switch_data = NULL;
int **conv_lr_data = NULL, **custom_conv_lr_data = NULL;
int **conv_ud_data = NULL, **custom_conv_ud_data = NULL;

#define STATIC_ENTRIES		MAX_WALL_CONNS

BOOL adding = FALSE;
int edit_data[STATIC_ENTRIES];

int templates[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, -2, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2, -2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2, -2 }
};

int static_wall_data[][STATIC_ENTRIES] = {
	{ 49, 10, 47, 51, 50, 89, 9, 7, 48, 8, 46, 11, 52, 6, 12, 13 },
	{ 54, 60, 53, 18, 59, 20, 58, 15, 56, 57, 55, 19, 17, 14, 16, 13 },
	{ 258, 259, 257, 262, 220, 177, 260, 216, 256, 219, 218, 222, 261, 217, 221, 178 },
	{ 311, 312, 310, 315, 273, 232, 313, 269, 309, 272, 271, 275, 314, 270, 274, 268 },
	{ 391, 392,390,395,353,396,393,349,389,352,351,355,394,350,354,356 },
	{ 476, 477, 475, 556, 514, 557, 554, 434, 474, 477, 476, 516, 455, 435, 515, 517 },
	{ 538, 539, 540, 537, 578, 579, 580, 577, 618, 619, 620, 617, 658, 659, 660, 13 },
	{ 505, 506, 507, 504, 545, 546, 547, 544, 585, 586, 587, 584, 625, 626, 627, 13 },
	{ 698, 699, 700, 701, 738, 739, 740, 741, 778, 779, 780, 781, 818, 819, 820, 821 },
	{ 694, 695, 696, 697, 734, 735, 736, 737, 774, 775, 776, 777, 814, 815, 816, 817 },
	{ 2724, 2725, 2726, 2727, 2764, 2765, 2766, 2767, 2804, 2805, 2806, 2807, 2844, 2845, 2846, 2847 },
	{ 3040, 3041, 3042, 3084, 3080, 3081, 3082, 3083, 3120, 3121, 3122, 3124, 3163, 3161, 3164, 3044 },
	{ 2067, 2068, 2066, 2071, 2029, 2072, 2069, 2025, 2065, 2028, 2027, 2031, 2070, 2026, 2030, 2032 },
	{ 2151, 2152, 2150, 2231, 2189, 2232, 2229, 2109, 2149, 2112, 2111, 2191, 2230, 2110, 2190, 2192 },
	{ 1988, 1989, 1987, 1992, 1950, 1985, 1990, 1946, 1986, 1949, 1948, 1952, 1991, 1947, 1951, 1945 }
};
#define DEF_MAX_WALL_TYPES		(sizeof(static_wall_data)/sizeof(int[STATIC_ENTRIES]))
int static_flag_data[][STATIC_ENTRIES] = {
	{ ANIM(0x42), ANIM(0x43), ANIM(0x44), ANIM(0x45), ANIM(0x1C), ANIM(0x47), ANIM(0x48), ANIM(0x49), ANIM(0x4A), -2, -2, -2, -2, -2, -2, -2 },
	{ ANIM(0x4b), ANIM(0x4c), ANIM(0x4d), ANIM(0x4e), ANIM(0x25), ANIM(0x50), ANIM(0x51), ANIM(0x52), ANIM(0x53), -2, -2, -2, -2, -2, -2, -2 },
	{ ANIM(0x54), ANIM(0x55), ANIM(0x56), ANIM(0x57), ANIM(0x2E), ANIM(0x59), ANIM(0x5a), ANIM(0x5b), ANIM(0x5c), -2, -2, -2, -2, -2, -2, -2 },
	{ ANIM(0x5d), ANIM(0x5e), ANIM(0x5f), ANIM(0x60), ANIM(0x41), ANIM(0x62), ANIM(0x63), ANIM(0x64), ANIM(0x65), -2, -2, -2, -2, -2, -2, -2 },
	{ ANIM(0x66), ANIM(0x67), ANIM(0x68), ANIM(0x69), ANIM(0x8C), ANIM(0x6b), ANIM(0x6c), ANIM(0x6d), ANIM(0x6e), -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_FLAG_TYPES		(sizeof(static_flag_data)/sizeof(int[STATIC_ENTRIES]))
int static_pole_data[][STATIC_ENTRIES] = {
	{ ANIM(0x42), ANIM(0x43), ANIM(0x44), ANIM(0x45), -2, ANIM(0x47), ANIM(0x48), ANIM(0x49), ANIM(0x4A), ANIM(0x1C), ANIM(0x19), ANIM(0x1A), ANIM(0x1B), ANIM(0x80), -2, -2 },
	{ ANIM(0x4b), ANIM(0x4c), ANIM(0x4d), ANIM(0x4e), -2, ANIM(0x50), ANIM(0x51), ANIM(0x52), ANIM(0x53), ANIM(0x20), ANIM(0x25), ANIM(0x22), ANIM(0x23), ANIM(0x81), -2, -2 },
	{ ANIM(0x54), ANIM(0x55), ANIM(0x56), ANIM(0x57), -2, ANIM(0x59), ANIM(0x5a), ANIM(0x5b), ANIM(0x5c), ANIM(0x28), ANIM(0x29), ANIM(0x2E), ANIM(0x2B), ANIM(0x82), -2, -2 },
	{ ANIM(0x5d), ANIM(0x5e), ANIM(0x5f), ANIM(0x60), -2, ANIM(0x62), ANIM(0x63), ANIM(0x64), ANIM(0x65), ANIM(0x3A), ANIM(0x3B), ANIM(0x3C), ANIM(0x41), ANIM(0x83), -2, -2 },
	{ ANIM(0x66), ANIM(0x67), ANIM(0x68), ANIM(0x69), -2, ANIM(0x6b), ANIM(0x6c), ANIM(0x6d), ANIM(0x6e), ANIM(0x8C), ANIM(0x8C), ANIM(0x8C), ANIM(0x8C), ANIM(0x8C), -2, -2 }
	
};
#define DEF_MAX_POLE_TYPES		(sizeof(static_pole_data)/sizeof(int[STATIC_ENTRIES]))
int static_bunker_data[][STATIC_ENTRIES] = {
	{ 345, 346, 346, 347, 385, 337, 338, 339, 385, 377, 378, 379, 425, 426, 426, 427 },
	{ 306, 307, 307, 308, 336, 337, 338, 348, 376, 377, 378, 348, 386, 387, 387, 388 },
	{ 301, 302, 302, 303, 341, 337, 338, 343, 341, 377, 378, 343, 381, 417, 418, 383 },
	{ 342, 297, 298, 344, 382, 337, 338, 384, 382, 377, 378, 384, 422, 423, 423, 424 },
    { 3129, 3130, 3130, 3131, 3169, 2885, 2886, 2887, 3169, 2925, 2926, 2927, 3209, 3210, 3210, 3211 },
	{ 3090, 3091, 3091, 3092, 2885, 2886, 2887, 3132, 2925, 2926, 2927, 3132, 3170, 3171, 3171, 3172 },
    { 3085, 3086, 3086, 3087, 3125, 2885, 2886, 3127, 3125, 2925, 2926, 3127, 3165, 2965, 2966, 3167 },
	{ 3126, 2885, 2886, 3128, 3166, 2925, 2926, 3168, 3166, 2965, 2966, 3168, 3206, 3207, 3207, 3208 }
};
#define DEF_MAX_BUNKER_TYPES	(sizeof(static_bunker_data)/sizeof(int[STATIC_ENTRIES]))
int static_bridge_lr_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2 }
};
#define DEF_MAX_BRIDGE_LR_TYPES	(sizeof(static_bridge_lr_data)/sizeof(int[STATIC_ENTRIES]))
int static_bridge_ud_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2 }
};
#define DEF_MAX_BRIDGE_UD_TYPES	(sizeof(static_bridge_ud_data)/sizeof(int[STATIC_ENTRIES]))
int static_holding_pen_data[][STATIC_ENTRIES] = {
	{ 320, 146, 186, 276, 316, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2 },
	{ ANIM(0x8B), ANIM(0x8D), ANIM(0x8E), ANIM(0x8F), ANIM(0x90), -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_HOLDING_PEN_TYPES	(sizeof(static_holding_pen_data)/sizeof(int[STATIC_ENTRIES]))
int static_spawn_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_SPAWN_TYPES			(sizeof(static_spawn_data)/sizeof(int[STATIC_ENTRIES]))
int static_switch_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_SWITCH_TYPES		(sizeof(static_switch_data)/sizeof(int[STATIC_ENTRIES]))
int static_conv_lr_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_CONV_LR_TYPES		(sizeof(static_conv_lr_data)/sizeof(int[STATIC_ENTRIES]))
int static_conv_ud_data[][STATIC_ENTRIES] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, -2, -2, -2 }
};
#define DEF_MAX_CONV_UD_TYPES		(sizeof(static_conv_ud_data)/sizeof(int[STATIC_ENTRIES]))



void load_custom_data() {
	int fd, type = 0, number = 0, n;
	char vers[3], large_buf[256];
	long version;
	
	fd = _open("custom.dat", _O_RDONLY|_O_BINARY);
	if (fd > 0) {
		_read(fd, vers, 3);
		version = (vers[2] << 16) | (vers[1] << 8) | vers[0];
		
		if (version != CURRENT_VERSION) {
			char buffer[250];
			sprintf(buffer, "custom.%d.%d.%d.old", vers[2], vers[1], vers[0]);
			_close(fd);
			rename("custom.dat", buffer);
			sprintf(buffer, "Incorrect format: \n%d\n%d", version, CURRENT_VERSION);
			MessageBox(NULL, buffer, "Custom Data", MB_OK);
		}
		else {
			
			
			while (!_eof(fd)) {
				_read(fd, &type, 1);
				_read(fd, &number, 1);
				switch (type) {
				case CUSTOMIZE_WALL:
					if (wall_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						wall_data = (int **)malloc(sizeof(int *) * number);
						MAX_WALL_TYPES = number;
						for (n = 0; n < number; n++) {
							wall_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, wall_data[n], sizeof(int) * STATIC_ENTRIES);
							
						}	  
					}
					break;
					
				case CUSTOMIZE_BRIDGE_LR:
					if (bridge_lr_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number != 0) {
						bridge_lr_data = (int **)malloc(sizeof(int *) * number);
						MAX_BRIDGE_LR_TYPES = number;
						for (n = 0; n < number; n++) {
							bridge_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, bridge_lr_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_BRIDGE_UD:
					if (bridge_ud_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						bridge_ud_data = (int **)malloc(sizeof(int *) * number);
						MAX_BRIDGE_UD_TYPES = number;
						for (n = 0; n < number; n++) {
							bridge_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, bridge_ud_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_FLAG:
					if (flag_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						flag_data = (int **)malloc(sizeof(int *) * number);
						MAX_FLAG_TYPES = number;
						for (n = 0; n < number; n++) {
							flag_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, flag_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_BUNKER:
					if (bunker_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						bunker_data = (int **)malloc(sizeof(int *) * number);
						MAX_BUNKER_TYPES = number;
						for (n = 0; n < number; n++) {
							bunker_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, bunker_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_POLE:
					if (pole_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						pole_data = (int **)malloc(sizeof(int *) * number);
						MAX_POLE_TYPES = number;
						for (n = 0; n < number; n++) {
							pole_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, pole_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_SPAWN:
					if (spawn_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						spawn_data = (int **)malloc(sizeof(int *) * number);
						MAX_SPAWN_TYPES = number;
						for (n = 0; n < number; n++) {
							spawn_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, spawn_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
					
				case CUSTOMIZE_SWITCH:
					if (switch_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						switch_data = (int **)malloc(sizeof(int *) * number);
						MAX_SWITCH_TYPES = number;
						for (n = 0; n < number; n++) {
							switch_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, switch_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;

				case CUSTOMIZE_CONV_UD:
					if (conv_ud_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						conv_ud_data = (int **)malloc(sizeof(int *) * number);
						MAX_CONV_UD_TYPES = number;
						for (n = 0; n < number; n++) {
							conv_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, conv_ud_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;

				case CUSTOMIZE_CONV_LR:
					if (conv_lr_data) {
						sprintf(large_buf, "offset: %d\ntype: %d\nnumber: %d", 
							_lseek(fd, 0, SEEK_CUR), type, number);
						MessageBox(NULL, large_buf, "error", MB_OK);
						_lseek(fd, number * sizeof(int) * STATIC_ENTRIES, SEEK_CUR);
						break;
					}
					if (number) {
						conv_lr_data = (int **)malloc(sizeof(int *) * number);
						MAX_CONV_LR_TYPES = number;
						for (n = 0; n < number; n++) {
							conv_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
							_read(fd, conv_lr_data[n], sizeof(int) * STATIC_ENTRIES);
						}
					}
					break;
				
					
				default:
					break;
				}
				
			}
		}
	}
	
	if (!wall_data) {
		wall_data = (int **)malloc(sizeof(int *) * DEF_MAX_WALL_TYPES);
		for (number = 0; number < DEF_MAX_WALL_TYPES; number++) {
			wall_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(wall_data[number], static_wall_data[number], STATIC_ENTRIES * sizeof(int));
		}
		MAX_WALL_TYPES = DEF_MAX_WALL_TYPES;
	}
	if (!bridge_lr_data) {
		bridge_lr_data = (int **)malloc(sizeof(int *) * DEF_MAX_BRIDGE_LR_TYPES);
		for (number = 0; number < DEF_MAX_BRIDGE_LR_TYPES; number++) {
			bridge_lr_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(bridge_lr_data[number], static_bridge_lr_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_BRIDGE_LR_TYPES = DEF_MAX_BRIDGE_LR_TYPES;
	}
	if (!bridge_ud_data) {
		bridge_ud_data = (int **)malloc(sizeof(int *) * DEF_MAX_BRIDGE_UD_TYPES);
		for (number = 0; number < DEF_MAX_BRIDGE_UD_TYPES; number++) {
			bridge_ud_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(bridge_ud_data[number], static_bridge_ud_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_BRIDGE_UD_TYPES = DEF_MAX_BRIDGE_UD_TYPES;
	}
	if (!conv_ud_data) {
		conv_ud_data = (int **)malloc(sizeof(int *) * DEF_MAX_CONV_UD_TYPES);
		for (number = 0; number < DEF_MAX_CONV_UD_TYPES; number++) {
			conv_ud_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(conv_ud_data[number], static_conv_ud_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_CONV_UD_TYPES = DEF_MAX_BRIDGE_UD_TYPES;
	}
	if (!conv_lr_data) {
		conv_lr_data = (int **)malloc(sizeof(int *) * DEF_MAX_CONV_LR_TYPES);
		for (number = 0; number < DEF_MAX_CONV_LR_TYPES; number++) {
			conv_lr_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(conv_lr_data[number], static_conv_lr_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_CONV_UD_TYPES = DEF_MAX_BRIDGE_UD_TYPES;
	}
	if (!pole_data) {
		pole_data = (int **)malloc(sizeof(int *) * DEF_MAX_POLE_TYPES);
		for (number = 0; number < DEF_MAX_POLE_TYPES; number++) {
			pole_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(pole_data[number], static_pole_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_POLE_TYPES = DEF_MAX_POLE_TYPES;
	}
	if (!flag_data) {
		flag_data = (int **)malloc(sizeof(int *) * DEF_MAX_FLAG_TYPES);
		for (number = 0; number < DEF_MAX_FLAG_TYPES; number++) {
			flag_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(flag_data[number], static_flag_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_FLAG_TYPES = DEF_MAX_FLAG_TYPES;
	}
	if (!bunker_data) {
		bunker_data = (int **)malloc(sizeof(int *) * DEF_MAX_BUNKER_TYPES);
		for (number = 0; number < DEF_MAX_BUNKER_TYPES; number++) {
			bunker_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(bunker_data[number], static_bunker_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_BUNKER_TYPES = DEF_MAX_BUNKER_TYPES;
	}
	if (!holding_pen_data) {
		holding_pen_data = (int **)malloc(sizeof(int *) * DEF_MAX_HOLDING_PEN_TYPES);
		for (number = 0; number < DEF_MAX_HOLDING_PEN_TYPES; number++) {
			holding_pen_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(holding_pen_data[number], static_holding_pen_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_HOLDING_PEN_TYPES = DEF_MAX_HOLDING_PEN_TYPES;
	}
	if (!spawn_data) {
		spawn_data = (int **)malloc(sizeof(int *) * DEF_MAX_SPAWN_TYPES);
		for (number = 0; number < DEF_MAX_SPAWN_TYPES; number++) {
			spawn_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(spawn_data[number], static_spawn_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_SPAWN_TYPES = DEF_MAX_SPAWN_TYPES;
	}
	if (!switch_data) {
		switch_data = (int **)malloc(sizeof(int *) * DEF_MAX_SWITCH_TYPES);
		for (number = 0; number < DEF_MAX_SWITCH_TYPES; number++) {
			switch_data[number] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(switch_data[number], static_switch_data[number], sizeof(int) * STATIC_ENTRIES);
		}
		MAX_SWITCH_TYPES = DEF_MAX_SWITCH_TYPES;
	}
	
	if ((MAX_BUNKER_TYPES % 4))
		MAX_BUNKER_TYPES = MAX(0,(MAX_BUNKER_TYPES/4 - 1)*4);
	
	if ((MAX_SPAWN_TYPES % 4))
		MAX_SPAWN_TYPES = MAX(0,(MAX_SPAWN_TYPES/4 - 1)*4);
	
	_close(fd);
}

void save_custom_data() {
	int fd = 0;
	char n;
	long cv = CURRENT_VERSION;
	char dir[256];


	GetCurrentDirectory(sizeof(dir), dir);	
	SetCurrentDirectory(arcdir);
		
	if (wall_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_WALL;
		_write(fd, &n, 1);
		n = (char)MAX_WALL_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_WALL_TYPES; n++)
			_write(fd, wall_data[n], sizeof(int) * STATIC_ENTRIES);
		
	}
	if (flag_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_FLAG;
		_write(fd, &n, 1);
		n = (char)MAX_FLAG_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_FLAG_TYPES; n++)
			_write(fd, flag_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (pole_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_POLE;
		_write(fd, &n, 1);
		n = (char)MAX_POLE_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_POLE_TYPES; n++)
			_write(fd, pole_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (bunker_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_BUNKER;
		_write(fd, &n, 1);
		n = (char)MAX_BUNKER_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_BUNKER_TYPES; n++)
			_write(fd, bunker_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (bridge_lr_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_BRIDGE_LR;
		_write(fd, &n, 1);
		n = (char)MAX_BRIDGE_LR_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_BRIDGE_LR_TYPES; n++)
			_write(fd, bridge_lr_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (bridge_ud_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_BRIDGE_UD;
		_write(fd, &n, 1);
		n = (char)MAX_BRIDGE_UD_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_BRIDGE_UD_TYPES; n++)
			_write(fd, bridge_ud_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (conv_lr_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_CONV_LR;
		_write(fd, &n, 1);
		n = (char)MAX_CONV_LR_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_CONV_LR_TYPES; n++)
			_write(fd, conv_lr_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	if (conv_ud_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_CONV_UD;
		_write(fd, &n, 1);
		n = (char)MAX_CONV_UD_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_CONV_UD_TYPES; n++)
			_write(fd, conv_ud_data[n], sizeof(int) * STATIC_ENTRIES);
	}
	
	if (spawn_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_SPAWN;
		_write(fd, &n, 1);
		n = (char)MAX_SPAWN_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_SPAWN_TYPES; n++)
			_write(fd, spawn_data[n], sizeof(int) * STATIC_ENTRIES);
		
	}
	if (switch_data) {
		if (!fd) {
			fd = _open("custom.dat", _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);
			_write(fd, &cv, 3);
		}
		n = (char)CUSTOMIZE_SWITCH;
		_write(fd, &n, 1);
		n = (char)MAX_SWITCH_TYPES;
		_write(fd, &n, 1);
		for (n = 0; n < MAX_SWITCH_TYPES; n++)
			_write(fd, switch_data[n], sizeof(int) * STATIC_ENTRIES);
		
	}
	
	if (fd) { _commit(fd); _close(fd); }

	SetCurrentDirectory(dir);
}

void synch_custom_data(BOOL reverse) {
	int n;
	
	if (reverse) { // put in custom entries
		
		free_custom_edits(TRUE);
		
		custom_wall_data = (int **)malloc(sizeof(int *) * (MAX_WALL_TYPES + 1));
		custom_bridge_lr_data = (int **)malloc(sizeof(int *) * (MAX_BRIDGE_LR_TYPES + 1));
		custom_bridge_ud_data = (int **)malloc(sizeof(int *) * (MAX_BRIDGE_UD_TYPES + 1));
		custom_conv_ud_data = (int **)malloc(sizeof(int *) * (MAX_CONV_UD_TYPES + 1));
		custom_conv_lr_data = (int **)malloc(sizeof(int *) * (MAX_CONV_LR_TYPES + 1));
		custom_flag_data = (int **)malloc(sizeof(int *) * (MAX_FLAG_TYPES + 1));
		custom_pole_data = (int **)malloc(sizeof(int *) * (MAX_POLE_TYPES + 1));
		custom_bunker_data = (int **)malloc(sizeof(int *) * (MAX_BUNKER_TYPES + 1));
		custom_spawn_data = (int **)malloc(sizeof(int *) * (MAX_SPAWN_TYPES + 1));
		custom_switch_data = (int **)malloc(sizeof(int *) * (MAX_SWITCH_TYPES + 1));		
		
		for (n = 0; n < MAX_WALL_TYPES; n++) {
			custom_wall_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_wall_data[n], wall_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_wall_data[n] = NULL;
		for (n = 0; n < MAX_FLAG_TYPES; n++) {
			custom_flag_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_flag_data[n], flag_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_flag_data[n] = NULL;
		for (n = 0; n < MAX_POLE_TYPES; n++) {
			custom_pole_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_pole_data[n], pole_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_pole_data[n] = NULL;
		for (n = 0; n < MAX_BRIDGE_LR_TYPES; n++) {
			custom_bridge_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_bridge_lr_data[n], bridge_lr_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_bridge_lr_data[n] = NULL;
		for (n = 0; n < MAX_BRIDGE_UD_TYPES; n++) {
			custom_bridge_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_bridge_ud_data[n], bridge_ud_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_bridge_ud_data[n] = NULL;
		for (n = 0; n < MAX_CONV_UD_TYPES; n++) {
			custom_conv_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_conv_ud_data[n], conv_ud_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_conv_ud_data[n] = NULL;
		for (n = 0; n < MAX_CONV_LR_TYPES; n++) {
			custom_conv_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_conv_lr_data[n], conv_lr_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_conv_lr_data[n] = NULL;
		
		
		for (n = 0; n < MAX_BUNKER_TYPES; n++) {
			custom_bunker_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_bunker_data[n], bunker_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		
		custom_bunker_data[n] = NULL;
		for (n = 0; n < MAX_SPAWN_TYPES; n++) {
			custom_spawn_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_spawn_data[n], spawn_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_spawn_data[n] = NULL;
		for (n = 0; n < MAX_SWITCH_TYPES; n++) {
			custom_switch_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
			memcpy(custom_switch_data[n], switch_data[n], sizeof(int) * STATIC_ENTRIES);
		}
		custom_switch_data[n] = NULL;
	}
	else { // set button clicked, put custom into actual data
		free_custom_data(FALSE);
		
		if (custom_wall_data) {
			for (n = 0; custom_wall_data[n]; n++);
			wall_data = (int **)malloc(sizeof(int *) * (MAX_WALL_TYPES = n));
			for (n = 0; n < MAX_WALL_TYPES; n++) {
				wall_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(wall_data[n], custom_wall_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_flag_data) {
			for (n = 0; custom_flag_data[n]; n++);
			flag_data = (int **)malloc(sizeof(int *) * (MAX_FLAG_TYPES = n));
			for (n = 0; n < MAX_FLAG_TYPES; n++) {
				flag_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(flag_data[n], custom_flag_data[n], sizeof(int) * STATIC_ENTRIES);
			}
		}
		if (custom_bridge_lr_data) {
			for (n = 0; custom_bridge_lr_data[n]; n++);
			bridge_lr_data = (int **)malloc(sizeof(int *) * (MAX_BRIDGE_LR_TYPES = n));
			for (n = 0; n < MAX_BRIDGE_LR_TYPES; n++) {
				bridge_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(bridge_lr_data[n], custom_bridge_lr_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_bridge_ud_data) {
			for (n = 0; custom_bridge_ud_data[n]; n++);
			bridge_ud_data = (int **)malloc(sizeof(int *) * (MAX_BRIDGE_UD_TYPES = n));
			for (n = 0; n < MAX_BRIDGE_UD_TYPES; n++) {
				bridge_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(bridge_ud_data[n], custom_bridge_ud_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_conv_ud_data) {
			for (n = 0; custom_conv_ud_data[n]; n++);
			conv_ud_data = (int **)malloc(sizeof(int *) * (MAX_CONV_UD_TYPES = n));
			for (n = 0; n < MAX_CONV_UD_TYPES; n++) {
				conv_ud_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(conv_ud_data[n], custom_conv_ud_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_conv_lr_data) {
			for (n = 0; custom_conv_lr_data[n]; n++);
			conv_lr_data = (int **)malloc(sizeof(int *) * (MAX_CONV_LR_TYPES = n));
			for (n = 0; n < MAX_CONV_LR_TYPES; n++) {
				conv_lr_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(conv_lr_data[n], custom_conv_lr_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		
		if (custom_pole_data) {
			for (n = 0; custom_pole_data[n]; n++);
			pole_data = (int **)malloc(sizeof(int *) * (MAX_POLE_TYPES = n));
			for (n = 0; n < MAX_POLE_TYPES; n++) {
				pole_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(pole_data[n], custom_pole_data[n], sizeof(int) * STATIC_ENTRIES);
			}
		}
		if (custom_bunker_data) {
			for (n = 0; custom_bunker_data[n]; n++);
			bunker_data = (int **)malloc(sizeof(int *) * (MAX_BUNKER_TYPES = n));
			for (n = 0; n < MAX_BUNKER_TYPES; n++) {
				bunker_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(bunker_data[n], custom_bunker_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_spawn_data) {
			for (n = 0; custom_spawn_data[n]; n++);
			spawn_data = (int **)malloc(sizeof(int *) * (MAX_SPAWN_TYPES = n));
			for (n = 0; n < MAX_SPAWN_TYPES; n++) {
				spawn_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(spawn_data[n], custom_spawn_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		if (custom_switch_data) {
			for (n = 0; custom_switch_data[n]; n++);
			switch_data = (int **)malloc(sizeof(int *) * (MAX_SWITCH_TYPES = n));
			for (n = 0; n < MAX_SWITCH_TYPES; n++) {
				switch_data[n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
				memcpy(switch_data[n], custom_switch_data[n], sizeof(int) * STATIC_ENTRIES);
			}
			
		}
		
		free_custom_edits(FALSE);
		
	}
	
}

void free_custom_data(BOOL synch) {
	int n;
	if (wall_data) {
		if (!synch || (synch && custom_wall_data)) {
			for (n = MAX_WALL_TYPES; n--; )
				if (wall_data[n]) free(wall_data[n]);
				free(wall_data);
				wall_data = NULL;
		}
	}
	if (flag_data) {
		if (!synch || (synch && custom_flag_data)) {
			for (n = MAX_FLAG_TYPES; n--; )
				if (flag_data[n]) free(flag_data[n]);
				free(flag_data);
				flag_data = NULL;
		}
	}
	if (bunker_data) {
		if (!synch || (synch && custom_flag_data)) {
			for (n = MAX_BUNKER_TYPES; n--; )
				if (bunker_data[n]) free(bunker_data[n]);
				free(bunker_data);
				bunker_data = NULL;
		}
	}
	if (pole_data) {
		if (!synch || (synch && custom_pole_data)) {
			for (n = MAX_POLE_TYPES; n--; )
				if (pole_data[n]) free(pole_data[n]);
				free(pole_data);
				pole_data = NULL;
		}
	}
	if (bridge_lr_data) {
		if (!synch || (synch && custom_bridge_lr_data)) {
			for (n = MAX_BRIDGE_LR_TYPES; n--; )
				if (bridge_lr_data[n]) free(bridge_lr_data[n]);
				free(bridge_lr_data);
				bridge_lr_data = NULL;
		}
	}
	if (bridge_ud_data) {
		if (!synch || (synch && custom_bridge_ud_data)) {
			for (n = MAX_BRIDGE_UD_TYPES; n--; )
				if (bridge_ud_data[n]) free(bridge_ud_data[n]);
				free(bridge_ud_data);
				bridge_ud_data = NULL;
		}
	}
	if (conv_ud_data) {
		if (!synch || (synch && custom_bridge_ud_data)) {
			for (n = MAX_CONV_UD_TYPES; n--; )
				if (conv_ud_data[n]) free(conv_ud_data[n]);
				free(conv_ud_data);
				conv_ud_data = NULL;
		}
	}
	if (conv_lr_data) {
		if (!synch || (synch && custom_bridge_lr_data)) {
			for (n = MAX_CONV_LR_TYPES; n--; )
				if (conv_lr_data[n]) free(conv_lr_data[n]);
				free(conv_lr_data);
				conv_lr_data = NULL;
		}
	}
	
	if (spawn_data) {
		if (!synch || (synch && custom_spawn_data)) {
			for (n = MAX_SPAWN_TYPES; n--; )
				if (spawn_data[n]) free(spawn_data[n]);
				free(spawn_data);
				spawn_data = NULL;
		}
	}
	if (switch_data) {
		if (!synch || (synch && custom_switch_data)) {
			for (n = MAX_SWITCH_TYPES; n--; )
				if (switch_data[n]) free(switch_data[n]);
				free(switch_data);
				switch_data = NULL;
		}
	}
	if (holding_pen_data && !synch) {
		for (n = MAX_HOLDING_PEN_TYPES; n--; )
			if (holding_pen_data[n]) free(holding_pen_data[n]);
			free(holding_pen_data);
			holding_pen_data = NULL;
	}
}		


void free_custom_edits(BOOL synch) {
	int n;
	
	if (custom_wall_data) {
		if (!synch || (synch && wall_data)) {
			for (n = 0; custom_wall_data[n]; n++)
				free(custom_wall_data[n]);
			free(custom_wall_data);
			custom_wall_data = NULL;
		}
	}
	if (custom_bridge_lr_data) {
		if (!synch || (synch && bridge_lr_data)) {
			for (n = 0; custom_bridge_lr_data[n]; n++)
				free(custom_bridge_lr_data[n]);
			free(custom_bridge_lr_data);
			custom_bridge_lr_data = NULL;
		}
	}
	if (custom_bridge_ud_data) {
		if (!synch || (synch && bridge_ud_data)) {
			for (n = 0; custom_bridge_ud_data[n]; n++)
				free(custom_bridge_ud_data[n]);
			free(custom_bridge_ud_data);
			custom_bridge_ud_data = NULL;
		}
	}
	if (custom_conv_lr_data) {
		if (!synch || (synch && conv_lr_data)) {
			for (n = 0; custom_conv_lr_data[n]; n++)
				free(custom_conv_lr_data[n]);
			free(custom_conv_lr_data);
			custom_conv_lr_data = NULL;
		}
	}
	if (custom_conv_ud_data) {
		if (!synch || (synch && conv_ud_data)) {
			for (n = 0; custom_conv_ud_data[n]; n++)
				free(custom_conv_ud_data[n]);
			free(custom_conv_ud_data);
			custom_conv_ud_data = NULL;
		}
	}
	
	if (custom_bunker_data) {
		if (!synch || (synch && bunker_data)) {
			for (n = 0; custom_bunker_data[n]; n++)
				free(custom_bunker_data[n]);
			free(custom_bunker_data);
			custom_bunker_data = NULL;
		}
	}
	if (custom_flag_data) {
		if (!synch || (synch && flag_data)) {
			for (n = 0; custom_flag_data[n]; n++)
				free(custom_flag_data[n]);
			free(custom_flag_data);
			custom_flag_data = NULL;
		}
	}
	if (custom_pole_data) {
		if (!synch || (synch && pole_data)) {
			for (n = 0; custom_pole_data[n]; n++)
				free(custom_pole_data[n]);
			free(custom_pole_data);
			custom_pole_data = NULL;
		}
	}
	if (custom_spawn_data) {
		if (!synch || (synch && spawn_data)) {
			for (n = 0; custom_spawn_data[n]; n++)
				free(custom_spawn_data[n]);
			free(custom_spawn_data);
			custom_spawn_data = NULL;
		}
	}
	if (custom_switch_data) {
		if (!synch || (synch && switch_data)) {
			for (n = 0; custom_switch_data[n]; n++)
				free(custom_switch_data[n]);
			free(custom_switch_data);
			custom_switch_data = NULL;
		}
	}
}

int count_custom(int **array) {
	int n;
	
	if (!array) return 0;
	
	for (n = 0; array[n]; n++);
	
	return n;
}

int remove_custom(int ***array1, int n) {
	int cnt, **array = *array1, p;
	
	cnt = count_custom(array);
	
	if (n >= cnt) return cnt;
	
	free(array[n]);
	for (p = n; p < cnt-1; p++)
		array[p] = array[p+1];
	array[cnt-1] = NULL;
	
	*array1 = (int **)realloc(*array1, sizeof(int *) * cnt);
	
	return (cnt-1);
	
}


void customize_resize(HWND dlg, int nAmount) {
	HWND ctl = GetDlgItem(dlg, ID_CUSTOMIZELIST);
	int nMax, nMin;
	RECT rc;
	
    GetClientRect(ctl, &rc);
	
	nMin = 0;
	nMax = MAX(0, nAmount*16*ddim[hCustomizeMode].height - (rc.bottom - rc.top));
	
	SetScrollRange(ctl, SB_VERT, nMin, nMax, FALSE);
	SetScrollPos(ctl, SB_VERT, MIN(nMax, 16*ddim[hCustomizeMode].height*hCustomizeEntry), TRUE);
	RedrawWindow(dlg, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
}


int CALLBACK CustomizeProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	HWND clwnd;
	
	switch(msg) {
	case WM_INITDIALOG:
		dlgSettings = (HWND)lparm;
		dlgCustomize = hwnd;
		hCustomizeMode = CUSTOMIZE_WALL;
		synch_custom_data(TRUE);
		SendDlgItemMessage(hwnd, IDC_WALL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		clwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_customizel", NULL,
			WS_CHILD|WS_VISIBLE|WS_VSCROLL, 5, 43, 350, 300, 
			hwnd, (HMENU)ID_CUSTOMIZELIST, hInstance, NULL);
		ShowWindow(clwnd, SW_SHOW);
		
		break;
		
	case WM_DESTROY:
		ShowWindow(dlgSettings, SW_SHOW);
		dlgSettings = NULL;
		free_custom_edits(FALSE);
		break;
		
	case WM_COMMAND:
		switch (HIWORD(wparm)) {
		case 1:
			{
				switch (LOWORD(wparm)) {
				case COMM_TAB:
					{
						HWND ctrl = GetFocus();
						
						SetFocus(GetNextDlgTabItem(hwnd, ctrl, FALSE));
						
						if (LOWORD(wparm) == COMM_ENTER && ctrl == GetDlgItem(hwnd, IDC_SETSET))
							return SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SETSET, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_SETSET));
						return 1;
					}
					
				default:
					break;
				}
				break;
			}
			
		case BN_CLICKED:
			{
				switch (LOWORD(wparm)) {
				case IDC_CANSET:
					DestroyWindow(hwnd);
					break;
					
				case IDC_SETSET:
					synch_custom_data(FALSE);
					DestroyWindow(hwnd);
					break;
					
				case IDC_WALL:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_WALL;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_WALL_TYPES);
					break;
					
				case IDC_BRIDGE:
					{
						HMENU popup;
						MENUITEMINFO mii;
						POINT pt;
						int result, max;
						
						hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
						
						popup = CreatePopupMenu();
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_TYPE|MIIM_ID;
						mii.fType = MFT_STRING;
						mii.dwTypeData = "Left/Right";
						mii.cch = strlen("Left/Right");
						mii.wID = 1;
						InsertMenuItem(popup, 0, TRUE, &mii);
						mii.cch = strlen("Up/Down");
						mii.dwTypeData = "Up/Down";
						mii.wID = 2;
						InsertMenuItem(popup, 1, TRUE, &mii);
						
						GetCursorPos(&pt);
						
						result = TrackPopupMenuEx(popup, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
							pt.x, pt.y, hwnd, NULL);
						
						DestroyMenu(popup);
						switch(result) {
						case 1:
							hCustomizeMode = CUSTOMIZE_BRIDGE_LR;
							max = count_custom(custom_bridge_lr_data);
							break;
						case 2:
							hCustomizeMode = CUSTOMIZE_BRIDGE_UD;
							max = count_custom(custom_bridge_ud_data);
							break;
						default:
							return 0;
						}
						
						hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
						customize_resize(hwnd, max);
						break;
					}
					
				case IDC_FLAG:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_FLAG;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_FLAG_TYPES);
					break;
					
				case IDC_POLE:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_POLE;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_POLE_TYPES);
					break;
					
				case IDC_BUNKER:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_BUNKER;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_BUNKER_TYPES);
					break;
					
				case IDC_SPAWN:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_SPAWN;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_SPAWN_TYPES);
					break;
					
				case IDC_SWITCH2:
					hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
					hCustomizeMode = CUSTOMIZE_SWITCH;
					hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
					customize_resize(hwnd, MAX_SWITCH_TYPES);
					break;

				case IDC_CONV:
					{
						HMENU popup;
						MENUITEMINFO mii;
						POINT pt;
						int result, max;
						
						hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
						
						popup = CreatePopupMenu();
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_TYPE|MIIM_ID;
						mii.fType = MFT_STRING;
						mii.dwTypeData = "Left/Right";
						mii.cch = strlen("Left/Right");
						mii.wID = 1;
						InsertMenuItem(popup, 0, TRUE, &mii);
						mii.cch = strlen("Up/Down");
						mii.dwTypeData = "Up/Down";
						mii.wID = 2;
						InsertMenuItem(popup, 1, TRUE, &mii);
						
						GetCursorPos(&pt);
						
						result = TrackPopupMenuEx(popup, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
							pt.x, pt.y, hwnd, NULL);
						
						DestroyMenu(popup);
						switch(result) {
						case 1:
							hCustomizeMode = CUSTOMIZE_CONV_LR;
							max = count_custom(custom_conv_lr_data);
							break;
						case 2:
							hCustomizeMode = CUSTOMIZE_CONV_UD;
							max = count_custom(custom_conv_ud_data);
							break;
						default:
							return 0;
						}
						
						hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
						customize_resize(hwnd, max);
						break;
					}

					
				case IDC_REMOVE: 
					{
						if (hCustomizeEntry == -1) break;
						
						switch (hCustomizeMode) {
						case CUSTOMIZE_WALL:
							if (hCustomizeEntry == remove_custom(&custom_wall_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_FLAG:
							if (hCustomizeEntry == remove_custom(&custom_flag_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_POLE:
							if (hCustomizeEntry == remove_custom(&custom_pole_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_BUNKER:
							if (hCustomizeEntry == remove_custom(&custom_bunker_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_BRIDGE_LR:
							if (hCustomizeEntry == remove_custom(&custom_bridge_lr_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_BRIDGE_UD:
							if (hCustomizeEntry == remove_custom(&custom_bridge_ud_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_SPAWN:
							if (hCustomizeEntry == remove_custom(&custom_spawn_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						case CUSTOMIZE_SWITCH:
							if (hCustomizeEntry == remove_custom(&custom_switch_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;

						case CUSTOMIZE_CONV_UD:
							if (hCustomizeEntry == remove_custom(&custom_conv_ud_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;

						case CUSTOMIZE_CONV_LR:
							if (hCustomizeEntry == remove_custom(&custom_conv_lr_data, hCustomizeEntry))
								hCustomizeEntry--;
							break;
							
						default:
							break;
						}
						
						RedrawWindow(GetDlgItem(hwnd, ID_CUSTOMIZELIST), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						break;
					}
					
				case IDC_ADD:
					{
						int ***data, n, x;
						
						switch(hCustomizeMode) {
							
						case CUSTOMIZE_WALL:
							data = &custom_wall_data;
							break;
							
						case CUSTOMIZE_FLAG:
							data = &custom_flag_data;
							break;
							
						case CUSTOMIZE_POLE:
							data = &custom_pole_data;
							break;
							
						case CUSTOMIZE_BRIDGE_LR:
							data = &custom_bridge_lr_data;
							break;
							
						case CUSTOMIZE_BRIDGE_UD:
							data = &custom_bridge_ud_data;
							break;
							
						case CUSTOMIZE_BUNKER:
							data = &custom_bunker_data;
							break;
							
						case CUSTOMIZE_SPAWN:
							data = &custom_spawn_data;
							break;
							
						case CUSTOMIZE_SWITCH:
							data = &custom_switch_data;
							break;

						case CUSTOMIZE_CONV_UD:
							data = &custom_conv_ud_data;
							break;

						case CUSTOMIZE_CONV_LR:
							data = &custom_conv_lr_data;
							break;
							
						default:
							data = NULL;
							break;
						}
						
						if (data == NULL) break;
						
						n = count_custom(*data);
						
						*data = (int **)realloc(*data, sizeof(int *) * (n+2));
						data[0][n] = (int *)malloc(sizeof(int) * STATIC_ENTRIES);
						data[0][n+1] = NULL;
						
						for (x = 0; x < STATIC_ENTRIES; x++)
							edit_data[x] = data[0][n][x] = templates[hCustomizeMode][x];
						
						adding = TRUE;
						hCustomizeBacks[hCustomizeMode] = hCustomizeEntry;
						hCustomizeEntry = n;
						
						ShowWindow(GetDlgItem(frameWnd, ID_CFRAME), SW_SHOW);
						ShowWindow(GetDlgItem(frameWnd, ID_MDICLIENT), SW_HIDE);
						ShowWindow(hwnd, SW_HIDE);
						break;
					}
					
				case IDC_EDIT: 
					{
						int **data, n;
						if (hCustomizeEntry == -1) {
							MessageBox(NULL, "You must select an entry to edit", "Custom Data", MB_OK);
							break;
						}
						
						switch(hCustomizeMode) {
							
						case CUSTOMIZE_WALL:
							data = custom_wall_data;
							break;
							
						case CUSTOMIZE_FLAG:
							data = custom_flag_data;
							break;
							
						case CUSTOMIZE_POLE:
							data = custom_pole_data;
							break;
							
						case CUSTOMIZE_BRIDGE_LR:
							data = custom_bridge_lr_data;
							break;
							
						case CUSTOMIZE_BRIDGE_UD:
							data = custom_bridge_ud_data;
							break;
							
						case CUSTOMIZE_BUNKER:
							data = custom_bunker_data;
							break;
							
						case CUSTOMIZE_SPAWN:
							data = custom_spawn_data;
							break;
							
						case CUSTOMIZE_SWITCH:
							data = custom_switch_data;
							break;

						case CUSTOMIZE_CONV_LR:
							data = custom_conv_lr_data;
							break;

						case CUSTOMIZE_CONV_UD:
							data = custom_conv_ud_data;
							break;
							
						default:
							data = NULL;
							break;
						}
						
						if (data == NULL) break;
						
						for (n = 0; n < STATIC_ENTRIES; n++)
							edit_data[n] = data[hCustomizeEntry][n];
						
						adding = FALSE;
						
						ShowWindow(GetDlgItem(frameWnd, ID_CFRAME), SW_SHOW);
						RedrawWindow(GetDlgItem(frameWnd, ID_CFRAME), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						ShowWindow(GetDlgItem(frameWnd, ID_MDICLIENT), SW_HIDE);
						ShowWindow(hwnd, SW_HIDE);
						break;
					}
					
				default:
					break;
				}
				break;
			}
				
				
			default:
				break;
			}
			break;
			
		default:
			break;
	}
	return 0;
}

int CALLBACK CustomizeEditProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	int width, height;
	
	switch(msg) {
	case WM_CREATE:
		break;
		
	case WM_SIZE:
		width = LOWORD(lparm);
		height = HIWORD(lparm);
		
		if ((width % 16) || (height % 16)) {
			width += (16 - (width % 16));
			height += (16 - (height % 16));
			
			SetWindowPos(hwnd, NULL, 0, 0, width, height,
				SWP_SHOWWINDOW|SWP_NOREPOSITION|SWP_NOMOVE);
		}
		break;
		
	case WM_DESTROY:
		break;
		
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc, bbDC;
			int c, max = 0;
			int tile;
			struct Animation *anim = NULL;
			int scy = GetScrollPos(hwnd, SB_VERT);
			int scx = GetScrollPos(hwnd, SB_HORZ);
			RECT rc;
			HBITMAP backBuf;
			
			
			dc = BeginPaint(hwnd, &ps);
			SetWindowOrgEx(dc, scx, scy, NULL);
			
			GetClientRect(hwnd, &rc);
			backBuf = CreateCompatibleBitmap(tilesDC, rc.right-rc.left, rc.bottom-rc.top);
			bbDC = CreateCompatibleDC(tilesDC);
			
			backBuf = (HBITMAP)SelectObject(bbDC, backBuf);
			
			FillRect(bbDC, &rc, (HBRUSH)(COLOR_WINDOW+1));
			
			
			
			for (c = 0; c < STATIC_ENTRIES; c++) {
				int x,y;
				anim = NULL;
				
				x = (c%ddim[hCustomizeMode].width)*16 +
					(c/ddim[hCustomizeMode].width/ddim[hCustomizeMode].height)*ddim[hCustomizeMode].width*16;
				y = (c/ddim[hCustomizeMode].width % ddim[hCustomizeMode].height)*16;
				x -= scx;
				y -= scy;
				if (edit_data[c] > -1) {
					tile = edit_data[c];
					if (tile & 0x8000) 
						anim = get_anim((unsigned char)(tile & 0xFF));
					if (anim && anim->frameCount > 0)
						DrawTileBlock(anim->frames[0], bbDC, x, y, 1, 1, FALSE);
					else if (anim && tile != 0xFFFFFFFE)
						TextOut(bbDC, x, y, "a", 1);
					else if (!(tile & 0x80000000))
						DrawTileBlock(tile, bbDC, x, y, 1, 1, FALSE);
				}
				
			}
			
			
			BitBlt(dc, scx, scy, rc.right-rc.left, rc.bottom-rc.top, bbDC, 0, 0, SRCCOPY);
			backBuf = (HBITMAP)SelectObject(bbDC, backBuf);
			DeleteObject(backBuf);
			DeleteDC(bbDC);
			
			EndPaint(hwnd, &ps);
			break;
		}
		
	case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int h = ddim[hCustomizeMode].height;
			int w = ddim[hCustomizeMode].width;
			int idx;
			char buffer[200];
			
			
			if (y > h*16) {
				ClearStatus();
				break;
			}
			
			idx = x/(16 * w)*w*h + (x % (16 * w))/16;
			idx += y/16*w;
			
			if (idx > 15) {
				ClearStatus();
				break;
			}
			
			sprintf(buffer, "Index: %d", idx);
			SetStatusText(0, buffer);
			
			if (edit_data[idx] == -2)
				SetStatusText(1, "Read-Only");
			else if (edit_data[idx] == -1)
				SetStatusText(1, "Transparent");
			else if (edit_data[idx] & 0x8000) {
				sprintf(buffer, "Animation: %d", edit_data[idx] & 0xFF);
				SetStatusText(1, buffer);
			}
			else {
				sprintf(buffer, "Tile: %d", edit_data[idx]);
				SetStatusText(1, buffer);
			}
			
			break;
		}
		
	case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int h = ddim[hCustomizeMode].height;
			int w = ddim[hCustomizeMode].width;
			int idx, result, i, j;
			MSG mouseMsg;
			char buffer[200];
			
			
			if (y > h*16) {
				ClearStatus();
				break;
			}
			
			idx = x/(16 * w)*w*h + (x % (16 * w))/16;
			idx += y/16*w;
			
			if (idx > 15) {
				ClearStatus();
				break;
			}
			
			sprintf(buffer, "Index: %d", idx);
			SetStatusText(0, buffer);
			
			if (edit_data[idx] == -2)
				SetStatusText(1, "Read-Only");
			else if (edit_data[idx] == -1)
				SetStatusText(1, "Transparent");
			else if (edit_data[idx] & 0x8000) {
				sprintf(buffer, "Animation: %d", edit_data[idx] & 0xFF);
				SetStatusText(1, buffer);
			}
			else {
				sprintf(buffer, "Tile: %d", edit_data[idx]);
				SetStatusText(1, buffer);
			}
			
			
			if (edit_data[idx] == -2) {
				break;
			}
			
			if (useAnim && animSel.tile != -1)
				edit_data[idx] = animSel.tile | 0x8000;
			else if (tileSel.tile != -1) {
				for (i = y/16; i < (y/16 + tileSel.vert); i++) {
					for (j = x/16; j < (x/16 + tileSel.horz); j++) {
						idx = (j/w)*w*h + (j % w);
						idx += i*w;
						
						if (idx > 15 || edit_data[idx] == -2) continue;
						
						edit_data[idx] = tileArrange[tileSel.tile + (j-x/16) + (i-y/16)*40];
					}
				}
			}
			
			SetCapture(hwnd);
			
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			
			while (1) {
				result = GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
				if (result < 1) continue;
				
				if (mouseMsg.message == WM_LBUTTONUP || !(mouseMsg.wParam & MK_LBUTTON))
					break;
				
				x = GET_X_LPARAM(mouseMsg.lParam);
				y = GET_Y_LPARAM(mouseMsg.lParam);
				
				if (x < 0 || y < 0) {
					ClearStatus();
					continue;
				}
				
				if (y > h*16) {
					ClearStatus();
					continue;
				}
				
				idx = x/(16 * w)*w*h + (x % (16 * w))/16;
				idx += y/16*w;
				
				if (idx > 15) {
					ClearStatus();
					continue;
				}
				
				sprintf(buffer, "Index: %d", idx);
				SetStatusText(0, buffer);
				
				if (edit_data[idx] == -2)
					SetStatusText(1, "Read-Only");
				else if (edit_data[idx] == -1)
					SetStatusText(1, "Transparent");
				else if (edit_data[idx] & 0x8000) {
					sprintf(buffer, "Animation: %d", edit_data[idx] & 0xFF);
					SetStatusText(1, buffer);
				}
				else {
					sprintf(buffer, "Tile: %d", edit_data[idx]);
					SetStatusText(1, buffer);
				}
				
				
				if (edit_data[idx] == -2) {
					continue;
				}
				
				if (useAnim && animSel.tile != -1)
					edit_data[idx] = animSel.tile | 0x8000;
				else if (tileSel.tile != -1) {
					for (i = y/16; i < (y/16 + tileSel.vert); i++) {
						for (j = x/16; j < (x/16 + tileSel.horz); j++) {
							idx = (j/w)*w*h + (j % w);
							idx += i*w;
							
							if (idx > 15 || edit_data[idx] == -2) continue;
							
							edit_data[idx] = tileArrange[tileSel.tile + (j-x/16) + (i-y/16)*40];
						}
					}
				}
				
				RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				
			}
			
			ReleaseCapture();
			break;
			
		}
		
	case WM_RBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int h = ddim[hCustomizeMode].height;
			int w = ddim[hCustomizeMode].width;
			int idx, result;
			MSG mouseMsg;
			RECT rc;
			char buffer[200];
			
			
			if (y > h*16) {
				ClearStatus();
				break;
			}
			
			idx = x/(16 * w)*w*h + (x % (16 * w))/16;
			idx += y/16*w;
			
			if (idx > 15) {
				ClearStatus();
				break;
			}
			
			sprintf(buffer, "Index: %d", idx);
			SetStatusText(0, buffer);
			
			if (edit_data[idx] == -2)
				SetStatusText(1, "Read-Only");
			else if (edit_data[idx] == -1)
				SetStatusText(1, "Transparent");
			else if (edit_data[idx] & 0x8000) {
				sprintf(buffer, "Animation: %d", edit_data[idx] & 0xFF);
				SetStatusText(1, buffer);
			}
			else {
				sprintf(buffer, "Tile: %d", edit_data[idx]);
				SetStatusText(1, buffer);
			}
			
			if (edit_data[idx] == -2) {
				break;
			}
			
			edit_data[idx] = -1;
			rc.left = x/16*16;
			rc.right = rc.left + 16;
			rc.top = y/16*16;
			rc.bottom = rc.top + 16;
			RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			
			SetCapture(hwnd);
			
			while (1) {
				result = GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
				if (result < 1) continue;
				
				if (mouseMsg.message == WM_RBUTTONUP || !(mouseMsg.wParam & MK_RBUTTON))
					break;
				
				x = GET_X_LPARAM(mouseMsg.lParam);
				y = GET_Y_LPARAM(mouseMsg.lParam);
				
				if (y > h*16) {
					ClearStatus();
					break;
				}
				
				idx = x/(16 * w)*w*h + (x % (16 * w))/16;
				idx += y/16*w;
				
				if (idx > 15) {
					ClearStatus();
					continue;
				}
				
				sprintf(buffer, "Index: %d", idx);
				SetStatusText(0, buffer);
				
				if (edit_data[idx] == -2)
					SetStatusText(1, "Read-Only");
				else if (edit_data[idx] == -1)
					SetStatusText(1, "Transparent");
				else if (edit_data[idx] & 0x8000) {
					sprintf(buffer, "Animation: %d", edit_data[idx] & 0xFF);
					SetStatusText(1, buffer);
				}
				else {
					sprintf(buffer, "Tile: %d", edit_data[idx]);
					SetStatusText(1, buffer);
				}
				
				if (edit_data[idx] == -2) {
					continue;
				}
				
				edit_data[idx] = -1;
				rc.left = x/16*16;
				rc.right = rc.left + 16;
				rc.top = y/16*16;
				rc.bottom = rc.top + 16;
				RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);	
				
			}
			break;
		}
		
	default:
		return DefWindowProc(hwnd,msg,wparm,lparm);
	}
	return 0;
}

int CALLBACK CustomizeEditFrameProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	HWND tmp;
	switch(msg) {
	case WM_CREATE:
		tmp = CreateWindow("BUTTON", "Set", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_NOTIFY,
			120, 5, 80, 23, hwnd, (HMENU)ID_CUSTOMIZEEDITSET, hInstance, NULL);
		
		SetWindowFont(tmp, arialFont, TRUE);
		tmp = CreateWindow("BUTTON", "Cancel", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_NOTIFY,
			120, 32, 80, 23, hwnd, (HMENU)ID_CUSTOMIZEEDITCANCEL, hInstance, NULL);
		SetWindowFont(tmp, arialFont, TRUE);
		
		CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_customizee", NULL, WS_CHILD|WS_VISIBLE,
			5, 5, 100, 84, hwnd, (HMENU)ID_CUSTOMIZEEDIT, hInstance, NULL);
		break;
		
	case WM_COMMAND:
		{
			int code = HIWORD(wparm);
			int id = LOWORD(wparm);
			HWND item = (HWND)lparm;
			int **data, x, max;
			
			switch (code) {
			case BN_CLICKED:
				switch (id) {
				case ID_CUSTOMIZEEDITSET:
					switch(hCustomizeMode) {
						
					case CUSTOMIZE_WALL:
						data = custom_wall_data;
						break;
						
					case CUSTOMIZE_FLAG:
						data = custom_flag_data;
						break;
						
					case CUSTOMIZE_POLE:
						data = custom_pole_data;
						break;
						
					case CUSTOMIZE_BRIDGE_LR:
						data = custom_bridge_lr_data;
						break;
						
					case CUSTOMIZE_BRIDGE_UD:
						data = custom_bridge_ud_data;
						break;
						
					case CUSTOMIZE_BUNKER:
						data = custom_bunker_data;
						break;
						
					case CUSTOMIZE_SPAWN:
						data = custom_spawn_data;
						break;
						
					case CUSTOMIZE_SWITCH:
						data = custom_switch_data;
						break;

					case CUSTOMIZE_CONV_LR:
						data = custom_conv_lr_data;
						break;

					case CUSTOMIZE_CONV_UD:
						data = custom_conv_ud_data;
						break;
						
					default:
						data = NULL;
						break;
					}
					
					if (data != NULL) {
						for (x = 0; x < STATIC_ENTRIES; x++)
							data[hCustomizeEntry][x] = edit_data[x];
					}
					max = count_custom(data);
					ShowWindow(hwnd, SW_HIDE);
					ShowWindow(GetDlgItem(frameWnd, ID_MDICLIENT), SW_SHOW);
					ShowWindow(dlgCustomize, SW_SHOW);
					customize_resize(dlgCustomize, max);
					break;
					
					case ID_CUSTOMIZEEDITCANCEL:
						{
							int ***data;
							
							switch(hCustomizeMode) {
								
							case CUSTOMIZE_WALL:
								data = &custom_wall_data;
								break;
								
							case CUSTOMIZE_FLAG:
								data = &custom_flag_data;
								break;
								
							case CUSTOMIZE_POLE:
								data = &custom_pole_data;
								break;
								
							case CUSTOMIZE_BRIDGE_LR:
								data = &custom_bridge_lr_data;
								break;
								
							case CUSTOMIZE_BRIDGE_UD:
								data = &custom_bridge_ud_data;
								break;
								
							case CUSTOMIZE_BUNKER:
								data = &custom_bunker_data;
								break;
								
							case CUSTOMIZE_SPAWN:
								data = &custom_spawn_data;
								break;
								
							case CUSTOMIZE_SWITCH:
								data = &custom_switch_data;
								break;

							case CUSTOMIZE_CONV_LR:
								data = &custom_conv_lr_data;
								break;

							case CUSTOMIZE_CONV_UD:
								data = &custom_conv_ud_data;
								break;
								
							default:
								data = NULL;
								break;
							}
							
							if (adding) {
								remove_custom(data, hCustomizeEntry);
								hCustomizeEntry = hCustomizeBacks[hCustomizeMode];
							}
							
							
							max = count_custom(*data);
							
							ShowWindow(hwnd, SW_HIDE);
							ShowWindow(GetDlgItem(frameWnd, ID_MDICLIENT), SW_SHOW);
							ShowWindow(dlgCustomize, SW_SHOW);
							customize_resize(dlgCustomize, max);
							break;
						}
						
					default:
						break;
				}
				break;
			}
			break;
		}
		
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc;
			char *lines[] = {
				"Edit as you would edit a map.",
					"Right-Click to make a tile TRANSPARENT.",
					"Tiles that are TRANSPARENT will not be drawn when using that tool.",
					"",
					"You may only edit 1 tile at a time",
					NULL
			};
			int x;
			
			
			BeginPaint(hwnd, &ps);
			dc = ps.hdc;
			
			SetBkMode(dc, TRANSPARENT);
			SetTextColor(dc, ~GetSysColor(COLOR_BTNFACE));
			
			arialFont = (HFONT)SelectObject(dc, arialFont);
			
			for (x = 0; lines[x]; x++)
				TextOut(dc, 4, 110+x*20, lines[x], strlen(lines[x]));
			
			arialFont = (HFONT)SelectObject(dc, arialFont);
			
			EndPaint(hwnd, &ps);
			break;
		}
		
	case WM_MOUSEMOVE:
		ClearStatus();
		return DefWindowProc(hwnd, msg, wparm, lparm);
		
	default:
		return DefWindowProc(hwnd,msg,wparm,lparm);
	}
	return 0;
}


int CALLBACK CustomizeListProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	int i;
	
	switch(msg) {
	case WM_CREATE:
		for (i = 0; i < (int)(sizeof(hCustomizeBacks) / sizeof(*hCustomizeBacks)); i++)
			hCustomizeBacks[i] = -1;
		
		
		
		
		SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
		break;
		
	case WM_SIZE:
		{
			int width, height, max, nMax;
			
			switch (hCustomizeMode) {
			case CUSTOMIZE_WALL:
				max = count_custom(custom_wall_data);
				break;
				
			case CUSTOMIZE_FLAG:
				max = count_custom(custom_flag_data);
				break;
				
			case CUSTOMIZE_POLE:
				max = count_custom(custom_pole_data);
				break;
				
			case CUSTOMIZE_BRIDGE_LR:
				max = count_custom(custom_bridge_lr_data);
				break;
				
			case CUSTOMIZE_BRIDGE_UD:
				max = count_custom(custom_bridge_ud_data);
				break;
				
			case CUSTOMIZE_BUNKER:
				max = count_custom(custom_bunker_data);
				break;
				
			case CUSTOMIZE_SPAWN:
				max = count_custom(custom_spawn_data);
				break;
				
			case CUSTOMIZE_SWITCH:
				max = count_custom(custom_switch_data);
				break;

			case CUSTOMIZE_CONV_LR:
				max = count_custom(custom_conv_lr_data);
				break;

			case CUSTOMIZE_CONV_UD:
				max = count_custom(custom_conv_ud_data);
				break;
				
			default:
				break;
				
			}
			
			width = LOWORD(lparm);
			height = HIWORD(lparm);
			
			nMax = max * ddim[hCustomizeMode].height * 16 - height;
			if (nMax % 16)
				nMax += (16 - nMax % 16);
			
			SetScrollRange(hwnd, SB_VERT, 0, MAX(0, max * ddim[hCustomizeMode].height * 16 - height), TRUE);
			break;
		}
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc, bbDC;
			int n, c, max = 0;
			int **data, tile;
			struct Animation *anim = NULL;
			int scy = GetScrollPos(hwnd, SB_VERT);
			int scx = GetScrollPos(hwnd, SB_HORZ);
			RECT rc;
			HBITMAP backBuf;
			
			
			dc = BeginPaint(hwnd, &ps);
			
			switch (hCustomizeMode) {
			case CUSTOMIZE_WALL:
				max = count_custom(custom_wall_data);
				data = custom_wall_data;
				break;
				
			case CUSTOMIZE_FLAG:
				max = count_custom(custom_flag_data);
				data = custom_flag_data;
				break;
				
			case CUSTOMIZE_POLE:
				max = count_custom(custom_pole_data);
				data = custom_pole_data;
				break;
				
			case CUSTOMIZE_BRIDGE_LR:
				max = count_custom(custom_bridge_lr_data);
				data = custom_bridge_lr_data;
				break;
				
			case CUSTOMIZE_BRIDGE_UD:
				max = count_custom(custom_bridge_ud_data);
				data = custom_bridge_ud_data;
				break;
				
			case CUSTOMIZE_BUNKER:
				max = count_custom(custom_bunker_data);
				data = custom_bunker_data;
				break;
				
			case CUSTOMIZE_SPAWN:
				max = count_custom(custom_spawn_data);
				data = custom_spawn_data;
				break;
				
			case CUSTOMIZE_SWITCH:
				max = count_custom(custom_switch_data);
				data = custom_switch_data;
				break;

			case CUSTOMIZE_CONV_UD:
				max = count_custom(custom_conv_ud_data);
				data = custom_conv_ud_data;
				break;

			case CUSTOMIZE_CONV_LR:
				max = count_custom(custom_conv_lr_data);
				data = custom_conv_lr_data;
				break;
				
			default:
				break;
				
			}
			
			SetWindowOrgEx(dc, scx, scy, NULL);
			
			GetClientRect(hwnd, &rc);
			backBuf = CreateCompatibleBitmap(tilesDC, rc.right-rc.left, rc.bottom-rc.top);
			bbDC = CreateCompatibleDC(tilesDC);
			
			backBuf = (HBITMAP)SelectObject(bbDC, backBuf);
			
			FillRect(bbDC, &rc, (HBRUSH)(COLOR_WINDOW+1));
			
			
			
			for (n = 0; n < max; n++) {
				for (c = 0; c < STATIC_ENTRIES; c++) {
					int x,y;
					anim = NULL;
					
					x = (c%ddim[hCustomizeMode].width)*16 +
						(c/ddim[hCustomizeMode].width/ddim[hCustomizeMode].height)*ddim[hCustomizeMode].width*16;
					y = (c/ddim[hCustomizeMode].width % ddim[hCustomizeMode].height)*16 + n*ddim[hCustomizeMode].height*16;
					x -= scx;
					y -= scy;
					if (data[n][c] > -1) {
						tile = data[n][c];
						if (tile & 0x8000) 
							anim = get_anim((unsigned char)(tile & 0xFF));
						if (anim && anim->frameCount > 0)
							DrawTileBlock(anim->frames[0], bbDC, x, y, 1, 1, FALSE);
						else if (anim && tile != 0xFFFFFFFE)
							TextOut(bbDC, x, y, "a", 1);
						else if (!(tile & 0x80000000))
							DrawTileBlock(tile, bbDC, x, y, 1, 1, FALSE);
					}
				}
				
			}
			
			if (hCustomizeEntry != -1) {
				HPEN pen = CreatePen(PS_SOLID, 1, 0x888888);
				RECT rc;
				GetClientRect(hwnd, &rc);
				DrawSelection(bbDC, pen, 0, hCustomizeEntry * 16 * ddim[hCustomizeMode].height - scy, rc.right-rc.left, 16*ddim[hCustomizeMode].height-1);
				DeleteObject(pen);
			}
			
			BitBlt(dc, scx, scy, rc.right-rc.left, rc.bottom-rc.top, bbDC, 0, 0, SRCCOPY);
			backBuf = (HBITMAP)SelectObject(bbDC, backBuf);
			DeleteObject(backBuf);
			DeleteDC(bbDC);
			
			EndPaint(hwnd, &ps);
			break;
		}
		
	case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int scy = GetScrollPos(hwnd, SB_VERT);
			int entry, max;
			RECT rc;
			HDC dc;
			MSG mouseMsg;
			
			switch (hCustomizeMode) {
			case CUSTOMIZE_WALL:
				max = count_custom(custom_wall_data);
				break;
				
			case CUSTOMIZE_FLAG:
				max = count_custom(custom_flag_data);
				break;
				
			case CUSTOMIZE_POLE:
				max = count_custom(custom_pole_data);
				break;
				
			case CUSTOMIZE_BRIDGE_LR:
				max = count_custom(custom_bridge_lr_data);
				break;
				
			case CUSTOMIZE_BRIDGE_UD:
				max = count_custom(custom_bridge_ud_data);
				break;
				
			case CUSTOMIZE_BUNKER:
				max = count_custom(custom_bunker_data);
				break;
				
			case CUSTOMIZE_SPAWN:
				max = count_custom(custom_spawn_data);
				break;
				
			case CUSTOMIZE_SWITCH:
				max = count_custom(custom_switch_data);
				break;

			case CUSTOMIZE_CONV_UD:
				max = count_custom(custom_conv_ud_data);
				break;

			case CUSTOMIZE_CONV_LR:
				max = count_custom(custom_conv_lr_data);
				break;
				
			default:
				break;
				
			}
			
			entry = (scy+y)/(16 * ddim[hCustomizeMode].height);
			if (entry >= max || entry < 0 || entry == hCustomizeEntry) break;
			
			
			dc = GetDC(hwnd);
			SetWindowOrgEx(dc, 0, scy, NULL);
			GetClientRect(hwnd, &rc);
			
			if (hCustomizeEntry != -1) {
				rc.top = hCustomizeEntry * 16 * ddim[hCustomizeMode].height - scy; rc.bottom = rc.top + 16*ddim[hCustomizeMode].height;
				rc.right -= rc.left; rc.left = 0;
				hCustomizeEntry = -1;
				RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			}
			
			hCustomizeEntry = entry;
			{
				HPEN pen = CreatePen(PS_SOLID, 1, 0x888888);
				DrawSelection(dc, pen, 0, hCustomizeEntry * 16 * ddim[hCustomizeMode].height, rc.right-rc.left, 16*ddim[hCustomizeMode].height-1);
				DeleteObject(pen);
			}
			
			ReleaseDC(hwnd, dc);
			
			
			SetCapture(hwnd);
			while (1) {
				WaitMessage();
				GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
				if (mouseMsg.message == WM_LBUTTONUP || !mouseMsg.wParam & MK_LBUTTON)
					break;
				
				x = GET_X_LPARAM(mouseMsg.lParam);
				y = GET_Y_LPARAM(mouseMsg.lParam);
				
				entry = (scy+y)/(16 * ddim[hCustomizeMode].height);
				if (entry >= max || entry < 0 || entry == hCustomizeEntry) continue;
				
				
				dc = GetDC(hwnd);
				SetWindowOrgEx(dc, 0, scy, NULL);
				GetClientRect(hwnd, &rc);
				
				if (hCustomizeEntry != -1) {
					rc.top = hCustomizeEntry * 16 * ddim[hCustomizeMode].height - scy; rc.bottom = rc.top + 16*ddim[hCustomizeMode].height;
					rc.right -= rc.left; rc.left = 0;
					hCustomizeEntry = -1;
					RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				
				hCustomizeEntry = entry;
				{
					HPEN pen = CreatePen(PS_SOLID, 1, 0x888888);
					DrawSelection(dc, pen, 0, hCustomizeEntry * 16 * ddim[hCustomizeMode].height, rc.right-rc.left, 16*ddim[hCustomizeMode].height-1);
					DeleteObject(pen);
				}
				
				ReleaseDC(hwnd, dc);			
			}
			
			ReleaseCapture();
			break;
		}
		
	case WM_LBUTTONDBLCLK:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int scy = GetScrollPos(hwnd, SB_VERT);
			int entry, max;
			RECT rc;
			HDC dc;
			
			switch (hCustomizeMode) {
			case CUSTOMIZE_WALL:
				max = count_custom(custom_wall_data);
				break;
				
			case CUSTOMIZE_FLAG:
				max = count_custom(custom_flag_data);
				break;
				
			case CUSTOMIZE_POLE:
				max = count_custom(custom_pole_data);
				break;
				
			case CUSTOMIZE_BRIDGE_LR:
				max = count_custom(custom_bridge_lr_data);
				break;
				
			case CUSTOMIZE_BRIDGE_UD:
				max = count_custom(custom_bridge_ud_data);
				break;
				
			case CUSTOMIZE_BUNKER:
				max = count_custom(custom_bunker_data);
				break;
				
			case CUSTOMIZE_SPAWN:
				max = count_custom(custom_spawn_data);
				break;
				
			case CUSTOMIZE_SWITCH:
				max = count_custom(custom_switch_data);
				break;

			case CUSTOMIZE_CONV_LR:
				max = count_custom(custom_conv_lr_data);
				break;

			case CUSTOMIZE_CONV_UD:
				max = count_custom(custom_conv_ud_data);
				break;
				
			default:
				break;
				
			}
			
			entry = (scy+y)/(16 * ddim[hCustomizeMode].height);
			if (entry >= max || entry < 0) break;
			
			
			dc = GetDC(hwnd);
			SetWindowOrgEx(dc, 0, scy, NULL);
			GetClientRect(hwnd, &rc);
			
			if (hCustomizeEntry != -1) {
				rc.top = hCustomizeEntry * 16 * ddim[hCustomizeMode].height - scy; rc.bottom = rc.top + 16*ddim[hCustomizeMode].height;
				rc.right -= rc.left; rc.left = 0;
				hCustomizeEntry = -1;
				RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			}
			
			hCustomizeEntry = entry;
			{
				HPEN pen = CreatePen(PS_SOLID, 1, 0x888888);
				DrawSelection(dc, pen, 0, hCustomizeEntry * 16 * ddim[hCustomizeMode].height, rc.right-rc.left, 16*ddim[hCustomizeMode].height-1);
				DeleteObject(pen);
			}
			
			ReleaseDC(hwnd, dc);
			
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_EDIT, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_EDIT));
			
			break;
		}
		
	case WM_VSCROLL:
		{
			int nScrollCode = LOWORD(wparm);
			int nPos = HIWORD(wparm);
			
			int add_factor = 0;
			
			SCROLLINFO si;
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			
			//nPos = nPos/16 * 16;
			
			switch (nScrollCode) {
			case SB_BOTTOM:
			case SB_LINEDOWN:
				add_factor = ddim[hCustomizeMode].height * 16;
				break;
				
			case SB_TOP:
			case SB_LINEUP:
				add_factor = ddim[hCustomizeMode].height * -16;
				break;
				
			case SB_PAGEUP:
				add_factor = ddim[hCustomizeMode].height * -64;
				break;
				
			case SB_PAGEDOWN:
				add_factor = ddim[hCustomizeMode].height * 64;
				break;
				
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				add_factor = nPos - si.nPos;
				break;
				
			default:
				break;
			}
			
			si.fMask = SIF_POS;
			
			add_factor = MAX(si.nMin - si.nPos, MIN(si.nMax - si.nPos, add_factor));
			
			if (add_factor != 0) {
				si.nPos += add_factor;
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
				
				
				ScrollWindowEx(hwnd, 0, -add_factor, NULL, NULL, NULL, NULL, SW_ERASE);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
			}
			break;
		}
		
		
		
		
		
	default:
		return DefWindowProc(hwnd,msg,wparm,lparm);
	}
	return 0;
}

