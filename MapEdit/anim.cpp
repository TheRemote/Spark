#include "main.h"
#include "extern.h"

struct Animation anim_list[256];

int CALLBACK AnimAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	
	switch(msg) {
		// each case will have its own scope
    case WM_CREATE:
		break;
		
    case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
    case WM_SIZE:
		{
			int scr_range;
			SCROLLINFO si;
			
			memset(&si, 0, sizeof(si));
			scr_range = 4096 - (int)HIWORD(lparm);
			si.nMax = scr_range;
			si.fMask = SIF_RANGE;
			si.cbSize = sizeof(si);
			
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			
			return DefWindowProc(hwnd, msg, wparm, lparm);
		}
		
    case WM_PAINT:
		{
			HDC dc;
			PAINTSTRUCT ps;
			struct Animation *anim;
			char hex[3];
			RECT rc;
			int scry, pntamt, x, idx;
			SCROLLINFO si;
			
			char vals[] = { '0', '1', '2', '3', '4', '5', '6',
				'7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0 };
			
			BeginPaint(hwnd, &ps);
			dc = ps.hdc;
			
			SelectPalette(dc, tilesPal, FALSE);
			RealizePalette(dc);
			
			memset(&si, 0, sizeof(si));
			si.fMask = SIF_ALL;
			si.cbSize = sizeof(si);
			GetScrollInfo(hwnd, SB_VERT, &si);
			scry = si.nPos;
			
			idx = scry/16;
			anim = get_anim((unsigned char)idx);
			
			GetClientRect(hwnd, &rc);
			pntamt = (int)((rc.bottom - rc.top) >> 4) + 1;
			
			
			SetWindowOrgEx(dc, 0, scry, NULL);
			
			while (pntamt-- && anim) {
				*hex = vals[(int)idx >> 4];
				hex[1] = vals[idx % 16];
				hex[2] = 0;
				
				rc.top = (int)idx << 4;
				rc.bottom = rc.top + 16;
				FillRect(dc, &rc, (HBRUSH)(COLOR_WINDOW+1));
				
				TextOut(dc, 0, (int)idx << 4, hex, 2);
				
				for (x = 0; x < anim->frameCount; x++)
					DrawTileBlock(anim->frames[x], dc, 20 + (x<<4), (int)idx << 4, 1, 1, FALSE);
				
				idx++;
				anim = get_anim((unsigned char)idx);
			}
			
			GetClientRect(hwnd, &rc);
			
			if (animSel.tile != -1) {
				HPEN pen = CreatePen(PS_SOLID, 1, 0x888888);
				DrawSelection(dc, pen, 0, animSel.tile<<4, rc.right-rc.left, 15);
				DeleteObject(pen);
			}
			
			EndPaint(hwnd, &ps);
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
				add_factor = -96;
				break;
				
			case SB_PAGEDOWN:
				add_factor = 96;
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
				
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
				
				ScrollWindowEx(hwnd, 0, -add_factor, NULL, NULL, NULL, NULL, SW_ERASE);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
			}
			break;
		}
		
    case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lparm);
			int y = HIWORD(lparm);
			int scy = GetScrollPos(hwnd, SB_VERT);
			int scx = GetScrollPos(hwnd, SB_HORZ);
			int anim_sel;
			RECT rc;
			HDC dc;
			HPEN pen;
			MSG mouseMsg;
			BOOL err;
			
			// should only need to deal with y's here
			
			anim_sel = ((y + scy)>>4);
			
			if (animSel.tile != -1) { // remove old selection box
				GetClientRect(hwnd, &rc);
				rc.top = animSel.tile * 16 - scy;
				rc.bottom = rc.top + 16;
				rc.right = rc.right - rc.left;
				rc.left = 0;
				animSel.tile = -1;
				RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			}
			
			if (get_anim((unsigned char)anim_sel))
				animSel.tile = anim_sel;
			else
				animSel.tile = -1;
			
			if (animSel.tile != -1) {
				GetClientRect(hwnd, &rc);
				dc = GetDC(hwnd);
				pen = CreatePen(PS_SOLID, 1, 0x888888);
				DrawSelection(dc, pen, 0, animSel.tile << 4, rc.right-rc.left, 15);
				DeleteObject(pen);
				ReleaseDC(hwnd, dc);
			}
			
			SetCapture(hwnd);
			
			while (1) {
				err = GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
				if (err < 1) break;
				
				if (!(mouseMsg.wParam & MK_LBUTTON) || mouseMsg.message == WM_LBUTTONUP)
					break;
				
				x = LOWORD(mouseMsg.lParam);
				y = HIWORD(mouseMsg.lParam);
				
				anim_sel = ((y + scy)>>4);
				
				if (anim_sel == animSel.tile) continue;
				
				if (animSel.tile != -1) { // remove old selection box
					GetClientRect(hwnd, &rc);
					rc.top = animSel.tile * 16 - scy;
					rc.bottom = rc.top + 16;
					rc.right = rc.right - rc.left;
					rc.left = 0;
					animSel.tile = -1;
					RedrawWindow(hwnd, &rc, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				
				if (get_anim((unsigned char)anim_sel))
					animSel.tile = anim_sel;
				else
					animSel.tile = -1;
				
				if (animSel.tile != -1) {
					dc = GetDC(hwnd);
					SetWindowOrgEx(dc, scx, scy, NULL);
					pen = CreatePen(PS_SOLID, 1, 0x888888);
					GetClientRect(hwnd, &rc);
					DrawSelection(dc, pen, 0, animSel.tile << 4, rc.right-rc.left, 15);
					DeleteObject(pen);
					ReleaseDC(hwnd, dc);
				}
			}
			
			if (!useAnim) {
				useAnim = TRUE;
				SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
			
			if (currentTool != TOOL_FILL)
				currentTool = TOOL_EDIT;
			DrawToolbar();
			
			
			
			ReleaseCapture();
			break;
		}     
		
    default:
		return DefWindowProc(hwnd, msg, wparm, lparm);
  }
  return 0;
}

int load_anims(char *file) {
	FILE *aFile;

	if ((aFile = fopen(file, "rb")) == NULL)  {
		MessageBox(NULL, "Failed to load animations", "Error", MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	fseek(aFile, 0x642E0, SEEK_SET);
	for (int aLoop = 0; aLoop < 256; aLoop++) {
		fread(&(anim_list[aLoop].frameCount), sizeof(BYTE), 1, aFile);
		fread(&(anim_list[aLoop].speed), sizeof(BYTE), 1, aFile);
		if (anim_list[aLoop].speed == 0) anim_list[aLoop].speed = 255;
		if (anim_list[aLoop].frameCount == 0) anim_list[aLoop].frameCount = 1;

		fread(anim_list[aLoop].frames, sizeof(WORD), 32, aFile);
	}
	
	fclose(aFile);		
	
	return 1;
}

void killAnims() {
	
	/*struct Animation *anim, *next;
	
	for (anim = anim_list; anim; anim = next) {
		next = anim->next;
		
		if (anim->frames) free(anim->frames);
		free(anim);
	}
	*/
}

__inline struct Animation *get_anim(unsigned char idx) {
	return &anim_list[idx];	
}

