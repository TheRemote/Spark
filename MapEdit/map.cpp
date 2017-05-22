#include "main.h"
#include "extern.h"

extern "C" {
	extern __cdecl uncompress(BYTE *, long *, BYTE *, DWORD);
	extern __cdecl compress2(BYTE *, long *, BYTE *, DWORD, DWORD);
}

struct MapInfo *map_list = NULL;

BOOL useAnim = FALSE;
int constrict = 0;// 0 - none, 1 - horizontal, 2 - vertical

int flagPoles[] = {
	// this is set up so that flagPoles[x*6] = x teams flag
	0x1C, 0x19, 0x1A, 0x1B, 0x80,
		0x20, 0x25, 0x22, 0x23, 0x81,
		0x28, 0x29, 0x2E, 0x2B, 0x82,
		0x3A, 0x3B, 0x3C, 0x41, 0x83
};
int switches[] = {
	0x7B, -1, 0x0
};
int neutralFlags[] = {
	0x8C, -1, 0x0
};
int powerupPosses[] = {
	36, 37, 38, 39,
		76, 77, 78, 79,
		-1, 0x0
};
int warps[]		= { 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0x9e };
int NUM_WARPS	=	(sizeof(warps)/sizeof(*warps));

char *undo_types[] = {
	"edit",
	"select"
};


int hWallType = 0;
int hFlagType = 0;
int hPoleType = 0;
int hHoldingType = 0;
int hBunkerDir = 0;
int hBunkerType = 0;
int hSpawnType = 0;
int hBridgeDir = 0;
int hBridgeType = 0;
int hWarpDest = 0;
int hWarpSrc = 0;
int hWarpIdx = 0;
int hSwitchType = 0;
int hConvDir = 0;
int hConvType = 0;

HCURSOR oldCursor = NULL;
HCURSOR myCursor = NULL;

int CALLBACK MapAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map;
	
	map = get_map(hwnd);
	
	switch(msg) {
		// each case will have its own scope
    case WM_CREATE:
		break;
		
    case WM_DESTROY:
		if (map) killMap(map);
		PostQuitMessage(0);
		break;
		
    case WM_SIZE:
		{
			int scr_range;
			int height = HIWORD(lparm);
			int width = LOWORD(lparm);
			SCROLLINFO si;
			
			memset(&si, 0, sizeof(si));
			si.fMask = SIF_RANGE;
			si.cbSize = sizeof(si);
			
			if (map)
				scr_range = map->header.height*16 - height;
			else
				scr_range = 0;
			si.nMax = scr_range;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			if (map)
				scr_range = map->header.width*16 - width;
			else
				scr_range = 0;
			si.nMax = scr_range;
			SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
			
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			
			return DefMDIChildProc(hwnd, msg, wparm, lparm);
		}

	case WM_MDIACTIVATE:
		{
			if ((HWND)lparm == hwnd) {
				if (map && map->selection.tile != -1 && map->selection.horz && map->selection.vert)
					EnableButtons();
				else
					DisableSelectButtons();
			}
			break;
		}
		
	case WM_COMMAND:
		if (HIWORD(wparm) == 1 && lparm == 0)
			return SendMessage(frameWnd, msg, wparm, lparm);
		else
			return DefMDIChildProc(hwnd, msg, wparm, lparm);
		
    case WM_SYSCOMMAND:                                                  
		{
			int result = 0;
			OPENFILENAME ofn;
			char dir[256];
			char file[256];
			
			memset(&ofn, 0, sizeof(ofn));
			memset(dir, 0, sizeof(dir));
			memset(file, 0, sizeof(file));
			
			switch (wparm) {
			case SC_CLOSE:
				if (map && map->modify) {
					result = MessageBox(NULL, "Would you like to save your changes?",
						"Close", MB_YESNOCANCEL);
					switch (result) {
					case IDYES:
						
						memset(dir, 0, sizeof(dir));
						memset(file, 0, sizeof(file));
						memset(&ofn, 0, sizeof(ofn));
						GetCurrentDirectory(sizeof(dir), dir);
						
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hwnd;
						ofn.hInstance = hInstance;
						ofn.lpstrFilter = "ARC Map Files\0*.map\0\0";
						ofn.nFilterIndex = 1;
						ofn.lpstrFile = file;
						ofn.nMaxFile = sizeof(file);
						ofn.lpstrInitialDir = dir;
						ofn.Flags =
							OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|
							OFN_PATHMUSTEXIST|OFN_NOLONGNAMES;
						ofn.lpstrDefExt = "map";
						
						if (!map->file) {
							if (GetSaveFileName(&ofn))
								SaveMap(file, map);
							else
								return 0;
						}
						else
							SaveMap(map->file, map);
						
						return DefMDIChildProc(hwnd, msg, wparm, lparm);
						
					case IDNO:
						return DefMDIChildProc(hwnd, msg, wparm, lparm);
						
					case IDCANCEL:
						return 0;
					}
				}
				break;
				
				
			default:
				break;
			}
			return DefMDIChildProc(hwnd, msg, wparm, lparm);
		}
		
		
    case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc, bbuf;
			HBITMAP backBuf;
			RECT rc;
			SCROLLINFO si;
			int x, y, maxx, maxy, tile, W, H;
			int scrollx, scrolly;
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			scrolly = si.nPos;
			GetScrollInfo(hwnd, SB_HORZ, &si);
			scrollx = si.nPos;
			
			scrollx = (scrollx>>4)<<4;
			scrolly = (scrolly>>4)<<4;
			
			BeginPaint(hwnd, &ps);
			hdc = ps.hdc;

        	GetClientRect(hwnd, &rc);
			maxx = (rc.right - rc.left + GetScrollPos(hwnd, SB_HORZ))/16;
			maxy = (rc.bottom - rc.top + GetScrollPos(hwnd, SB_VERT))/16;

			SetWindowOrgEx(hdc, GetScrollPos(hwnd, SB_HORZ), GetScrollPos(hwnd, SB_VERT), NULL);
			
			maxx++;
			maxy++;
			
			
			if (!map || !map->mapData) {
				SetBkMode(hdc, TRANSPARENT);
				TextOut(hdc, 0, 0, "No map data", 11);
			}
			else {
				maxx = MIN((map->header.width), maxx);
				maxy = MIN((map->header.height), maxy);

				bbuf = CreateCompatibleDC(hdc);
				backBuf = CreateCompatibleBitmap(tilesDC, rc.right - rc.left + GetScrollPos(hwnd, SB_HORZ)%16, rc.bottom - rc.top + GetScrollPos(hwnd, SB_VERT)%16);
				W = rc.right - rc.left + GetScrollPos(hwnd, SB_HORZ)%16; H = rc.bottom - rc.top + GetScrollPos(hwnd, SB_VERT) % 16;
				backBuf = (HBITMAP)SelectObject(bbuf, backBuf);


				/*
				if (map->mouseOver.valid)
					DrawFocusRect(hdc, &map->mouseOver.rc);				

				
				// clip selection
				{
					HRGN larger, smallr, r3;
					RECT rc;

					GetClientRect(hwnd, &rc);
					larger = CreateRectRgnIndirect(&rc);

                    if (map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
						rc.left = ((map->selection.tile % 256) << 4) - GetScrollPos(hwnd, SB_HORZ);
						rc.top = ((map->selection.tile / 256) << 4) - GetScrollPos(hwnd, SB_VERT);
						rc.right = rc.left + (map->selection.horz << 4);
						rc.bottom = rc.top + (map->selection.vert << 4);
						r3 = CreateRectRgnIndirect(&rc);
						rc.left++;
						rc.right--;
						rc.bottom--;
						rc.top++;
						smallr = CreateRectRgnIndirect(&rc);
						CombineRgn(r3, r3, smallr, RGN_DIFF);
						CombineRgn(larger, larger, r3, RGN_DIFF);
						DeleteObject(smallr);
						DeleteObject(r3);
					}

					SelectClipRgn(hdc, larger);
					DeleteObject(larger);
				}
				*/
				
				for (x = scrollx/16; x < maxx; x++) {
					int drawx = (x-scrollx/16)*16;
					for (y = scrolly/16; y < maxy; y++) {
						int drawy = (y-scrolly/16)*16;
						tile = map->mapData[(y*map->header.width)+x];
						if (!(tile & 0x8000))
							DrawTileBlock(tile, bbuf, drawx, drawy, 1, 1, FALSE);
						else {
							unsigned char offset;
							unsigned char idx;
							struct Animation *anim;
							
							offset = (char)((tile & 0x7F00) >> 8);
							idx = (char)((tile & 0x00FF));
							
							SetBkMode(bbuf, OPAQUE);
							
							if ((anim = get_anim(idx)) && anim->frameCount > 0)
								DrawTileBlock(anim->frames[(offset + map->anim_frame /*/ anim->speed*/) % anim->frameCount], bbuf, drawx, drawy, 1, 1, FALSE);
							else
								TextOut(bbuf, drawx, drawy, "a", 1);
						}
					}
				}

				//SelectClipRgn(hdc, NULL);				
				
				if (map->mouseOver.valid) {
					RECT rc = map->mouseOver.rc;
					rc.left -= scrollx;
					rc.right -= scrollx;
					rc.top -= scrolly;
					rc.bottom -= scrolly;
					FrameRect(bbuf, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
				}
				if (map->lineTool.valid) {
					HPEN pen = (HPEN)GetStockObject(WHITE_PEN);
					pen = (HPEN)SelectObject(bbuf, pen);
					MoveToEx(bbuf, map->lineTool.start.x-scrollx, map->lineTool.start.y - scrolly, NULL);
					LineTo(bbuf, map->lineTool.stop.x - scrollx, map->lineTool.stop.y - scrolly);
					pen = (HPEN)SelectObject(bbuf, pen);
					DeleteObject(pen);

					RECT rc;
					rc.left = (map->lineTool.start.x / 16) * 16 - scrollx;
					rc.right = rc.left + 17;
					rc.top = (map->lineTool.start.y / 16) * 16 - scrolly;
					rc.bottom = rc.top + 17;
					FrameRect(bbuf, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
					rc.left = (map->lineTool.stop.x / 16) * 16 - scrollx;
					rc.right = rc.left + 17;
					rc.top = (map->lineTool.stop.y / 16) * 16 - scrolly;
					rc.bottom = rc.top + 17;
					FrameRect(bbuf, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));


				}
				if (map->selection.tile > -1 && map->selection.horz && map->selection.vert) {
					HPEN pen = (HPEN)GetStockObject(WHITE_PEN);
					
					DrawSelection(bbuf, pen, (map->selection.tile % map->header.width)*16 - scrollx,
                        (map->selection.tile / map->header.width)*16 - scrolly,
                        map->selection.horz * 16,
                        map->selection.vert * 16);
					DeleteObject(pen);
				}

				BitBlt(hdc, GetScrollPos(hwnd, SB_HORZ)-GetScrollPos(hwnd, SB_HORZ)%16, GetScrollPos(hwnd, SB_VERT)-GetScrollPos(hwnd, SB_VERT)%16, W, H, bbuf, 0, 0, SRCCOPY);
				backBuf = (HBITMAP)SelectObject(bbuf, backBuf);
				DeleteObject(backBuf);
				DeleteDC(bbuf);
				
			}

			EndPaint(hwnd, &ps);
			break;
    }   
	
    case WM_VSCROLL:
		{
			int nScrollCode = LOWORD(wparm);
			int nPos = HIWORD(wparm);
			int add_factor = 0;
			int page;
			RECT rc;
			SCROLLINFO si;
			
			GetClientRect(hwnd, &rc);
			page = rc.bottom - rc.top;
			
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			
			switch (nScrollCode) {
			case SB_BOTTOM:
			case SB_LINEDOWN:
				add_factor = 16;
				break;
				
			case SB_TOP:
			case SB_LINEUP:
				add_factor = -16;
				break;
				
			case SB_PAGEUP:
				add_factor = -page;
				break;
				
			case SB_PAGEDOWN:
				add_factor = page;
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
				if (overDlg)
					RedrawWindow(overDlg, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);

			}
			break;
		}
		
    case WM_HSCROLL:
		{
			int nScrollCode = LOWORD(wparm);
			int nPos = HIWORD(wparm);
			int add_factor = 0;
			int page;
			RECT rc;		
			SCROLLINFO si;
			
			GetClientRect(hwnd, &rc);
			page = rc.right - rc.left;
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_HORZ, &si);
			
			switch (nScrollCode) {
			case SB_BOTTOM:
			case SB_LINEDOWN:
				add_factor = 96;
				break;
				
			case SB_TOP:
			case SB_LINEUP:
				add_factor = -96;
				break;
				
			case SB_PAGEUP:
				add_factor = -160;
				break;
				
			case SB_PAGEDOWN:
				add_factor = 160;
				break;
				
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				add_factor = nPos - si.nPos;
				break;
				
			default:
				break;
			}
			
			add_factor = MAX(si.nMin - si.nPos, MIN(si.nMax - si.nPos, add_factor));
			
			if (add_factor != 0) {
				si.fMask = SIF_POS;
				si.nPos += add_factor;

				SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
				ScrollWindowEx(hwnd, -add_factor, 0, NULL, NULL, NULL, NULL, SW_ERASE);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
				if (overDlg)
					RedrawWindow(overDlg, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);

			}
			break;
		}
		
    case WM_MOUSEMOVE:
		{
			int x = LOWORD(lparm);
			int y = HIWORD(lparm);
			int scy, scx;
			RECT rc;
			char buf[256];
			POINT pos;
			WINDOWPLACEMENT wp;
			int inselect, w, h;
			
			if (!map) break;

			pos.x = x;
			pos.y = y;

			memset(&wp, 0, sizeof(wp));
			wp.length = sizeof(wp);
			GetWindowPlacement(hwnd, &wp);

			GetClientRect(hwnd, &rc);
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;

			switch (wp.showCmd) {
				case SW_SHOWMAXIMIZED:
					rc.left = wp.ptMaxPosition.x;
					rc.top = wp.ptMaxPosition.y + GetSystemMetrics(SM_CYMENU);
					rc.right = rc.left + w;
					rc.bottom = rc.top + h;
					break;

				case SW_SHOWMINIMIZED:
					rc.left = wp.ptMinPosition.x;
					rc.right = rc.left + w;
					rc.top = wp.ptMinPosition.y;
					rc.bottom = rc.top + h;
					break;

				default:
					rc.left = wp.rcNormalPosition.left;
					rc.right = rc.left + w;
					rc.top = wp.rcNormalPosition.top;
					rc.bottom = rc.top + h;
					break;
			}

			
			scy = GetScrollPos(hwnd, SB_VERT);
			scx = GetScrollPos(hwnd, SB_HORZ);

			if (PtInRect(&rc, pos)) {
				if (GetCapture() != hwnd) 
					SetCapture(hwnd);
				else {
					/* check that this is our window */
					HDC mydc = GetDC(hwnd), sdc = GetDC(NULL);
					POINT screen_coords = { pos.x, pos.y };
					ClientToScreen(hwnd, &screen_coords);
					COLORREF screen_color = GetPixel(sdc, screen_coords.x, screen_coords.y), mycol = GetPixel(mydc, pos.x+scx, pos.y+scy);
					ReleaseDC(hwnd, mydc);
					ReleaseDC(NULL, sdc);

					if (mycol != screen_color) {
						SendMessage(hwnd, WM_MOUSELEAVE, wparm, lparm);
						ReleaseCapture();
						break;
					}
				}
			}
			else {
				if (GetCapture() == hwnd) {
					SendMessage(hwnd, WM_MOUSELEAVE, wparm, lparm);
					ReleaseCapture();
					break;
				}
			}

			if (autoPan) {
				map->pan = 0;
				if (abs(rc.left - pos.x) < 16)
					map->pan |= APAN_LEFT;
				else if (abs(rc.right - pos.x) < 16)
					map->pan |= APAN_RIGHT;
				if (abs(rc.top - pos.y) < 16)
					map->pan |= APAN_UP;
				else if (abs(rc.bottom - pos.y) < 16)
					map->pan |= APAN_DOWN;

				if (map->pan && map->pan_timer == 0)
					map->pan_timer = SetTimer(hwnd, 0x4AC, 100, NULL);
				else if (!map->pan && map->pan_timer) {
					KillTimer(hwnd, map->pan_timer);
					map->pan_timer = 0;
				}
			}
			
			inselect = select_check(map, &pos);
			
			if (map->mouseOver.valid) {
				if ((scx+x)/16*16 == map->mouseOver.rc.left &&
					(scy+y)/16*16 == map->mouseOver.rc.top)
					break;
			}
			
			ClearStatus();
			sprintf(buf, "x: %-5d y: %-5d", (scx+x)/16 + 1, (scy+y)/16 + 1);
			SetStatusText(0, buf);
			if (map->mapData[(scx+x)/16 + (scy+y)/16*map->header.width] & 0x8000) {
				sprintf(buf, "Animation: %-5d (%-2x)",
					map->mapData[(scx+x)/16 + (scy+y)/16*map->header.width] & 0xFF,
					map->mapData[(scx+x)/16 + (scy+y)/16*map->header.width] & 0xFF);
				SetStatusText(1, buf);
				sprintf(buf, "Offset: %-5d",
					(map->mapData[(scx+x)/16 + (scy+y)/16*map->header.width] & 0x7F00)>>8);
				SetStatusText(2, buf);
			}
			else {
				sprintf(buf, "Tile: %-5d",
					map->mapData[(scx+x)/16 + (scy+y)/16*map->header.width]);
				SetStatusText(1, buf);
			}
			
			
			GetClientRect(hwnd, &rc);

			map->mouseOver.valid = FALSE;
			
			switch (currentTool) {
			case TOOL_EDIT:
				{
					memset(&map->mouseOver, 0, sizeof(map->mouseOver));
					
					y += scy;
					x += scx;
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					if (!useAnim && tileSel.tile != -1 && tileSel.horz != 0 && tileSel.vert != 0) {
						map->mouseOver.rc.bottom = map->mouseOver.rc.top +
							(MIN(tileSel.vert, map->header.height - y/16) * 16) - 2;
						map->mouseOver.rc.right = map->mouseOver.rc.left +
							(MIN(tileSel.horz, map->header.width - x/16) * 16) - 2;
						map->mouseOver.valid = TRUE;
					}
					else if (animSel.tile != -1) {
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
						map->mouseOver.valid = TRUE;
					}

					if (myCursor) {
						SetCursor(oldCursor);
						DestroyCursor(myCursor);
						myCursor = oldCursor = NULL;
					}													
					
					break;
				}

			case TOOL_LINE:
				{
					y += scy;
					x += scx;

					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.valid = TRUE;

					if (myCursor) {
						SetCursor(oldCursor);
						DestroyCursor(myCursor);
						myCursor = oldCursor = NULL;
					}
					break;
				}
				
			case TOOL_WALL:
				{
					
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					break;
				}
				
			case TOOL_PICKER:
				{
					
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor()) break;
					
					myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PICKER));
					oldCursor = SetCursor(myCursor);
					break;
				}
				
				
			case TOOL_SPAWN:
				{
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					
					map->mouseOver.valid = TRUE;
					
					break;
				}
				
			case TOOL_FLAG:
			case TOOL_POLE:
				{
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor())
						break;
					
					
					myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_FLAGGER));
					oldCursor = SetCursor(myCursor);
					break;
				}
				
			case TOOL_SWITCH:
				{
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					
					map->mouseOver.valid = TRUE;
					break;
				}
				
			case TOOL_HOLDING_PEN:
				{
					y += scy;
					x += scx;				
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor()) 
						break;
					
					
					myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SELECTER));
					oldCursor = SetCursor(myCursor);
					
					break;
				}

			case TOOL_CONV:
				{
					y += scy;
					x += scx;				
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
											
					break;
				}


			case TOOL_SELECT:
				{

					LPTSTR id;
					HINSTANCE inst = NULL;

					if (myCursor == GetCursor()) break;

					switch (inselect) {
					case 0:
						id = MAKEINTRESOURCE(IDC_SELECTER);
						inst = hInstance;
						break;
					case 1:
						id = IDC_SIZEALL;
						break;
					case 2:
					case 3:
						id = IDC_SIZEWE;
						break;
					case 4:
					case 5:
						id = IDC_SIZENS;
						break;
					case 6:
					case 9:
						id = IDC_SIZENESW;
						break;
					case 7:
					case 8:
						id = IDC_SIZENWSE;
						break;
					default:
						id = NULL;
						break;
					}

					if (id == NULL) break;

					
					
					myCursor = LoadCursor(inst, id);
					oldCursor = SetCursor(myCursor);

					break;
				}
				
			case TOOL_BUNKER:
				{
					int id;
					
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor())
						break;
					
					switch (hBunkerDir) {
					case 0:
						id = IDC_BUNKERLEFT;
						break;
						
					case 1:
						id = IDC_BUNKERRIGHT;
						break;
						
					case 2:
						id = IDC_BUNKERUP;
						break;
						
					case 3:
						id = IDC_BUNKERDOWN;
						break;
						
					default:
						id = -1;
						break;
					}
					
					if (id != -1) {
						myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(id));
						oldCursor = SetCursor(myCursor);
					}
					
					break;
				}
				
			case TOOL_BRIDGE:
				{
					int id = -1;
					
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor())
						break;
					
					switch (hBridgeDir) {
					case 0:
						id = IDC_BRIDGELR;
						break;
						
					case 1:
						id = IDC_BRIDGEUD;
						break;
						
					}
					
					if (id != -1) {
						
						
						myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(id));
						oldCursor = SetCursor(myCursor);
					}
					
					break;
				}
				
			case TOOL_FILL:
				{
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					
					if (myCursor == GetCursor()) 
						break;
					
					
					myCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_FILLBUCKET));
					oldCursor = SetCursor(myCursor);
					
					break;
				}
				
				
			case TOOL_WARP:
				{
					y += scy;
					x += scx;
					
					map->mouseOver.rc.top = y/16*16 + 1;
					map->mouseOver.rc.left = x/16*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					map->mouseOver.valid = TRUE;
					break;
				}
				
			default:
				break;
			}
			
			if (!myCursor || myCursor != GetCursor()) {
				myCursor = oldCursor = LoadCursor(NULL, IDC_ARROW);
				SetCursor(myCursor);
			}

			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			
			break;
    }

	case WM_NCMOUSEMOVE:
		{
			if (wparm != HTCLIENT)
				SendMessage(hwnd, WM_MOUSELEAVE, 0, 0);

			break;
		}
	
    case WM_MOUSELEAVE:
		{
			int x = LOWORD(lparm);
			int y = HIWORD(lparm);
			int scy, scx;
			
			if (oldCursor) {
				SetCursor(oldCursor);
				myCursor = oldCursor = NULL;
			}
			
			if (!map) break;

			if (map->pan_timer) {
				KillTimer(hwnd, map->pan_timer);
				map->pan_timer = 0;
				map->pan = 0;
			}
						
			memset(&map->mouseOver, 0, sizeof(map->mouseOver));
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			break;
		}

	case WM_TIMER:
		{
			int nTimerID = (int)wparm;
			int addx, addy;
			SCROLLINFO sify, sifx;

			sify.cbSize = sifx.cbSize = sizeof(SCROLLINFO);
			sify.fMask = sifx.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &sify);
			GetScrollInfo(hwnd, SB_HORZ, &sifx);

			if (!map) break;
			
			switch (nTimerID) {
				case 0x4AC:
					addx = addy = 0;
					
					if (map->pan & APAN_LEFT)
						addx = -16;
					else if (map->pan & APAN_RIGHT)
						addx = 16;
					if (map->pan & APAN_UP)
						addy = -16;
					else if (map->pan & APAN_DOWN)
						addy = 16;
					
					addx = MAX(sifx.nMin - sifx.nPos, MIN(sifx.nMax - sifx.nPos, addx));
					addy = MAX(sify.nMin - sify.nPos, MIN(sify.nMax - sify.nPos, addy));

					if (addx || addy) {
						sifx.nPos += addx;
						sify.nPos += addy;
						sifx.fMask = sify.fMask = SIF_POS;
						SetScrollInfo(hwnd, SB_VERT, &sify, TRUE);
						SetScrollInfo(hwnd, SB_HORZ, &sifx, TRUE);
						
						ScrollWindowEx(hwnd, -addx, -addy, NULL, NULL, NULL, NULL, SW_ERASE);
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						break;
					}
					else if (!addx && !addy) {
						KillTimer(hwnd, map->pan_timer);
						map->pan_timer = 0;
						map->pan = 0;
					}
					break;

				case TIMER_ANIMFRAME:
					map->anim_frame++;
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					break;

				default:
					break;
			}

			break;
		}

    case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int scx = GetScrollPos(hwnd, SB_HORZ);
			int scy = GetScrollPos(hwnd, SB_VERT);
			int rx, ry, n, tile, w, h, first = 0;
			char buf[256];
			struct undo_buf *undoInfo;
			struct seldata oldselection;
			bool deselect = true;
			
			WORD *oldBuf;
			MSG mouseMsg;
			BOOL err;
			WINDOWPLACEMENT wp;
			
			RECT rc, wprc;
			POINT pos;
			

			pos.x = x;
			pos.y = y;

			wp.length = sizeof(wp);
			GetWindowPlacement(hwnd, &wp);
			GetClientRect(hwnd, &rc);
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;

			switch (wp.showCmd) {
			case SW_SHOWMAXIMIZED:
				wprc.left = wp.ptMaxPosition.x;
				wprc.top = wp.ptMaxPosition.y + GetSystemMetrics(SM_CYMENU);
				wprc.right = wprc.left + w;
				wprc.bottom = wprc.top + h;
				break;

			case SW_SHOWMINIMIZED:
				wprc.left = wp.ptMinPosition.x;
				wprc.right = wprc.left + w;
				wprc.top = wp.ptMinPosition.y;
				wprc.bottom = wprc.top + h;
				break;

			default:
				wprc.left = wp.rcNormalPosition.left;
				wprc.right = wprc.left + w;
				wprc.top = wp.rcNormalPosition.top;
				wprc.bottom = wprc.top + h;
				break;
			}

			SetCapture(hwnd);
			
			x = rx = (x+scx)/16;
			y = ry = (y+scy)/16;

						
			if (map) {
				int h, w, j, k;
				
				oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
				oldselection = map->selection;
				memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
				
				
				switch (currentTool) {
				case TOOL_EDIT:
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					}
					
					if (x < map->header.width && y < map->header.height) {
						EditMap(x, y, useAnim, map);
						rc.left = x*16 - scx; rc.right = rc.left + (!useAnim?tileSel.horz*16:16);
						rc.top = y*16 - scy; rc.bottom = rc.top + (!useAnim?tileSel.vert*16:16);
//						RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				
				case TOOL_LINE:
					{
						map->mouseOver.valid = FALSE;
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						if (x < map->header.width && y < map->header.height) {
							map->lineTool.start.x = map->lineTool.stop.x = (pos.x + scx)/16 * 16 + 8;
							map->lineTool.start.y = map->lineTool.stop.y = (pos.y + scy)/16 * 16 + 8;
							map->lineTool.valid = TRUE;
						}
						break;
					}
					
							
				case TOOL_WALL:
					{
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
/*
						if (x < map->header.width && y < map->header.height) {
							set_wall_tile(map, x, y, hWallType);
							rc.left = MAX(0,(x-1)*16 - scx); 
							rc.right = rc.left + 48;
							rc.top = MAX(0,(y-1)*16 - scy); 
							rc.bottom = rc.top + 48;
							RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						}
*/
						break;
					}
					
				case TOOL_PICKER:
					{
						char offset[4];
						WORD tile;
						
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						tile = map->mapData[x+y*map->header.width];
						if (tile & 0x8000) {
							animSel.horz = animSel.vert = 1;
							animSel.tile = tile & 0xFF;
							sprintf(offset, "%d", (tile & 0x7F00) >> 8);
							SetDlgItemText(GetDlgItem(frameWnd, ID_INFOAREA), ID_OFFSET, offset);
							
							useAnim = TRUE;
							SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							SetScrollPos(GetDlgItem(frameWnd, ID_ANIMATIONS), SB_VERT, animSel.tile * 16, TRUE);
							RedrawWindow(GetDlgItem(frameWnd, ID_ANIMATIONS), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
						}
						else {
							int i;
							tileSel.horz = tileSel.vert = 1;
							tileSel.tile = tile;
							useAnim = FALSE;
							
							SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							
							for (i = 0; i < numTiles; i++)
								if (tileArrange[i] == tile) break;
								if (i >= numTiles) break;
								
								SetScrollPos(GetDlgItem(frameWnd, ID_TILEAREA), SB_HORZ, (i % 40) * 16, TRUE);
								SetScrollPos(GetDlgItem(frameWnd, ID_TILEAREA), SB_VERT, (i / 40) * 16, TRUE);
								RedrawWindow(GetDlgItem(frameWnd, ID_TILEAREA), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);							
						}

						map->mouseOver.rc.top = y*16 + 1;
						map->mouseOver.rc.left = x*16 + 1;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
						
						
						break;
					}
					
					
					
				case TOOL_FLAG:
					{
						int i, j;
						
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						for (i = y; i < (y + 3); i++) {
							if (i > map->header.height-1 || i < 0) continue;
							for (j = x; j < (x + 3); j++) {
								if (j > -1 && j < map->header.width) {
									map->mapData[j+i*map->header.width] = flag_data[hFlagType][(i-y)*3+(j-x)];
								}
							}
						}
						
						rc.left = MAX(0,x*16 - scx);
						rc.right = rc.left + 48;
						rc.top = MAX(0,y*16 - scy);
						rc.bottom = rc.top + 48;
						RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
						
						map->header.objective = OBJECTIVE_FLAG;
						
						break;
					}
					
				case TOOL_POLE:
					{
						int i, j;
						
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						for (i = y; i < (y + 3); i++) {
							if (i > map->header.height-1 || i < 0) continue;
							for (j = x; j < (x + 3); j++) {
								if (j > -1 && j < map->header.width) {
									if ((i-y) == 1 && (j-x) == 1) 
										map->mapData[j+i*map->header.width] = pole_data[hPoleType][9+hTeamSelected];
									else
										map->mapData[j+i*map->header.width] = pole_data[hPoleType][(i-y)*3+(j-x)];
								}
							}
						}
						
						rc.left = MAX(0,x*16 - scx);
						rc.right = rc.left + 48;
						rc.top = MAX(0,y*16 - scy);
						rc.bottom = rc.top + 48;
						RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
						
						map->header.objective = OBJECTIVE_FLAG;
						
						break;
					}
					
				case TOOL_SWITCH:
					{
						int i, j;
						
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						for (i = y; i < (y + 3); i++) {
							if (i > map->header.height-1 || i < 0) continue;
							for (j = x; j < (x + 3); j++) {
								if (j > -1 && j < map->header.width) {
									map->mapData[j+i*map->header.width] = switch_data[hSwitchType][(i-y)*3+(j-x)];
								}
							}
						}
						
						rc.left = MAX(0,x*16 - scx);
						rc.right = rc.left + 48;
						rc.top = MAX(0,y*16 - scy);
						rc.bottom = rc.top + 48;
						
					
						map->mouseOver.rc.top = y*16 + 1;
						map->mouseOver.rc.left = x*16 + 1;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
						
						map->mouseOver.valid = TRUE;    
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);

						
						
						map->header.objective = OBJECTIVE_SWITCH;
						
						break;
					}
					
				case TOOL_SPAWN:
					{
						int i, j;
						
						ClearStatus();
						sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
						SetStatusText(0, buf);
						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
						
						
						for (i = y; i < (y + 3); i++) {
							if (i < 0 || i > map->header.height-1) continue;
							for (j = x; j < (x + 3); j++) {
								if (j < 0 || j > map->header.width-1 ||
									spawn_data[hTeamSelected+hSpawnType*4][(i-y)*3+(j-x)] < 0) continue;
								map->mapData[i*map->header.width+j] = spawn_data[hTeamSelected+hSpawnType*4][(i-y)*3+(j-x)];
							}
						}
						
						rc.left = MAX(0,x*16 - scx);
						rc.right = rc.left + 48;
						rc.top = MAX(0,y*16 - scy);
						rc.bottom = rc.top + 48;
						
						
						
						map->mouseOver.rc.top = y*16 + 1;
						map->mouseOver.rc.left = x*16 + 1;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
						
						map->mouseOver.valid = TRUE;    
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						
						break;
					}
					
				case TOOL_WARP:
					{
						WORD tile;
						
						tile = warps[hWarpIdx] | 0x8000 | (((hWarpDest * 10) + hWarpSrc) << 8);
						map->mapData[x+y*map->header.width] = tile;
						
						rc.left = MAX(0,x*16 - scx);
						rc.right = rc.left + 16;
						rc.top = MAX(0,y*16 - scy);
						rc.bottom = rc.top + 16;
											
						map->mouseOver.rc.top = y*16 + 1;
						map->mouseOver.rc.left = x*16 + 1;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
						
						map->mouseOver.valid = TRUE;    
						
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						break;
					}			

				case TOOL_SELECT:
					{
						RECT rc;

						map->seltype = select_check(map, &pos);

						if (!map->seltype) {
							struct seldata newsel;
							//HPEN whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
							
							newsel.tile = x + (y*map->header.width);
							newsel.horz = 0;
							newsel.vert = 0;

							/*
							if (map->selection.tile > -1) { // get rid of the old one
								memset(&rc, 0, sizeof(rc));
								rc.left = (map->selection.tile % map->header.width)*16 - scx;
								rc.right = rc.left + (map->selection.horz << 4);
								rc.top = (map->selection.tile / map->header.width)*16 - scy;
								rc.bottom = (map->selection.vert << 4) + rc.top;
								map->selection.tile = -1;
								RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
							}
							*/

							map->selection = newsel;
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);
							//DeleteObject(whitepen);
							
							while ((n = ShowCursor(FALSE)) > -1) continue;
						}
						else {
						//	rx = map->selection.tile % map->header.width * 16;
						//	ry = map->selection.tile / map->header.width * 16;
						}
						
						ClearStatus();
						sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
																   map->selection.tile / map->header.width + 1);
						SetStatusText(0, buf);
						sprintf(buf, "Horizontal: %-5d", map->selection.horz);
						SetStatusText(1, buf);
						sprintf(buf, "Vertical: %-5d", map->selection.vert);
						SetStatusText(2, buf);
						
/*
						memset(&rc, 0, sizeof(rc));
						rc.left = (map->selection.tile % 256)<<4;
						rc.right = rc.left + (map->selection.horz << 4);
						rc.top = (map->selection.tile >> 8)<<4;
						rc.bottom = (map->selection.vert << 4) + rc.top;
						RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
*/
									
						break;
					}
					
				case TOOL_HOLDING_PEN:
				case TOOL_BUNKER:
				case TOOL_BRIDGE:
				case TOOL_CONV:
					{
						//HPEN whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
						struct seldata newsel;

						newsel.tile = x + (y*map->header.width);
						newsel.horz = 0;
						newsel.vert = 0;
						
						/*if (map->selection.tile > -1) { // get rid of the old one
							memset(&rc, 0, sizeof(rc));
							rc.left = (map->selection.tile % map->header.width)*16 - scx;
							rc.right = rc.left + (map->selection.horz << 4);
							rc.top = (map->selection.tile / map->header.width)*16 - scy;
							rc.bottom = (map->selection.vert << 4) + rc.top;
							map->selection.tile = -1;
							RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						}*/

						//NewSelection(dc, whitepen, &map->selection, &newsel, map);
						//DeleteObject(whitepen);
						map->selection = newsel;
						
						ClearStatus();
						sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
																   map->selection.tile / map->header.width + 1);
						SetStatusText(0, buf);
						sprintf(buf, "Horizontal: %-5d", map->selection.horz);
						SetStatusText(1, buf);
						sprintf(buf, "Vertical: %-5d", map->selection.vert);
						SetStatusText(2, buf);
						
						/*
						memset(&rc, 0, sizeof(rc));
						rc.left = (map->selection.tile % 256)<<4;
						rc.right = rc.left + (map->selection.horz << 4);
						rc.top = (map->selection.tile >> 8)<<4;
						rc.bottom = (map->selection.vert << 4) + rc.top;
						RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						*/

						while ((n = ShowCursor(FALSE)) > -1) continue;
						
						break;
					}
					
				case TOOL_FILL:
					{
						WORD filltile;
						
						filltile = map->mapData[y*map->header.width + x];
						
						FillBits(filltile, x, y, map);
					}
					break;
					
					
				default:
					break;
        }

		RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);	
        
        
		if (autoPan) {
			map->pan = 0;
			if (abs(wprc.left - pos.x) < 16)
				map->pan |= APAN_LEFT;
			else if (abs(wprc.right - pos.x) < 16)
				map->pan |= APAN_RIGHT;
			if (abs(wprc.top - pos.y) < 16)
				map->pan |= APAN_UP;
			else if (abs(wprc.bottom - pos.y) < 16)
				map->pan |= APAN_DOWN;
			
			if (map->pan && map->pan_timer == 0)
				map->pan_timer = SetTimer(hwnd, 0x4AC, 100, NULL);
			else if (!map->pan && map->pan_timer) {
				KillTimer(hwnd, map->pan_timer);
				map->pan_timer = 0;
			}
		}
			
        
        while (1) {
			err = GetMessage(&mouseMsg, hwnd, 0, 0);
			if (err < 1) break;

			if (mouseMsg.message < WM_MOUSEFIRST || mouseMsg.message > WM_MOUSELAST) {
				CallWindowProc((WNDPROC)MapAreaProc, hwnd, mouseMsg.message, mouseMsg.wParam, mouseMsg.lParam);
				continue;
			}
			
			if (mouseMsg.message == WM_LBUTTONUP || !(mouseMsg.wParam & MK_LBUTTON))
				break;

			scx = GetScrollPos(hwnd, SB_HORZ);
			scy = GetScrollPos(hwnd, SB_VERT);
			
			
			x = GET_X_LPARAM(mouseMsg.lParam);
			y = GET_Y_LPARAM(mouseMsg.lParam);

			pos.x = x;
			pos.y = y;

			if (autoPan) {
				map->pan = 0;
				if (abs(wprc.left - pos.x) < 16)
					map->pan |= APAN_LEFT;
				else if (abs(wprc.right - pos.x) < 16)
					map->pan |= APAN_RIGHT;
				if (abs(wprc.top - pos.y) < 16)
					map->pan |= APAN_UP;
				else if (abs(wprc.bottom - pos.y) < 16)
					map->pan |= APAN_DOWN;
				
				if (map->pan && map->pan_timer == 0)
					map->pan_timer = SetTimer(hwnd, 0x4AC, 100, NULL);
				else if (!map->pan && map->pan_timer) {
					KillTimer(hwnd, map->pan_timer);
					map->pan_timer = 0;
				}
			}
			
			x = MAX(0,(x + scx)/16);
			y = MAX(0,(y + scy)/16);
			
			
			switch(currentTool) {
            case TOOL_EDIT:
				if (x > map->header.width-1 || y > map->header.height-1) break;
				ClearStatus();
				sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
				SetStatusText(0, buf);
				if (map->mapData[x + y*map->header.width] & 0x8000) {
					sprintf(buf, "Animation: %-5d (%-2x)",
						map->mapData[x + y*map->header.width] & 0xFF,
						map->mapData[x + y*map->header.width] & 0xFF);
					SetStatusText(1, buf);
					sprintf(buf, "Offset: %-5d",
						(map->mapData[x + y*map->header.width] & 0x7F00)>>8);
					SetStatusText(2, buf);
				}
				else {
					sprintf(buf, "Tile: %-5d",
						map->mapData[x+y*map->header.width]);
					SetStatusText(1, buf);
				}
				
				if (x < map->header.width && y < map->header.height &&
					x > -1 && y > -1)
				{
					EditMap(MAX(x,0), MAX(y,0), useAnim, map);
					
					
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					if (!useAnim) {
						map->mouseOver.rc.bottom = map->mouseOver.rc.top +
							(MIN(tileSel.vert, map->header.height - y - 1) * 16) - 2;
						map->mouseOver.rc.right = map->mouseOver.rc.left +
							(MIN(tileSel.horz, map->header.width - x - 1) * 16) - 2;
					}
					else {
						map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					}
					map->mouseOver.valid = TRUE; 

				}
				break;

				
			case TOOL_LINE:
				{
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);

					if (x > -1 && x < map->header.width && y > -1 && y < map->header.height)
					{

						if (map->mapData[x+y*map->header.width] & 0x8000) {
							sprintf(buf, "Animation: %-5d (%-2x)",
								map->mapData[x+y*map->header.width] & 0xFF,
								map->mapData[x+y*map->header.width] & 0xFF);
							SetStatusText(1, buf);
							sprintf(buf, "Offset: %-5d",
								(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
							SetStatusText(2, buf);
						}
						else {
							sprintf(buf, "Tile: %-5d",
								map->mapData[x+y*map->header.width]);
							SetStatusText(1, buf);
						} 
					}

					if (x > -1 && x < map->header.width)
						map->lineTool.stop.x = (pos.x + scx)/16 * 16 + 8;
					else {
						if (x < 0) map->lineTool.stop.x = 0;
						else map->lineTool.stop.x = map->header.width * 16 - 8;
					}
					if (y > -1 && y < map->header.height)
						map->lineTool.stop.y = (pos.y + scy)/16 * 16 + 8;
					else {
						if (y < 0) map->lineTool.stop.y = 0;
						else map->lineTool.stop.y = map->header.height * 16 - 8;
					}
						
					
					break;
				}
				
				
			case TOOL_WALL:
				{
					WORD shift;

					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", (scx+x)/16 + 1, (scy+y)/16 + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x + y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 

					shift = (SHORT)GetAsyncKeyState(VK_SHIFT);
					if (!first) {
						if (abs(rx-x) > abs(ry-y)) first = 1;
						else if (abs(ry-y) > abs(rx-x)) first = 2;
					}
					
					if (x < map->header.width && y < map->header.height && x > -1 && y > -1) {
						if (!shift) 
							set_wall_tile(map, x, y, hWallType);
						else {
							if (!constrict)	constrict = first;
							set_wall_tile(map, constrict==1?x:rx, constrict==1?ry:y, hWallType);
						}

											
						map->mouseOver.rc.top = y*16 + 1;
						map->mouseOver.rc.left = x*16 + 1;
						map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
						map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					}
					map->mouseOver.valid = TRUE;    
					
					break;
				}
				
			case TOOL_PICKER:
				{
					char offset[4];
					WORD tile;
					
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 
					
					tile = map->mapData[x+y*map->header.width];
					if (tile & 0x8000) {
						animSel.horz = animSel.vert = 1;
						animSel.tile = tile & 0xFF;
						sprintf(offset, "%d", (tile & 0x7F00) >> 8);
						SetDlgItemText(GetDlgItem(frameWnd, ID_INFOAREA), ID_OFFSET, offset);
						
						useAnim = TRUE;
						SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						SetScrollPos(GetDlgItem(frameWnd, ID_ANIMATIONS), SB_VERT, animSel.tile * 16, TRUE);
						RedrawWindow(GetDlgItem(frameWnd, ID_ANIMATIONS), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					}
					else {
						int i;
						tileSel.horz = tileSel.vert = 1;
						tileSel.tile = tile;
						useAnim = FALSE;
						
						SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						
						for (i = 0; i < numTiles; i++)
							if (tileArrange[i] == tile) break;
						if (i >= numTiles) break;
							
						SetScrollPos(GetDlgItem(frameWnd, ID_TILEAREA), SB_HORZ, (i % 40) * 16, TRUE);
						SetScrollPos(GetDlgItem(frameWnd, ID_TILEAREA), SB_VERT, (i / 40) * 16, TRUE);
						RedrawWindow(GetDlgItem(frameWnd, ID_TILEAREA), NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);							
					}

					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					
					
					break;
				}
				
				
			case TOOL_FLAG:
				{
					int i, j;
					
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 
					
					for (i = y; i < (y + 3); i++) {
						if (i > map->header.height-1 || i < 0) continue;
						for (j = x; j < (x + 3); j++) {
							if (j > -1 && j < map->header.width) {
								map->mapData[j+i*map->header.width] = flag_data[hFlagType][(i-y)*3+(j-x)];
							}
						}
					}
										
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					
					map->mouseOver.valid = TRUE;    
					
					
					map->header.objective = OBJECTIVE_FLAG;
					
					break;
				}
				
			case TOOL_POLE:
				{
					int i, j;
					
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 
					
					for (i = y; i < (y + 3); i++) {
						if (i > map->header.height-1 || i < 0) continue;
						for (j = x; j < (x + 3); j++) {
							if (j > -1 && j < map->header.width) {
								if ((i-y) == 1 && (j-x) == 1) 
									map->mapData[j+i*map->header.width] = pole_data[hPoleType][9+hTeamSelected];
								else
									map->mapData[j+i*map->header.width] = pole_data[hPoleType][(i-y)*3+(j-x)];
							}
						}
					}
					
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					
					map->mouseOver.valid = TRUE;    
					
					
					map->header.objective = OBJECTIVE_FLAG;
					
					break;
				}
				
			case TOOL_SWITCH:
				{
					int i, j;
					
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 
					
					for (i = y; i < (y + 3); i++) {
						if (i > map->header.height-1 || i < 0) continue;
						for (j = x; j < (x + 3); j++) {
							if (j > -1 && j < map->header.width) {
								map->mapData[j+i*map->header.width] = switch_data[hSwitchType][(i-y)*3+(j-x)];
							}
						}
					}
					
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					
					map->mouseOver.valid = TRUE;    
					
					map->header.objective = OBJECTIVE_SWITCH;		
					break;
				}
				
				
			case TOOL_SPAWN:
				{
					int i, j;
					
					ClearStatus();
					sprintf(buf, "x: %-5d y: %-5d", x + 1, y + 1);
					SetStatusText(0, buf);
					if (map->mapData[x+y*map->header.width] & 0x8000) {
						sprintf(buf, "Animation: %-5d (%-2x)",
							map->mapData[x+y*map->header.width] & 0xFF,
							map->mapData[x+y*map->header.width] & 0xFF);
						SetStatusText(1, buf);
						sprintf(buf, "Offset: %-5d",
							(map->mapData[x+y*map->header.width] & 0x7F00)>>8);
						SetStatusText(2, buf);
					}
					else {
						sprintf(buf, "Tile: %-5d",
							map->mapData[x+y*map->header.width]);
						SetStatusText(1, buf);
					} 
					
					
					for (i = y; i < (y + 3); i++) {
						if (i < 0 || i > map->header.height-1) continue;
						for (j = x; j < (x + 3); j++) {
							if (j < 0 || j > map->header.width-1 ||
								spawn_data[hTeamSelected+hSpawnType*4][(i-y)*3+(j-x)] < 0) continue;
							map->mapData[i*map->header.width+j] = spawn_data[hTeamSelected+hSpawnType*4][(i-y)*3+(j-x)];
						}
					}
					
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 46;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 46;
					
					map->mouseOver.valid = TRUE;    
					
					
					
					break;
				}
				
			case TOOL_WARP:
				{
					WORD tile;
					
					tile = warps[hWarpIdx] | 0x8000 | (((hWarpDest * 10) + hWarpSrc) << 8);
					map->mapData[x+y*map->header.width] = tile;
					
					
					map->mouseOver.rc.top = y*16 + 1;
					map->mouseOver.rc.left = x*16 + 1;
					map->mouseOver.rc.right = map->mouseOver.rc.left + 14;
					map->mouseOver.rc.bottom = map->mouseOver.rc.top + 14;
					
					map->mouseOver.valid = TRUE;    
					
					break;
				}			

			case TOOL_SELECT:
				{
					int h, j, k;
					deselect = false;

					switch (map->seltype) {
					case 0:
						{
							struct seldata newsel;
							//HPEN whitepen;

							y = MAX(0, y);
							x = MAX(0, x);
							
							h = MIN(y, ry)*map->header.width + MIN(x, rx);
							j = MIN(MAX(1, ABS(x - rx)), (map->header.width - (h%map->header.width)));
							k = MIN(MAX(1, ABS(y - ry)), (map->header.height - (h/map->header.width)));
							
							if (map->selection.tile == h &&
								map->selection.horz == j &&
								map->selection.vert == k) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
														
							newsel.horz = j;
							newsel.vert = k;
							newsel.tile = h;

//							NewSelection(dc, whitepen, &map->selection, &newsel, map);
//							DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "Horizontal: %-5d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %-5d", map->selection.vert);
							SetStatusText(2, buf);
						}
						break;

					case 1:
						{
							struct seldata newsel;
							//HPEN whitepen;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							j = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							k = map->selection.tile;

							newsel = map->selection;
							newsel.tile = k + h*map->header.width + j;

							if (!h && !j) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %-5d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %-5d", map->selection.vert);
							SetStatusText(2, buf);

							rx = x;
							ry = y;

							break;
						}

					case 2:
						{
							struct seldata newsel;
							//HPEN whitepen;

							h = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							k = map->selection.tile;
							j = (map->selection.tile % map->header.width) + map->selection.horz;


							newsel = map->selection;
							newsel.tile = MIN(k + h, k + j);
							newsel.horz = MAX(0, j - (k % map->header.width) - h);

							if (!newsel.horz || !h) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 3:
						{
							struct seldata newsel;
							//HPEN whitepen;

							h = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							k = map->selection.tile;
							
							newsel = map->selection;
							newsel.horz = MAX(0, newsel.horz + h);

							if (!newsel.horz || !h) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 4:
						{
							struct seldata newsel;
							//HPEN whitepen;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							k = map->selection.tile;
							
							newsel = map->selection;
							newsel.vert = MAX(0, newsel.vert + h);

							if (!newsel.vert || !h) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 5:
						{
							struct seldata newsel;
						//	HPEN whitepen;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							k = map->selection.tile;
							j = (map->selection.tile / map->header.width) + map->selection.vert;


							newsel = map->selection;
							newsel.tile = MIN(k + h*map->header.width, k + j*map->header.width);
							newsel.vert = MAX(0, j - (k / map->header.width) - h);
							
							if (!newsel.vert || !h) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 6:
						{
							struct seldata newsel;
							//HPEN whitepen;
							int px;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							j = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							px = (map->selection.tile % map->header.width) + map->selection.horz;
							k = map->selection.tile;


							newsel.tile = MIN(k + j, k + px);
							newsel.horz = MAX(0, px - (k % map->header.width) - j);
							newsel.vert = MAX(0, map->selection.vert + h);
							
							if (!newsel.vert || !newsel.horz || !(h || j)) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 7:
						{
							struct seldata newsel;
							//HPEN whitepen;
						
							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							j = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							k = map->selection.tile;


							newsel = map->selection;
							newsel.horz = MAX(0, newsel.horz + j);
							newsel.vert = MAX(0, newsel.vert + h);
							
							if (!newsel.vert || !newsel.horz || !(h || j)) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 8:
						{
							struct seldata newsel;
							//HPEN whitepen;
							int px, py;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							j = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							px = (map->selection.tile % map->header.width) + map->selection.horz;
							py = (map->selection.tile / map->header.width) + map->selection.vert;
							k = map->selection.tile;


							newsel = map->selection;
							newsel.tile = MIN(k + j + h*map->header.width, k + px + py*map->header.width);
							newsel.horz = MAX(0, px - (k % map->header.width) - j);
							newsel.vert = MAX(0, py - (k / map->header.width) - h);
							
							if (!newsel.vert || !newsel.horz || !(h || j)) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					case 9:
						{
							struct seldata newsel;
							//HPEN whitepen;
							int py;

							h = MAX(-(map->selection.tile / map->header.width), MIN(map->header.height - map->selection.vert - map->selection.tile / map->header.width, y - ry));
							j = MAX(-(map->selection.tile % map->header.width), MIN(map->header.width - map->selection.horz - map->selection.tile % map->header.width, x - rx));
							py = (map->selection.tile / map->header.width) + map->selection.vert;
							k = map->selection.tile;


							newsel.tile = MIN(k + h*map->header.width, k + py*map->header.width);
							newsel.horz = MAX(0, map->selection.horz + j);
							newsel.vert = MAX(0, py - (k / map->header.width) - h);
							
							if (!newsel.vert || !newsel.horz || !(h || j)) break;

							//whitepen = CreatePen(PS_SOLID, 1, 0x00000000);
							//NewSelection(dc, whitepen, &map->selection, &newsel, map);							
							//DeleteObject(whitepen);
							map->selection = newsel;
							
							sprintf(buf, "start: x: %-5d y: %-5d", map->selection.tile % map->header.width + 1,
											   map->selection.tile / map->header.width + 1);
							SetStatusText(0, buf);
							sprintf(buf, "Horizontal: %d", map->selection.horz);
							SetStatusText(1, buf);
							sprintf(buf, "Vertical: %d", map->selection.vert);
							SetStatusText(2, buf);
							
							rx = x;
							ry = y;

							break;
						}

					default:
						break;
					}

					break;
				}
				
				
			case TOOL_HOLDING_PEN:
			case TOOL_BUNKER:
			case TOOL_BRIDGE:
			case TOOL_CONV:
				{
					struct seldata newsel;
					HPEN whitepen;
					int h, j, k;
					
					y = MAX(0, y);
					x = MAX(0, x);
					
					h = MIN(y, ry)*map->header.width + MIN(x, rx);
					j = MIN(MAX(1, ABS(x - rx)), (map->header.width - 1 - (h%map->header.width)));
					k = MIN(MAX(1, ABS(y - ry)), (map->header.height - 1 - (h/map->header.width)));
					
					if (map->selection.tile == h &&
						map->selection.horz == j &&
						map->selection.vert == k) break;
					
					/*
					if (map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
						HRGN largeRegion, smallRegion;
						rc.left = ((map->selection.tile % 256) << 4) - scx;
						rc.top = ((map->selection.tile / 256) << 4) - scy;
						rc.right = rc.left + (map->selection.horz << 4);
						rc.bottom = rc.top + (map->selection.vert << 4);
						
						largeRegion = CreateRectRgnIndirect(&rc);
						rc.left++;
						rc.right--;
						rc.top++;
						rc.bottom--;
						smallRegion = CreateRectRgnIndirect(&rc);
						CombineRgn(largeRegion, largeRegion, smallRegion, RGN_DIFF);
						map->selection.tile = -1;
						RedrawWindow(hwnd, NULL, largeRegion, RDW_UPDATENOW|RDW_INVALIDATE);
						
						DeleteObject(largeRegion);
						DeleteObject(smallRegion);
					}
					*/

					newsel.tile = h;
					newsel.horz = j;
					newsel.vert = k;
					//whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);

					//NewSelection(dc, whitepen, &map->selection, &newsel, map);
					//DeleteObject(whitepen);
					map->selection = newsel;

					sprintf(buf, "Horizontal: %-5d", map->selection.horz);
					SetStatusText(1, buf);
					sprintf(buf, "Vertical: %-5d", map->selection.vert);
					SetStatusText(2, buf);
					
					break;
				}
				
            default:
				break;
          }
		  
          RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		  
		  
          
        }
		
		/* do mouse release settings/drawings */
		
		switch (currentTool) {
			case TOOL_PICKER:
				currentTool = TOOL_EDIT;
				break;
				
			case TOOL_LINE:
				{
					Polynomial line;
					
					if (map->lineTool.start.x > map->lineTool.stop.x) {
						swap(map->lineTool.start.x, map->lineTool.stop.x);
						swap(map->lineTool.start.y, map->lineTool.stop.y);
					}

					double coeffs[2];
					coeffs[0] = (double)map->lineTool.start.y;
					coeffs[1] = 0;

					if (map->lineTool.start.x/16 != map->lineTool.stop.x/16) {
						coeffs[1] = (double)(map->lineTool.stop.y - map->lineTool.start.y) / (double)(map->lineTool.stop.x - map->lineTool.start.x);
						line.SetCoefficients(coeffs, 2);

						int lz = -1;
						for (int p = map->lineTool.start.x; p <= map->lineTool.stop.x; p +=16)
						{
							int z = (int)(line.Evaluate((double)p - map->lineTool.start.x) + .5)/16;
							//map->mapData[z*map->header.width+p/16] = 0;
							set_wall_tile(map, p/16, z, hWallType);
							if (z != lz && lz != -1) {
								if (lz < z) {
									int next = (lz + z)/2;
									for (; lz <= next; lz++)
										set_wall_tile(map, p/16-1, lz, hWallType);
									set_wall_tile(map, p/16, lz-1, hWallType);
									for (; lz < z; lz++)
										set_wall_tile(map, p/16, lz, hWallType);
								}
								else {
									int next = (lz + z)/2;
									for (; lz >= next; lz--)
										set_wall_tile(map, p/16-1, lz, hWallType);
									set_wall_tile(map, p/16, lz+1, hWallType);
									for (; lz > z; lz--)
										set_wall_tile(map, p/16, lz, hWallType);
								}
							}
							lz = z;
						}
					}
					else {
						if (map->lineTool.stop.y < map->lineTool.start.y)
							swap(map->lineTool.start.y, map->lineTool.stop.y);
						for (int p = map->lineTool.start.y; p <= map->lineTool.stop.y; p += 16)
							set_wall_tile(map, map->lineTool.start.x/16, p/16, hWallType);
					}

					map->lineTool.valid = 0;
					break;
				}

			case TOOL_BUNKER:
				j = hBunkerType * 4 + hBunkerDir;
				for (h = 0; h < map->selection.vert; h++) {
					for (k = 0; k < map->selection.horz; k++) {
						if (h == 0 && k == 0)
							tile =
							bunker_data[j][0];
						else if (h == 0 && k == map->selection.horz-1)
							tile =
							bunker_data[j][3];
						else if (h == 0)
							tile =
							bunker_data[j][1+k%2];
						else if (h == map->selection.vert-1 && k == 0) 
							tile =
							bunker_data[j][12];
						else if (h == map->selection.vert-1 && k == map->selection.horz-1)
							tile =
							bunker_data[j][15];
						else if (h == map->selection.vert-1)
							tile =
							bunker_data[j][13+k%2];
						else if (k == 0)
							tile =
							bunker_data[j][4];
						else if (k == map->selection.horz-1)
							tile =
							bunker_data[j][7];
						else
							tile =
							bunker_data[j][5+(k%2)+4*(h%2)];
						if (tile > -1)
							map->mapData[map->selection.tile + k + h*map->header.width] = tile;
					}
				}
				break;

			case TOOL_BRIDGE:
				for (h = 0; h < map->selection.vert; h++) {
					for (k = 0; k < map->selection.horz; k++) {
						tile = -1;
						if (!hBridgeDir) {	/* left and right */
							if (h == 0 && k == 0) 
								tile = bridge_lr_data[hBridgeType][0];
							else if (h == 0 && k == map->selection.horz-1)
								tile = bridge_lr_data[hBridgeType][2];
							else if (h == 0)
								tile = bridge_lr_data[hBridgeType][1];
							else if (h == 1 && k == 0)
								tile = bridge_lr_data[hBridgeType][3];
							else if (h == 1 && k == map->selection.horz-1)
								tile = bridge_lr_data[hBridgeType][5];
							else if (h == 1)
								tile = bridge_lr_data[hBridgeType][4];
							else if (h == map->selection.vert-2 && k == 0)
								tile = bridge_lr_data[hBridgeType][9];
							else if (h == map->selection.vert-2 && k == map->selection.horz-1)
								tile = bridge_lr_data[hBridgeType][11];
							else if (h == map->selection.vert-2)
								tile = bridge_lr_data[hBridgeType][10];
							else if (h == map->selection.vert-1 && k == 0)
								tile = bridge_lr_data[hBridgeType][12];
							else if (h == map->selection.vert-1 && k == map->selection.horz-1) 
								tile = bridge_lr_data[hBridgeType][14];
							else if (h == map->selection.vert-1)
								tile = bridge_lr_data[hBridgeType][13];
							else if (k == 0)
								tile = bridge_lr_data[hBridgeType][6];
							else if (k == map->selection.horz-1)
								tile = bridge_lr_data[hBridgeType][8];
							else
								tile = bridge_lr_data[hBridgeType][7];
						}
						else {
							if (h == 0 && k == 0) 
								tile = bridge_ud_data[hBridgeType][0];
							else if (h == 0 && k == map->selection.horz-1)
								tile = bridge_ud_data[hBridgeType][4];
							else if (h == 0 && k == 1)
								tile = bridge_ud_data[hBridgeType][1];
							else if (h == 0 && k == map->selection.horz-2)
								tile = bridge_ud_data[hBridgeType][3];
							else if (h == 0)
								tile = bridge_ud_data[hBridgeType][2];
							else if (h == map->selection.vert-1 && k == 0)
								tile = bridge_ud_data[hBridgeType][10];
							else if (h == map->selection.vert-1 && k == 1)
								tile = bridge_ud_data[hBridgeType][11];
							else if (h == map->selection.vert-1 && k == map->selection.horz-2) 
								tile = bridge_ud_data[hBridgeType][13];
							else if (h == map->selection.vert-1 && k == map->selection.horz-1)
								tile = bridge_ud_data[hBridgeType][14];
							else if (h == map->selection.vert-1)
								tile = bridge_ud_data[hBridgeType][12];
							else if (k == 0)
								tile = bridge_ud_data[hBridgeType][5];
							else if (k == map->selection.horz-1)
								tile = bridge_ud_data[hBridgeType][9];
							else if (k == 1)
								tile = bridge_ud_data[hBridgeType][6];
							else if (k == map->selection.horz-2)
								tile = bridge_ud_data[hBridgeType][8];
							else
								tile = bridge_ud_data[hBridgeType][7];
						}

						if (tile > -1)
							map->mapData[map->selection.tile + k + h*map->header.width] = tile;
					}
				}
				break;

			case TOOL_HOLDING_PEN:
				for (h = 0; h < map->selection.vert; h++) {
					for (k = 0; k < map->selection.horz; k++) {
						if (h == 0 && k == 0)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][0];
						else if (h == 0 && k == map->selection.horz-1)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][2];
						else if (h == map->selection.vert-1 && k == 0)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][8];
						else if (h == map->selection.vert-1 && k == map->selection.horz-1)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][10];
						else if (h == 0 || h == map->selection.vert-1)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][13];
						else if (k == 0 || k == map->selection.horz-1)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							wall_data[0][7];
						else if (k == map->selection.horz/2 && h == map->selection.vert/2)
							map->mapData[map->selection.tile + k + h*map->header.width] =
							holding_pen_data[hHoldingType][1+hTeamSelected];
						else
							map->mapData[map->selection.tile + k + h*map->header.width] =
							holding_pen_data[hHoldingType][0];
					}
				}
				break;

			case TOOL_CONV:
				{
					for (k = 0; k < map->selection.vert; k++) {
						for (h = 0; h < map->selection.horz; h++) {
							if (hConvDir) {
								// only doubles allowed
								if (map->selection.horz % 2 && h == map->selection.horz - 1) break;
								if (k == 0) map->mapData[map->selection.tile + h + k*map->header.width] = conv_ud_data[hConvType][h%2];
								else if (k == map->selection.vert - 1) map->mapData[map->selection.tile + h + k*map->header.width] = conv_ud_data[hConvType][h%2+6];
								else map->mapData[map->selection.tile + h + k*map->header.width] = conv_ud_data[hConvType][(k%2+1)*2+h%2];
							}
							else {
								// only doubles allowed
								if (map->selection.vert % 2 && k == map->selection.vert - 1) break;
								if (h == 0) map->mapData[map->selection.tile + h + k*map->header.width] = conv_lr_data[hConvType][(k%2)*4];
								else if (h == map->selection.horz - 1) map->mapData[map->selection.tile + h + k*map->header.width] = conv_lr_data[hConvType][(k%2)*4+3];
								else map->mapData[map->selection.tile + h + k*map->header.width] = conv_lr_data[hConvType][1+(k%2)*4+h%2];
							}
						}
					}
					break;
				}

			case TOOL_SELECT:
				{
					map->seltype = 0;
					if (deselect) {
						map->selection.horz = map->selection.vert = 0;
						map->selection.tile = -1;
					}
					break;
				}

			default:
				break;
		}

		/* if the selection changes, add it to undo */
		if (memcmp(&oldselection, &map->selection, sizeof(struct seldata)))
			addSelectUndo(&map->undo_list, &oldselection, map);	
		
        w = map->header.width;
        h = map->header.height;
        BufferCompare(map->mapData, oldBuf, w, h, &undoInfo);
		
        if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
        free(oldBuf);
		
      }
	  
      // invalidate selection box
      if (map && map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
		  rc.left = (map->selection.tile % 256) * 16 - scx;
		  rc.top = (map->selection.tile / 256) * 16 - scy;
		  rc.right = rc.left + (map->selection.horz << 4);
		  rc.bottom = rc.top + (map->selection.vert << 4);
		  RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);      
		  EnableButtons();
	  }
	  else
		  DisableSelectButtons();

	  constrict = 0; // get rid of direction constriction
	  map->lineTool.valid = FALSE; // get rid of line tool crosses
	  
      while ((n = ShowCursor(TRUE)) < 0);
      ReleaseCapture();
      break;
    }
	
	
    default:
		return DefMDIChildProc(hwnd, msg, wparm, lparm);
  }
  return 0;
}

int CreateNewMap() {
	RECT rc;
	int p;
	SCROLLINFO si;
	struct MapInfo *map;
	HWND hwnd;
	
	map = (struct MapInfo *)malloc(sizeof(struct MapInfo));
	if (!map) return 0;
	memset(map, 0, sizeof(struct MapInfo)); // 0 the structure out
	memset(&map->mouseOver, 0, sizeof(map->mouseOver));
	
	map->header.width = 256;
	map->header.height = 256;
	
	map->header.name = dupestr("New Map");
	map->header.nameLength = 7;
	map->header.description = dupestr("New map");
	map->header.descriptionLength = strlen(map->header.description);
	
	map->header.misslesEnabled = map->header.bombsEnabled = map->header.bounciesEnabled = 1;
	map->header.specialDamage = map->header.laserDamage = map->header.rechargeRate = 2;
	map->header.holdingTime = 15;
	map->header.maxPlayers = 16;
	map->header.numTeams = 2;
	map->header.objective = 0;
	map->header.maxSimulPowerups = 12;
	
	map->mapData = (WORD *)malloc(map->header.width * map->header.height * 2);
	if (!map->mapData) {
		free(map);
		return 0;
	}
	for (p = map->header.height * map->header.width; p--; )
		map->mapData[p] = 280;
	
	if (!map_list) {
		EnableButtons();	
		DisableSelectButtons();
	}
	map->next = map_list;
	map_list = map;
	
	hwnd = CreateMDIWindow("MalletEdit_maparea", "New Map",
		WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
		0, 0, 200, 100,
		GetDlgItem(frameWnd, ID_MDICLIENT),
		hInstance, 0);
	
	map->hwnd = hwnd;
	if (displayAnims) SetTimer(map->hwnd, TIMER_ANIMFRAME, ANIMFRAME_DURATION, NULL);
	
	ShowScrollBar(hwnd, SB_VERT, TRUE);
	ShowScrollBar(hwnd, SB_HORZ, TRUE);
	
	GetClientRect(hwnd, &rc);
	memset(&si, 0, sizeof(si));
	si.fMask = SIF_RANGE;
	si.cbSize = sizeof(si);
	si.nMax = 4096 - rc.right + rc.left;
	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
	si.nMax = 4096 - rc.bottom + rc.top;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	
	RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
	
	return 1;
}

int OpenMap(char *file) {
	int fd;
	int x, sz;
	RECT rc;
	SCROLLINFO si;
	struct MapInfo *map;
	HWND hwnd;
	
	fd = _open(file, _O_RDONLY|_O_BINARY);
	if (fd < 1) return 0;

	GetCurrentDirectory(sizeof(lastdir), lastdir);
	
	
	map = (struct MapInfo *)malloc(sizeof(struct MapInfo));
	if (!map) {
		_close(fd);
		return 0;
	}
	memset(map, 0, sizeof(struct MapInfo)); // 0 the structure out
	
	map->file = dupestr(file);

	sz = _filelength(fd);

	_read(fd, &map->header.id, 2);
	_read(fd, &map->header.header, 2);
	_read(fd, &map->header.version, 1);
	_read(fd, &map->header.width, 2);
	_read(fd, &map->header.height, 2);
	_read(fd, &map->header.maxPlayers, 1);
	_read(fd, &map->header.holdingTime, 1);
	_read(fd, &map->header.numTeams, 1);
	_read(fd, &map->header.objective, 1);
	_read(fd, &map->header.laserDamage, 1);
	_read(fd, &map->header.specialDamage, 1);
	_read(fd, &map->header.rechargeRate, 1);
	_read(fd, &map->header.misslesEnabled, 1);
	_read(fd, &map->header.bombsEnabled, 1);
	_read(fd, &map->header.bounciesEnabled, 1);
	_read(fd, &map->header.powerupCount, 2);
	_read(fd, &map->header.maxSimulPowerups, 1);
	_read(fd, &map->header.switchCount, 1);

	
	if (map->header.id != 0x4278 && sz != 131072) {
		MessageBox(NULL, "This is not a map file.", "Error", MB_OK);
		killMap(map);
		return 0;
	}
	else if (sz == 131072) {
		_lseek(fd, 0, SEEK_SET);
		memset(&(map->header), 0, sizeof(map->header));
		map->header.version = 1;
		map->header.width = map->header.height = 256;
		map->header.bombsEnabled = map->header.bounciesEnabled = map->header.misslesEnabled = 1;
		map->header.description = strdup("old version1 map");
		map->header.descriptionLength = strlen(map->header.description);
		map->header.name = strdup("old version1 map");
		map->header.nameLength = strlen(map->header.name);
		map->header.objective = OBJECTIVE_FLAG;
		map->mapData = (WORD *)malloc(map->header.width * map->header.height * 2);
		if (!map->mapData) {
			free(map);
			_close(fd);
			return 0;
		}
		memset(map->mapData, 0, map->header.width * map->header.height * 2);
	}

	if (map->header.version == 3) { // compressed
		long comprLen;
		long uncomprLen = map->header.width * map->header.height * 2;
		BYTE *compBuf;

		_read(fd, map->header.flagCount, map->header.numTeams);
		_read(fd, map->header.flagPoleCount, map->header.numTeams);

		map->mapData = (WORD *)malloc(map->header.width * map->header.height * 2);
		if (!map->mapData) {
			free(map);
			_close(fd);
			return 0;
		}
		memset(map->mapData, 0, map->header.width * map->header.height * 2);

		for (x = 0; x < map->header.numTeams; x++) {
			if (map->header.flagPoleCount[x]) {
				map->header.flagPoleData[x] = (char *)malloc(map->header.flagPoleCount[x]);
				_read(fd, map->header.flagPoleData[x], map->header.flagPoleCount[x]);
			}
			else map->header.flagPoleData[x] = NULL;
		}

		_read(fd, &map->header.nameLength, 2);
		if (map->header.nameLength < 0) map->header.nameLength = 0;
		if (map->header.nameLength == 0) {
			map->header.name = strdup("New map");
			map->header.nameLength = strlen("New map");
		}
		else {
			map->header.name = (char *)malloc(map->header.nameLength+1);
			_read(fd, map->header.name, map->header.nameLength);
			map->header.name[map->header.nameLength] = 0;
		}
		
		_read(fd, &map->header.descriptionLength, 2);
		if (map->header.descriptionLength < 0) map->header.descriptionLength = 0;
		if (map->header.descriptionLength == 0) {
			map->header.description = strdup("New map");
			map->header.descriptionLength = strlen("New map");
		}
		else {
			map->header.description = (char *)malloc(map->header.descriptionLength+1);
			_read(fd, map->header.description, map->header.descriptionLength);
			map->header.description[map->header.descriptionLength] = 0;
		}

		_read(fd, &map->header.neutralCount, 1);

		map->header.description[map->header.descriptionLength] = 0;
		map->header.name[map->header.nameLength] = 0;

		_lseek(fd, map->header.header + 2, SEEK_SET);
			
		comprLen = _filelength(fd) - _lseek(fd, 0, SEEK_CUR);
		
		compBuf = (BYTE *)malloc(comprLen+1);
		memset(compBuf, 0, comprLen+1);
		
		_read(fd, compBuf, comprLen);
		
		uncompress((BYTE *)map->mapData, &uncomprLen, compBuf, comprLen);
		
		free(compBuf);
		
	}
	else {
		_read(fd, map->mapData, map->header.width * map->header.height * 2);
	}
	
	_close(fd);
	
	hwnd = CreateMDIWindow("MalletEdit_maparea", "New Map",
		WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|
		WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
		0, 0, 200, 100,
		GetDlgItem(frameWnd, ID_MDICLIENT),
		hInstance, 0);
	map->hwnd = hwnd;
	if (displayAnims) SetTimer(map->hwnd, TIMER_ANIMFRAME, ANIMFRAME_DURATION, NULL);
	
	SetWindowText(hwnd, map->header.name);
	
	if (!map_list)
		EnableButtons();
	map->next = map_list;
	map_list = map;
	
	
	GetClientRect(hwnd, &rc);
	memset(&si, 0, sizeof(si));
	si.fMask = SIF_RANGE;
	si.cbSize = sizeof(si);
	
	ShowScrollBar(hwnd, SB_HORZ, TRUE);
	ShowScrollBar(hwnd, SB_VERT, TRUE);
	si.nMax = (map->header.width * 16) - (rc.right - rc.left);
	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
	si.nMax = (map->header.height * 16) - (rc.bottom - rc.top);
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	
	RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
	
	return 1;
}  

void killMap(struct MapInfo *map) {
	char **p;
	struct MapInfo *t;
	int n;
	
	if (map) {
		if (map->mapData) free(map->mapData);
		if (map->header.description) free(map->header.description);
		if (map->header.name) free(map->header.name);
		if (map->file) free(map->file);
		if (map->overview) free(map->overview);
		
		for (p = &map->header.flagPoleData[n=0]; p && *p && n < 4; p = &map->header.flagPoleData[++n])
			free(*p);
		
		killUndoBlocks(&map->redo_list);
		killUndoBlocks(&map->undo_list);
		
		if (map == map_list)
			map_list = map->next;
		else {
			for (t = map_list; t && t->next && t->next != map; t = t->next);
			if (t && t->next)
				t->next = map->next;
		}
		
		if (infomap == map) infomap = NULL;
		if (overmap == map && overDlg && map_list) RedrawWindow(overDlg, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		free(map);    
		
	}
	
	if (!map_list) {
		DisableButtons();
		if (overDlg) DestroyWindow(overDlg);
		overDlg = NULL;
	}
	
}

void EditMap(int x, int y, BOOL anim, struct MapInfo *map) {
	int p, q;// totalX, totalY, start;
	
	if (map && ((tileSel.tile != -1 && !anim) || anim)) {

		if (!anim) {
			for (p = 0; p < tileSel.vert; p++) {
				if (p+y >= map->header.height) break;
				for (q = 0; q < tileSel.horz; q++) {
					if (q+x >= map->header.width) break;
					map->mapData[(p+y)*map->header.width+q+x] = tileArrange[tileSel.tile + q + p*40];
				}
			}
		}
		else if (animSel.tile != -1) {
			WORD tile;
			WORD offset;
			char osbuf[4];

			memset(osbuf, 0, sizeof(osbuf));
			GetDlgItemText(GetDlgItem(frameWnd, ID_INFOAREA), ID_OFFSET, osbuf, sizeof(osbuf));
			offset = atoi(osbuf);

			tile = (offset << 8)|0x8000|(animSel.tile & 0xFF);

			map->mapData[x+(y*map->header.width)] = tile;
		}    
	}
}

void killUndoBlocks(struct undo_blocks **list) {
	struct undo_blocks *next, *l;
	
	if (list) {
		for (l = *list; l; l = next) {
			next = l->next;
			if (l->undo.ptr) {
				if (l->type == UNDO_NORMAL && l->undo.ub->buffer) free(l->undo.ub->buffer);
				free(l->undo.ptr);
			}
			free(l);
		}
		
		*list = NULL;
	}
}

BOOL performUndo(struct MapInfo *map) {
	int n, x, y;
	struct undo_blocks *use, *temp;
	
	if (map && map->undo_list) {
		use = map->undo_list;
		switch (use->type) {
		case UNDO_NORMAL:
			{
				for (n = 0, y = use->undo.ub->y; y < (use->undo.ub->y + use->undo.ub->height); y++) {
					for (x = use->undo.ub->x; x < (use->undo.ub->x + use->undo.ub->width); x++) {
						if (use->undo.ub->buffer[n] != 0xFFFF)
							map->mapData[y*map->header.width + x] ^=
							use->undo.ub->buffer[n] ^=
							map->mapData[y*map->header.width + x] ^=
							use->undo.ub->buffer[n];
						n++;
					}
				}
				break;
			}

		case UNDO_SELECT:
			{
				struct seldata newsel = map->selection;
				map->selection = *(use->undo.sel);
				*(use->undo.sel) = newsel;
				break;
			}

		default:
			break;
		}
		
		temp = map->undo_list->next;
		map->undo_list->next = map->redo_list;
		map->redo_list = map->undo_list;
		map->undo_list = temp;
		return TRUE;
	}
	return FALSE;
}

BOOL performRedo(struct MapInfo *map) {
	int n, x, y;
	struct undo_blocks *use, *temp;
	
	if (map && map->redo_list) {
		use = map->redo_list;
		switch (use->type) {
		case UNDO_NORMAL:
			{
				for (n = 0, y = use->undo.ub->y; y < (use->undo.ub->y + use->undo.ub->height); y++) {
					for (x = use->undo.ub->x; x < (use->undo.ub->x + use->undo.ub->width); x++) {
						if (use->undo.ub->buffer[n] != 0xFFFF)
							map->mapData[y*map->header.width + x] ^=
							use->undo.ub->buffer[n] ^=
							map->mapData[y*map->header.width + x] ^=
							use->undo.ub->buffer[n];
						n++;
					}
				}
				break;
			}

		case UNDO_SELECT:
			{
				struct seldata newsel = map->selection;
				map->selection = *(use->undo.sel);
				*(use->undo.sel) = newsel;
				break;
			}

		default:
			break;
		}
		
		temp = map->redo_list->next;
		map->redo_list->next = map->undo_list;
		map->undo_list = map->redo_list;
		map->redo_list = temp;
		return TRUE;
	}
	return FALSE;
}

BOOL performDelete(struct MapInfo *map) {
	WORD *oldBuf;
	struct undo_buf *undoInfo;
	
	if (map && map->mapData) {
		if (map->selection.tile == -1 || !map->selection.horz || !map->selection.vert)
			MessageBox(NULL, "You have note selected an area", "Error", MB_OK);
		else {
			int x, y, maxx, maxy, stx;
			
			x = stx = map->selection.tile % map->header.width;
			y = map->selection.tile / map->header.width; 
			
			maxx = x + (map->selection.horz);
			maxy = y + (map->selection.vert);
			
			oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
			memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
			
			for (; y < maxy; y++) {
				for (x = stx; x < maxx; x++)
					map->mapData[(y*map->header.width)+x] = 280;
			}
			
			BufferCompare(map->mapData, oldBuf, map->header.width, map->header.height, &undoInfo);
			if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
			free(oldBuf);
			
			return TRUE;
		}
	}
	return FALSE;
}     

void addSelectUndo(struct undo_blocks **list, struct seldata *oldsel, struct MapInfo *map) {
	int p, q;
	
	struct undo_blocks *newBlock;
	
	newBlock = (struct undo_blocks *)malloc(sizeof(struct undo_blocks));
	newBlock->undo.sel = (struct seldata *)malloc(sizeof(struct seldata));
	*(newBlock->undo.sel) = *oldsel;
	newBlock->next = *list;
	newBlock->type = UNDO_SELECT;
	*list = newBlock;
	
	// now make sure we're not hogging memory with these
	if ((p=count_list(map->undo_list)) + (q=count_list(map->redo_list)) > 50) {
		if (q > p)
			chop_list(&map->redo_list);
		else
			chop_list(&map->undo_list);
		
	}
}


void addUndo(struct undo_blocks **list, struct undo_buf *newUndoBlock, struct MapInfo *map) {
	
	int p, q;
	
	struct undo_blocks *newBlock;
	
	newBlock = (struct undo_blocks *)malloc(sizeof(struct undo_blocks));
	newBlock->undo.ub = newUndoBlock;
	newBlock->next = *list;
	newBlock->type = UNDO_NORMAL;
	*list = newBlock;
	
	// now make sure we're not hogging memory with these
	if ((p=count_list(map->undo_list)) + (q=count_list(map->redo_list)) > 50) {
		if (q > p)
			chop_list(&map->redo_list);
		else
			chop_list(&map->undo_list);
		
	}
	
	map->modify = 1;
}

void LookOverFlags(struct MapInfo *map) {
	int x = 0, j, i;
	unsigned char idx;
	WORD tile;
	
	if (map) {
		
		memset(map->header.flagPoleCount, 0, sizeof(map->header.flagPoleCount));
		memset(map->header.flagCount, 0, sizeof(map->header.flagCount));
		map->header.switchCount = 0;
		map->header.neutralCount = 0;
		map->header.powerupCount = 0;
		
		for (i = 0; i < map->header.numTeams; i++)
			if (map->header.flagPoleData[i]) free(map->header.flagPoleData[i]);
			memset(&map->header.flagPoleData, 0, sizeof(map->header.flagPoleData));
			// kills the ptrs
			
			
			for (x = 0; x < 65536; x++) {
				tile = map->mapData[x];
				
				if (!(tile & 0x8000)) {
					for (j = 0; powerupPosses[j] != -1; j++)
						if (tile == powerupPosses[j]) map->header.powerupCount++;
						continue;
				}
				
				idx = (tile & 0xFF);
				
				for (j = 0; j < 20 && j/5 < map->header.numTeams; j++) {
					if (idx == flagPoles[j]) {
						int n = map->header.flagPoleCount[j/5]++;
						if (j/5 == j%5) map->header.flagCount[j/5]++;
						map->header.flagPoleData[j/5] = (char *)realloc(map->header.flagPoleData[j/5],
							map->header.flagPoleCount[j/5]);
						map->header.flagPoleData[j/5][n] = j%5;
					}
				}
				for (j = 0; switches[j] != -1; j++)
					if (idx == switches[j]) map->header.switchCount++;
					for (j = 0; neutralFlags[j] != -1; j++)
						if (idx == neutralFlags[j]) map->header.neutralCount++;
			}
	}
}

BOOL SaveMap(char *file, struct MapInfo *map) {
	int fd, x;
	BYTE *compBuf;
	int compSize, uncompSize;
	
	if (!map) return FALSE;
	
	_unlink(file);
	fd = _open(file, _O_CREAT|_O_RDWR|_O_BINARY, _S_IWRITE|_S_IREAD);
	if (fd < 1) return FALSE;
	
	LookOverFlags(map); // new flag data in header now
	// calculate header size
	
	map->header.descriptionLength = map->header.description?strlen(map->header.description):0;
	if (!map->header.descriptionLength) {
		if (map->header.description) free(map->header.description);
		map->header.description = dupestr("MalletEdit v2");
	}
	else {
		char *p;
		p = strstr(map->header.description, "MalletEdit v2");
		if (!p) {
			p = (char *)malloc(map->header.descriptionLength + 10);
			sprintf(p, "%s\nMalletEdit v2", map->header.description);
			free(map->header.description);
			map->header.description = p;
		}
	}
	map->header.descriptionLength = map->header.description?strlen(map->header.description):0;
	map->header.nameLength = map->header.name?strlen(map->header.name):0;
	
	map->header.header = 26; // prelim
	map->header.header += map->header.numTeams * 2;
	map->header.header += map->header.nameLength + map->header.descriptionLength;
	for (x = 0; x < map->header.numTeams; x++)
		map->header.header += map->header.flagPoleCount[x];
	
	map->header.version = 3; // crucial
	map->header.id = 0x4278;
	
	if (file != map->file && map->file) {
		free(map->file);
		map->file = dupestr(file);
	}
	else if (!map->file)
		map->file = dupestr(file);
	
	_write(fd, &map->header.id, 2);
	_write(fd, &map->header.header, 2);
	_write(fd, &map->header.version, 1);
	_write(fd, &map->header.width, 2);
	_write(fd, &map->header.height, 2);
	_write(fd, &map->header.maxPlayers, 1);
	_write(fd, &map->header.holdingTime, 1);
	_write(fd, &map->header.numTeams, 1);
	_write(fd, &map->header.objective, 1);
	_write(fd, &map->header.laserDamage, 1);
	_write(fd, &map->header.specialDamage, 1);
	_write(fd, &map->header.rechargeRate, 1);
	_write(fd, &map->header.misslesEnabled, 1);
	_write(fd, &map->header.bombsEnabled, 1);
	_write(fd, &map->header.bounciesEnabled, 1);
	_write(fd, &map->header.powerupCount, 2);
	_write(fd, &map->header.maxSimulPowerups, 1);
	_write(fd, &map->header.switchCount, 1);
	_write(fd, map->header.flagCount, map->header.numTeams);
	_write(fd, map->header.flagPoleCount, map->header.numTeams);
	for (x = 0; x < map->header.numTeams; x++)
		_write(fd, map->header.flagPoleData[x], map->header.flagPoleCount[x]);
	_write(fd, &map->header.nameLength, 2);
	_write(fd, map->header.name, map->header.nameLength);
	_write(fd, &map->header.descriptionLength, 2);
	_write(fd, map->header.description, map->header.descriptionLength);
	_write(fd, &map->header.neutralCount, 1);
	
	compSize = map->header.width * map->header.height * 2;
	compBuf = (BYTE *)malloc(compSize);
	uncompSize = compSize;
	
	compress2(compBuf, (long *)&compSize, (BYTE *)map->mapData, uncompSize, 9);
	_write(fd, compBuf, compSize);
	free(compBuf);
	
	_close(fd);
	
	map->modify = 0;
	
	
	return TRUE;
}

BOOL RotateBits(struct MapInfo *map) {
	WORD *tiles, *oldBuf;
	struct undo_buf *undoInfo;
	int x, y, i, j, width, height;
	rdTbl pseudo;
	
	
	if (map && map->selection.tile != -1 && map->selection.horz && map->selection.vert) { 
		
		tiles = (WORD *)malloc(map->selection.vert * map->selection.horz * 2);
		oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
		memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
		
		width = (WORD)map->selection.vert;    // Looks backwards.. but the whole thing
		height = (WORD)map->selection.horz;   // is flipped, so we need to trade WxH
		
		for (i = 0; i < map->selection.vert; i++) {
			for (j = 0; j < map->selection.horz; j++) {
				tiles[i*map->selection.horz+j] = map->mapData[map->selection.tile + i*map->header.width+j];
				map->mapData[map->selection.tile + i*map->header.width+j] = 280;
			}
		}
		
		
		// Ok we have our "list" of what needs flipped.
		// 0 goes at x of height, y of 0
		// height goes at x of height, y of width
		
		for (y = 0, j = 0; j < height; y++, j++) {
			for (x = 0, i = width-1; i > -1; x++, i--) {
				
				if (map->selection.tile+y*map->header.width+x >
					map->header.width*map->header.height - 1)
				{
					j = height;
					break;
				}
				
				if (!(tiles[i*height+j] & 0x8000))
					pseudo = PseudoRD(tiles[i*height+j], FALSE);
				else
					pseudo = PseudoRD(tiles[i*height+j] & 0xFF, TRUE);
				
				if (pseudo.deg90 == -1)
					map->mapData[map->selection.tile+y*map->header.width+x] = tiles[i*height+j];
				else if (!(pseudo.deg90 & 0x8000))
					map->mapData[map->selection.tile+y*256+x] = pseudo.deg90;
				else
					map->mapData[map->selection.tile+y*256+x] = pseudo.deg90 | 0x8000 | (tiles[i*height+j] & 0x7F00);
			}
		}
		
		free(tiles);
		
		map->selection.horz = width;
		map->selection.vert = height;
		if (map->selection.tile/map->header.width + map->selection.vert > map->header.height)
			map->selection.vert = (map->header.height - map->selection.tile/map->header.width);
		if ((map->selection.tile%map->header.width) + map->selection.horz > map->header.width)
			map->selection.horz = (map->header.width - (map->selection.tile % map->header.width));
		
		BufferCompare(map->mapData, oldBuf, map->header.width, map->header.height, &undoInfo);
		if (undoInfo)
			addUndo(&map->undo_list, undoInfo, map);
		free(oldBuf);
		
		return TRUE;
	}
	else if (map)
		MessageBox(NULL, "You have not selected an area", "Error", MB_OK);
	
	return FALSE;
}

BOOL MirrorBits(int direction, struct MapInfo *map) {
	int x, y, i, j, mirStart, p;
	int sx, xinc, yinc;
	mdTbl pseudo;
	WORD *oldBuf;
	struct undo_buf *undoInfo;
	
	// 0 = mirror to the left
	// 1 = mirror to the right
	// 2 = mirror up
	// 3 = mirror down
	
	if (map && map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
		
		xinc = yinc = 1;
		
		oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
		memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
		
		switch (direction) {
		case 0:
			mirStart = map->selection.tile - map->selection.horz;
			y = 0;
			sx = map->selection.horz - 1;
			xinc = -1;
			break;
			
		case 1:
			mirStart = map->selection.tile + map->selection.horz;
			y = 0;
			sx = map->selection.horz - 1;
			xinc = -1;
			break;
			
		case 2:
			mirStart = map->selection.tile - map->selection.vert*map->header.width;
			sx = 0;
			y = map->selection.vert - 1;
			yinc = -1;
			break;
			
		case 3:
			mirStart = map->selection.tile + map->selection.vert*map->header.width;
			y = map->selection.vert - 1;
			yinc = -1;
			sx = 0;
			break;
			
		default:
			break;
		}
        
		for (i = 0; i < map->selection.vert; i++, y += yinc) {
			x = sx;
			
			for (j = 0; j < map->selection.horz; j++, x += xinc) {
				
				p = mirStart+y*256+x;
				if (p > (map->header.width*map->header.height)-1 || p < 0)
					continue;
				
				if (map->mapData[map->selection.tile+i*256+j] & 0x8000)
					pseudo = PseudoMD(map->mapData[map->selection.tile+i*256+j]&0xFF, TRUE);
				else
					pseudo = PseudoMD(map->mapData[map->selection.tile+i*256+j], FALSE);
				
				if ((direction < 2?pseudo.horz:pseudo.vert) == -1)
					map->mapData[mirStart+y*256+x] = map->mapData[map->selection.tile+i*256+j];
				else if (!((direction < 2?pseudo.horz:pseudo.vert) & 0x8000))
					map->mapData[mirStart+y*256+x] = (direction < 2?pseudo.horz:pseudo.vert);
				else
					map->mapData[mirStart+y*256+x] = (direction < 2?pseudo.horz:pseudo.vert) | 0x8000 | (map->mapData[map->selection.tile+i*256+j]&0x7F00);
				
			}
		}
		
		BufferCompare(map->mapData, oldBuf, map->header.width, map->header.height, &undoInfo);
		if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
		free(oldBuf);
		
		return TRUE;
		
	}
	else if (map)
		MessageBox(NULL, "You did not select an area", "Error", MB_OK);
	
	return FALSE;
}

BOOL copyBits(BOOL cut, struct MapInfo *map) {
	UINT ARC_FORMAT;
	HANDLE hMem;
	WORD *oldBuf, *buffer;
	struct undo_buf *undoInfo;
	int x, y, pos = 2;
	
	if (map && map->mapData && map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
		
		if (!OpenClipboard(frameWnd)) {
			MessageBox(NULL, "Unable to open clipboard", "Error", MB_OK);
			return FALSE;
		}
		if (!EmptyClipboard()) {
			MessageBox(NULL, "Error emptying contents", "Error", MB_OK);
			CloseClipboard();
			return FALSE;
		}
		
		hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
			4 + map->selection.horz * map->selection.vert * 2);
		buffer = (WORD *)GlobalLock(hMem);
		
		ARC_FORMAT = RegisterClipboardFormat("MalletEdit");
		
		buffer[0] = map->selection.horz;
		buffer[1] = map->selection.vert;
		
		oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
		memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
		
		for (y = 0; y < map->selection.vert; y++) {
			for (x = 0; x < map->selection.horz; x++) {
				buffer[pos++] =
					map->mapData[map->selection.tile + y*map->header.width + x];
				if (cut) map->mapData[map->selection.tile + y*map->header.width + x] = 280;
			}
		}
		
		BufferCompare(map->mapData, oldBuf, map->header.width, map->header.height, &undoInfo);
		if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
		free(oldBuf);
		
		GlobalUnlock(hMem);
		if (SetClipboardData(ARC_FORMAT, hMem) == NULL) {
			MessageBox(NULL, "Error settings data", "Error", MB_OK);
			CloseClipboard();
			return FALSE;
		}
		CloseClipboard();
		return TRUE;
	}
	else if (map)
		MessageBox(NULL, "You have not selected an area", "Error", MB_OK);
	return FALSE;
}

BOOL pasteBits(struct MapInfo *map) {
	UINT ARC_FORMAT;
	HANDLE hMem;
	WORD *oldBuf, *buffer, *data;
	struct undo_buf *undoInfo;
	int x, y, pos = 2;
	WORD width, height;
	
	if (map && map->mapData && map->selection.tile != -1 && map->selection.horz && map->selection.vert) {
		if (!OpenClipboard(frameWnd)) {
			MessageBox(NULL, "Unable to open clipboard", "Error", MB_OK);
			return FALSE;
		}
		ARC_FORMAT = RegisterClipboardFormat("MalletEdit");
		
		hMem = GetClipboardData(ARC_FORMAT);
		if (!hMem) return FALSE;
		data = (WORD *)GlobalLock(hMem);
		buffer = (WORD *)malloc(GlobalSize(hMem));
		memcpy(buffer, data, GlobalSize(hMem));
		GlobalUnlock(hMem);
		CloseClipboard();
		
		oldBuf = (WORD *)malloc(map->header.width * map->header.height * 2);
		memcpy(oldBuf, map->mapData, map->header.width * map->header.height * 2);
        
		width = buffer[0];
		height = buffer[1];
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (map->selection.tile + y*map->header.width + x > map->header.width*map->header.height-1)
					continue;
				
				map->mapData[map->selection.tile + y*map->header.width + x] = buffer[pos++];
			}
		}
		
		free(buffer);
		
		BufferCompare(map->mapData, oldBuf, map->header.width, map->header.height, &undoInfo);
		if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
		free(oldBuf);
		
		return TRUE;
	}
	else if (map)
		MessageBox(NULL, "You have not selected an area", "Error", MB_OK);
	return FALSE;
}

/* FillBits
** Function: fill an area block that matches a specific tile(set)
** Parameters:
**      WORD tile - the tile to match against
**      int  x - the x position of our starting point
**      int  y - the y position of our starting point
**		struct MapInfo *map - the map being filled
**
** Return Value:
**      TRUE if filled
**      FALSE if tile at x,y is the same as tile matching
**
** Comments:
**      Originally this function was designed to be  recursive for ease.
**      However, that was causing stack faults in some cases. Instead we use a heap-stack
**      algorithm designed by sedition
*/
#define FB_RIGHT	1
#define FB_LEFT		2
#define FB_UP		4
#define FB_DOWN		8

typedef struct _fbPosition {
	WORD x;
	WORD y;
} fbpos;

signed char _fill_tiledata[4000] = {
	0,		/*    0 */
		0,		/*    1 */
		0,		/*    2 */
		0,		/*    3 */
		1,		/*    4 */
		1,		/*    5 */
		1,		/*    6 */
		1,		/*    7 */
		1,		/*    8 */
		1,		/*    9 */
		1,		/*   10 */
		1,		/*   11 */
		1,		/*   12 */
		-1,		/*   13 */
		2,		/*   14 */
		2,		/*   15 */
		2,		/*   16 */
		2,		/*   17 */
		2,		/*   18 */
		2,		/*   19 */
		2,		/*   20 */
		-1,		/*   21 */
		-1,		/*   22 */
		-1,		/*   23 */
		1,		/*   24 */
		1,		/*   25 */
		1,		/*   26 */
		-1,		/*   27 */
		-1,		/*   28 */
		-1,		/*   29 */
		-1,		/*   30 */
		-1,		/*   31 */
		-1,		/*   32 */
		-1,		/*   33 */
		-1,		/*   34 */
		-1,		/*   35 */
		9,		/*   36 */
		5,		/*   37 */
		4,		/*   38 */
		-1,		/*   39 */
		0,		/*   40 */
		0,		/*   41 */
		0,		/*   42 */
		0,		/*   43 */
		0,		/*   44 */
		0,		/*   45 */
		1,		/*   46 */
		1,		/*   47 */
		1,		/*   48 */
		1,		/*   49 */
		1,		/*   50 */
		1,		/*   51 */
		1,		/*   52 */
		2,		/*   53 */
		2,		/*   54 */
		2,		/*   55 */
		2,		/*   56 */
		2,		/*   57 */
		2,		/*   58 */
		2,		/*   59 */
		2,		/*   60 */
		-1,		/*   61 */
		-1,		/*   62 */
		-1,		/*   63 */
		1,		/*   64 */
		-1,		/*   65 */
		-1,		/*   66 */
		-1,		/*   67 */
		-1,		/*   68 */
		-1,		/*   69 */
		-1,		/*   70 */
		-1,		/*   71 */
		-1,		/*   72 */
		-1,		/*   73 */
		-1,		/*   74 */
		-1,		/*   75 */
		0,		/*   76 */
		-1,		/*   77 */
		-1,		/*   78 */
		-1,		/*   79 */
		0,		/*   80 */
		0,		/*   81 */
		0,		/*   82 */
		0,		/*   83 */
		5,		/*   84 */
		5,		/*   85 */
		5,		/*   86 */
		5,		/*   87 */
		7,		/*   88 */
		1,		/*   89 */
		-1,		/*   90 */
		-1,		/*   91 */
		1,		/*   92 */
		1,		/*   93 */
		-1,		/*   94 */
		-1,		/*   95 */
		-1,		/*   96 */
		-1,		/*   97 */
		-1,		/*   98 */
		1,		/*   99 */
		-1,		/*  100 */
		-1,		/*  101 */
		-1,		/*  102 */
		-1,		/*  103 */
		-1,		/*  104 */
		-1,		/*  105 */
		-1,		/*  106 */
		-1,		/*  107 */
		-1,		/*  108 */
		-1,		/*  109 */
		-1,		/*  110 */
		-1,		/*  111 */
		-1,		/*  112 */
		-1,		/*  113 */
		-1,		/*  114 */
		-1,		/*  115 */
		4,		/*  116 */
		4,		/*  117 */
		4,		/*  118 */
		4,		/*  119 */
		0,		/*  120 */
		0,		/*  121 */
		0,		/*  122 */
		0,		/*  123 */
		6,		/*  124 */
		6,		/*  125 */
		6,		/*  126 */
		5,		/*  127 */
		7,		/*  128 */
		-1,		/*  129 */
		-1,		/*  130 */
		-1,		/*  131 */
		1,		/*  132 */
		1,		/*  133 */
		-1,		/*  134 */
		-1,		/*  135 */
		-1,		/*  136 */
		-1,		/*  137 */
		-1,		/*  138 */
		1,		/*  139 */
		1,		/*  140 */
		-1,		/*  141 */
		-1,		/*  142 */
		-1,		/*  143 */
		-1,		/*  144 */
		-1,		/*  145 */
		5,		/*  146 */
		0,		/*  147 */
		-1,		/*  148 */
		-1,		/*  149 */
		-1,		/*  150 */
		-1,		/*  151 */
		-1,		/*  152 */
		-1,		/*  153 */
		-1,		/*  154 */
		-1,		/*  155 */
		4,		/*  156 */
		4,		/*  157 */
		4,		/*  158 */
		4,		/*  159 */
		5,		/*  160 */
		-1,		/*  161 */
		5,		/*  162 */
		5,		/*  163 */
		7,		/*  164 */
		7,		/*  165 */
		7,		/*  166 */
		7,		/*  167 */
		7,		/*  168 */
		5,		/*  169 */
		5,		/*  170 */
		5,		/*  171 */
		-1,		/*  172 */
		-1,		/*  173 */
		-1,		/*  174 */
		-1,		/*  175 */
		-1,		/*  176 */
		3,		/*  177 */
		3,		/*  178 */
		1,		/*  179 */
		-1,		/*  180 */
		-1,		/*  181 */
		-1,		/*  182 */
		-1,		/*  183 */
		-1,		/*  184 */
		-1,		/*  185 */
		5,		/*  186 */
		0,		/*  187 */
		-1,		/*  188 */
		-1,		/*  189 */
		-1,		/*  190 */
		-1,		/*  191 */
		-1,		/*  192 */
		-1,		/*  193 */
		-1,		/*  194 */
		-1,		/*  195 */
		4,		/*  196 */
		4,		/*  197 */
		4,		/*  198 */
		4,		/*  199 */
		5,		/*  200 */
		5,		/*  201 */
		5,		/*  202 */
		5,		/*  203 */
		7,		/*  204 */
		7,		/*  205 */
		7,		/*  206 */
		7,		/*  207 */
		8,		/*  208 */
		8,		/*  209 */
		8,		/*  210 */
		8,		/*  211 */
		8,		/*  212 */
		8,		/*  213 */
		8,		/*  214 */
		8,		/*  215 */
		3,		/*  216 */
		3,		/*  217 */
		3,		/*  218 */
		3,		/*  219 */
		3,		/*  220 */
		3,		/*  221 */
		3,		/*  222 */
		-1,		/*  223 */
		-1,		/*  224 */
		-1,		/*  225 */
		-1,		/*  226 */
		-1,		/*  227 */
		-1,		/*  228 */
		-1,		/*  229 */
		-1,		/*  230 */
		-1,		/*  231 */
		10,		/*  232 */
		-1,		/*  233 */
		-1,		/*  234 */
		-1,		/*  235 */
		4,		/*  236 */
		4,		/*  237 */
		4,		/*  238 */
		4,		/*  239 */
		-1,		/*  240 */
		5,		/*  241 */
		5,		/*  242 */
		5,		/*  243 */
		7,		/*  244 */
		7,		/*  245 */
		7,		/*  246 */
		7,		/*  247 */
		8,		/*  248 */
		8,		/*  249 */
		8,		/*  250 */
		8,		/*  251 */
		8,		/*  252 */
		8,		/*  253 */
		8,		/*  254 */
		8,		/*  255 */
		3,		/*  256 */
		3,		/*  257 */
		3,		/*  258 */
		3,		/*  259 */
		3,		/*  260 */
		3,		/*  261 */
		3,		/*  262 */
		-1,		/*  263 */
		-1,		/*  264 */
		-1,		/*  265 */
		-1,		/*  266 */
		-1,		/*  267 */
		10,		/*  268 */
		10,		/*  269 */
		10,		/*  270 */
		10,		/*  271 */
		10,		/*  272 */
		10,		/*  273 */
		10,		/*  274 */
		10,		/*  275 */
		5,		/*  276 */
		0,		/*  277 */
		-1,		/*  278 */
		-1,		/*  279 */
		-1,		/*  280 */
		5,		/*  281 */
		5,		/*  282 */
		5,		/*  283 */
		6,		/*  284 */
		6,		/*  285 */
		6,		/*  286 */
		5,		/*  287 */
		5,		/*  288 */
		8,		/*  289 */
		8,		/*  290 */
		8,		/*  291 */
		8,		/*  292 */
		8,		/*  293 */
		-1,		/*  294 */
		-1,		/*  295 */
		-1,		/*  296 */
		0,		/*  297 */
		0,		/*  298 */
		-1,		/*  299 */
		-1,		/*  300 */
		13,		/*  301 */
		13,		/*  302 */
		13,		/*  303 */
		13,		/*  304 */
		13,		/*  305 */
		13,		/*  306 */
		13,		/*  307 */
		13,		/*  308 */
		10,		/*  309 */
		10,		/*  310 */
		10,		/*  311 */
		10,		/*  312 */
		10,		/*  313 */
		10,		/*  314 */
		10,		/*  315 */
		5,		/*  316 */
		0,		/*  317 */
		-1,		/*  318 */
		-1,		/*  319 */
		5,		/*  320 */
		-1,		/*  321 */
		-1,		/*  322 */
		-1,		/*  323 */
		6,		/*  324 */
		6,		/*  325 */
		6,		/*  326 */
		5,		/*  327 */
		5,		/*  328 */
		8,		/*  329 */
		8,		/*  330 */
		8,		/*  331 */
		8,		/*  332 */
		8,		/*  333 */
		8,		/*  334 */
		-1,		/*  335 */
		0,		/*  336 */
		0,		/*  337 */
		0,		/*  338 */
		0,		/*  339 */
		-1,		/*  340 */
		13,		/*  341 */
		13,		/*  342 */
		13,		/*  343 */
		13,		/*  344 */
		13,		/*  345 */
		13,		/*  346 */
		13,		/*  347 */
		13,		/*  348 */
		11,		/*  349 */
		11,		/*  350 */
		11,		/*  351 */
		11,		/*  352 */
		11,		/*  353 */
		11,		/*  354 */
		11,		/*  355 */
		11,		/*  356 */
		-1,		/*  357 */
		-1,		/*  358 */
		-1,		/*  359 */
		-1,		/*  360 */
		-1,		/*  361 */
		-1,		/*  362 */
		-1,		/*  363 */
		6,		/*  364 */
		6,		/*  365 */
		6,		/*  366 */
		-1,		/*  367 */
		-1,		/*  368 */
		8,		/*  369 */
		8,		/*  370 */
		8,		/*  371 */
		8,		/*  372 */
		8,		/*  373 */
		8,		/*  374 */
		-1,		/*  375 */
		0,		/*  376 */
		0,		/*  377 */
		0,		/*  378 */
		0,		/*  379 */
		-1,		/*  380 */
		13,		/*  381 */
		13,		/*  382 */
		13,		/*  383 */
		13,		/*  384 */
		13,		/*  385 */
		13,		/*  386 */
		13,		/*  387 */
		13,		/*  388 */
		11,		/*  389 */
		11,		/*  390 */
		11,		/*  391 */
		11,		/*  392 */
		11,		/*  393 */
		11,		/*  394 */
		11,		/*  395 */
		11,		/*  396 */
		-1,		/*  397 */
		-1,		/*  398 */
		-1,		/*  399 */
		-1,		/*  400 */
		-1,		/*  401 */
		-1,		/*  402 */
		-1,		/*  403 */
		6,		/*  404 */
		6,		/*  405 */
		6,		/*  406 */
		-1,		/*  407 */
		-1,		/*  408 */
		-1,		/*  409 */
		14,		/*  410 */
		14,		/*  411 */
		-1,		/*  412 */
		9,		/*  413 */
		9,		/*  414 */
		9,		/*  415 */
		9,		/*  416 */
		0,		/*  417 */
		0,		/*  418 */
		14,		/*  419 */
		14,		/*  420 */
		14,		/*  421 */
		13,		/*  422 */
		13,		/*  423 */
		13,		/*  424 */
		13,		/*  425 */
		13,		/*  426 */
		13,		/*  427 */
		-1,		/*  428 */
		-1,		/*  429 */
		-1,		/*  430 */
		-1,		/*  431 */
		-1,		/*  432 */
		-1,		/*  433 */
		12,		/*  434 */
		12,		/*  435 */
		12,		/*  436 */
		12,		/*  437 */
		-1,		/*  438 */
		-1,		/*  439 */
		-1,		/*  440 */
		-1,		/*  441 */
		-1,		/*  442 */
		-1,		/*  443 */
		-1,		/*  444 */
		-1,		/*  445 */
		-1,		/*  446 */
		-1,		/*  447 */
		-1,		/*  448 */
		-1,		/*  449 */
		14,		/*  450 */
		14,		/*  451 */
		-1,		/*  452 */
		9,		/*  453 */
		9,		/*  454 */
		9,		/*  455 */
		9,		/*  456 */
		-1,		/*  457 */
		14,		/*  458 */
		14,		/*  459 */
		14,		/*  460 */
		14,		/*  461 */
		14,		/*  462 */
		14,		/*  463 */
		-1,		/*  464 */
		-1,		/*  465 */
		13,		/*  466 */
		13,		/*  467 */
		-1,		/*  468 */
		-1,		/*  469 */
		-1,		/*  470 */
		-1,		/*  471 */
		-1,		/*  472 */
		-1,		/*  473 */
		12,		/*  474 */
		12,		/*  475 */
		12,		/*  476 */
		12,		/*  477 */
		-1,		/*  478 */
		-1,		/*  479 */
		-1,		/*  480 */
		-1,		/*  481 */
		-1,		/*  482 */
		-1,		/*  483 */
		-1,		/*  484 */
		-1,		/*  485 */
		-1,		/*  486 */
		-1,		/*  487 */
		-1,		/*  488 */
		-1,		/*  489 */
		-1,		/*  490 */
		-1,		/*  491 */
		-1,		/*  492 */
		9,		/*  493 */
		9,		/*  494 */
		9,		/*  495 */
		9,		/*  496 */
		-1,		/*  497 */
		-1,		/*  498 */
		14,		/*  499 */
		14,		/*  500 */
		14,		/*  501 */
		14,		/*  502 */
		14,		/*  503 */
		15,		/*  504 */
		15,		/*  505 */
		15,		/*  506 */
		15,		/*  507 */
		-1,		/*  508 */
		-1,		/*  509 */
		-1,		/*  510 */
		-1,		/*  511 */
		-1,		/*  512 */
		-1,		/*  513 */
		12,		/*  514 */
		12,		/*  515 */
		12,		/*  516 */
		12,		/*  517 */
		-1,		/*  518 */
		-1,		/*  519 */
		-1,		/*  520 */
		-1,		/*  521 */
		-1,		/*  522 */
		-1,		/*  523 */
		-1,		/*  524 */
		-1,		/*  525 */
		-1,		/*  526 */
		-1,		/*  527 */
		-1,		/*  528 */
		-1,		/*  529 */
		-1,		/*  530 */
		-1,		/*  531 */
		-1,		/*  532 */
		9,		/*  533 */
		9,		/*  534 */
		9,		/*  535 */
		9,		/*  536 */
		16,		/*  537 */
		16,		/*  538 */
		16,		/*  539 */
		16,		/*  540 */
		14,		/*  541 */
		14,		/*  542 */
		14,		/*  543 */
		15,		/*  544 */
		15,		/*  545 */
		15,		/*  546 */
		15,		/*  547 */
		-1,		/*  548 */
		-1,		/*  549 */
		-1,		/*  550 */
		-1,		/*  551 */
		-1,		/*  552 */
		-1,		/*  553 */
		12,		/*  554 */
		12,		/*  555 */
		12,		/*  556 */
		12,		/*  557 */
		-1,		/*  558 */
		-1,		/*  559 */
		-1,		/*  560 */
		-1,		/*  561 */
		-1,		/*  562 */
		-1,		/*  563 */
		-1,		/*  564 */
		-1,		/*  565 */
		-1,		/*  566 */
		-1,		/*  567 */
		-1,		/*  568 */
		9,		/*  569 */
		9,		/*  570 */
		-1,		/*  571 */
		-1,		/*  572 */
		-1,		/*  573 */
		-1,		/*  574 */
		-1,		/*  575 */
		-1,		/*  576 */
		16,		/*  577 */
		16,		/*  578 */
		16,		/*  579 */
		16,		/*  580 */
		-1,		/*  581 */
		-1,		/*  582 */
		-1,		/*  583 */
		15,		/*  584 */
		15,		/*  585 */
		15,		/*  586 */
		15,		/*  587 */
		-1,		/*  588 */
		-1,		/*  589 */
		-1,		/*  590 */
		-1,		/*  591 */
		-1,		/*  592 */
		-1,		/*  593 */
		-1,		/*  594 */
		-1,		/*  595 */
		-1,		/*  596 */
		-1,		/*  597 */
		-1,		/*  598 */
		-1,		/*  599 */
		-1,		/*  600 */
		-1,		/*  601 */
		-1,		/*  602 */
		-1,		/*  603 */
		-1,		/*  604 */
		-1,		/*  605 */
		-1,		/*  606 */
		9,		/*  607 */
		9,		/*  608 */
		9,		/*  609 */
		9,		/*  610 */
		-1,		/*  611 */
		-1,		/*  612 */
		-1,		/*  613 */
		-1,		/*  614 */
		-1,		/*  615 */
		-1,		/*  616 */
		16,		/*  617 */
		16,		/*  618 */
		16,		/*  619 */
		16,		/*  620 */
		-1,		/*  621 */
		19,		/*  622 */
		19,		/*  623 */
		-1,		/*  624 */
		15,		/*  625 */
		15,		/*  626 */
		15,		/*  627 */
		-1,		/*  628 */
		-1,		/*  629 */
		-1,		/*  630 */
		-1,		/*  631 */
		-1,		/*  632 */
		-1,		/*  633 */
		-1,		/*  634 */
		-1,		/*  635 */
		-1,		/*  636 */
		-1,		/*  637 */
		-1,		/*  638 */
		-1,		/*  639 */
		-1,		/*  640 */
		-1,		/*  641 */
		-1,		/*  642 */
		-1,		/*  643 */
		-1,		/*  644 */
		-1,		/*  645 */
		-1,		/*  646 */
		9,		/*  647 */
		9,		/*  648 */
		9,		/*  649 */
		-1,		/*  650 */
		-1,		/*  651 */
		-1,		/*  652 */
		-1,		/*  653 */
		-1,		/*  654 */
		-1,		/*  655 */
		-1,		/*  656 */
		-1,		/*  657 */
		16,		/*  658 */
		16,		/*  659 */
		16,		/*  660 */
		19,		/*  661 */
		19,		/*  662 */
		19,		/*  663 */
		-1,		/*  664 */
		-1,		/*  665 */
		-1,		/*  666 */
		-1,		/*  667 */
		-1,		/*  668 */
		-1,		/*  669 */
		-1,		/*  670 */
		-1,		/*  671 */
		-1,		/*  672 */
		-1,		/*  673 */
		-1,		/*  674 */
		-1,		/*  675 */
		-1,		/*  676 */
		-1,		/*  677 */
		-1,		/*  678 */
		-1,		/*  679 */
		-1,		/*  680 */
		-1,		/*  681 */
		-1,		/*  682 */
		-1,		/*  683 */
		-1,		/*  684 */
		-1,		/*  685 */
		17,		/*  686 */
		17,		/*  687 */
		17,		/*  688 */
		17,		/*  689 */
		17,		/*  690 */
		17,		/*  691 */
		17,		/*  692 */
		17,		/*  693 */
		18,		/*  694 */
		18,		/*  695 */
		18,		/*  696 */
		18,		/*  697 */
		19,		/*  698 */
		19,		/*  699 */
		19,		/*  700 */
		19,		/*  701 */
		20,		/*  702 */
		20,		/*  703 */
		20,		/*  704 */
		-1,		/*  705 */
		-1,		/*  706 */
		-1,		/*  707 */
		-1,		/*  708 */
		-1,		/*  709 */
		-1,		/*  710 */
		-1,		/*  711 */
		-1,		/*  712 */
		-1,		/*  713 */
		-1,		/*  714 */
		-1,		/*  715 */
		-1,		/*  716 */
		-1,		/*  717 */
		-1,		/*  718 */
		-1,		/*  719 */
		-1,		/*  720 */
		-1,		/*  721 */
		-1,		/*  722 */
		-1,		/*  723 */
		-1,		/*  724 */
		-1,		/*  725 */
		17,		/*  726 */
		17,		/*  727 */
		17,		/*  728 */
		17,		/*  729 */
		17,		/*  730 */
		17,		/*  731 */
		17,		/*  732 */
		17,		/*  733 */
		18,		/*  734 */
		18,		/*  735 */
		18,		/*  736 */
		18,		/*  737 */
		19,		/*  738 */
		19,		/*  739 */
		19,		/*  740 */
		19,		/*  741 */
		20,		/*  742 */
		20,		/*  743 */
		20,		/*  744 */
		-1,		/*  745 */
		-1,		/*  746 */
		-1,		/*  747 */
		-1,		/*  748 */
		-1,		/*  749 */
		-1,		/*  750 */
		-1,		/*  751 */
		-1,		/*  752 */
		-1,		/*  753 */
		-1,		/*  754 */
		-1,		/*  755 */
		-1,		/*  756 */
		-1,		/*  757 */
		-1,		/*  758 */
		-1,		/*  759 */
		-1,		/*  760 */
		-1,		/*  761 */
		-1,		/*  762 */
		-1,		/*  763 */
		-1,		/*  764 */
		-1,		/*  765 */
		17,		/*  766 */
		17,		/*  767 */
		17,		/*  768 */
		17,		/*  769 */
		17,		/*  770 */
		17,		/*  771 */
		17,		/*  772 */
		17,		/*  773 */
		18,		/*  774 */
		18,		/*  775 */
		18,		/*  776 */
		18,		/*  777 */
		19,		/*  778 */
		19,		/*  779 */
		19,		/*  780 */
		19,		/*  781 */
		20,		/*  782 */
		20,		/*  783 */
		20,		/*  784 */
		-1,		/*  785 */
		-1,		/*  786 */
		-1,		/*  787 */
		-1,		/*  788 */
		-1,		/*  789 */
		-1,		/*  790 */
		-1,		/*  791 */
		-1,		/*  792 */
		-1,		/*  793 */
		-1,		/*  794 */
		-1,		/*  795 */
		-1,		/*  796 */
		-1,		/*  797 */
		-1,		/*  798 */
		-1,		/*  799 */
		-1,		/*  800 */
		-1,		/*  801 */
		-1,		/*  802 */
		-1,		/*  803 */
		-1,		/*  804 */
		-1,		/*  805 */
		17,		/*  806 */
		17,		/*  807 */
		17,		/*  808 */
		17,		/*  809 */
		17,		/*  810 */
		17,		/*  811 */
		17,		/*  812 */
		17,		/*  813 */
		18,		/*  814 */
		18,		/*  815 */
		18,		/*  816 */
		18,		/*  817 */
		19,		/*  818 */
		19,		/*  819 */
		19,		/*  820 */
		19,		/*  821 */
		-1,		/*  822 */
		-1,		/*  823 */
		-1,		/*  824 */
		-1,		/*  825 */
		-1,		/*  826 */
		-1,		/*  827 */
		-1,		/*  828 */
		-1,		/*  829 */
		-1,		/*  830 */
		-1,		/*  831 */
		-1,		/*  832 */
		-1,		/*  833 */
		-1,		/*  834 */
		-1,		/*  835 */
		-1,		/*  836 */
		-1,		/*  837 */
		0,		/*  838 */
		-1,		/*  839 */
		-1,		/*  840 */
		-1,		/*  841 */
		-1,		/*  842 */
		-1,		/*  843 */
		-1,		/*  844 */
		-1,		/*  845 */
		-1,		/*  846 */
		-1,		/*  847 */
		-1,		/*  848 */
		-1,		/*  849 */
		-1,		/*  850 */
		-1,		/*  851 */
		-1,		/*  852 */
		-1,		/*  853 */
		-1,		/*  854 */
		-1,		/*  855 */
		-1,		/*  856 */
		-1,		/*  857 */
		-1,		/*  858 */
		-1,		/*  859 */
		-1,		/*  860 */
		-1,		/*  861 */
		-1,		/*  862 */
		-1,		/*  863 */
		-1,		/*  864 */
		-1,		/*  865 */
		-1,		/*  866 */
		-1,		/*  867 */
		-1,		/*  868 */
		-1,		/*  869 */
		-1,		/*  870 */
		-1,		/*  871 */
		-1,		/*  872 */
		-1,		/*  873 */
		-1,		/*  874 */
		-1,		/*  875 */
		-1,		/*  876 */
		0,		/*  877 */
		-1,		/*  878 */
		-1,		/*  879 */
		-1,		/*  880 */
		-1,		/*  881 */
		-1,		/*  882 */
		-1,		/*  883 */
		-1,		/*  884 */
		-1,		/*  885 */
		-1,		/*  886 */
		-1,		/*  887 */
		-1,		/*  888 */
		-1,		/*  889 */
		-1,		/*  890 */
		-1,		/*  891 */
		-1,		/*  892 */
		-1,		/*  893 */
		-1,		/*  894 */
		-1,		/*  895 */
		-1,		/*  896 */
		-1,		/*  897 */
		-1,		/*  898 */
		-1,		/*  899 */
		-1,		/*  900 */
		-1,		/*  901 */
		-1,		/*  902 */
		-1,		/*  903 */
		-1,		/*  904 */
		-1,		/*  905 */
		-1,		/*  906 */
		-1,		/*  907 */
		-1,		/*  908 */
		-1,		/*  909 */
		-1,		/*  910 */
		-1,		/*  911 */
		-1,		/*  912 */
		-1,		/*  913 */
		-1,		/*  914 */
		-1,		/*  915 */
		-1,		/*  916 */
		-1,		/*  917 */
		-1,		/*  918 */
		-1,		/*  919 */
		-1,		/*  920 */
		-1,		/*  921 */
		-1,		/*  922 */
		-1,		/*  923 */
		-1,		/*  924 */
		-1,		/*  925 */
		-1,		/*  926 */
		-1,		/*  927 */
		-1,		/*  928 */
		-1,		/*  929 */
		-1,		/*  930 */
		-1,		/*  931 */
		-1,		/*  932 */
		-1,		/*  933 */
		-1,		/*  934 */
		-1,		/*  935 */
		-1,		/*  936 */
		-1,		/*  937 */
		-1,		/*  938 */
		-1,		/*  939 */
		-1,		/*  940 */
		-1,		/*  941 */
		-1,		/*  942 */
		-1,		/*  943 */
		-1,		/*  944 */
		-1,		/*  945 */
		-1,		/*  946 */
		-1,		/*  947 */
		-1,		/*  948 */
		-1,		/*  949 */
		-1,		/*  950 */
		-1,		/*  951 */
		-1,		/*  952 */
		-1,		/*  953 */
		-1,		/*  954 */
		-1,		/*  955 */
		-1,		/*  956 */
		-1,		/*  957 */
		-1,		/*  958 */
		-1,		/*  959 */
		-1,		/*  960 */
		-1,		/*  961 */
		-1,		/*  962 */
		-1,		/*  963 */
		-1,		/*  964 */
		-1,		/*  965 */
		-1,		/*  966 */
		-1,		/*  967 */
		-1,		/*  968 */
		-1,		/*  969 */
		-1,		/*  970 */
		-1,		/*  971 */
		-1,		/*  972 */
		-1,		/*  973 */
		-1,		/*  974 */
		-1,		/*  975 */
		-1,		/*  976 */
		-1,		/*  977 */
		-1,		/*  978 */
		-1,		/*  979 */
		-1,		/*  980 */
		-1,		/*  981 */
		-1,		/*  982 */
		-1,		/*  983 */
		-1,		/*  984 */
		-1,		/*  985 */
		-1,		/*  986 */
		-1,		/*  987 */
		-1,		/*  988 */
		-1,		/*  989 */
		-1,		/*  990 */
		-1,		/*  991 */
		-1,		/*  992 */
		-1,		/*  993 */
		-1,		/*  994 */
		-1,		/*  995 */
		-1,		/*  996 */
		-1,		/*  997 */
		-1,		/*  998 */
		-1,		/*  999 */
		-1,		/* 1000 */
		-1,		/* 1001 */
		-1,		/* 1002 */
		-1,		/* 1003 */
		-1,		/* 1004 */
		-1,		/* 1005 */
		-1,		/* 1006 */
		-1,		/* 1007 */
		-1,		/* 1008 */
		-1,		/* 1009 */
		-1,		/* 1010 */
		-1,		/* 1011 */
		-1,		/* 1012 */
		-1,		/* 1013 */
		-1,		/* 1014 */
		-1,		/* 1015 */
		-1,		/* 1016 */
		-1,		/* 1017 */
		-1,		/* 1018 */
		-1,		/* 1019 */
		-1,		/* 1020 */
		-1,		/* 1021 */
		-1,		/* 1022 */
		-1,		/* 1023 */
		-1,		/* 1024 */
		-1,		/* 1025 */
		-1,		/* 1026 */
		-1,		/* 1027 */
		-1,		/* 1028 */
		-1,		/* 1029 */
		-1,		/* 1030 */
		-1,		/* 1031 */
		-1,		/* 1032 */
		-1,		/* 1033 */
		-1,		/* 1034 */
		-1,		/* 1035 */
		-1,		/* 1036 */
		-1,		/* 1037 */
		-1,		/* 1038 */
		-1,		/* 1039 */
		-1,		/* 1040 */
		-1,		/* 1041 */
		-1,		/* 1042 */
		-1,		/* 1043 */
		-1,		/* 1044 */
		-1,		/* 1045 */
		-1,		/* 1046 */
		-1,		/* 1047 */
		-1,		/* 1048 */
		-1,		/* 1049 */
		-1,		/* 1050 */
		-1,		/* 1051 */
		-1,		/* 1052 */
		-1,		/* 1053 */
		-1,		/* 1054 */
		-1,		/* 1055 */
		-1,		/* 1056 */
		-1,		/* 1057 */
		-1,		/* 1058 */
		-1,		/* 1059 */
		-1,		/* 1060 */
		-1,		/* 1061 */
		-1,		/* 1062 */
		-1,		/* 1063 */
		-1,		/* 1064 */
		-1,		/* 1065 */
		-1,		/* 1066 */
		-1,		/* 1067 */
		-1,		/* 1068 */
		-1,		/* 1069 */
		-1,		/* 1070 */
		-1,		/* 1071 */
		-1,		/* 1072 */
		-1,		/* 1073 */
		-1,		/* 1074 */
		-1,		/* 1075 */
		-1,		/* 1076 */
		-1,		/* 1077 */
		-1,		/* 1078 */
		-1,		/* 1079 */
		-1,		/* 1080 */
		-1,		/* 1081 */
		-1,		/* 1082 */
		-1,		/* 1083 */
		-1,		/* 1084 */
		-1,		/* 1085 */
		-1,		/* 1086 */
		-1,		/* 1087 */
		-1,		/* 1088 */
		-1,		/* 1089 */
		-1,		/* 1090 */
		-1,		/* 1091 */
		-1,		/* 1092 */
		-1,		/* 1093 */
		-1,		/* 1094 */
		-1,		/* 1095 */
		-1,		/* 1096 */
		-1,		/* 1097 */
		-1,		/* 1098 */
		-1,		/* 1099 */
		-1,		/* 1100 */
		-1,		/* 1101 */
		-1,		/* 1102 */
		-1,		/* 1103 */
		-1,		/* 1104 */
		-1,		/* 1105 */
		-1,		/* 1106 */
		-1,		/* 1107 */
		-1,		/* 1108 */
		-1,		/* 1109 */
		-1,		/* 1110 */
		-1,		/* 1111 */
		-1,		/* 1112 */
		-1,		/* 1113 */
		-1,		/* 1114 */
		-1,		/* 1115 */
		-1,		/* 1116 */
		-1,		/* 1117 */
		-1,		/* 1118 */
		-1,		/* 1119 */
		-1,		/* 1120 */
		-1,		/* 1121 */
		-1,		/* 1122 */
		-1,		/* 1123 */
		-1,		/* 1124 */
		-1,		/* 1125 */
		-1,		/* 1126 */
		-1,		/* 1127 */
		-1,		/* 1128 */
		-1,		/* 1129 */
		-1,		/* 1130 */
		-1,		/* 1131 */
		-1,		/* 1132 */
		-1,		/* 1133 */
		-1,		/* 1134 */
		-1,		/* 1135 */
		-1,		/* 1136 */
		-1,		/* 1137 */
		-1,		/* 1138 */
		-1,		/* 1139 */
		-1,		/* 1140 */
		-1,		/* 1141 */
		-1,		/* 1142 */
		-1,		/* 1143 */
		-1,		/* 1144 */
		-1,		/* 1145 */
		-1,		/* 1146 */
		-1,		/* 1147 */
		-1,		/* 1148 */
		-1,		/* 1149 */
		-1,		/* 1150 */
		-1,		/* 1151 */
		-1,		/* 1152 */
		-1,		/* 1153 */
		-1,		/* 1154 */
		-1,		/* 1155 */
		-1,		/* 1156 */
		-1,		/* 1157 */
		-1,		/* 1158 */
		-1,		/* 1159 */
		-1,		/* 1160 */
		-1,		/* 1161 */
		-1,		/* 1162 */
		-1,		/* 1163 */
		-1,		/* 1164 */
		-1,		/* 1165 */
		-1,		/* 1166 */
		-1,		/* 1167 */
		-1,		/* 1168 */
		-1,		/* 1169 */
		-1,		/* 1170 */
		-1,		/* 1171 */
		-1,		/* 1172 */
		-1,		/* 1173 */
		-1,		/* 1174 */
		-1,		/* 1175 */
		-1,		/* 1176 */
		-1,		/* 1177 */
		-1,		/* 1178 */
		-1,		/* 1179 */
		-1,		/* 1180 */
		-1,		/* 1181 */
		-1,		/* 1182 */
		-1,		/* 1183 */
		-1,		/* 1184 */
		-1,		/* 1185 */
		-1,		/* 1186 */
		-1,		/* 1187 */
		-1,		/* 1188 */
		-1,		/* 1189 */
		-1,		/* 1190 */
		-1,		/* 1191 */
		-1,		/* 1192 */
		-1,		/* 1193 */
		-1,		/* 1194 */
		-1,		/* 1195 */
		-1,		/* 1196 */
		-1,		/* 1197 */
		-1,		/* 1198 */
		-1,		/* 1199 */
		-1,		/* 1200 */
		-1,		/* 1201 */
		-1,		/* 1202 */
		-1,		/* 1203 */
		-1,		/* 1204 */
		-1,		/* 1205 */
		-1,		/* 1206 */
		-1,		/* 1207 */
		-1,		/* 1208 */
		-1,		/* 1209 */
		-1,		/* 1210 */
		-1,		/* 1211 */
		-1,		/* 1212 */
		-1,		/* 1213 */
		-1,		/* 1214 */
		-1,		/* 1215 */
		-1,		/* 1216 */
		-1,		/* 1217 */
		-1,		/* 1218 */
		-1,		/* 1219 */
		-1,		/* 1220 */
		-1,		/* 1221 */
		-1,		/* 1222 */
		-1,		/* 1223 */
		-1,		/* 1224 */
		-1,		/* 1225 */
		-1,		/* 1226 */
		-1,		/* 1227 */
		-1,		/* 1228 */
		-1,		/* 1229 */
		-1,		/* 1230 */
		-1,		/* 1231 */
		-1,		/* 1232 */
		-1,		/* 1233 */
		-1,		/* 1234 */
		-1,		/* 1235 */
		-1,		/* 1236 */
		-1,		/* 1237 */
		-1,		/* 1238 */
		-1,		/* 1239 */
		-1,		/* 1240 */
		-1,		/* 1241 */
		-1,		/* 1242 */
		-1,		/* 1243 */
		-1,		/* 1244 */
		-1,		/* 1245 */
		-1,		/* 1246 */
		-1,		/* 1247 */
		-1,		/* 1248 */
		-1,		/* 1249 */
		-1,		/* 1250 */
		-1,		/* 1251 */
		-1,		/* 1252 */
		-1,		/* 1253 */
		-1,		/* 1254 */
		-1,		/* 1255 */
		-1,		/* 1256 */
		-1,		/* 1257 */
		-1,		/* 1258 */
		-1,		/* 1259 */
		-1,		/* 1260 */
		-1,		/* 1261 */
		-1,		/* 1262 */
		-1,		/* 1263 */
		-1,		/* 1264 */
		-1,		/* 1265 */
		-1,		/* 1266 */
		-1,		/* 1267 */
		-1,		/* 1268 */
		-1,		/* 1269 */
		-1,		/* 1270 */
		-1,		/* 1271 */
		-1,		/* 1272 */
		-1,		/* 1273 */
		-1,		/* 1274 */
		-1,		/* 1275 */
		-1,		/* 1276 */
		-1,		/* 1277 */
		-1,		/* 1278 */
		-1,		/* 1279 */
		-1,		/* 1280 */
		-1,		/* 1281 */
		-1,		/* 1282 */
		-1,		/* 1283 */
		-1,		/* 1284 */
		-1,		/* 1285 */
		-1,		/* 1286 */
		-1,		/* 1287 */
		-1,		/* 1288 */
		-1,		/* 1289 */
		-1,		/* 1290 */
		-1,		/* 1291 */
		-1,		/* 1292 */
		-1,		/* 1293 */
		-1,		/* 1294 */
		-1,		/* 1295 */
		-1,		/* 1296 */
		-1,		/* 1297 */
		-1,		/* 1298 */
		-1,		/* 1299 */
		-1,		/* 1300 */
		-1,		/* 1301 */
		-1,		/* 1302 */
		-1,		/* 1303 */
		-1,		/* 1304 */
		-1,		/* 1305 */
		-1,		/* 1306 */
		-1,		/* 1307 */
		-1,		/* 1308 */
		-1,		/* 1309 */
		-1,		/* 1310 */
		-1,		/* 1311 */
		-1,		/* 1312 */
		-1,		/* 1313 */
		-1,		/* 1314 */
		-1,		/* 1315 */
		-1,		/* 1316 */
		-1,		/* 1317 */
		-1,		/* 1318 */
		-1,		/* 1319 */
		-1,		/* 1320 */
		-1,		/* 1321 */
		-1,		/* 1322 */
		-1,		/* 1323 */
		-1,		/* 1324 */
		-1,		/* 1325 */
		-1,		/* 1326 */
		-1,		/* 1327 */
		-1,		/* 1328 */
		-1,		/* 1329 */
		-1,		/* 1330 */
		-1,		/* 1331 */
		-1,		/* 1332 */
		-1,		/* 1333 */
		-1,		/* 1334 */
		-1,		/* 1335 */
		-1,		/* 1336 */
		-1,		/* 1337 */
		-1,		/* 1338 */
		-1,		/* 1339 */
		-1,		/* 1340 */
		-1,		/* 1341 */
		-1,		/* 1342 */
		-1,		/* 1343 */
		-1,		/* 1344 */
		-1,		/* 1345 */
		-1,		/* 1346 */
		-1,		/* 1347 */
		-1,		/* 1348 */
		-1,		/* 1349 */
		-1,		/* 1350 */
		-1,		/* 1351 */
		-1,		/* 1352 */
		-1,		/* 1353 */
		-1,		/* 1354 */
		-1,		/* 1355 */
		-1,		/* 1356 */
		-1,		/* 1357 */
		-1,		/* 1358 */
		0,		/* 1359 */
		-1,		/* 1360 */
		-1,		/* 1361 */
		-1,		/* 1362 */
		-1,		/* 1363 */
		-1,		/* 1364 */
		-1,		/* 1365 */
		-1,		/* 1366 */
		-1,		/* 1367 */
		-1,		/* 1368 */
		-1,		/* 1369 */
		-1,		/* 1370 */
		-1,		/* 1371 */
		-1,		/* 1372 */
		-1,		/* 1373 */
		-1,		/* 1374 */
		-1,		/* 1375 */
		-1,		/* 1376 */
		-1,		/* 1377 */
		-1,		/* 1378 */
		-1,		/* 1379 */
		-1,		/* 1380 */
		-1,		/* 1381 */
		-1,		/* 1382 */
		-1,		/* 1383 */
		-1,		/* 1384 */
		-1,		/* 1385 */
		-1,		/* 1386 */
		-1,		/* 1387 */
		-1,		/* 1388 */
		-1,		/* 1389 */
		-1,		/* 1390 */
		-1,		/* 1391 */
		-1,		/* 1392 */
		-1,		/* 1393 */
		-1,		/* 1394 */
		-1,		/* 1395 */
		-1,		/* 1396 */
		-1,		/* 1397 */
		-1,		/* 1398 */
		4,		/* 1399 */
		-1,		/* 1400 */
		-1,		/* 1401 */
		-1,		/* 1402 */
		-1,		/* 1403 */
		-1,		/* 1404 */
		-1,		/* 1405 */
		-1,		/* 1406 */
		-1,		/* 1407 */
		-1,		/* 1408 */
		-1,		/* 1409 */
		-1,		/* 1410 */
		-1,		/* 1411 */
		-1,		/* 1412 */
		-1,		/* 1413 */
		-1,		/* 1414 */
		-1,		/* 1415 */
		-1,		/* 1416 */
		-1,		/* 1417 */
		-1,		/* 1418 */
		-1,		/* 1419 */
		-1,		/* 1420 */
		-1,		/* 1421 */
		-1,		/* 1422 */
		-1,		/* 1423 */
		-1,		/* 1424 */
		-1,		/* 1425 */
		-1,		/* 1426 */
		-1,		/* 1427 */
		-1,		/* 1428 */
		-1,		/* 1429 */
		-1,		/* 1430 */
		-1,		/* 1431 */
		-1,		/* 1432 */
		-1,		/* 1433 */
		-1,		/* 1434 */
		-1,		/* 1435 */
		-1,		/* 1436 */
		-1,		/* 1437 */
		-1,		/* 1438 */
		9,		/* 1439 */
		-1,		/* 1440 */
		-1,		/* 1441 */
		-1,		/* 1442 */
		-1,		/* 1443 */
		-1,		/* 1444 */
		-1,		/* 1445 */
		-1,		/* 1446 */
		-1,		/* 1447 */
		-1,		/* 1448 */
		-1,		/* 1449 */
		-1,		/* 1450 */
		-1,		/* 1451 */
		-1,		/* 1452 */
		-1,		/* 1453 */
		-1,		/* 1454 */
		-1,		/* 1455 */
		-1,		/* 1456 */
		-1,		/* 1457 */
		-1,		/* 1458 */
		-1,		/* 1459 */
		-1,		/* 1460 */
		-1,		/* 1461 */
		-1,		/* 1462 */
		-1,		/* 1463 */
		-1,		/* 1464 */
		-1,		/* 1465 */
		-1,		/* 1466 */
		-1,		/* 1467 */
		-1,		/* 1468 */
		-1,		/* 1469 */
		-1,		/* 1470 */
		-1,		/* 1471 */
		-1,		/* 1472 */
		-1,		/* 1473 */
		-1,		/* 1474 */
		-1,		/* 1475 */
		-1,		/* 1476 */
		-1,		/* 1477 */
		-1,		/* 1478 */
		-1,		/* 1479 */
		-1,		/* 1480 */
		-1,		/* 1481 */
		-1,		/* 1482 */
		-1,		/* 1483 */
		-1,		/* 1484 */
		-1,		/* 1485 */
		-1,		/* 1486 */
		-1,		/* 1487 */
		-1,		/* 1488 */
		-1,		/* 1489 */
		-1,		/* 1490 */
		-1,		/* 1491 */
		-1,		/* 1492 */
		-1,		/* 1493 */
		-1,		/* 1494 */
		-1,		/* 1495 */
		-1,		/* 1496 */
		-1,		/* 1497 */
		-1,		/* 1498 */
		-1,		/* 1499 */
		-1,		/* 1500 */
		-1,		/* 1501 */
		-1,		/* 1502 */
		-1,		/* 1503 */
		-1,		/* 1504 */
		-1,		/* 1505 */
		-1,		/* 1506 */
		-1,		/* 1507 */
		-1,		/* 1508 */
		-1,		/* 1509 */
		-1,		/* 1510 */
		-1,		/* 1511 */
		-1,		/* 1512 */
		-1,		/* 1513 */
		-1,		/* 1514 */
		-1,		/* 1515 */
		-1,		/* 1516 */
		-1,		/* 1517 */
		-1,		/* 1518 */
		-1,		/* 1519 */
		-1,		/* 1520 */
		-1,		/* 1521 */
		-1,		/* 1522 */
		-1,		/* 1523 */
		-1,		/* 1524 */
		-1,		/* 1525 */
		-1,		/* 1526 */
		-1,		/* 1527 */
		-1,		/* 1528 */
		-1,		/* 1529 */
		-1,		/* 1530 */
		-1,		/* 1531 */
		-1,		/* 1532 */
		-1,		/* 1533 */
		-1,		/* 1534 */
		-1,		/* 1535 */
		-1,		/* 1536 */
		-1,		/* 1537 */
		-1,		/* 1538 */
		-1,		/* 1539 */
		-1,		/* 1540 */
		-1,		/* 1541 */
		-1,		/* 1542 */
		-1,		/* 1543 */
		-1,		/* 1544 */
		-1,		/* 1545 */
		-1,		/* 1546 */
		-1,		/* 1547 */
		-1,		/* 1548 */
		-1,		/* 1549 */
		-1,		/* 1550 */
		-1,		/* 1551 */
		-1,		/* 1552 */
		-1,		/* 1553 */
		-1,		/* 1554 */
		-1,		/* 1555 */
		-1,		/* 1556 */
		-1,		/* 1557 */
		-1,		/* 1558 */
		-1,		/* 1559 */
		-1,		/* 1560 */
		-1,		/* 1561 */
		-1,		/* 1562 */
		-1,		/* 1563 */
		-1,		/* 1564 */
		-1,		/* 1565 */
		-1,		/* 1566 */
		-1,		/* 1567 */
		-1,		/* 1568 */
		-1,		/* 1569 */
		-1,		/* 1570 */
		-1,		/* 1571 */
		-1,		/* 1572 */
		-1,		/* 1573 */
		-1,		/* 1574 */
		-1,		/* 1575 */
		-1,		/* 1576 */
		-1,		/* 1577 */
		-1,		/* 1578 */
		-1,		/* 1579 */
		-1,		/* 1580 */
		-1,		/* 1581 */
		-1,		/* 1582 */
		-1,		/* 1583 */
		-1,		/* 1584 */
		-1,		/* 1585 */
		-1,		/* 1586 */
		-1,		/* 1587 */
		-1,		/* 1588 */
		-1,		/* 1589 */
		-1,		/* 1590 */
		-1,		/* 1591 */
		-1,		/* 1592 */
		-1,		/* 1593 */
		-1,		/* 1594 */
		-1,		/* 1595 */
		-1,		/* 1596 */
		-1,		/* 1597 */
		-1,		/* 1598 */
		-1,		/* 1599 */
		-1,		/* 1600 */
		-1,		/* 1601 */
		-1,		/* 1602 */
		-1,		/* 1603 */
		-1,		/* 1604 */
		-1,		/* 1605 */
		-1,		/* 1606 */
		-1,		/* 1607 */
		-1,		/* 1608 */
		-1,		/* 1609 */
		-1,		/* 1610 */
		-1,		/* 1611 */
		-1,		/* 1612 */
		-1,		/* 1613 */
		-1,		/* 1614 */
		-1,		/* 1615 */
		-1,		/* 1616 */
		-1,		/* 1617 */
		-1,		/* 1618 */
		-1,		/* 1619 */
		-1,		/* 1620 */
		-1,		/* 1621 */
		-1,		/* 1622 */
		-1,		/* 1623 */
		-1,		/* 1624 */
		-1,		/* 1625 */
		-1,		/* 1626 */
		-1,		/* 1627 */
		-1,		/* 1628 */
		-1,		/* 1629 */
		-1,		/* 1630 */
		-1,		/* 1631 */
		-1,		/* 1632 */
		-1,		/* 1633 */
		-1,		/* 1634 */
		-1,		/* 1635 */
		-1,		/* 1636 */
		-1,		/* 1637 */
		-1,		/* 1638 */
		-1,		/* 1639 */
		-1,		/* 1640 */
		-1,		/* 1641 */
		-1,		/* 1642 */
		-1,		/* 1643 */
		-1,		/* 1644 */
		-1,		/* 1645 */
		-1,		/* 1646 */
		-1,		/* 1647 */
		-1,		/* 1648 */
		-1,		/* 1649 */
		-1,		/* 1650 */
		-1,		/* 1651 */
		-1,		/* 1652 */
		-1,		/* 1653 */
		-1,		/* 1654 */
		-1,		/* 1655 */
		-1,		/* 1656 */
		-1,		/* 1657 */
		-1,		/* 1658 */
		-1,		/* 1659 */
		-1,		/* 1660 */
		-1,		/* 1661 */
		-1,		/* 1662 */
		-1,		/* 1663 */
		-1,		/* 1664 */
		-1,		/* 1665 */
		-1,		/* 1666 */
		-1,		/* 1667 */
		-1,		/* 1668 */
		-1,		/* 1669 */
		-1,		/* 1670 */
		-1,		/* 1671 */
		-1,		/* 1672 */
		-1,		/* 1673 */
		-1,		/* 1674 */
		-1,		/* 1675 */
		-1,		/* 1676 */
		-1,		/* 1677 */
		-1,		/* 1678 */
		-1,		/* 1679 */
		-1,		/* 1680 */
		-1,		/* 1681 */
		-1,		/* 1682 */
		-1,		/* 1683 */
		-1,		/* 1684 */
		-1,		/* 1685 */
		-1,		/* 1686 */
		-1,		/* 1687 */
		-1,		/* 1688 */
		-1,		/* 1689 */
		-1,		/* 1690 */
		-1,		/* 1691 */
		-1,		/* 1692 */
		-1,		/* 1693 */
		-1,		/* 1694 */
		-1,		/* 1695 */
		-1,		/* 1696 */
		-1,		/* 1697 */
		-1,		/* 1698 */
		-1,		/* 1699 */
		-1,		/* 1700 */
		-1,		/* 1701 */
		-1,		/* 1702 */
		-1,		/* 1703 */
		-1,		/* 1704 */
		-1,		/* 1705 */
		-1,		/* 1706 */
		-1,		/* 1707 */
		-1,		/* 1708 */
		-1,		/* 1709 */
		-1,		/* 1710 */
		-1,		/* 1711 */
		-1,		/* 1712 */
		-1,		/* 1713 */
		-1,		/* 1714 */
		-1,		/* 1715 */
		-1,		/* 1716 */
		-1,		/* 1717 */
		-1,		/* 1718 */
		-1,		/* 1719 */
		-1,		/* 1720 */
		-1,		/* 1721 */
		-1,		/* 1722 */
		-1,		/* 1723 */
		-1,		/* 1724 */
		-1,		/* 1725 */
		-1,		/* 1726 */
		-1,		/* 1727 */
		-1,		/* 1728 */
		-1,		/* 1729 */
		-1,		/* 1730 */
		-1,		/* 1731 */
		-1,		/* 1732 */
		-1,		/* 1733 */
		-1,		/* 1734 */
		-1,		/* 1735 */
		-1,		/* 1736 */
		-1,		/* 1737 */
		-1,		/* 1738 */
		-1,		/* 1739 */
		-1,		/* 1740 */
		-1,		/* 1741 */
		-1,		/* 1742 */
		-1,		/* 1743 */
		-1,		/* 1744 */
		-1,		/* 1745 */
		-1,		/* 1746 */
		-1,		/* 1747 */
		-1,		/* 1748 */
		-1,		/* 1749 */
		-1,		/* 1750 */
		-1,		/* 1751 */
		-1,		/* 1752 */
		-1,		/* 1753 */
		-1,		/* 1754 */
		-1,		/* 1755 */
		-1,		/* 1756 */
		-1,		/* 1757 */
		-1,		/* 1758 */
		-1,		/* 1759 */
		-1,		/* 1760 */
		-1,		/* 1761 */
		-1,		/* 1762 */
		-1,		/* 1763 */
		-1,		/* 1764 */
		-1,		/* 1765 */
		21,		/* 1766 */
		21,		/* 1767 */
		21,		/* 1768 */
		21,		/* 1769 */
		-1,		/* 1770 */
		-1,		/* 1771 */
		-1,		/* 1772 */
		-1,		/* 1773 */
		-1,		/* 1774 */
		-1,		/* 1775 */
		-1,		/* 1776 */
		-1,		/* 1777 */
		-1,		/* 1778 */
		-1,		/* 1779 */
		-1,		/* 1780 */
		-1,		/* 1781 */
		-1,		/* 1782 */
		-1,		/* 1783 */
		-1,		/* 1784 */
		22,		/* 1785 */
		22,		/* 1786 */
		-1,		/* 1787 */
		-1,		/* 1788 */
		-1,		/* 1789 */
		-1,		/* 1790 */
		-1,		/* 1791 */
		-1,		/* 1792 */
		-1,		/* 1793 */
		-1,		/* 1794 */
		-1,		/* 1795 */
		-1,		/* 1796 */
		-1,		/* 1797 */
		-1,		/* 1798 */
		-1,		/* 1799 */
		-1,		/* 1800 */
		-1,		/* 1801 */
		-1,		/* 1802 */
		-1,		/* 1803 */
		-1,		/* 1804 */
		-1,		/* 1805 */
		21,		/* 1806 */
		21,		/* 1807 */
		21,		/* 1808 */
		21,		/* 1809 */
		-1,		/* 1810 */
		-1,		/* 1811 */
		-1,		/* 1812 */
		-1,		/* 1813 */
		-1,		/* 1814 */
		-1,		/* 1815 */
		-1,		/* 1816 */
		-1,		/* 1817 */
		-1,		/* 1818 */
		-1,		/* 1819 */
		-1,		/* 1820 */
		-1,		/* 1821 */
		-1,		/* 1822 */
		-1,		/* 1823 */
		22,		/* 1824 */
		22,		/* 1825 */
		22,		/* 1826 */
		22,		/* 1827 */
		-1,		/* 1828 */
		-1,		/* 1829 */
		-1,		/* 1830 */
		-1,		/* 1831 */
		-1,		/* 1832 */
		-1,		/* 1833 */
		-1,		/* 1834 */
		-1,		/* 1835 */
		-1,		/* 1836 */
		-1,		/* 1837 */
		-1,		/* 1838 */
		-1,		/* 1839 */
		-1,		/* 1840 */
		-1,		/* 1841 */
		-1,		/* 1842 */
		-1,		/* 1843 */
		-1,		/* 1844 */
		-1,		/* 1845 */
		-1,		/* 1846 */
		-1,		/* 1847 */
		-1,		/* 1848 */
		-1,		/* 1849 */
		-1,		/* 1850 */
		-1,		/* 1851 */
		-1,		/* 1852 */
		-1,		/* 1853 */
		-1,		/* 1854 */
		-1,		/* 1855 */
		-1,		/* 1856 */
		-1,		/* 1857 */
		-1,		/* 1858 */
		-1,		/* 1859 */
		-1,		/* 1860 */
		-1,		/* 1861 */
		-1,		/* 1862 */
		-1,		/* 1863 */
		22,		/* 1864 */
		22,		/* 1865 */
		22,		/* 1866 */
		22,		/* 1867 */
		-1,		/* 1868 */
		-1,		/* 1869 */
		-1,		/* 1870 */
		-1,		/* 1871 */
		-1,		/* 1872 */
		-1,		/* 1873 */
		-1,		/* 1874 */
		-1,		/* 1875 */
		-1,		/* 1876 */
		-1,		/* 1877 */
		-1,		/* 1878 */
		-1,		/* 1879 */
		-1,		/* 1880 */
		-1,		/* 1881 */
		-1,		/* 1882 */
		-1,		/* 1883 */
		-1,		/* 1884 */
		-1,		/* 1885 */
		-1,		/* 1886 */
		-1,		/* 1887 */
		-1,		/* 1888 */
		-1,		/* 1889 */
		-1,		/* 1890 */
		-1,		/* 1891 */
		-1,		/* 1892 */
		-1,		/* 1893 */
		-1,		/* 1894 */
		-1,		/* 1895 */
		-1,		/* 1896 */
		-1,		/* 1897 */
		-1,		/* 1898 */
		-1,		/* 1899 */
		-1,		/* 1900 */
		23,		/* 1901 */
		23,		/* 1902 */
		23,		/* 1903 */
		23,		/* 1904 */
		22,		/* 1905 */
		22,		/* 1906 */
		-1,		/* 1907 */
		-1,		/* 1908 */
		-1,		/* 1909 */
		-1,		/* 1910 */
		-1,		/* 1911 */
		-1,		/* 1912 */
		-1,		/* 1913 */
		-1,		/* 1914 */
		-1,		/* 1915 */
		-1,		/* 1916 */
		-1,		/* 1917 */
		-1,		/* 1918 */
		-1,		/* 1919 */
		-1,		/* 1920 */
		-1,		/* 1921 */
		-1,		/* 1922 */
		-1,		/* 1923 */
		-1,		/* 1924 */
		-1,		/* 1925 */
		-1,		/* 1926 */
		-1,		/* 1927 */
		-1,		/* 1928 */
		-1,		/* 1929 */
		-1,		/* 1930 */
		-1,		/* 1931 */
		-1,		/* 1932 */
		-1,		/* 1933 */
		-1,		/* 1934 */
		-1,		/* 1935 */
		-1,		/* 1936 */
		-1,		/* 1937 */
		-1,		/* 1938 */
		-1,		/* 1939 */
		-1,		/* 1940 */
		23,		/* 1941 */
		23,		/* 1942 */
		23,		/* 1943 */
		23,		/* 1944 */
		24,		/* 1945 */
		24,		/* 1946 */
		24,		/* 1947 */
		24,		/* 1948 */
		24,		/* 1949 */
		24,		/* 1950 */
		24,		/* 1951 */
		24,		/* 1952 */
		-1,		/* 1953 */
		-1,		/* 1954 */
		-1,		/* 1955 */
		-1,		/* 1956 */
		-1,		/* 1957 */
		-1,		/* 1958 */
		-1,		/* 1959 */
		-1,		/* 1960 */
		-1,		/* 1961 */
		-1,		/* 1962 */
		-1,		/* 1963 */
		-1,		/* 1964 */
		-1,		/* 1965 */
		-1,		/* 1966 */
		-1,		/* 1967 */
		-1,		/* 1968 */
		-1,		/* 1969 */
		-1,		/* 1970 */
		-1,		/* 1971 */
		-1,		/* 1972 */
		-1,		/* 1973 */
		-1,		/* 1974 */
		-1,		/* 1975 */
		-1,		/* 1976 */
		-1,		/* 1977 */
		-1,		/* 1978 */
		-1,		/* 1979 */
		-1,		/* 1980 */
		23,		/* 1981 */
		23,		/* 1982 */
		23,		/* 1983 */
		23,		/* 1984 */
		24,		/* 1985 */
		24,		/* 1986 */
		24,		/* 1987 */
		24,		/* 1988 */
		24,		/* 1989 */
		24,		/* 1990 */
		24,		/* 1991 */
		24,		/* 1992 */
		-1,		/* 1993 */
		-1,		/* 1994 */
		-1,		/* 1995 */
		-1,		/* 1996 */
		-1,		/* 1997 */
		-1,		/* 1998 */
		-1,		/* 1999 */
		28,		/* 2000 */
		28,		/* 2001 */
		28,		/* 2002 */
		28,		/* 2003 */
		-1,		/* 2004 */
		-1,		/* 2005 */
		-1,		/* 2006 */
		-1,		/* 2007 */
		-1,		/* 2008 */
		-1,		/* 2009 */
		-1,		/* 2010 */
		-1,		/* 2011 */
		-1,		/* 2012 */
		-1,		/* 2013 */
		-1,		/* 2014 */
		-1,		/* 2015 */
		-1,		/* 2016 */
		-1,		/* 2017 */
		-1,		/* 2018 */
		-1,		/* 2019 */
		-1,		/* 2020 */
		23,		/* 2021 */
		23,		/* 2022 */
		23,		/* 2023 */
		23,		/* 2024 */
		25,		/* 2025 */
		25,		/* 2026 */
		25,		/* 2027 */
		25,		/* 2028 */
		25,		/* 2029 */
		25,		/* 2030 */
		25,		/* 2031 */
		25,		/* 2032 */
		-1,		/* 2033 */
		-1,		/* 2034 */
		-1,		/* 2035 */
		-1,		/* 2036 */
		-1,		/* 2037 */
		-1,		/* 2038 */
		-1,		/* 2039 */
		28,		/* 2040 */
		28,		/* 2041 */
		28,		/* 2042 */
		28,		/* 2043 */
		-1,		/* 2044 */
		-1,		/* 2045 */
		-1,		/* 2046 */
		-1,		/* 2047 */
		-1,		/* 2048 */
		-1,		/* 2049 */
		-1,		/* 2050 */
		-1,		/* 2051 */
		-1,		/* 2052 */
		-1,		/* 2053 */
		-1,		/* 2054 */
		-1,		/* 2055 */
		-1,		/* 2056 */
		23,		/* 2057 */
		23,		/* 2058 */
		-1,		/* 2059 */
		-1,		/* 2060 */
		-1,		/* 2061 */
		-1,		/* 2062 */
		-1,		/* 2063 */
		-1,		/* 2064 */
		25,		/* 2065 */
		25,		/* 2066 */
		25,		/* 2067 */
		25,		/* 2068 */
		25,		/* 2069 */
		25,		/* 2070 */
		25,		/* 2071 */
		25,		/* 2072 */
		-1,		/* 2073 */
		-1,		/* 2074 */
		-1,		/* 2075 */
		-1,		/* 2076 */
		-1,		/* 2077 */
		-1,		/* 2078 */
		-1,		/* 2079 */
		28,		/* 2080 */
		28,		/* 2081 */
		28,		/* 2082 */
		28,		/* 2083 */
		46,		/* 2084 */
		46,		/* 2085 */
		46,		/* 2086 */
		46,		/* 2087 */
		-1,		/* 2088 */
		-1,		/* 2089 */
		-1,		/* 2090 */
		-1,		/* 2091 */
		-1,		/* 2092 */
		-1,		/* 2093 */
		-1,		/* 2094 */
		23,		/* 2095 */
		23,		/* 2096 */
		23,		/* 2097 */
		23,		/* 2098 */
		-1,		/* 2099 */
		-1,		/* 2100 */
		-1,		/* 2101 */
		-1,		/* 2102 */
		-1,		/* 2103 */
		-1,		/* 2104 */
		-1,		/* 2105 */
		-1,		/* 2106 */
		-1,		/* 2107 */
		-1,		/* 2108 */
		26,		/* 2109 */
		26,		/* 2110 */
		26,		/* 2111 */
		26,		/* 2112 */
		-1,		/* 2113 */
		-1,		/* 2114 */
		-1,		/* 2115 */
		-1,		/* 2116 */
		-1,		/* 2117 */
		-1,		/* 2118 */
		-1,		/* 2119 */
		28,		/* 2120 */
		28,		/* 2121 */
		28,		/* 2122 */
		28,		/* 2123 */
		-1,		/* 2124 */
		-1,		/* 2125 */
		-1,		/* 2126 */
		46,		/* 2127 */
		-1,		/* 2128 */
		-1,		/* 2129 */
		-1,		/* 2130 */
		-1,		/* 2131 */
		-1,		/* 2132 */
		-1,		/* 2133 */
		-1,		/* 2134 */
		23,		/* 2135 */
		23,		/* 2136 */
		23,		/* 2137 */
		-1,		/* 2138 */
		-1,		/* 2139 */
		-1,		/* 2140 */
		-1,		/* 2141 */
		-1,		/* 2142 */
		-1,		/* 2143 */
		-1,		/* 2144 */
		-1,		/* 2145 */
		-1,		/* 2146 */
		-1,		/* 2147 */
		-1,		/* 2148 */
		26,		/* 2149 */
		26,		/* 2150 */
		26,		/* 2151 */
		26,		/* 2152 */
		-1,		/* 2153 */
		-1,		/* 2154 */
		-1,		/* 2155 */
		-1,		/* 2156 */
		-1,		/* 2157 */
		-1,		/* 2158 */
		-1,		/* 2159 */
		46,		/* 2160 */
		-1,		/* 2161 */
		-1,		/* 2162 */
		-1,		/* 2163 */
		-1,		/* 2164 */
		-1,		/* 2165 */
		-1,		/* 2166 */
		-1,		/* 2167 */
		-1,		/* 2168 */
		46,		/* 2169 */
		46,		/* 2170 */
		46,		/* 2171 */
		-1,		/* 2172 */
		-1,		/* 2173 */
		-1,		/* 2174 */
		-1,		/* 2175 */
		-1,		/* 2176 */
		-1,		/* 2177 */
		-1,		/* 2178 */
		-1,		/* 2179 */
		-1,		/* 2180 */
		-1,		/* 2181 */
		-1,		/* 2182 */
		27,		/* 2183 */
		27,		/* 2184 */
		27,		/* 2185 */
		27,		/* 2186 */
		-1,		/* 2187 */
		-1,		/* 2188 */
		26,		/* 2189 */
		26,		/* 2190 */
		26,		/* 2191 */
		26,		/* 2192 */
		-1,		/* 2193 */
		-1,		/* 2194 */
		-1,		/* 2195 */
		-1,		/* 2196 */
		-1,		/* 2197 */
		-1,		/* 2198 */
		-1,		/* 2199 */
		46,		/* 2200 */
		46,		/* 2201 */
		-1,		/* 2202 */
		-1,		/* 2203 */
		-1,		/* 2204 */
		-1,		/* 2205 */
		-1,		/* 2206 */
		-1,		/* 2207 */
		-1,		/* 2208 */
		-1,		/* 2209 */
		-1,		/* 2210 */
		-1,		/* 2211 */
		-1,		/* 2212 */
		-1,		/* 2213 */
		-1,		/* 2214 */
		-1,		/* 2215 */
		-1,		/* 2216 */
		-1,		/* 2217 */
		-1,		/* 2218 */
		-1,		/* 2219 */
		-1,		/* 2220 */
		-1,		/* 2221 */
		-1,		/* 2222 */
		27,		/* 2223 */
		27,		/* 2224 */
		27,		/* 2225 */
		27,		/* 2226 */
		-1,		/* 2227 */
		-1,		/* 2228 */
		26,		/* 2229 */
		26,		/* 2230 */
		26,		/* 2231 */
		26,		/* 2232 */
		-1,		/* 2233 */
		-1,		/* 2234 */
		-1,		/* 2235 */
		-1,		/* 2236 */
		-1,		/* 2237 */
		-1,		/* 2238 */
		-1,		/* 2239 */
		29,		/* 2240 */
		29,		/* 2241 */
		29,		/* 2242 */
		29,		/* 2243 */
		-1,		/* 2244 */
		-1,		/* 2245 */
		-1,		/* 2246 */
		-1,		/* 2247 */
		-1,		/* 2248 */
		-1,		/* 2249 */
		-1,		/* 2250 */
		-1,		/* 2251 */
		-1,		/* 2252 */
		-1,		/* 2253 */
		-1,		/* 2254 */
		-1,		/* 2255 */
		-1,		/* 2256 */
		-1,		/* 2257 */
		-1,		/* 2258 */
		-1,		/* 2259 */
		-1,		/* 2260 */
		-1,		/* 2261 */
		-1,		/* 2262 */
		27,		/* 2263 */
		27,		/* 2264 */
		27,		/* 2265 */
		27,		/* 2266 */
		-1,		/* 2267 */
		-1,		/* 2268 */
		-1,		/* 2269 */
		-1,		/* 2270 */
		-1,		/* 2271 */
		-1,		/* 2272 */
		-1,		/* 2273 */
		-1,		/* 2274 */
		-1,		/* 2275 */
		-1,		/* 2276 */
		-1,		/* 2277 */
		-1,		/* 2278 */
		-1,		/* 2279 */
		29,		/* 2280 */
		29,		/* 2281 */
		29,		/* 2282 */
		29,		/* 2283 */
		-1,		/* 2284 */
		-1,		/* 2285 */
		-1,		/* 2286 */
		46,		/* 2287 */
		46,		/* 2288 */
		-1,		/* 2289 */
		-1,		/* 2290 */
		-1,		/* 2291 */
		-1,		/* 2292 */
		-1,		/* 2293 */
		-1,		/* 2294 */
		-1,		/* 2295 */
		-1,		/* 2296 */
		-1,		/* 2297 */
		-1,		/* 2298 */
		-1,		/* 2299 */
		-1,		/* 2300 */
		-1,		/* 2301 */
		-1,		/* 2302 */
		27,		/* 2303 */
		27,		/* 2304 */
		27,		/* 2305 */
		27,		/* 2306 */
		-1,		/* 2307 */
		-1,		/* 2308 */
		-1,		/* 2309 */
		-1,		/* 2310 */
		-1,		/* 2311 */
		-1,		/* 2312 */
		-1,		/* 2313 */
		-1,		/* 2314 */
		-1,		/* 2315 */
		-1,		/* 2316 */
		-1,		/* 2317 */
		-1,		/* 2318 */
		-1,		/* 2319 */
		29,		/* 2320 */
		29,		/* 2321 */
		29,		/* 2322 */
		29,		/* 2323 */
		-1,		/* 2324 */
		-1,		/* 2325 */
		-1,		/* 2326 */
		46,		/* 2327 */
		46,		/* 2328 */
		-1,		/* 2329 */
		-1,		/* 2330 */
		-1,		/* 2331 */
		-1,		/* 2332 */
		-1,		/* 2333 */
		-1,		/* 2334 */
		-1,		/* 2335 */
		-1,		/* 2336 */
		-1,		/* 2337 */
		-1,		/* 2338 */
		-1,		/* 2339 */
		-1,		/* 2340 */
		-1,		/* 2341 */
		-1,		/* 2342 */
		-1,		/* 2343 */
		-1,		/* 2344 */
		-1,		/* 2345 */
		-1,		/* 2346 */
		-1,		/* 2347 */
		-1,		/* 2348 */
		-1,		/* 2349 */
		-1,		/* 2350 */
		-1,		/* 2351 */
		-1,		/* 2352 */
		-1,		/* 2353 */
		-1,		/* 2354 */
		-1,		/* 2355 */
		-1,		/* 2356 */
		-1,		/* 2357 */
		-1,		/* 2358 */
		-1,		/* 2359 */
		-1,		/* 2360 */
		29,		/* 2361 */
		29,		/* 2362 */
		29,		/* 2363 */
		-1,		/* 2364 */
		-1,		/* 2365 */
		-1,		/* 2366 */
		-1,		/* 2367 */
		-1,		/* 2368 */
		-1,		/* 2369 */
		-1,		/* 2370 */
		-1,		/* 2371 */
		-1,		/* 2372 */
		-1,		/* 2373 */
		-1,		/* 2374 */
		-1,		/* 2375 */
		-1,		/* 2376 */
		-1,		/* 2377 */
		-1,		/* 2378 */
		-1,		/* 2379 */
		-1,		/* 2380 */
		-1,		/* 2381 */
		-1,		/* 2382 */
		-1,		/* 2383 */
		-1,		/* 2384 */
		-1,		/* 2385 */
		-1,		/* 2386 */
		-1,		/* 2387 */
		-1,		/* 2388 */
		-1,		/* 2389 */
		-1,		/* 2390 */
		-1,		/* 2391 */
		-1,		/* 2392 */
		-1,		/* 2393 */
		-1,		/* 2394 */
		-1,		/* 2395 */
		-1,		/* 2396 */
		-1,		/* 2397 */
		-1,		/* 2398 */
		-1,		/* 2399 */
		-1,		/* 2400 */
		-1,		/* 2401 */
		-1,		/* 2402 */
		-1,		/* 2403 */
		-1,		/* 2404 */
		-1,		/* 2405 */
		-1,		/* 2406 */
		-1,		/* 2407 */
		-1,		/* 2408 */
		-1,		/* 2409 */
		-1,		/* 2410 */
		-1,		/* 2411 */
		-1,		/* 2412 */
		-1,		/* 2413 */
		-1,		/* 2414 */
		-1,		/* 2415 */
		-1,		/* 2416 */
		-1,		/* 2417 */
		-1,		/* 2418 */
		-1,		/* 2419 */
		-1,		/* 2420 */
		-1,		/* 2421 */
		-1,		/* 2422 */
		-1,		/* 2423 */
		-1,		/* 2424 */
		-1,		/* 2425 */
		-1,		/* 2426 */
		-1,		/* 2427 */
		-1,		/* 2428 */
		-1,		/* 2429 */
		-1,		/* 2430 */
		-1,		/* 2431 */
		-1,		/* 2432 */
		-1,		/* 2433 */
		-1,		/* 2434 */
		-1,		/* 2435 */
		-1,		/* 2436 */
		-1,		/* 2437 */
		-1,		/* 2438 */
		-1,		/* 2439 */
		-1,		/* 2440 */
		-1,		/* 2441 */
		-1,		/* 2442 */
		-1,		/* 2443 */
		-1,		/* 2444 */
		-1,		/* 2445 */
		-1,		/* 2446 */
		-1,		/* 2447 */
		-1,		/* 2448 */
		-1,		/* 2449 */
		-1,		/* 2450 */
		-1,		/* 2451 */
		-1,		/* 2452 */
		-1,		/* 2453 */
		-1,		/* 2454 */
		-1,		/* 2455 */
		-1,		/* 2456 */
		-1,		/* 2457 */
		-1,		/* 2458 */
		-1,		/* 2459 */
		-1,		/* 2460 */
		-1,		/* 2461 */
		-1,		/* 2462 */
		-1,		/* 2463 */
		-1,		/* 2464 */
		-1,		/* 2465 */
		-1,		/* 2466 */
		-1,		/* 2467 */
		-1,		/* 2468 */
		-1,		/* 2469 */
		-1,		/* 2470 */
		-1,		/* 2471 */
		-1,		/* 2472 */
		-1,		/* 2473 */
		-1,		/* 2474 */
		-1,		/* 2475 */
		-1,		/* 2476 */
		-1,		/* 2477 */
		-1,		/* 2478 */
		-1,		/* 2479 */
		-1,		/* 2480 */
		-1,		/* 2481 */
		-1,		/* 2482 */
		-1,		/* 2483 */
		-1,		/* 2484 */
		-1,		/* 2485 */
		-1,		/* 2486 */
		-1,		/* 2487 */
		-1,		/* 2488 */
		-1,		/* 2489 */
		-1,		/* 2490 */
		-1,		/* 2491 */
		-1,		/* 2492 */
		-1,		/* 2493 */
		-1,		/* 2494 */
		-1,		/* 2495 */
		-1,		/* 2496 */
		-1,		/* 2497 */
		-1,		/* 2498 */
		-1,		/* 2499 */
		-1,		/* 2500 */
		-1,		/* 2501 */
		-1,		/* 2502 */
		-1,		/* 2503 */
		-1,		/* 2504 */
		-1,		/* 2505 */
		-1,		/* 2506 */
		-1,		/* 2507 */
		-1,		/* 2508 */
		-1,		/* 2509 */
		-1,		/* 2510 */
		-1,		/* 2511 */
		-1,		/* 2512 */
		-1,		/* 2513 */
		-1,		/* 2514 */
		-1,		/* 2515 */
		-1,		/* 2516 */
		-1,		/* 2517 */
		-1,		/* 2518 */
		-1,		/* 2519 */
		-1,		/* 2520 */
		-1,		/* 2521 */
		-1,		/* 2522 */
		-1,		/* 2523 */
		-1,		/* 2524 */
		-1,		/* 2525 */
		-1,		/* 2526 */
		-1,		/* 2527 */
		-1,		/* 2528 */
		-1,		/* 2529 */
		-1,		/* 2530 */
		-1,		/* 2531 */
		-1,		/* 2532 */
		-1,		/* 2533 */
		-1,		/* 2534 */
		-1,		/* 2535 */
		-1,		/* 2536 */
		-1,		/* 2537 */
		-1,		/* 2538 */
		-1,		/* 2539 */
		-1,		/* 2540 */
		-1,		/* 2541 */
		-1,		/* 2542 */
		-1,		/* 2543 */
		-1,		/* 2544 */
		-1,		/* 2545 */
		-1,		/* 2546 */
		-1,		/* 2547 */
		-1,		/* 2548 */
		-1,		/* 2549 */
		-1,		/* 2550 */
		-1,		/* 2551 */
		-1,		/* 2552 */
		-1,		/* 2553 */
		-1,		/* 2554 */
		-1,		/* 2555 */
		-1,		/* 2556 */
		-1,		/* 2557 */
		-1,		/* 2558 */
		-1,		/* 2559 */
		-1,		/* 2560 */
		-1,		/* 2561 */
		-1,		/* 2562 */
		-1,		/* 2563 */
		-1,		/* 2564 */
		-1,		/* 2565 */
		-1,		/* 2566 */
		-1,		/* 2567 */
		-1,		/* 2568 */
		-1,		/* 2569 */
		-1,		/* 2570 */
		-1,		/* 2571 */
		-1,		/* 2572 */
		-1,		/* 2573 */
		-1,		/* 2574 */
		-1,		/* 2575 */
		-1,		/* 2576 */
		-1,		/* 2577 */
		-1,		/* 2578 */
		-1,		/* 2579 */
		-1,		/* 2580 */
		-1,		/* 2581 */
		-1,		/* 2582 */
		-1,		/* 2583 */
		-1,		/* 2584 */
		-1,		/* 2585 */
		-1,		/* 2586 */
		-1,		/* 2587 */
		-1,		/* 2588 */
		-1,		/* 2589 */
		-1,		/* 2590 */
		-1,		/* 2591 */
		-1,		/* 2592 */
		-1,		/* 2593 */
		-1,		/* 2594 */
		-1,		/* 2595 */
		-1,		/* 2596 */
		-1,		/* 2597 */
		-1,		/* 2598 */
		-1,		/* 2599 */
		-1,		/* 2600 */
		-1,		/* 2601 */
		-1,		/* 2602 */
		-1,		/* 2603 */
		30,		/* 2604 */
		30,		/* 2605 */
		30,		/* 2606 */
		-1,		/* 2607 */
		-1,		/* 2608 */
		-1,		/* 2609 */
		-1,		/* 2610 */
		-1,		/* 2611 */
		-1,		/* 2612 */
		-1,		/* 2613 */
		-1,		/* 2614 */
		-1,		/* 2615 */
		-1,		/* 2616 */
		-1,		/* 2617 */
		-1,		/* 2618 */
		-1,		/* 2619 */
		-1,		/* 2620 */
		-1,		/* 2621 */
		-1,		/* 2622 */
		-1,		/* 2623 */
		-1,		/* 2624 */
		-1,		/* 2625 */
		-1,		/* 2626 */
		-1,		/* 2627 */
		-1,		/* 2628 */
		-1,		/* 2629 */
		-1,		/* 2630 */
		-1,		/* 2631 */
		-1,		/* 2632 */
		-1,		/* 2633 */
		-1,		/* 2634 */
		-1,		/* 2635 */
		-1,		/* 2636 */
		-1,		/* 2637 */
		-1,		/* 2638 */
		-1,		/* 2639 */
		-1,		/* 2640 */
		-1,		/* 2641 */
		-1,		/* 2642 */
		-1,		/* 2643 */
		30,		/* 2644 */
		30,		/* 2645 */
		30,		/* 2646 */
		-1,		/* 2647 */
		-1,		/* 2648 */
		-1,		/* 2649 */
		-1,		/* 2650 */
		-1,		/* 2651 */
		-1,		/* 2652 */
		-1,		/* 2653 */
		-1,		/* 2654 */
		-1,		/* 2655 */
		-1,		/* 2656 */
		-1,		/* 2657 */
		-1,		/* 2658 */
		-1,		/* 2659 */
		-1,		/* 2660 */
		-1,		/* 2661 */
		-1,		/* 2662 */
		-1,		/* 2663 */
		-1,		/* 2664 */
		-1,		/* 2665 */
		-1,		/* 2666 */
		-1,		/* 2667 */
		-1,		/* 2668 */
		-1,		/* 2669 */
		-1,		/* 2670 */
		-1,		/* 2671 */
		-1,		/* 2672 */
		-1,		/* 2673 */
		-1,		/* 2674 */
		-1,		/* 2675 */
		-1,		/* 2676 */
		-1,		/* 2677 */
		-1,		/* 2678 */
		-1,		/* 2679 */
		31,		/* 2680 */
		31,		/* 2681 */
		31,		/* 2682 */
		31,		/* 2683 */
		30,		/* 2684 */
		30,		/* 2685 */
		30,		/* 2686 */
		-1,		/* 2687 */
		-1,		/* 2688 */
		-1,		/* 2689 */
		-1,		/* 2690 */
		-1,		/* 2691 */
		-1,		/* 2692 */
		-1,		/* 2693 */
		-1,		/* 2694 */
		-1,		/* 2695 */
		-1,		/* 2696 */
		-1,		/* 2697 */
		-1,		/* 2698 */
		-1,		/* 2699 */
		-1,		/* 2700 */
		-1,		/* 2701 */
		-1,		/* 2702 */
		-1,		/* 2703 */
		-1,		/* 2704 */
		-1,		/* 2705 */
		-1,		/* 2706 */
		-1,		/* 2707 */
		-1,		/* 2708 */
		-1,		/* 2709 */
		-1,		/* 2710 */
		-1,		/* 2711 */
		-1,		/* 2712 */
		-1,		/* 2713 */
		-1,		/* 2714 */
		-1,		/* 2715 */
		-1,		/* 2716 */
		-1,		/* 2717 */
		-1,		/* 2718 */
		-1,		/* 2719 */
		31,		/* 2720 */
		31,		/* 2721 */
		31,		/* 2722 */
		31,		/* 2723 */
		34,		/* 2724 */
		34,		/* 2725 */
		34,		/* 2726 */
		34,		/* 2727 */
		38,		/* 2728 */
		38,		/* 2729 */
		38,		/* 2730 */
		38,		/* 2731 */
		-1,		/* 2732 */
		-1,		/* 2733 */
		-1,		/* 2734 */
		-1,		/* 2735 */
		-1,		/* 2736 */
		-1,		/* 2737 */
		-1,		/* 2738 */
		-1,		/* 2739 */
		-1,		/* 2740 */
		-1,		/* 2741 */
		-1,		/* 2742 */
		-1,		/* 2743 */
		-1,		/* 2744 */
		-1,		/* 2745 */
		-1,		/* 2746 */
		-1,		/* 2747 */
		-1,		/* 2748 */
		-1,		/* 2749 */
		-1,		/* 2750 */
		-1,		/* 2751 */
		-1,		/* 2752 */
		-1,		/* 2753 */
		-1,		/* 2754 */
		-1,		/* 2755 */
		-1,		/* 2756 */
		-1,		/* 2757 */
		-1,		/* 2758 */
		-1,		/* 2759 */
		31,		/* 2760 */
		31,		/* 2761 */
		31,		/* 2762 */
		31,		/* 2763 */
		34,		/* 2764 */
		34,		/* 2765 */
		34,		/* 2766 */
		34,		/* 2767 */
		39,		/* 2768 */
		39,		/* 2769 */
		39,		/* 2770 */
		39,		/* 2771 */
		39,		/* 2772 */
		39,		/* 2773 */
		-1,		/* 2774 */
		-1,		/* 2775 */
		-1,		/* 2776 */
		-1,		/* 2777 */
		-1,		/* 2778 */
		-1,		/* 2779 */
		-1,		/* 2780 */
		-1,		/* 2781 */
		-1,		/* 2782 */
		-1,		/* 2783 */
		-1,		/* 2784 */
		-1,		/* 2785 */
		-1,		/* 2786 */
		-1,		/* 2787 */
		-1,		/* 2788 */
		-1,		/* 2789 */
		-1,		/* 2790 */
		-1,		/* 2791 */
		-1,		/* 2792 */
		-1,		/* 2793 */
		-1,		/* 2794 */
		-1,		/* 2795 */
		-1,		/* 2796 */
		-1,		/* 2797 */
		-1,		/* 2798 */
		-1,		/* 2799 */
		32,		/* 2800 */
		32,		/* 2801 */
		32,		/* 2802 */
		32,		/* 2803 */
		34,		/* 2804 */
		34,		/* 2805 */
		34,		/* 2806 */
		34,		/* 2807 */
		39,		/* 2808 */
		39,		/* 2809 */
		39,		/* 2810 */
		39,		/* 2811 */
		39,		/* 2812 */
		39,		/* 2813 */
		-1,		/* 2814 */
		-1,		/* 2815 */
		-1,		/* 2816 */
		-1,		/* 2817 */
		-1,		/* 2818 */
		-1,		/* 2819 */
		-1,		/* 2820 */
		-1,		/* 2821 */
		-1,		/* 2822 */
		-1,		/* 2823 */
		-1,		/* 2824 */
		-1,		/* 2825 */
		-1,		/* 2826 */
		-1,		/* 2827 */
		-1,		/* 2828 */
		-1,		/* 2829 */
		-1,		/* 2830 */
		-1,		/* 2831 */
		-1,		/* 2832 */
		-1,		/* 2833 */
		-1,		/* 2834 */
		-1,		/* 2835 */
		-1,		/* 2836 */
		-1,		/* 2837 */
		-1,		/* 2838 */
		-1,		/* 2839 */
		32,		/* 2840 */
		32,		/* 2841 */
		32,		/* 2842 */
		32,		/* 2843 */
		34,		/* 2844 */
		34,		/* 2845 */
		34,		/* 2846 */
		34,		/* 2847 */
		39,		/* 2848 */
		39,		/* 2849 */
		39,		/* 2850 */
		39,		/* 2851 */
		39,		/* 2852 */
		39,		/* 2853 */
		-1,		/* 2854 */
		-1,		/* 2855 */
		-1,		/* 2856 */
		-1,		/* 2857 */
		-1,		/* 2858 */
		-1,		/* 2859 */
		-1,		/* 2860 */
		-1,		/* 2861 */
		-1,		/* 2862 */
		-1,		/* 2863 */
		-1,		/* 2864 */
		-1,		/* 2865 */
		-1,		/* 2866 */
		-1,		/* 2867 */
		-1,		/* 2868 */
		-1,		/* 2869 */
		-1,		/* 2870 */
		-1,		/* 2871 */
		-1,		/* 2872 */
		-1,		/* 2873 */
		-1,		/* 2874 */
		-1,		/* 2875 */
		-1,		/* 2876 */
		-1,		/* 2877 */
		-1,		/* 2878 */
		-1,		/* 2879 */
		32,		/* 2880 */
		32,		/* 2881 */
		32,		/* 2882 */
		32,		/* 2883 */
		35,		/* 2884 */
		36,		/* 2885 */
		36,		/* 2886 */
		36,		/* 2887 */
		36,		/* 2888 */
		40,		/* 2889 */
		40,		/* 2890 */
		40,		/* 2891 */
		41,		/* 2892 */
		41,		/* 2893 */
		-1,		/* 2894 */
		-1,		/* 2895 */
		-1,		/* 2896 */
		-1,		/* 2897 */
		-1,		/* 2898 */
		-1,		/* 2899 */
		-1,		/* 2900 */
		-1,		/* 2901 */
		-1,		/* 2902 */
		-1,		/* 2903 */
		-1,		/* 2904 */
		-1,		/* 2905 */
		-1,		/* 2906 */
		-1,		/* 2907 */
		-1,		/* 2908 */
		-1,		/* 2909 */
		-1,		/* 2910 */
		-1,		/* 2911 */
		-1,		/* 2912 */
		-1,		/* 2913 */
		-1,		/* 2914 */
		-1,		/* 2915 */
		-1,		/* 2916 */
		-1,		/* 2917 */
		-1,		/* 2918 */
		-1,		/* 2919 */
		33,		/* 2920 */
		33,		/* 2921 */
		33,		/* 2922 */
		33,		/* 2923 */
		35,		/* 2924 */
		36,		/* 2925 */
		36,		/* 2926 */
		36,		/* 2927 */
		36,		/* 2928 */
		42,		/* 2929 */
		42,		/* 2930 */
		42,		/* 2931 */
		42,		/* 2932 */
		42,		/* 2933 */
		42,		/* 2934 */
		-1,		/* 2935 */
		-1,		/* 2936 */
		-1,		/* 2937 */
		-1,		/* 2938 */
		-1,		/* 2939 */
		-1,		/* 2940 */
		-1,		/* 2941 */
		-1,		/* 2942 */
		-1,		/* 2943 */
		-1,		/* 2944 */
		-1,		/* 2945 */
		-1,		/* 2946 */
		-1,		/* 2947 */
		-1,		/* 2948 */
		-1,		/* 2949 */
		-1,		/* 2950 */
		-1,		/* 2951 */
		-1,		/* 2952 */
		-1,		/* 2953 */
		-1,		/* 2954 */
		-1,		/* 2955 */
		-1,		/* 2956 */
		-1,		/* 2957 */
		-1,		/* 2958 */
		-1,		/* 2959 */
		33,		/* 2960 */
		33,		/* 2961 */
		33,		/* 2962 */
		33,		/* 2963 */
		35,		/* 2964 */
		36,		/* 2965 */
		36,		/* 2966 */
		36,		/* 2967 */
		36,		/* 2968 */
		42,		/* 2969 */
		42,		/* 2970 */
		42,		/* 2971 */
		42,		/* 2972 */
		42,		/* 2973 */
		42,		/* 2974 */
		-1,		/* 2975 */
		-1,		/* 2976 */
		-1,		/* 2977 */
		-1,		/* 2978 */
		-1,		/* 2979 */
		-1,		/* 2980 */
		-1,		/* 2981 */
		-1,		/* 2982 */
		-1,		/* 2983 */
		-1,		/* 2984 */
		-1,		/* 2985 */
		-1,		/* 2986 */
		-1,		/* 2987 */
		-1,		/* 2988 */
		-1,		/* 2989 */
		-1,		/* 2990 */
		-1,		/* 2991 */
		-1,		/* 2992 */
		-1,		/* 2993 */
		-1,		/* 2994 */
		-1,		/* 2995 */
		-1,		/* 2996 */
		-1,		/* 2997 */
		-1,		/* 2998 */
		-1,		/* 2999 */
		33,		/* 3000 */
		33,		/* 3001 */
		33,		/* 3002 */
		33,		/* 3003 */
		35,		/* 3004 */
		36,		/* 3005 */
		36,		/* 3006 */
		36,		/* 3007 */
		36,		/* 3008 */
		42,		/* 3009 */
		42,		/* 3010 */
		42,		/* 3011 */
		42,		/* 3012 */
		42,		/* 3013 */
		42,		/* 3014 */
		-1,		/* 3015 */
		-1,		/* 3016 */
		-1,		/* 3017 */
		-1,		/* 3018 */
		-1,		/* 3019 */
		-1,		/* 3020 */
		-1,		/* 3021 */
		-1,		/* 3022 */
		-1,		/* 3023 */
		-1,		/* 3024 */
		-1,		/* 3025 */
		-1,		/* 3026 */
		-1,		/* 3027 */
		-1,		/* 3028 */
		-1,		/* 3029 */
		-1,		/* 3030 */
		-1,		/* 3031 */
		-1,		/* 3032 */
		-1,		/* 3033 */
		-1,		/* 3034 */
		-1,		/* 3035 */
		-1,		/* 3036 */
		-1,		/* 3037 */
		-1,		/* 3038 */
		-1,		/* 3039 */
		43,		/* 3040 */
		43,		/* 3041 */
		43,		/* 3042 */
		43,		/* 3043 */
		43,		/* 3044 */
		37,		/* 3045 */
		37,		/* 3046 */
		37,		/* 3047 */
		37,		/* 3048 */
		42,		/* 3049 */
		42,		/* 3050 */
		42,		/* 3051 */
		42,		/* 3052 */
		42,		/* 3053 */
		42,		/* 3054 */
		-1,		/* 3055 */
		-1,		/* 3056 */
		-1,		/* 3057 */
		-1,		/* 3058 */
		-1,		/* 3059 */
		-1,		/* 3060 */
		-1,		/* 3061 */
		-1,		/* 3062 */
		-1,		/* 3063 */
		-1,		/* 3064 */
		-1,		/* 3065 */
		-1,		/* 3066 */
		-1,		/* 3067 */
		-1,		/* 3068 */
		-1,		/* 3069 */
		-1,		/* 3070 */
		-1,		/* 3071 */
		-1,		/* 3072 */
		-1,		/* 3073 */
		-1,		/* 3074 */
		-1,		/* 3075 */
		-1,		/* 3076 */
		-1,		/* 3077 */
		-1,		/* 3078 */
		-1,		/* 3079 */
		43,		/* 3080 */
		43,		/* 3081 */
		43,		/* 3082 */
		43,		/* 3083 */
		43,		/* 3084 */
		44,		/* 3085 */
		44,		/* 3086 */
		44,		/* 3087 */
		44,		/* 3088 */
		44,		/* 3089 */
		44,		/* 3090 */
		44,		/* 3091 */
		44,		/* 3092 */
		-1,		/* 3093 */
		-1,		/* 3094 */
		-1,		/* 3095 */
		-1,		/* 3096 */
		-1,		/* 3097 */
		-1,		/* 3098 */
		-1,		/* 3099 */
		-1,		/* 3100 */
		-1,		/* 3101 */
		-1,		/* 3102 */
		-1,		/* 3103 */
		-1,		/* 3104 */
		-1,		/* 3105 */
		-1,		/* 3106 */
		-1,		/* 3107 */
		-1,		/* 3108 */
		-1,		/* 3109 */
		-1,		/* 3110 */
		-1,		/* 3111 */
		-1,		/* 3112 */
		-1,		/* 3113 */
		-1,		/* 3114 */
		-1,		/* 3115 */
		-1,		/* 3116 */
		-1,		/* 3117 */
		-1,		/* 3118 */
		-1,		/* 3119 */
		43,		/* 3120 */
		43,		/* 3121 */
		43,		/* 3122 */
		43,		/* 3123 */
		43,		/* 3124 */
		44,		/* 3125 */
		44,		/* 3126 */
		44,		/* 3127 */
		44,		/* 3128 */
		44,		/* 3129 */
		44,		/* 3130 */
		44,		/* 3131 */
		44,		/* 3132 */
		-1,		/* 3133 */
		-1,		/* 3134 */
		-1,		/* 3135 */
		-1,		/* 3136 */
		-1,		/* 3137 */
		-1,		/* 3138 */
		-1,		/* 3139 */
		-1,		/* 3140 */
		-1,		/* 3141 */
		-1,		/* 3142 */
		-1,		/* 3143 */
		-1,		/* 3144 */
		-1,		/* 3145 */
		-1,		/* 3146 */
		-1,		/* 3147 */
		-1,		/* 3148 */
		-1,		/* 3149 */
		-1,		/* 3150 */
		-1,		/* 3151 */
		-1,		/* 3152 */
		-1,		/* 3153 */
		-1,		/* 3154 */
		-1,		/* 3155 */
		-1,		/* 3156 */
		-1,		/* 3157 */
		-1,		/* 3158 */
		-1,		/* 3159 */
		43,		/* 3160 */
		43,		/* 3161 */
		43,		/* 3162 */
		43,		/* 3163 */
		43,		/* 3164 */
		44,		/* 3165 */
		44,		/* 3166 */
		44,		/* 3167 */
		44,		/* 3168 */
		44,		/* 3169 */
		44,		/* 3170 */
		44,		/* 3171 */
		44,		/* 3172 */
		-1,		/* 3173 */
		-1,		/* 3174 */
		-1,		/* 3175 */
		-1,		/* 3176 */
		-1,		/* 3177 */
		-1,		/* 3178 */
		-1,		/* 3179 */
		-1,		/* 3180 */
		-1,		/* 3181 */
		-1,		/* 3182 */
		-1,		/* 3183 */
		-1,		/* 3184 */
		-1,		/* 3185 */
		-1,		/* 3186 */
		-1,		/* 3187 */
		-1,		/* 3188 */
		-1,		/* 3189 */
		-1,		/* 3190 */
		-1,		/* 3191 */
		-1,		/* 3192 */
		-1,		/* 3193 */
		-1,		/* 3194 */
		-1,		/* 3195 */
		-1,		/* 3196 */
		-1,		/* 3197 */
		-1,		/* 3198 */
		-1,		/* 3199 */
		45,		/* 3200 */
		45,		/* 3201 */
		45,		/* 3202 */
		45,		/* 3203 */
		-1,		/* 3204 */
		44,		/* 3205 */
		44,		/* 3206 */
		44,		/* 3207 */
		44,		/* 3208 */
		44,		/* 3209 */
		44,		/* 3210 */
		44,		/* 3211 */
		44,		/* 3212 */
		-1,		/* 3213 */
		-1,		/* 3214 */
		-1,		/* 3215 */
		-1,		/* 3216 */
		-1,		/* 3217 */
		-1,		/* 3218 */
		-1,		/* 3219 */
		-1,		/* 3220 */
		-1,		/* 3221 */
		-1,		/* 3222 */
		-1,		/* 3223 */
		-1,		/* 3224 */
		-1,		/* 3225 */
		-1,		/* 3226 */
		-1,		/* 3227 */
		-1,		/* 3228 */
		-1,		/* 3229 */
		-1,		/* 3230 */
		-1,		/* 3231 */
		-1,		/* 3232 */
		-1,		/* 3233 */
		-1,		/* 3234 */
		-1,		/* 3235 */
		-1,		/* 3236 */
		-1,		/* 3237 */
		-1,		/* 3238 */
		-1,		/* 3239 */
		45,		/* 3240 */
		45,		/* 3241 */
		45,		/* 3242 */
		45,		/* 3243 */
		-1,		/* 3244 */
		43,		/* 3245 */
		43,		/* 3246 */
		43,		/* 3247 */
		43,		/* 3248 */
		43,		/* 3249 */
		-1,		/* 3250 */
		-1,		/* 3251 */
		-1,		/* 3252 */
		-1,		/* 3253 */
		-1,		/* 3254 */
		-1,		/* 3255 */
		-1,		/* 3256 */
		-1,		/* 3257 */
		-1,		/* 3258 */
		-1,		/* 3259 */
		-1,		/* 3260 */
		-1,		/* 3261 */
		-1,		/* 3262 */
		-1,		/* 3263 */
		-1,		/* 3264 */
		-1,		/* 3265 */
		-1,		/* 3266 */
		-1,		/* 3267 */
		-1,		/* 3268 */
		-1,		/* 3269 */
		-1,		/* 3270 */
		-1,		/* 3271 */
		-1,		/* 3272 */
		-1,		/* 3273 */
		-1,		/* 3274 */
		-1,		/* 3275 */
		-1,		/* 3276 */
		-1,		/* 3277 */
		-1,		/* 3278 */
		-1,		/* 3279 */
		45,		/* 3280 */
		45,		/* 3281 */
		45,		/* 3282 */
		45,		/* 3283 */
		-1,		/* 3284 */
		-1,		/* 3285 */
		-1,		/* 3286 */
		-1,		/* 3287 */
		-1,		/* 3288 */
		-1,		/* 3289 */
		-1,		/* 3290 */
		-1,		/* 3291 */
		-1,		/* 3292 */
		-1,		/* 3293 */
		-1,		/* 3294 */
		-1,		/* 3295 */
		-1,		/* 3296 */
		-1,		/* 3297 */
		-1,		/* 3298 */
		-1,		/* 3299 */
		-1,		/* 3300 */
		-1,		/* 3301 */
		-1,		/* 3302 */
		-1,		/* 3303 */
		-1,		/* 3304 */
		-1,		/* 3305 */
		-1,		/* 3306 */
		-1,		/* 3307 */
		-1,		/* 3308 */
		-1,		/* 3309 */
		-1,		/* 3310 */
		-1,		/* 3311 */
		-1,		/* 3312 */
		-1,		/* 3313 */
		-1,		/* 3314 */
		-1,		/* 3315 */
		-1,		/* 3316 */
		-1,		/* 3317 */
		-1,		/* 3318 */
		-1,		/* 3319 */
		45,		/* 3320 */
		45,		/* 3321 */
		45,		/* 3322 */
		45,		/* 3323 */
		-1,		/* 3324 */
		-1,		/* 3325 */
		-1,		/* 3326 */
		-1,		/* 3327 */
		-1,		/* 3328 */
		-1,		/* 3329 */
		-1,		/* 3330 */
		-1,		/* 3331 */
		-1,		/* 3332 */
		-1,		/* 3333 */
		-1,		/* 3334 */
		-1,		/* 3335 */
		-1,		/* 3336 */
		-1,		/* 3337 */
		-1,		/* 3338 */
		-1,		/* 3339 */
		-1,		/* 3340 */
		-1,		/* 3341 */
		-1,		/* 3342 */
		-1,		/* 3343 */
		-1,		/* 3344 */
		-1,		/* 3345 */
		-1,		/* 3346 */
		-1,		/* 3347 */
		-1,		/* 3348 */
		-1,		/* 3349 */
		-1,		/* 3350 */
		-1,		/* 3351 */
		-1,		/* 3352 */
		-1,		/* 3353 */
		-1,		/* 3354 */
		-1,		/* 3355 */
		-1,		/* 3356 */
		-1,		/* 3357 */
		-1,		/* 3358 */
		-1,		/* 3359 */
		-1,		/* 3360 */
		-1,		/* 3361 */
		-1,		/* 3362 */
		-1,		/* 3363 */
		-1,		/* 3364 */
		-1,		/* 3365 */
		-1,		/* 3366 */
		-1,		/* 3367 */
		-1,		/* 3368 */
		-1,		/* 3369 */
		-1,		/* 3370 */
		-1,		/* 3371 */
		-1,		/* 3372 */
		-1,		/* 3373 */
		-1,		/* 3374 */
		-1,		/* 3375 */
		-1,		/* 3376 */
		-1,		/* 3377 */
		-1,		/* 3378 */
		-1,		/* 3379 */
		-1,		/* 3380 */
		-1,		/* 3381 */
		-1,		/* 3382 */
		-1,		/* 3383 */
		-1,		/* 3384 */
		-1,		/* 3385 */
		-1,		/* 3386 */
		-1,		/* 3387 */
		-1,		/* 3388 */
		-1,		/* 3389 */
		-1,		/* 3390 */
		-1,		/* 3391 */
		-1,		/* 3392 */
		-1,		/* 3393 */
		-1,		/* 3394 */
		-1,		/* 3395 */
		-1,		/* 3396 */
		-1,		/* 3397 */
		-1,		/* 3398 */
		-1,		/* 3399 */
		-1,		/* 3400 */
		-1,		/* 3401 */
		-1,		/* 3402 */
		-1,		/* 3403 */
		-1,		/* 3404 */
		-1,		/* 3405 */
		-1,		/* 3406 */
		-1,		/* 3407 */
		-1,		/* 3408 */
		-1,		/* 3409 */
		-1,		/* 3410 */
		-1,		/* 3411 */
		-1,		/* 3412 */
		-1,		/* 3413 */
		-1,		/* 3414 */
		-1,		/* 3415 */
		-1,		/* 3416 */
		-1,		/* 3417 */
		-1,		/* 3418 */
		-1,		/* 3419 */
		-1,		/* 3420 */
		-1,		/* 3421 */
		-1,		/* 3422 */
		-1,		/* 3423 */
		-1,		/* 3424 */
		-1,		/* 3425 */
		-1,		/* 3426 */
		-1,		/* 3427 */
		-1,		/* 3428 */
		-1,		/* 3429 */
		-1,		/* 3430 */
		-1,		/* 3431 */
		-1,		/* 3432 */
		-1,		/* 3433 */
		-1,		/* 3434 */
		-1,		/* 3435 */
		-1,		/* 3436 */
		-1,		/* 3437 */
		-1,		/* 3438 */
		-1,		/* 3439 */
		-1,		/* 3440 */
		-1,		/* 3441 */
		-1,		/* 3442 */
		-1,		/* 3443 */
		-1,		/* 3444 */
		-1,		/* 3445 */
		-1,		/* 3446 */
		-1,		/* 3447 */
		-1,		/* 3448 */
		-1,		/* 3449 */
		-1,		/* 3450 */
		-1,		/* 3451 */
		-1,		/* 3452 */
		-1,		/* 3453 */
		-1,		/* 3454 */
		-1,		/* 3455 */
		-1,		/* 3456 */
		-1,		/* 3457 */
		-1,		/* 3458 */
		-1,		/* 3459 */
		-1,		/* 3460 */
		-1,		/* 3461 */
		-1,		/* 3462 */
		-1,		/* 3463 */
		-1,		/* 3464 */
		-1,		/* 3465 */
		-1,		/* 3466 */
		-1,		/* 3467 */
		-1,		/* 3468 */
		-1,		/* 3469 */
		-1,		/* 3470 */
		-1,		/* 3471 */
		-1,		/* 3472 */
		-1,		/* 3473 */
		-1,		/* 3474 */
		-1,		/* 3475 */
		-1,		/* 3476 */
		-1,		/* 3477 */
		-1,		/* 3478 */
		-1,		/* 3479 */
		-1,		/* 3480 */
		-1,		/* 3481 */
		-1,		/* 3482 */
		-1,		/* 3483 */
		-1,		/* 3484 */
		-1,		/* 3485 */
		-1,		/* 3486 */
		-1,		/* 3487 */
		-1,		/* 3488 */
		-1,		/* 3489 */
		-1,		/* 3490 */
		-1,		/* 3491 */
		-1,		/* 3492 */
		-1,		/* 3493 */
		-1,		/* 3494 */
		-1,		/* 3495 */
		-1,		/* 3496 */
		-1,		/* 3497 */
		-1,		/* 3498 */
		-1,		/* 3499 */
		-1,		/* 3500 */
		-1,		/* 3501 */
		-1,		/* 3502 */
		-1,		/* 3503 */
		-1,		/* 3504 */
		-1,		/* 3505 */
		-1,		/* 3506 */
		-1,		/* 3507 */
		-1,		/* 3508 */
		-1,		/* 3509 */
		-1,		/* 3510 */
		-1,		/* 3511 */
		-1,		/* 3512 */
		-1,		/* 3513 */
		-1,		/* 3514 */
		-1,		/* 3515 */
		-1,		/* 3516 */
		-1,		/* 3517 */
		-1,		/* 3518 */
		-1,		/* 3519 */
		-1,		/* 3520 */
		-1,		/* 3521 */
		-1,		/* 3522 */
		-1,		/* 3523 */
		-1,		/* 3524 */
		-1,		/* 3525 */
		-1,		/* 3526 */
		-1,		/* 3527 */
		-1,		/* 3528 */
		-1,		/* 3529 */
		-1,		/* 3530 */
		-1,		/* 3531 */
		-1,		/* 3532 */
		-1,		/* 3533 */
		-1,		/* 3534 */
		-1,		/* 3535 */
		-1,		/* 3536 */
		-1,		/* 3537 */
		-1,		/* 3538 */
		-1,		/* 3539 */
		-1,		/* 3540 */
		-1,		/* 3541 */
		-1,		/* 3542 */
		-1,		/* 3543 */
		-1,		/* 3544 */
		-1,		/* 3545 */
		-1,		/* 3546 */
		-1,		/* 3547 */
		-1,		/* 3548 */
		-1,		/* 3549 */
		-1,		/* 3550 */
		-1,		/* 3551 */
		-1,		/* 3552 */
		-1,		/* 3553 */
		-1,		/* 3554 */
		-1,		/* 3555 */
		-1,		/* 3556 */
		-1,		/* 3557 */
		-1,		/* 3558 */
		-1,		/* 3559 */
		-1,		/* 3560 */
		-1,		/* 3561 */
		-1,		/* 3562 */
		-1,		/* 3563 */
		-1,		/* 3564 */
		-1,		/* 3565 */
		-1,		/* 3566 */
		-1,		/* 3567 */
		-1,		/* 3568 */
		-1,		/* 3569 */
		-1,		/* 3570 */
		-1,		/* 3571 */
		-1,		/* 3572 */
		-1,		/* 3573 */
		-1,		/* 3574 */
		-1,		/* 3575 */
		-1,		/* 3576 */
		-1,		/* 3577 */
		-1,		/* 3578 */
		-1,		/* 3579 */
		-1,		/* 3580 */
		-1,		/* 3581 */
		-1,		/* 3582 */
		-1,		/* 3583 */
		-1,		/* 3584 */
		-1,		/* 3585 */
		-1,		/* 3586 */
		-1,		/* 3587 */
		-1,		/* 3588 */
		-1,		/* 3589 */
		-1,		/* 3590 */
		-1,		/* 3591 */
		-1,		/* 3592 */
		-1,		/* 3593 */
		-1,		/* 3594 */
		-1,		/* 3595 */
		-1,		/* 3596 */
		-1,		/* 3597 */
		-1,		/* 3598 */
		-1,		/* 3599 */
		-1,		/* 3600 */
		-1,		/* 3601 */
		-1,		/* 3602 */
		-1,		/* 3603 */
		-1,		/* 3604 */
		-1,		/* 3605 */
		-1,		/* 3606 */
		-1,		/* 3607 */
		-1,		/* 3608 */
		-1,		/* 3609 */
		-1,		/* 3610 */
		-1,		/* 3611 */
		-1,		/* 3612 */
		-1,		/* 3613 */
		-1,		/* 3614 */
		-1,		/* 3615 */
		-1,		/* 3616 */
		-1,		/* 3617 */
		-1,		/* 3618 */
		-1,		/* 3619 */
		-1,		/* 3620 */
		-1,		/* 3621 */
		-1,		/* 3622 */
		-1,		/* 3623 */
		-1,		/* 3624 */
		-1,		/* 3625 */
		-1,		/* 3626 */
		-1,		/* 3627 */
		-1,		/* 3628 */
		-1,		/* 3629 */
		-1,		/* 3630 */
		-1,		/* 3631 */
		-1,		/* 3632 */
		-1,		/* 3633 */
		-1,		/* 3634 */
		-1,		/* 3635 */
		-1,		/* 3636 */
		-1,		/* 3637 */
		-1,		/* 3638 */
		-1,		/* 3639 */
		-1,		/* 3640 */
		-1,		/* 3641 */
		-1,		/* 3642 */
		-1,		/* 3643 */
		-1,		/* 3644 */
		-1,		/* 3645 */
		-1,		/* 3646 */
		-1,		/* 3647 */
		-1,		/* 3648 */
		-1,		/* 3649 */
		-1,		/* 3650 */
		-1,		/* 3651 */
		-1,		/* 3652 */
		-1,		/* 3653 */
		-1,		/* 3654 */
		-1,		/* 3655 */
		-1,		/* 3656 */
		-1,		/* 3657 */
		-1,		/* 3658 */
		-1,		/* 3659 */
		-1,		/* 3660 */
		-1,		/* 3661 */
		-1,		/* 3662 */
		-1,		/* 3663 */
		-1,		/* 3664 */
		-1,		/* 3665 */
		-1,		/* 3666 */
		-1,		/* 3667 */
		-1,		/* 3668 */
		-1,		/* 3669 */
		-1,		/* 3670 */
		-1,		/* 3671 */
		-1,		/* 3672 */
		-1,		/* 3673 */
		-1,		/* 3674 */
		-1,		/* 3675 */
		-1,		/* 3676 */
		-1,		/* 3677 */
		-1,		/* 3678 */
		-1,		/* 3679 */
		-1,		/* 3680 */
		-1,		/* 3681 */
		-1,		/* 3682 */
		-1,		/* 3683 */
		-1,		/* 3684 */
		-1,		/* 3685 */
		-1,		/* 3686 */
		-1,		/* 3687 */
		-1,		/* 3688 */
		-1,		/* 3689 */
		-1,		/* 3690 */
		-1,		/* 3691 */
		-1,		/* 3692 */
		-1,		/* 3693 */
		-1,		/* 3694 */
		-1,		/* 3695 */
		-1,		/* 3696 */
		-1,		/* 3697 */
		-1,		/* 3698 */
		-1,		/* 3699 */
		-1,		/* 3700 */
		-1,		/* 3701 */
		-1,		/* 3702 */
		-1,		/* 3703 */
		-1,		/* 3704 */
		-1,		/* 3705 */
		-1,		/* 3706 */
		-1,		/* 3707 */
		-1,		/* 3708 */
		-1,		/* 3709 */
		-1,		/* 3710 */
		-1,		/* 3711 */
		-1,		/* 3712 */
		-1,		/* 3713 */
		-1,		/* 3714 */
		-1,		/* 3715 */
		-1,		/* 3716 */
		-1,		/* 3717 */
		-1,		/* 3718 */
		-1,		/* 3719 */
		-1,		/* 3720 */
		-1,		/* 3721 */
		-1,		/* 3722 */
		-1,		/* 3723 */
		-1,		/* 3724 */
		-1,		/* 3725 */
		-1,		/* 3726 */
		-1,		/* 3727 */
		-1,		/* 3728 */
		-1,		/* 3729 */
		-1,		/* 3730 */
		-1,		/* 3731 */
		-1,		/* 3732 */
		-1,		/* 3733 */
		-1,		/* 3734 */
		-1,		/* 3735 */
		-1,		/* 3736 */
		-1,		/* 3737 */
		-1,		/* 3738 */
		-1,		/* 3739 */
		-1,		/* 3740 */
		-1,		/* 3741 */
		-1,		/* 3742 */
		-1,		/* 3743 */
		-1,		/* 3744 */
		-1,		/* 3745 */
		-1,		/* 3746 */
		-1,		/* 3747 */
		-1,		/* 3748 */
		-1,		/* 3749 */
		-1,		/* 3750 */
		-1,		/* 3751 */
		-1,		/* 3752 */
		-1,		/* 3753 */
		-1,		/* 3754 */
		-1,		/* 3755 */
		-1,		/* 3756 */
		-1,		/* 3757 */
		-1,		/* 3758 */
		-1,		/* 3759 */
		-1,		/* 3760 */
		-1,		/* 3761 */
		-1,		/* 3762 */
		-1,		/* 3763 */
		-1,		/* 3764 */
		-1,		/* 3765 */
		-1,		/* 3766 */
		-1,		/* 3767 */
		-1,		/* 3768 */
		-1,		/* 3769 */
		-1,		/* 3770 */
		-1,		/* 3771 */
		-1,		/* 3772 */
		-1,		/* 3773 */
		-1,		/* 3774 */
		-1,		/* 3775 */
		-1,		/* 3776 */
		-1,		/* 3777 */
		-1,		/* 3778 */
		-1,		/* 3779 */
		-1,		/* 3780 */
		-1,		/* 3781 */
		-1,		/* 3782 */
		-1,		/* 3783 */
		-1,		/* 3784 */
		-1,		/* 3785 */
		-1,		/* 3786 */
		-1,		/* 3787 */
		-1,		/* 3788 */
		-1,		/* 3789 */
		-1,		/* 3790 */
		-1,		/* 3791 */
		-1,		/* 3792 */
		-1,		/* 3793 */
		-1,		/* 3794 */
		-1,		/* 3795 */
		-1,		/* 3796 */
		-1,		/* 3797 */
		-1,		/* 3798 */
		-1,		/* 3799 */
		-1,		/* 3800 */
		-1,		/* 3801 */
		-1,		/* 3802 */
		-1,		/* 3803 */
		-1,		/* 3804 */
		-1,		/* 3805 */
		-1,		/* 3806 */
		-1,		/* 3807 */
		-1,		/* 3808 */
		-1,		/* 3809 */
		-1,		/* 3810 */
		-1,		/* 3811 */
		-1,		/* 3812 */
		-1,		/* 3813 */
		-1,		/* 3814 */
		-1,		/* 3815 */
		-1,		/* 3816 */
		-1,		/* 3817 */
		-1,		/* 3818 */
		-1,		/* 3819 */
		-1,		/* 3820 */
		-1,		/* 3821 */
		-1,		/* 3822 */
		-1,		/* 3823 */
		-1,		/* 3824 */
		-1,		/* 3825 */
		-1,		/* 3826 */
		-1,		/* 3827 */
		-1,		/* 3828 */
		-1,		/* 3829 */
		-1,		/* 3830 */
		-1,		/* 3831 */
		-1,		/* 3832 */
		-1,		/* 3833 */
		-1,		/* 3834 */
		-1,		/* 3835 */
		-1,		/* 3836 */
		-1,		/* 3837 */
		-1,		/* 3838 */
		-1,		/* 3839 */
		-1,		/* 3840 */
		-1,		/* 3841 */
		-1,		/* 3842 */
		-1,		/* 3843 */
		-1,		/* 3844 */
		-1,		/* 3845 */
		-1,		/* 3846 */
		-1,		/* 3847 */
		-1,		/* 3848 */
		-1,		/* 3849 */
		-1,		/* 3850 */
		-1,		/* 3851 */
		-1,		/* 3852 */
		-1,		/* 3853 */
		-1,		/* 3854 */
		-1,		/* 3855 */
		-1,		/* 3856 */
		-1,		/* 3857 */
		-1,		/* 3858 */
		-1,		/* 3859 */
		-1,		/* 3860 */
		-1,		/* 3861 */
		-1,		/* 3862 */
		-1,		/* 3863 */
		-1,		/* 3864 */
		-1,		/* 3865 */
		-1,		/* 3866 */
		-1,		/* 3867 */
		-1,		/* 3868 */
		-1,		/* 3869 */
		-1,		/* 3870 */
		-1,		/* 3871 */
		-1,		/* 3872 */
		-1,		/* 3873 */
		-1,		/* 3874 */
		-1,		/* 3875 */
		-1,		/* 3876 */
		-1,		/* 3877 */
		-1,		/* 3878 */
		-1,		/* 3879 */
		-1,		/* 3880 */
		-1,		/* 3881 */
		-1,		/* 3882 */
		-1,		/* 3883 */
		-1,		/* 3884 */
		-1,		/* 3885 */
		-1,		/* 3886 */
		-1,		/* 3887 */
		-1,		/* 3888 */
		-1,		/* 3889 */
		-1,		/* 3890 */
		-1,		/* 3891 */
		-1,		/* 3892 */
		-1,		/* 3893 */
		-1,		/* 3894 */
		-1,		/* 3895 */
		-1,		/* 3896 */
		-1,		/* 3897 */
		-1,		/* 3898 */
		-1,		/* 3899 */
		-1,		/* 3900 */
		-1,		/* 3901 */
		-1,		/* 3902 */
		-1,		/* 3903 */
		-1,		/* 3904 */
		-1,		/* 3905 */
		-1,		/* 3906 */
		-1,		/* 3907 */
		-1,		/* 3908 */
		-1,		/* 3909 */
		-1,		/* 3910 */
		-1,		/* 3911 */
		-1,		/* 3912 */
		-1,		/* 3913 */
		-1,		/* 3914 */
		-1,		/* 3915 */
		-1,		/* 3916 */
		-1,		/* 3917 */
		-1,		/* 3918 */
		-1,		/* 3919 */
		-1,		/* 3920 */
		-1,		/* 3921 */
		-1,		/* 3922 */
		-1,		/* 3923 */
		-1,		/* 3924 */
		-1,		/* 3925 */
		-1,		/* 3926 */
		-1,		/* 3927 */
		-1,		/* 3928 */
		-1,		/* 3929 */
		-1,		/* 3930 */
		-1,		/* 3931 */
		-1,		/* 3932 */
		-1,		/* 3933 */
		-1,		/* 3934 */
		-1,		/* 3935 */
		-1,		/* 3936 */
		-1,		/* 3937 */
		-1,		/* 3938 */
		-1,		/* 3939 */
		-1,		/* 3940 */
		-1,		/* 3941 */
		-1,		/* 3942 */
		-1,		/* 3943 */
		-1,		/* 3944 */
		-1,		/* 3945 */
		-1,		/* 3946 */
		-1,		/* 3947 */
		-1,		/* 3948 */
		-1,		/* 3949 */
		-1,		/* 3950 */
		-1,		/* 3951 */
		-1,		/* 3952 */
		-1,		/* 3953 */
		-1,		/* 3954 */
		-1,		/* 3955 */
		-1,		/* 3956 */
		-1,		/* 3957 */
		-1,		/* 3958 */
		-1,		/* 3959 */
		-1,		/* 3960 */
		-1,		/* 3961 */
		-1,		/* 3962 */
		-1,		/* 3963 */
		-1,		/* 3964 */
		-1,		/* 3965 */
		-1,		/* 3966 */
		-1,		/* 3967 */
		-1,		/* 3968 */
		-1,		/* 3969 */
		-1,		/* 3970 */
		-1,		/* 3971 */
		-1,		/* 3972 */
		-1,		/* 3973 */
		-1,		/* 3974 */
		-1,		/* 3975 */
		-1,		/* 3976 */
		-1,		/* 3977 */
		-1,		/* 3978 */
		-1,		/* 3979 */
		-1,		/* 3980 */
		-1,		/* 3981 */
		-1,		/* 3982 */
		-1,		/* 3983 */
		-1,		/* 3984 */
		-1,		/* 3985 */
		-1,		/* 3986 */
		-1,		/* 3987 */
		-1,		/* 3988 */
		-1,		/* 3989 */
		-1,		/* 3990 */
		-1,		/* 3991 */
		-1,		/* 3992 */
		-1,		/* 3993 */
		-1,		/* 3994 */
		-1,		/* 3995 */
		-1,		/* 3996 */
		-1,		/* 3997 */
		-1,		/* 3998 */
		-1		/* 3999 */
};


signed char _fill_animdata[256] = {
	90,		/*    0 */
		90,		/*    1 */
		90,		/*    2 */
		90,		/*    3 */
		91,		/*    4 */
		91,		/*    5 */
		91,		/*    6 */
		91,		/*    7 */
		92,		/*    8 */
		92,		/*    9 */
		92,		/*   10 */
		92,		/*   11 */
		93,		/*   12 */
		93,		/*   13 */
		94,		/*   14 */
		94,		/*   15 */
		94,		/*   16 */
		94,		/*   17 */
		94,		/*   18 */
		94,		/*   19 */
		94,		/*   20 */
		94,		/*   21 */
		-1,		/*   22 */
		-1,		/*   23 */
		-1,		/*   24 */
		-1,		/*   25 */
		-1,		/*   26 */
		-1,		/*   27 */
		-1,		/*   28 */
		-1,		/*   29 */
		-1,		/*   30 */
		-1,		/*   31 */
		-1,		/*   32 */
		-1,		/*   33 */
		-1,		/*   34 */
		-1,		/*   35 */
		-1,		/*   36 */
		-1,		/*   37 */
		-1,		/*   38 */
		-1,		/*   39 */
		-1,		/*   40 */
		-1,		/*   41 */
		-1,		/*   42 */
		-1,		/*   43 */
		-1,		/*   44 */
		-1,		/*   45 */
		-1,		/*   46 */
		-1,		/*   47 */
		-1,		/*   48 */
		-1,		/*   49 */
		95,		/*   50 */
		95,		/*   51 */
		95,		/*   52 */
		95,		/*   53 */
		96,		/*   54 */
		96,		/*   55 */
		96,		/*   56 */
		96,		/*   57 */
		-1,		/*   58 */
		-1,		/*   59 */
		-1,		/*   60 */
		-1,		/*   61 */
		-1,		/*   62 */
		-1,		/*   63 */
		-1,		/*   64 */
		-1,		/*   65 */
		97,		/*   66 */
		97,		/*   67 */
		97,		/*   68 */
		97,		/*   69 */
		-1,		/*   70 */
		97,		/*   71 */
		97,		/*   72 */
		97,		/*   73 */
		97,		/*   74 */
		98,		/*   75 */
		98,		/*   76 */
		98,		/*   77 */
		98,		/*   78 */
		-1,		/*   79 */
		98,		/*   80 */
		98,		/*   81 */
		98,		/*   82 */
		98,		/*   83 */
		99,		/*   84 */
		99,		/*   85 */
		99,		/*   86 */
		99,		/*   87 */
		-1,		/*   88 */
		99,		/*   89 */
		99,		/*   90 */
		99,		/*   91 */
		99,		/*   92 */
		100,		/*   93 */
		100,		/*   94 */
		100,		/*   95 */
		100,		/*   96 */
		-1,		/*   97 */
		100,		/*   98 */
		100,		/*   99 */
		100,		/*  100 */
		100,		/*  101 */
		101,		/*  102 */
		101,		/*  103 */
		101,		/*  104 */
		101,		/*  105 */
		-1,		/*  106 */
		101,		/*  107 */
		101,		/*  108 */
		101,		/*  109 */
		101,		/*  100 */
		102,		/*  111 */
		102,		/*  112 */
		102,		/*  113 */
		103,		/*  114 */
		-1,		/*  115 */
		104,		/*  116 */
		104,		/*  117 */
		104,		/*  118 */
		105,		/*  119 */
		103,		/*  120 */
		103,		/*  121 */
		103,		/*  122 */
		20,		/*  123 */
		20,		/*  124 */
		20,		/*  125 */
		20,		/*  126 */
		20,		/*  127 */
		-1,		/*  128 */
		-1,		/*  129 */
		-1,		/*  130 */
		-1,		/*  131 */
		-1,		/*  132 */
		-1,		/*  133 */
		-1,		/*  134 */
		-1,		/*  135 */
		-1,		/*  136 */
		105,		/*  137 */
		105,		/*  138 */
		106,		/*  139 */
		101,		/*  140 */
		106,		/*  141 */
		106,		/*  142 */
		106,		/*  143 */
		106,		/*  144 */
		-1,		/*  145 */
		-1,		/*  146 */
		-1,		/*  147 */
		107,		/*  148 */
		107,		/*  149 */
		107,		/*  150 */
		107,		/*  151 */
		107,		/*  152 */
		107,		/*  153 */
		108,		/*  154 */
		108,		/*  155 */
		108,		/*  156 */
		108,		/*  157 */
		108,		/*  158 */
		108,		/*  159 */
		108,		/*  160 */
		108,		/*  161 */
		108,		/*  162 */
		-1,		/*  163 */
		-1,		/*  164 */
		-1,		/*  165 */
		-1,		/*  166 */
		109,		/*  167 */
		109,		/*  168 */
		109,		/*  169 */
		109,		/*  170 */
		109,		/*  171 */
		109,		/*  172 */
		110,		/*  173 */
		110,		/*  174 */
		110,		/*  175 */
		110,		/*  176 */
		111,		/*  177 */
		111,		/*  178 */
		111,		/*  179 */
		111,		/*  180 */
		111,		/*  181 */
		111,		/*  182 */
		112,		/*  183 */
		112,		/*  184 */
		112,		/*  185 */
		112,		/*  186 */
		112,		/*  187 */
		112,		/*  188 */
		-1,		/*  189 */
		-1,		/*  190 */
		-1,		/*  191 */
		-1,		/*  192 */
		-1,		/*  193 */
		-1,		/*  194 */
		-1,		/*  195 */
		-1,		/*  196 */
		-1,		/*  197 */
		-1,		/*  198 */
		-1,		/*  199 */
		-1,		/*  200 */
		-1,		/*  201 */
		-1,		/*  202 */
		-1,		/*  203 */
		-1,		/*  204 */
		-1,		/*  205 */
		-1,		/*  206 */
		-1,		/*  207 */
		-1,		/*  208 */
		-1,		/*  209 */
		-1,		/*  210 */
		-1,		/*  211 */
		-1,		/*  212 */
		-1,		/*  213 */
		113,		/*  214 */
		113,		/*  215 */
		113,		/*  216 */
		113,		/*  217 */
		114,		/*  218 */
		114,		/*  219 */
		114,		/*  220 */
		114,		/*  221 */
		115,		/*  222 */
		115,		/*  223 */
		115,		/*  224 */
		115,		/*  225 */
		116,		/*  226 */
		116,		/*  227 */
		116,		/*  228 */
		116,		/*  229 */
		117,		/*  230 */
		117,		/*  231 */
		117,		/*  232 */
		117,		/*  233 */
		118,		/*  234 */
		118,		/*  235 */
		118,		/*  236 */
		118,		/*  237 */
		119,		/*  238 */
		119,		/*  239 */
		119,		/*  240 */
		119,		/*  241 */
		120,		/*  242 */
		120,		/*  243 */
		120,		/*  244 */
		120,		/*  245 */
		0,		/*  246 */
		36,		/*  247 */
		0,		/*  248 */
		4,		/*  249 */
		9,		/*  250 */
		-1,		/*  251 */
		-1,		/*  252 */
		-1,		/*  253 */
		-1,		/*  254 */
		-1		/*  255 */
};

#define get_tileset(a)		(int)((a & 0x8000) ? _fill_animdata[a & 0xFF] : _fill_tiledata[a])

BOOL FillBits(WORD tile, int x, int y, struct MapInfo *map) {
	fbpos *posArray = NULL;
	int nMax = 0, nIndex = 0;
	int startx, starty, tileset, ts;
	WORD filltile, comptile, lastx, lasty;
	BYTE dirs;
	
	if (!useAnim && tileSel.tile == -1) 
		return FALSE;
	else if (useAnim && animSel.tile == -1)
		return FALSE;
	
	startx = x;
	starty = y;
	
	/* don't account for offsets in animations */
	if (tile & 0x8000)
		tile &= 0x80FF; 
	
	tileset = get_tileset(tile);
	
	if (useAnim) {
		char osbuf[4];
		int offset;
		
		GetDlgItemText(GetDlgItem(frameWnd, ID_INFOAREA), ID_OFFSET, osbuf, sizeof(osbuf));
		offset = atoi(osbuf);
		
		filltile = 0x8000 | (offset << 8) | animSel.tile;
		
		if (tileset == -1) {
			if ((filltile & 0x80FF) == tile)
				return FALSE;
		}
		else if (get_tileset(filltile) == tileset)
			return FALSE;
	}
	else {
		filltile = tileSel.tile;
		
		if (tileset == -1) {
			if (filltile == tile) 
				return FALSE;
		}
		else if (get_tileset(filltile) == tileset)
			return FALSE;
	}
	
	
	do {
		dirs = 0;
		
		if (!useAnim) {
			int yof, xof;
			filltile = tileSel.tile;
			xof = x - startx;
			yof = y - starty;
			
			yof = (yof % tileSel.vert);
			xof = (xof % tileSel.horz);
			
			if (xof < 0)
				xof += tileSel.horz;
			if (yof < 0)
				yof += tileSel.vert;
			
			filltile += xof + yof*40;
			map->mapData[y*map->header.width+x] = tileArrange[filltile];
		}
		else
			map->mapData[y*map->header.width+x] = filltile;
		
		if (x != map->header.width - 1) {
			comptile = map->mapData[y*map->header.width + x + 1];
			if (comptile & 0x8000)
				comptile &= 0x80FF;
			if (tileset != -1) {
				ts = get_tileset(comptile);
				if (ts == tileset)
					dirs |= FB_RIGHT;
			}
			else if (comptile == tile)
				dirs |= FB_RIGHT;
		}
		if (x != 0) {
			comptile = map->mapData[y*map->header.width + x - 1];
			if (comptile & 0x8000)
				comptile &= 0x80FF;
			if (tileset != -1) {
				ts = get_tileset(comptile);
				if (ts == tileset)
					dirs |= FB_LEFT;
			}
			else if (comptile == tile)
				dirs |= FB_LEFT;
		}
		if (y != map->header.height - 1) {
			comptile = map->mapData[(y+1)*map->header.width + x];
			if (comptile & 0x8000)
				comptile &= 0x80FF;
			if (tileset != -1) {
				ts = get_tileset(comptile);
				if (ts == tileset)
					dirs |= FB_DOWN;
			}
			else if (comptile == tile)
				dirs |= FB_DOWN;
		}
		if (y != 0) {
			comptile = map->mapData[(y-1)*map->header.width + x];
			if (comptile & 0x8000)
				comptile &= 0x80FF;
			if (tileset != -1) {
				ts = get_tileset(comptile);
				if (ts == tileset)
					dirs |= FB_UP;
			}
			else if (comptile == tile)
				dirs |= FB_UP;
		}
		
		lastx = (WORD)x;
		lasty = (WORD)y;
		
		if (dirs & FB_RIGHT) {
			x++;
			dirs &= ~FB_RIGHT;
		}
		else if (dirs & FB_LEFT) {
			x--;
			dirs &= ~FB_LEFT;
		}
		else if (dirs & FB_DOWN) {
			y++;
			dirs &= ~FB_DOWN;
		}
		else if (dirs & FB_UP) {
			y--;
			dirs &= ~FB_UP;
		}
		else {
			/* we can't move anywhere, we need to go back to the last saved position */
			/* make sure we didn't try to fill just 1 tile */
			if (nIndex-- == 0)
				break;
			x = posArray[nIndex].x;
			y = posArray[nIndex].y;
			continue;
		}
		
		
		if (dirs) {
			/* do we need room to save this position? */
			if (nIndex == nMax) {
				nMax += 10;
				
				posArray = (fbpos *)realloc(posArray, nMax * sizeof(fbpos));
			}
			posArray[nIndex].x = lastx;
			posArray[nIndex].y = lasty;
			nIndex++;
		}
		
	} while (1);
	
	if (posArray) free(posArray);
	
	return TRUE;
}

/* FillSelection
** Function: tiles the selected block area within a selection - repeating or cropping if necessary
** Parameters:
**      struct MapInfo *map - map to perform the action on
** 
** Return value:
**      TRUE - function performed the action
**      FALSE - did not perform the action
*/

BOOL FillSelection(struct MapInfo *map) {
	int x, y, w, h, k, l;
	WORD *oldbuf;
	struct undo_buf *undoInfo;
	int offset;
	char ofsbuf[4];
	
	GetDlgItemText(GetDlgItem(frameWnd, ID_INFOAREA), ID_OFFSET, ofsbuf, sizeof(ofsbuf));
	offset = atoi(ofsbuf);
	
	oldbuf = (WORD *)malloc(sizeof(WORD) * map->header.width * map->header.height);
	memcpy(oldbuf, map->mapData, sizeof(WORD) * map->header.width * map->header.height);
	
	if (!useAnim && tileSel.tile == -1) 
		return FALSE;
	else if (useAnim && animSel.tile == -1)
		return FALSE;
	
	if (!map)
		return FALSE;
	else if (map->selection.horz == 0 || map->selection.vert == 0 || map->selection.tile == -1) {
		MessageBox(NULL, "You did not select an area.", "Error", MB_OK);
		return FALSE;
	}
	
	w = map->selection.horz;
	h = map->selection.vert;
	if (useAnim) {
		k = 1;
		l = 1;
	}
	else {
		k = tileSel.horz;
		l = tileSel.vert;
	}
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (!useAnim)
				map->mapData[y*map->header.width + x + map->selection.tile] =
				tileSel.tile + (x % k) + (y % l) * 40;
			else 
				map->mapData[y*map->header.width + x + map->selection.tile] =
				animSel.tile | (offset << 8) | 0x8000;
		}
	}
	
	BufferCompare(map->mapData, oldbuf, map->header.width, map->header.height, &undoInfo);
	
    if (undoInfo) addUndo(&map->undo_list, undoInfo, map);
    free(oldbuf); 
	
	return TRUE;
}

struct MapInfo *get_map(HWND hwnd) {
	struct MapInfo *loop;
	
	for (loop = map_list; loop; loop = loop->next)
		if (loop->hwnd == hwnd) break;
		return loop;
}


BOOL iswarp(WORD x) {
	int p;
	WORD f = x & 0xFF;
	
	if (!(x & 0x8000)) return FALSE;
	
	for (p = 0; p < NUM_WARPS; p++)
		if (warps[p] == f) return TRUE;
		return FALSE;
}

int select_check(struct MapInfo *map, POINT *pt) {
	int selcheck = 0;
	int xoff, yoff;
	POINT pos;
	RECT selrect;

	xoff = GetScrollPos(map->hwnd, SB_HORZ);
	yoff = GetScrollPos(map->hwnd, SB_VERT);

	pos.x = pt->x + xoff;
	pos.y = pt->y + yoff;

	if (map->selection.tile == -1 ||
		map->selection.horz == 0 ||
		map->selection.vert == 0) 
		return selcheck;

	selrect.left = (map->selection.tile % map->header.width) * 16;
	selrect.right = selrect.left + map->selection.horz * 16;
	selrect.top = (map->selection.tile / map->header.width) * 16;
	selrect.bottom = selrect.top + map->selection.vert * 16;

	if (PtInRect(&selrect, pos))
		selcheck = 1;

	if (pos.y >= selrect.top && pos.y <= selrect.bottom) {
		if (abs(selrect.left - pos.x) < 8)
			selcheck = 2;
		else if (abs(selrect.right - pos.x) < 8)
			selcheck = 3;
	}
	if (pos.x >= selrect.left && pos.x <= selrect.right) {
		if (abs(selrect.bottom - pos.y) < 8) {
			if (selcheck == 2) selcheck = 6;
			else if (selcheck == 3) selcheck = 7;
			else selcheck = 4;
		}
		else if (abs(selrect.top - pos.y) < 8) {
			if (selcheck == 2) selcheck = 8;
			else if (selcheck == 3) selcheck = 9;
			else selcheck = 5;
		}
	}
	
	return selcheck;
}

	






