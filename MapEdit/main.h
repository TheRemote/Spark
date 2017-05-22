
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>

extern "C" {
#include "jpeg/jpeglib.h"
}

#include "resource.h"

#include "Polynomial.h"

#define DONTALLOWCUSTOMIZE
#define JPEG_OVERVIEW_FORMAT

#define MYVERSION		0x00020200
#define mymajor			((MYVERSION & 0xFF0000) >> 16)
#define myminor			((MYVERSION & 0xFF00) >> 8)
#define myrevision		((MYVERSION & 0xFF))
#define mytree			"Stable"

	// MACROS
#define ANIM(a)                 (int)((a | 0x8000))
#define ABS(a)					((a) < 0 ? -(a) : (a))
#define MIN						min
#define MAX						max

	struct seldata {
		int tile;
		int horz;
		int vert;
	};

	struct undo_buf {
		WORD *buffer;
		int width;
		int height;
		int x;
		int y;
	};

	struct undo_blocks { 
		struct undo_blocks *next;

		int type;
		union {
			struct undo_buf *ub;
			struct seldata  *sel;
			char *ptr;
		} undo;
	};

	struct MapInfo {
		WORD *mapData;
		struct mapheader {
			short       id;
			short       header;
			char        version;
			short       width;
			short       height;
			char        maxPlayers;
			char        holdingTime;
			char        numTeams;
			char        objective;
			char        laserDamage;
			char        specialDamage;
			char        rechargeRate;
			char        misslesEnabled;
			char        bombsEnabled;
			char        bounciesEnabled;
			short       powerupCount;
			char        maxSimulPowerups;
			char        switchCount;
			char        flagCount[4];
			char        flagPoleCount[4];
			char        *flagPoleData[4];
			short       nameLength;
			char        *name;
			short       descriptionLength;
			char        *description;
			char        neutralCount;
		} header;
		struct mouseoverdata {
			RECT rc;
			BOOL valid;
		} mouseOver;
		struct linetooldata {
			POINT start;
			POINT stop;
			BOOL valid;
		} lineTool;
		struct seldata selection;
		int seltype;
		unsigned int anim_frame;
		char *file;
		BOOL modify;
		HWND hwnd;
		struct MapInfo *next;
		struct undo_blocks *redo_list, *undo_list;
		BYTE pan;
		UINT pan_timer;
		BITMAPINFO *overview;
	};

	struct Animation {
		BYTE frameCount;
		BYTE speed;
		WORD frames[32];
	};

	typedef struct RotateData {
		int            tile;
		int            deg90;
		int            deg180;
		int            deg270;
	} rdTbl;

	typedef struct MirrorData {
		int            tile;
		int            vert;
		int            horz;
		int            hv;
	} mdTbl; 

	struct ApplicationLayout {
		long showtype;			/* restore/minimize/maximize */
		POINT position;			/* x,y */
		SIZE dimensions;		/* w,h */
		int tw_height;			/* tile window height */
	};


	// UNDO TYPES
#define UNDO_NORMAL						0
#define UNDO_SELECT						1

	// AUTO PAN BITS
#define APAN_LEFT						1
#define APAN_RIGHT						2
#define APAN_UP							4
#define APAN_DOWN						8

	// FRAME CHILD IDS

#define ID_MAPAREA                      100
#define ID_TOOLBAR                      101
#define ID_ANIMATIONS                   102
#define ID_TILEAREA                     103
#define ID_INFOAREA                     104
#define ID_OVWINDOW                     105
#define ID_STATUS                       106
#define ID_MDICLIENT					107
#define ID_CFRAME						108
#define ID_MINIVIEW						109
#define ID_OVDATA						110

	// INFO CHILD IDS

#define ID_TILE                         200
#define ID_ANIM                         201
#define ID_OFFSET                       202
#define ID_REPEATX                      203
#define ID_REPEATY                      204
#define ID_SETTINGS                     205
#define ID_CUSTOMIZELIST				207
#define ID_CUSTOMIZEEDIT				208
#define ID_CUSTOMIZEEDITSET				209
#define ID_CUSTOMIZEEDITCANCEL			210
#define ID_GREEN						211
#define ID_RED							212
#define ID_BLUE							213
#define ID_YELLOW						214
#define ID_NEUTRAL						215

	// TOOLBAR 

	typedef struct toolbar_button {
		struct toolbar_button *next;
		struct coords {
			long x;
			long y;
			long w;
			long h;
		} coords;
		char state;
		HBITMAP bmp;
		HBITMAP disabled;
		char *text;
		int id;
	} button;

#define BSTATE_NORMAL                           0
#define BSTATE_PUSHIN                           1
#define BSTATE_MOUSEOVER                        2
#define BSTATE_SELECT                           3
#define BSTATE_DISABLED							4

#define TB_SEPARATOR                            -1


#define WM_TBCOMMAND                            WM_USER+1
#define WM_STMESSAGE                            WM_USER+2

	/*
#define COMM_EDIT                               403
#define COMM_SHOWGRID                           405
#define COMM_OVERVIEW                           406
#define COMM_SELECT                             410
#define COMM_HELP                               411
#define COMM_MIRROR                             413
#define COMM_ROTATE                             414
#define COMM_SELECTALL                          415
#define COMM_COPY                               419
#define COMM_CUT                                420
#define COMM_PASTE                              421
#define COMM_WALL								422
#define COMM_FLAG								423
#define COMM_POLE								424
#define COMM_FILL								425
#define COMM_HOLDING_PEN						426
#define COMM_BUNKER								427
#define COMM_SPAWN								428
#define COMM_BRIDGE								429
#define COMM_WARP								430
#define COMM_SWITCH								431
#define COMM_PICKER								432
#define COMM_CONV								433
#define COMM_LINE								434

#define COMM_DEBUG								600
*/
#define TOOL_NEW								COMM_NEW
#define TOOL_EDIT                               COMM_EDIT
#define TOOL_SELECT                             COMM_SELECT
#define TOOL_WALL								COMM_WALL
#define TOOL_FLAG								COMM_FLAG
#define TOOL_POLE								COMM_POLE
#define TOOL_FILL								COMM_FILL
#define TOOL_HOLDING_PEN						COMM_HOLDING_PEN
#define TOOL_BUNKER								COMM_BUNKER
#define TOOL_SPAWN								COMM_SPAWN
#define TOOL_BRIDGE								COMM_BRIDGE
#define TOOL_WARP								COMM_WARP
#define TOOL_SWITCH								COMM_SWITCH
#define TOOL_PICKER								COMM_PICKER
#define TOOL_CONV								COMM_CONV
#define TOOL_LINE								COMM_LINE

	// WALL connections

#define WALL_LEFT                               0x1
#define WALL_UP                                 0x2
#define WALL_RIGHT                              0x4
#define WALL_DOWN                               0x8
#define MAX_WALL_CONNS							16

	// GAME objectives
#define OBJECTIVE_FRAG							0
#define OBJECTIVE_FLAG							1
#define OBJECTIVE_SWITCH						2

	// TIMERS
#define TIMER_OVUPDATE							0x4A00
#define TIMER_ANIMFRAME							0x4AD
#define ANIMFRAME_DURATION						150
