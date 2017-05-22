#include "main.h"
#include "extern.h"

button          *button_list = NULL;
int             currentTool = TOOL_EDIT;


char *tool_names[] = {
	"Edit",
		"Select",
		NULL
};


void init_toolbar(HWND tbwnd) {
	int i;
	TOOLINFO ti;	
	HWND ttwnd;

	struct toolbar_creation_data {
		long x;
		long y;
		long w;
		long h;
		char *text;
		char *tiptext;
		long resource;
		long id;
	} tcd[] = {
		{   2, 5, 23, 23, NULL, "New Map",	IDB_NEW, COMM_NEW },
		{  28, 5, 23, 23, NULL, "Open Map",	IDB_OPEN, COMM_OPEN },
		{  54, 5, 23, 23, NULL, "Save Map",	IDB_SAVE, COMM_SAVE },
		{  80, 5, 2, 23, "SEPARATOR", NULL, TB_SEPARATOR, TB_SEPARATOR },
		{  85, 5, 23, 23, NULL, "Edit",		IDB_EDIT, COMM_EDIT },
		{  111, 5, 23, 23, NULL, "Select",	IDB_SELECT, COMM_SELECT },
		{  137, 5, 23, 23, NULL, "Mirror",	IDB_MIRROR, COMM_MIRROR },
		{  163, 5, 23, 23, NULL, "Rotate",	IDB_ROTATE, COMM_ROTATE },
		{  189, 5, 23, 23, NULL, "Copy",	IDB_COPY, COMM_COPY },
		{  215, 5, 23, 23, NULL, "Cut",		IDB_CUT, COMM_CUT },
		{  241, 5, 23, 23, NULL, "Paste",	IDB_PASTE, COMM_PASTE },
		{  267, 5, 23, 23, NULL, "Fill",	IDB_FILL, COMM_FILL },
		{  293, 5, 23, 23, NULL, "Tile picker", IDB_DROPPER, COMM_PICKER },
		{  319, 5, 2, 23, "SEPARATOR", NULL, TB_SEPARATOR, TB_SEPARATOR },
		{  324, 5, 23, 23, NULL, "Wall Tool", IDB_WALL, COMM_WALL },
		{  350, 5, 23, 23, NULL, "Flag Tool", IDB_FLAG, COMM_FLAG },
		{  376, 5, 23, 23, NULL, "Flag Pole Tool", IDB_POLE, COMM_POLE },
		{  402, 5, 23, 23, NULL, "Holding Pen Tool", IDB_HPEN, COMM_HOLDING_PEN },
		{  428, 5, 23, 23, NULL, "Bunker Tool", IDB_BUNKER, COMM_BUNKER },
		{  454, 5, 23, 23, NULL, "Spawn Point Tool", IDB_SPAWN, COMM_SPAWN },
		{  480, 5, 27, 23, NULL, "Bridge Tool", IDB_BRIDGE, COMM_BRIDGE },
		{  510, 5, 23, 23, NULL, "Warp Tool", IDB_WARP, COMM_WARP },
		{  536, 5, 23, 23, NULL, "Switch Tool", IDB_SWITCH, COMM_SWITCH },
		{  562, 5, 23, 23, NULL, "Conveyor Tool", IDB_CONV, COMM_CONV },
		{  588, 5, 23, 23, NULL, "Line Tool", IDB_LINE, COMM_LINE },
		{ 0, 0, 0, 0, 0, 0, 0 }
	};

	ttwnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
					WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
					0, 0, 0, 0,
					tbwnd, NULL, hInstance, NULL);
	SetWindowPos(tbwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ti.cbSize = sizeof(ti);
	ti.hinst = hInstance;
	ti.hwnd = tbwnd;
	ti.uFlags = TTF_SUBCLASS;
    	
	for (i = 0; tcd[i].w && tcd[i].h; i++) {
		if (create_button(tcd[i].x, tcd[i].y, tcd[i].w, tcd[i].h)) {
			button_list->id = tcd[i].id;
			
			if (tcd[i].tiptext) {
				ti.uId = tcd[i].id;
				ti.rect.top = tcd[i].y;
				ti.rect.bottom = ti.rect.top + tcd[i].h;
				ti.rect.left = tcd[i].x;
				ti.rect.right = ti.rect.left + tcd[i].w;
				ti.lpszText = tcd[i].tiptext;
				SendMessage(ttwnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
			}
			if (tcd[i].text)
				set_button_text(button_list, tcd[i].text);
			else {
				button_list->bmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(tcd[i].resource),
					IMAGE_BITMAP, 0, 0,
					LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADMAP3DCOLORS);
				obtain_disabled_bmp(button_list, tcd[i].resource);
				
			}
		}
	}
	DisableButtons();
}

RGBQUAD RGBQCR(COLORREF CR) {
	RGBQUAD foo;

	foo.rgbBlue = (BYTE)((CR >> 16) & 0xFF);
	foo.rgbGreen = (BYTE)((CR >> 8) & 0xFF);
	foo.rgbRed = (BYTE)(CR & 0xFF);
	foo.rgbReserved = 0;
	return foo;
}

void obtain_disabled_bmp(button *btn, int resource) {
	BITMAP bm;
	BITMAPINFO *bmi;
	RGBQUAD pal[256];
	HBITMAP bmp = NULL;
	int x;
	HDC dc, dc2;
	BYTE *bits;
	
	if (!btn || !btn->bmp) return;

	dc = dc2 = NULL;
	
	for (x = 0; x < 256; x++) {
		pal[x].rgbBlue = x;
		pal[x].rgbGreen = x;
		pal[x].rgbRed = x;
		pal[x].rgbReserved = 0;
	}

	COLORREF face3d = GetSysColor(COLOR_3DFACE);
	COLORREF shad3d = GetSysColor(COLOR_3DSHADOW);
	COLORREF lite3d = GetSysColor(COLOR_3DLIGHT);
	pal[128] = RGBQCR(shad3d);
	pal[192] = RGBQCR(face3d);
	pal[223] = RGBQCR(lite3d);

	
	/* create the bitmap info */
	dc = CreateCompatibleDC(NULL);
	GetObject(btn->bmp, sizeof(bm), &bm);
	if ((bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256))) {
		bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
		bmi->bmiHeader.biBitCount = 0;
		if (GetDIBits(dc, btn->bmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS) == 0) goto error;
	}
	else goto error;
	
	GdiFlush();
	
	/* copy the palette in & create */
	memcpy(bmi->bmiColors, pal, sizeof(pal));
	bmi->bmiHeader.biBitCount = 8;
	bmi->bmiHeader.biClrImportant = bmi->bmiHeader.biClrUsed = 256;
	bmp = CreateDIBSection(dc, bmi, DIB_RGB_COLORS, (LPVOID *)&bits, NULL, 0);
	if (!bmp) goto error;
	
	(void)SelectObject(dc, bmp);
	if (SetDIBColorTable(dc, 0, 256, pal) != 256) goto error;
	
	dc2 = CreateCompatibleDC(NULL);
	(void)SelectObject(dc2, btn->bmp);
	BitBlt(dc, 0, 0, bm.bmWidth, bm.bmHeight, dc2, 0, 0, SRCCOPY);

	btn->disabled = bmp;
	DeleteDC(dc);
	DeleteDC(dc2);
	free(bmi);
	
	return;
	
error:
	if (dc) DeleteDC(dc);
	if (dc2) DeleteDC(dc2);
	if (bmp) DeleteObject(bmp);
	if (bmi) free(bmi);
	btn->disabled = NULL;
	
}

int btn_depend_array[] = {
	COMM_NEW,
		COMM_OPEN,
		COMM_EDIT,
		COMM_SELECT,
		COMM_DEBUG
};
#define num_depend		(int)(sizeof(btn_depend_array)/sizeof(*btn_depend_array))

int btn_select_array[] = {
	COMM_COPY,
		COMM_PASTE,
		COMM_CUT,
		COMM_ROTATE,
		COMM_MIRROR
};
#define num_select		(int)(sizeof(btn_select_array)/sizeof(*btn_select_array))

void EnableButtons() {
	/* enable all the buttons */
	button *loop;
	
	for (loop = button_list; loop; loop = loop->next)
		loop->state = BSTATE_NORMAL;
	
	DrawToolbar();
}

void DisableButtons() {
	/* disable all buttons not in the btn_depend_array */
	button *loop;
	int x;
	
	for (loop = button_list; loop; loop = loop->next) {
		if (loop->state == BSTATE_DISABLED) continue;
		for (x = 0; x < num_depend; x++)
			if (loop->id == btn_depend_array[x]) break;
			if (x >= num_depend) loop->state = BSTATE_DISABLED;
	}
	
	DrawToolbar();
}

void DisableSelectButtons() {
	/* disable all buttons IN the btn_select_array */
	button *loop;
	int x;

	for (loop = button_list; loop; loop = loop->next) {
		if (loop->state == BSTATE_DISABLED) continue;
		for (x = 0; x < num_select; x++) {
			if (loop->id == btn_select_array[x]) {
				loop->state = BSTATE_DISABLED;
				break;
			}
		}
	}

	DrawToolbar();
}


button *create_button(long x, long y, long w, long h) {
	button *new_button;
	
	new_button = (button *)malloc(sizeof(button));
	if (!new_button) return NULL;
	
	memset(new_button, 0, sizeof(button));
	
	new_button->text = NULL;
	new_button->bmp = NULL;
	new_button->disabled = NULL;
	new_button->coords.x = x;
	new_button->coords.y = y;
	new_button->coords.w = w;
	new_button->coords.h = h;
	new_button->state = BSTATE_NORMAL;
	new_button->next = button_list;
	button_list = new_button;
	
	return new_button;
}

void set_button_text(button *btn, char *text) {
	if (btn && text && *text) {
		if (btn->bmp) {
			DeleteObject(btn->bmp);
			btn->bmp = NULL;
		}
		if (btn->text)
			free(btn->text);
		
		btn->text = dupestr(text);
	}
}

button *get_button(long x, long y) {
	button *loop;
	
	for (loop = button_list; loop; loop = loop->next) {
		if (x >= loop->coords.x && x <= (loop->coords.x + loop->coords.w) &&
			y >= loop->coords.y && y <= (loop->coords.y + loop->coords.h))
			break;
	}
	return loop;
}

void DrawTBState(HDC dc, long state, long x, long y, long w, long h) {
	HPEN pen, oldpen;
	
	if (state == BSTATE_MOUSEOVER)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	else if (state == BSTATE_PUSHIN)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
	else
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	oldpen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, x+w+1, y, NULL);
	LineTo(dc, x, y);
	LineTo(dc, x, y+h+1);
	oldpen = (HPEN)SelectObject(dc, oldpen);
	DeleteObject(oldpen);
	
	if (state == BSTATE_MOUSEOVER)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
	else if (state == BSTATE_PUSHIN)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	else
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	oldpen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, x, y+h+1, NULL);
	LineTo(dc, x+w+1, y+h+1);
	LineTo(dc, x+w+1, y);
	oldpen = (HPEN)SelectObject(dc, oldpen);
	DeleteObject(oldpen);
	
	if (state == BSTATE_MOUSEOVER)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DLIGHT));
	else if (state == BSTATE_PUSHIN)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	else
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	oldpen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, x+w, y+1, NULL);
	LineTo(dc, x+1, y+1);
	LineTo(dc, x+1, y+h);
	oldpen = (HPEN)SelectObject(dc, oldpen);
	DeleteObject(oldpen);
	
	if (state == BSTATE_MOUSEOVER)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	else if (state == BSTATE_PUSHIN)
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DLIGHT));
	else
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	oldpen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, x+1, y+h, NULL);
	LineTo(dc, x+w, y+h);
	LineTo(dc, x+w, y+1);
	oldpen = (HPEN)SelectObject(dc, oldpen);
	DeleteObject(oldpen);
}

void DrawToolbar() {
	HDC sdc = CreateCompatibleDC(NULL);
	HDC dc = GetDC(GetDlgItem(frameWnd, ID_TOOLBAR));
	button *loop;
	HBITMAP bmp;
	HPEN pen;
	RECT rc;
	
	SetBkMode(dc, TRANSPARENT);
	
	GetClientRect(GetDlgItem(frameWnd, ID_TOOLBAR), &rc);
	
	pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	pen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, 0, 0, NULL);
	LineTo(dc, rc.right-rc.left+1, 0);
	pen = (HPEN)SelectObject(dc, pen);
	DeleteObject(pen);
	pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	pen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, 1, 1, NULL);
	LineTo(dc, rc.right-rc.left+1, 1);
	pen = (HPEN)SelectObject(dc, pen);
	DeleteObject(pen);
	
	for (loop = button_list; loop; loop = loop->next) {
		if (loop->id == TB_SEPARATOR) {
			pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
			pen = (HPEN)SelectObject(dc, pen);
			MoveToEx(dc, loop->coords.x, loop->coords.y, NULL);
			LineTo(dc, loop->coords.x, loop->coords.y + loop->coords.h + 1);
			pen = (HPEN)SelectObject(dc, pen);
			DeleteObject(pen);
			pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
			pen = (HPEN)SelectObject(dc, pen);
			MoveToEx(dc, loop->coords.x + 1, loop->coords.y, NULL);
			LineTo(dc, loop->coords.x + 1, loop->coords.y + loop->coords.h + 1);
			pen = (HPEN)SelectObject(dc, pen);
			DeleteObject(pen);
		}
		else if (loop->bmp || loop->disabled) {
			int p, q;
			long state;
			BITMAP bm;
			HBITMAP draw;
			
			if (loop->state == BSTATE_DISABLED)
				draw = loop->disabled;
			else
				draw = loop->bmp;
			
			if (!draw) draw = loop->bmp; /* disabled can be NULL */
			
			GetObject(draw, sizeof(bm), &bm);
			p = (int)((float)(loop->coords.w - bm.bmWidth) / 2.0 + .5) + loop->coords.x;
			q = (int)((float)(loop->coords.h - bm.bmHeight) / 2.0 + .5) + loop->coords.y;
			// calculate the centering x,y values
			
			bmp = (HBITMAP)SelectObject(sdc, draw);
			BitBlt(dc, p, q,
				bm.bmWidth, bm.bmHeight,
				sdc, 0, 0, SRCCOPY);
			(void)SelectObject(sdc, bmp);
			
			state = (loop->state != BSTATE_DISABLED && loop->state != BSTATE_MOUSEOVER && currentTool == loop->id?
BSTATE_PUSHIN:loop->state);
			
			DrawTBState(dc, state, loop->coords.x, loop->coords.y,
				loop->coords.w, loop->coords.h);
		}
		else {
			SIZE sz;
			int p, q;
			long state;
			
			GetTextExtentPoint32(dc, loop->text, string_len(loop->text), &sz);
			p = (loop->coords.w - sz.cx) / 2 + loop->coords.x;
			q = (loop->coords.h - sz.cy) / 2 + loop->coords.y;
			
			if (loop->state == BSTATE_DISABLED)
				SetTextColor(dc, 0x00666666);
			else
				SetTextColor(dc, 0x00000000);
			
			TextOut(dc, p, q,
				loop->text, string_len(loop->text));
			state = (loop->state != BSTATE_DISABLED && loop->state != BSTATE_MOUSEOVER && currentTool == loop->id?
BSTATE_PUSHIN:loop->state);
			DrawTBState(dc, state,
				loop->coords.x, loop->coords.y,
				loop->coords.w, loop->coords.h);
		}
	}
	
	DeleteDC(sdc);
	ReleaseDC(GetDlgItem(frameWnd, ID_TOOLBAR), dc);
}



int CALLBACK ToolbarWindowProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	button *loop;
	PAINTSTRUCT ps;
	HDC dc, sdc;
	HBITMAP bmp;
	HPEN pen;
	RECT rc;
	
	switch (msg) {
    case WM_CREATE:
		init_toolbar(hwnd);
		break;
		
    case WM_DESTROY:
		break;
		
    case WM_PAINT:
		BeginPaint(hwnd, &ps);
		
		dc = ps.hdc;
		
		sdc = CreateCompatibleDC(NULL);
		
		SetBkMode(dc, TRANSPARENT);
		
		GetClientRect(hwnd, &rc);
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		pen = (HPEN)SelectObject(dc, pen);
		MoveToEx(dc, 0, 0, NULL);
		LineTo(dc, rc.right-rc.left+1, 0);
		pen = (HPEN)SelectObject(dc, pen);
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		pen = (HPEN)SelectObject(dc, pen);
		MoveToEx(dc, 1, 1, NULL);
		LineTo(dc, rc.right-rc.left+1, 1);
		pen = (HPEN)SelectObject(dc, pen);
		DeleteObject(pen);
		
		for (loop = button_list; loop; loop = loop->next) {
			if (loop->id == TB_SEPARATOR) {
				pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
				pen = (HPEN)SelectObject(dc, pen);
				MoveToEx(dc, loop->coords.x, loop->coords.y, NULL);
				LineTo(dc, loop->coords.x, loop->coords.y + loop->coords.h + 1);
				pen = (HPEN)SelectObject(dc, pen);
				DeleteObject(pen);
				pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
				pen = (HPEN)SelectObject(dc, pen);
				MoveToEx(dc, loop->coords.x + 1, loop->coords.y, NULL);
				LineTo(dc, loop->coords.x + 1, loop->coords.y + loop->coords.h + 1);
				pen = (HPEN)SelectObject(dc, pen);
				DeleteObject(pen);
			}
			else if (loop->bmp || loop->disabled) {
				int p, q;
				BITMAP bm;
				HBITMAP draw;
				
				if (loop->state == BSTATE_DISABLED)
					draw = loop->disabled;
				else 
					draw = loop->bmp;
				
				if (!draw) draw = loop->bmp;
				
				GetObject(loop->bmp, sizeof(bm), &bm);
				p = (int)((float)(loop->coords.w - bm.bmWidth) / 2.0 + .5) + loop->coords.x;
				q = (int)((float)(loop->coords.h - bm.bmHeight) / 2.0 + .5) + loop->coords.y;
				// calculate the centering x,y values
				
				bmp = (HBITMAP)SelectObject(sdc, draw);
				BitBlt(dc, p, q,
					bm.bmWidth, bm.bmHeight,
					sdc, 0, 0, SRCCOPY);
				(void)SelectObject(sdc, bmp);
				DrawTBState(dc, loop->state != BSTATE_DISABLED && loop->state != BSTATE_MOUSEOVER && currentTool == loop->id?BSTATE_PUSHIN:loop->state,
                    loop->coords.x, loop->coords.y,
                    loop->coords.w, loop->coords.h);
			}
			else {
				SIZE sz;
				int p, q;
				
				GetTextExtentPoint32(dc, loop->text, loop->text?string_len(loop->text):0, &sz);
				p = (loop->coords.w - sz.cx) / 2 + loop->coords.x;
				q = (loop->coords.h - sz.cy) / 2 + loop->coords.y;
				
				if (loop->state == BSTATE_DISABLED)
					SetTextColor(dc, 0x00666666);
				else
					SetTextColor(dc, 0x00000000);
				
				TextOut(dc, p, q,
					loop->text, string_len(loop->text));
				DrawTBState(dc, loop->state != BSTATE_MOUSEOVER && currentTool == loop->id?BSTATE_PUSHIN:loop->state,
					loop->coords.x, loop->coords.y,
					loop->coords.w, loop->coords.h);
			}
		}
		
		DeleteDC(sdc);
		
		EndPaint(hwnd, &ps);
		break;
		
    case WM_MOUSEMOVE:
		{
			signed short x = LOWORD(lparm);
			signed short y = HIWORD(lparm);
			RECT rc;
			button *btn = get_button(x, y);
			button *loop;
			
			SetCapture(hwnd);
			GetClientRect(hwnd, &rc);
			
			if (!(wparm & MK_LBUTTON)) { // pop out
				
				if (btn && (btn->state == BSTATE_MOUSEOVER || btn->state == BSTATE_DISABLED)) break;
				
				for (loop = button_list; loop; loop = loop->next) {
					if (loop == btn || loop->state == BSTATE_DISABLED) continue;
					loop->state = BSTATE_NORMAL;
				}
				
				if (btn && btn->id != TB_SEPARATOR) btn->state = BSTATE_MOUSEOVER;
				DrawToolbar();
			}
			else { // push in
				if (btn && (btn->state == BSTATE_PUSHIN || btn->state == BSTATE_DISABLED)) break;
				
				for (loop = button_list; loop; loop = loop->next) {
					if (loop->state == BSTATE_DISABLED) continue;
					if (loop->state == BSTATE_PUSHIN) loop->state = BSTATE_SELECT;
					else if (loop->state != BSTATE_SELECT)
						loop->state = BSTATE_NORMAL;
				}
				
				if (btn && btn->state == BSTATE_SELECT) btn->state = BSTATE_PUSHIN;
			}
			
			DrawToolbar();
			
			if (x > (rc.right - rc.left) || x < 0 ||
				y > (rc.bottom - rc.top) || y < 0)
				ReleaseCapture();
			break;
		}
		
    case WM_LBUTTONDOWN:
		{
			signed short x = LOWORD(lparm);
			signed short y = HIWORD(lparm);
			RECT rc;
			button *btn = get_button(x, y);
			button *loop;
			
			SetCapture(hwnd);
			GetClientRect(hwnd, &rc);
			
			if (btn && btn->state == BSTATE_DISABLED) break;
			
			for (loop = button_list; loop; loop = loop->next) {
				if (loop == btn || loop->state == BSTATE_DISABLED) continue;
				if (loop->state != BSTATE_NORMAL) loop->state = BSTATE_NORMAL;
			}
			
			if (btn && btn->id != TB_SEPARATOR) btn->state = BSTATE_PUSHIN;
			
			DrawToolbar();
			if (x > (rc.right - rc.left) || x < 0 ||
				y > (rc.bottom - rc.top) || y < 0)
				ReleaseCapture();
			break;
		}
		
    case WM_LBUTTONUP:
		{
			signed short x = LOWORD(lparm);
			signed short y = HIWORD(lparm);
			button *btn = get_button(x, y);
			button *loop;
			int id = 0;
			
			BringWindowToTop(GetParent(hwnd));
			
			
			if (btn && (btn->state == BSTATE_PUSHIN || btn->state == BSTATE_SELECT))
				id = btn->id;
			
			for (loop = button_list; loop; loop = loop->next)
				if (loop->state != BSTATE_NORMAL && loop->state != BSTATE_DISABLED) loop->state = BSTATE_NORMAL;
				
				SendMessage(GetParent(hwnd), WM_TBCOMMAND, MAKEWPARAM(id, 0), 0);
				DrawToolbar();
				break;
		}      
		
	case WM_COMMAND:
		if (HIWORD(wparm) == 1)
			return SendMessage(GetParent(hwnd), msg, wparm, lparm);
		else
			return DefWindowProc(hwnd, msg, wparm, lparm);         
		
    default:
		return DefWindowProc(hwnd, msg, wparm, lparm);
  }
  return 0;
}

void create_shell_toolbar() {
	HWND hwnd;
	WNDCLASS wc;
	char *lpszClassname = "MalletEdit_tb";
	
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_OWNDC|CS_BYTEALIGNCLIENT|CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = (WNDPROC)ToolbarWindowProc;
	wc.lpszClassName = lpszClassname;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	if (!RegisterClass(&wc)) DestroyWindow(frameWnd);
	
	hwnd = CreateWindow(lpszClassname, NULL,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
		0, 0, 400, 32,
		frameWnd, (HMENU)ID_TOOLBAR, hInstance, NULL);
	ShowWindow(hwnd, SW_SHOW);
}



