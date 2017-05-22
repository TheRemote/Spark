#include "main.h"
#include "extern.h"

HWND frameWnd;
HINSTANCE hInstance;
HMENU hMenu;
HACCEL hAccel;
BOOL frameKilled = FALSE;

char arcdir[256], lastdir[256];

// settings
BOOL minSysTray = FALSE;
BOOL autoAssociate = TRUE;
BOOL autoPan = FALSE;
BOOL saveArrange = TRUE;
BOOL analWallMode = TRUE;
BOOL miniOverview = TRUE;
BOOL displayAnims = TRUE;
struct ApplicationLayout applayout;

NOTIFYICONDATA nid;

int hTeamSelected = 0;

struct MapInfo *infomap = NULL;

char *infosettings[] = {
	"very low",
		"low",
		"normal",
		"high",
		"very high" 
};


HFONT arialFont;
HBRUSH whiteBrush;

void GetAppLayout(HWND hwnd) {
	WINDOWPLACEMENT wp;
	RECT rc;


	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);

	rc = wp.rcNormalPosition;
				
	applayout.showtype = wp.showCmd;
	applayout.dimensions.cx = rc.right - rc.left;
	applayout.dimensions.cy = rc.bottom - rc.top;
	applayout.position.x = rc.left;
	applayout.position.y = rc.top;
}


int CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	switch (msg) {
	case WM_INITDIALOG:
		{
			char version_buf[128];

			sprintf(version_buf, "MalletEdit v%d.%02d.%02d\n%s",
								mymajor, myminor, myrevision, mytree);
			SetDlgItemText(hwnd, IDC_VERSION, version_buf);

			SetWindowLong(GetDlgItem(hwnd, IDC_AUTHOR), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
			SetWindowPos(GetDlgItem(hwnd, IDC_AUTHOR), NULL, 0, 0, 0, 0,
					SWP_NOMOVE|SWP_NOSIZE|SWP_NOREPOSITION|SWP_FRAMECHANGED|SWP_NOCOPYBITS);


			SetFocus(GetDlgItem(hwnd, IDOK));
			return 0;
		}

	case WM_DESTROY: 
		{
			EndDialog(hwnd, 0);
			break;
		}

	case WM_SYSCOMMAND:
		{
			UINT uCmdType = (UINT)wparm;
			int xPos = LOWORD(lparm);
			int yPos = HIWORD(lparm);


			switch (uCmdType) {
			case SC_CLOSE:
				DestroyWindow(hwnd);
				break;

			default:
				return 0;
			}

			break;
		}

	case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wparm);
			int wID = LOWORD(wparm);
			HWND hwndCtl = (HWND)lparm;

			switch (wNotifyCode) {
			case BN_CLICKED:
				{
					switch(wID) {
					case IDOK:
						DestroyWindow(hwnd);
						break;

					default:
						return 0;
					}
					break;
				}

			default:
				break;
			}

			break;
		}

	default:
		return 0;
	}
	return 1;
}

int CALLBACK WarpToolDlgProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	switch(msg) {
	case WM_INITDIALOG:
		{
			long style;
			int x, wp_count = 0;
			
			SetFocus(GetDlgItem(hwnd, IDC_SOURCE));
			style = GetWindowLong(GetDlgItem(hwnd, IDC_MAPVIEW), GWL_STYLE);
			style |= WS_HSCROLL|WS_VSCROLL;
			SetWindowLong(GetDlgItem(hwnd, IDC_MAPVIEW), GWL_STYLE, style);
			style = GetWindowLong(GetDlgItem(hwnd, IDC_MAPVIEW), GWL_EXSTYLE);
			style |= WS_EX_CLIENTEDGE;
			SetWindowLong(GetDlgItem(hwnd, IDC_MAPVIEW), GWL_EXSTYLE, style);
			SetWindowPos(GetDlgItem(hwnd, IDC_MAPVIEW), NULL, 0, 0, 0, 0,
						SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

			SendDlgItemMessage(hwnd, IDC_SOURCE, EM_SETLIMITTEXT, 1, 0);
			SendDlgItemMessage(hwnd, IDC_DESTINATION, EM_SETLIMITTEXT, 1, 0);

			for (x = 0; x < infomap->header.width * infomap->header.height; x++) {
				if (iswarp(infomap->mapData[x])) {
					char add_buff[32];
					int idx;
					sprintf(add_buff, "x,y: %d,%d", (x % infomap->header.width)+1, (x / infomap->header.width)+1);
					idx = (int)SendDlgItemMessage(hwnd, IDC_WARPS, LB_ADDSTRING, 0, (LPARAM)add_buff);
					SendDlgItemMessage(hwnd, IDC_WARPS, LB_SETITEMDATA, (WPARAM)idx, (LPARAM)(((x % infomap->header.width) << 16) | (x / infomap->header.width)));
					wp_count++;
				}
			}

			return 0;
		}
		
	case WM_DESTROY:
		EndDialog(hwnd, 0);
		break;
		
	case WM_SYSCOMMAND:
		{
			switch (wparm) {
			case SC_CLOSE:
				EndDialog(hwnd, 0);
				break;
				
			default:
				return 0;
			}
			break;
		}
		
	case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wparm);
			int wID = LOWORD(wparm);
			HWND hwndCtl = (HWND)lparm;


			if (hwndCtl) {

				switch (wNotifyCode) {
					case BN_CLICKED:
					{
						switch (wID) {
							case IDOK:
							{
								char *buf;
								int srclen, dstlen;

								srclen = GetWindowTextLength(GetDlgItem(hwnd, IDC_SOURCE));
								dstlen = GetWindowTextLength(GetDlgItem(hwnd, IDC_DESTINATION));

								buf = (char *)malloc(srclen+1);
								GetDlgItemText(hwnd, IDC_SOURCE, buf, srclen+1);
								if (srclen != 1 || atoi(buf) != (*buf - '0')) {
									free(buf);
									MessageBox(hwnd, "Enter a valid source\nAn integer 0-9 inclusive", "MalletEdit Warp Tool", MB_OK);
									break;
								}
								hWarpSrc = atoi(buf);
								free(buf);

								buf = (char *)malloc(dstlen+1);
								GetDlgItemText(hwnd, IDC_DESTINATION, buf, dstlen+1);
								if (dstlen != 1 || atoi(buf) != (*buf - '0')) {
									free(buf);
									MessageBox(hwnd, "Enter or choose a valid destination\nAn integer 0-9 inclusive", "MalletEdit Warp Tool", MB_OK);
									break;
								}
								hWarpDest = atoi(buf);
								free(buf);

								hWarpIdx = GetScrollPos(GetDlgItem(hwnd, IDC_ANIMS), SB_VERT);

								EndDialog(hwnd, 1); /* OK! Warp ready to go */
								break;
							}

							case IDCANCEL:
								DestroyWindow(hwnd);
								break;

							default:
								return 0;
						} // switch (wID) 

						break;
					} // case BN_CLICKED

					case LBN_SELCHANGE:
						{
							int scrollx, scrolly, sel;
							int x, y, w, h;
							DWORD data;
							RECT rc;
							//char *text, *xptr, *yptr;;

							GetClientRect(GetDlgItem(hwnd, IDC_MAPVIEW), &rc);
							w = rc.right - rc.left;
							h = rc.bottom - rc.top;

							sel = (int)SendDlgItemMessage(hwnd, wID, LB_GETCURSEL, 0, 0);
							data = (DWORD)SendDlgItemMessage(hwnd, wID, LB_GETITEMDATA, (WPARAM)sel, 0);

							x = HIWORD(data) * 16 - w/2;
							y = LOWORD(data) * 16 - h/2;

							/*
							text = (char *)malloc((int)SendDlgItemMessage(hwnd, wID, LB_GETTEXTLEN, (WPARAM)sel, 0)+1);
							SendDlgItemMessage(hwnd, wID, LB_GETTEXT, (WPARAM)sel, (LPARAM)text);


							for (xptr = text; *xptr && !isdigit(*xptr); xptr++);
							for (yptr = xptr; *yptr && isdigit(*yptr); yptr++);
							for (;*yptr && !isdigit(*yptr); ) *yptr++ = 0;


							x = atoi(xptr) * 16 - w/2;
							y = atoi(yptr) * 16 - h/2;

							free(text);
							*/

							scrollx = x - GetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_HORZ);
							scrolly = y - GetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_VERT);

							if (scrollx || scrolly) {
								SetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_HORZ, GetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_HORZ) + scrollx, TRUE);
								SetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_VERT, GetScrollPos(GetDlgItem(hwnd, IDC_MAPVIEW), SB_VERT) + scrolly, TRUE);
								ScrollWindowEx(GetDlgItem(hwnd, IDC_MAPVIEW), -scrollx, -scrolly,
									NULL, NULL, NULL, NULL, SW_ERASE);

								RedrawWindow(GetDlgItem(hwnd, IDC_MAPVIEW), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
							}

							break;
						} // case LBN_SELCHANGE

					default:
						return 1;
				} // switch (wNotifyCode)
			}
			break;
		}

	default:
		return 0;
	}
	return 1;
}

int CALLBACK WarpAnimsProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	switch(msg) {
		case WM_CREATE:
			{
				int style;
				style = GetWindowLong(hwnd, GWL_EXSTYLE);
				style |= WS_EX_CLIENTEDGE;
				SetWindowLong(hwnd, GWL_EXSTYLE, style);
				style = GetWindowLong(hwnd, GWL_STYLE);
				style |= WS_VSCROLL;
				SetWindowLong(hwnd, GWL_STYLE, style);
				SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
				break;
			}

		case WM_SIZE:
			{
				RECT outrect;
				RECT inrect;
				int h;
				SCROLLINFO sif;

				GetWindowRect(hwnd, &outrect);
				GetClientRect(hwnd, &inrect);

				h = (outrect.bottom - outrect.top) - (inrect.bottom - inrect.top);

				SetWindowPos(hwnd, NULL, 0, 0, outrect.right - outrect.left, 16+h, SWP_NOMOVE|SWP_NOREPOSITION|SWP_SHOWWINDOW);

				memset(&sif, 0, sizeof(sif));
				sif.cbSize = sizeof(sif);
				sif.fMask = SIF_RANGE|SIF_POS;
				sif.nMin = 0;
				sif.nMax = NUM_WARPS-1;
				sif.nPos = 0;
				SetScrollInfo(hwnd, SB_VERT, &sif, TRUE);			

				break;
			}

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc;
				int sy = GetScrollPos(hwnd, SB_VERT);
				int x;
				struct Animation *anim;
				RECT rc;

				BeginPaint(hwnd, &ps);
				dc = ps.hdc;

				GetClientRect(hwnd, &rc);

				SetWindowOrgEx(dc, 0, sy*16, NULL);
				rc.top += sy*16; rc.bottom += sy*16;
				FillRect(dc, &rc, GetSysColorBrush(COLOR_WINDOW));

				for (x = 0; x < NUM_WARPS; x++) {
					anim = get_anim((unsigned char)warps[x]);
					if (!anim) continue;

					DrawTileBlock(anim->frames[0], dc, 0, x*16, 1, 1, TRUE);
				}

				EndPaint(hwnd, &ps);
				break;
			}		

		case WM_VSCROLL:
			{
				int nScrollCode = (int)LOWORD(wparm);
				int nPos = (int)HIWORD(wparm);
				HWND hwndCtl = (HWND)lparm;
				SCROLLINFO sif;
				int add;

				memset(&sif, 0, sizeof(sif));
				sif.fMask = SIF_ALL;
				sif.cbSize = sizeof(sif);
				GetScrollInfo(hwnd, SB_VERT, &sif);


				switch (nScrollCode) {
		case SB_LINEUP:
		case SB_PAGEUP:
			add = -1;
			break;

		case SB_PAGEDOWN:
		case SB_LINEDOWN:
			add = 1;
			break;

		default:
			return 0;
				}

				add = MAX(sif.nMin - sif.nPos, MIN(sif.nMax - sif.nPos, add));
				if (add) {
					sif.fMask = SIF_POS;
					sif.nPos += add;
					SetScrollInfo(hwnd, SB_VERT, &sif, TRUE);
					ScrollWindowEx(hwnd, 0, -add, NULL, NULL, NULL, NULL, SW_ERASE);
					RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}

				break;
			}



		default:
			return DefWindowProc(hwnd, msg, wparm, lparm);
	}
	return 0;
}


int CALLBACK WarpToolAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	switch (msg) {
	case WM_CREATE:
		break; /* shouldn't ever get here */
					
	case WM_DESTROY:
		break;

	case WM_SIZE: 
		{
			int width, height;
			SCROLLINFO sif;
			width = LOWORD(lparm);
			height = HIWORD(lparm);

			memset(&sif, 0, sizeof(&sif));
			sif.cbSize = sizeof(sif);
			sif.fMask = SIF_RANGE;
			sif.nMin = 0;
			sif.nMax = (infomap->header.width * 16) - width;
			SetScrollInfo(hwnd, SB_HORZ, &sif, TRUE);
			sif.nMax = (infomap->header.height * 16) - height;
			SetScrollInfo(hwnd, SB_VERT, &sif, TRUE);


			//SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOREPOSITION|SWP_NOMOVE);
			break;
		}

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			SCROLLINFO si;
			int x, y, maxx, maxy, tile;
			int scrollx, scrolly;
			struct MapInfo *map = infomap;

			if (!map) break;
			
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
			
			
			if (!map || !map->mapData) 
			{
				SetBkMode(hdc, TRANSPARENT);
				TextOut(hdc, 0, 0, "No map data", 11);
			}
			else 
			{
				maxx = MIN((map->header.width), maxx);
				maxy = MIN((map->header.height), maxy);
				
				rc.left = maxx<<4;
				rc.right = rc.left + 16;
				rc.top = 0;
				rc.bottom = (maxy<<4) + 16;
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
				rc.left = 0;
				rc.right = (maxx<<4) + 16;
				rc.top = maxy<<4;
				rc.bottom = rc.top + 16;
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));        
				GetClientRect(hwnd, &rc);
				
				for (x = scrollx>>4; x < maxx; x++) {
					for (y = scrolly>>4; y < maxy; y++) {
						tile = map->mapData[(y*map->header.width)+x];
						if (!(tile & 0x8000))
							DrawTileBlock(tile, hdc, x<<4, y<<4, 1, 1, FALSE);
						else {
							unsigned char offset;
							unsigned char idx;
							struct Animation *anim;
							
							offset = (char)((tile & 0x7F00) >> 8);
							idx = (char)((tile & 0x00FF));
							
							SetBkMode(hdc, OPAQUE);
							
							if ((anim = get_anim(idx)) && anim->frameCount > 0)
								DrawTileBlock(anim->frames[offset % anim->frameCount], hdc, x<<4, y<<4, 1, 1, FALSE);
							else
								TextOut(hdc, x<<4, y<<4, "a", 1);

							SetBkMode(hdc, TRANSPARENT);
							if (iswarp((WORD)tile)) 
							{
								HPEN whitepen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
								DrawSelection(hdc, whitepen, x<<4, y<<4, 16, 16);
								DeleteObject(whitepen);
							}
						}
					}
				}				
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
			}
			break;
		}

	case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lparm) + GetScrollPos(hwnd, SB_HORZ);
			int y = GET_Y_LPARAM(lparm) + GetScrollPos(hwnd, SB_VERT);
			WORD tile;
			int dest;
			char buf[12];

			tile = infomap->mapData[(y/16)*infomap->header.width + (x/16)];
			if (!iswarp(tile)) 
			{
				MessageBox(NULL, "You must select a tile that is a warp.\nThis area is not used to place warps. Please select the warp type on your right,\n"
					             "select an address for the warp (0-9), and a destination by entering 0-9 or by clicking a warp that already exists.\n"
								 "Then press OK.", "MalletEdit Warp Tool", MB_OK);
				break;
			}

			dest = ((tile & 0x7F00) >> 8) / 10;
			sprintf(buf, "%d", dest);
			SetDlgItemText(GetParent(hwnd), IDC_DESTINATION, buf);

			dest = ((tile & 0x7F00) >> 8) - dest * 10;
			sprintf(buf, "%d", dest);
			SetDlgItemText(GetParent(hwnd), IDC_SOURCE, buf);
			break;
		}

				
	default:
		return DefWindowProc(hwnd, msg, wparm, lparm);
	}
	return 0;
}


int CALLBACK MapInfoProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) 
{
				char buf[5], info[256];
				int i;
				
				struct MapInfo *map = infomap;
				
				switch(msg) {
				case WM_INITDIALOG:
					{
						char buf[50];
						SendDlgItemMessage(hwnd, IDC_MAPNAME, WM_SETTEXT, 0, (LPARAM)map->header.name);
						SendDlgItemMessage(hwnd, IDC_DESC, WM_SETTEXT, 0, (LPARAM)map->header.description);
						if (map->header.bounciesEnabled)
							SendDlgItemMessage(hwnd, IDC_BOUNCIES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						if (map->header.misslesEnabled)
							SendDlgItemMessage(hwnd, IDC_MISSLES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						if (map->header.bombsEnabled)
							SendDlgItemMessage(hwnd, IDC_BOMBS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						if (map->header.objective == OBJECTIVE_FRAG)
							SendDlgItemMessage(hwnd, IDC_FRAG, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						else if (map->header.objective == OBJECTIVE_FLAG)
							SendDlgItemMessage(hwnd, IDC_FLAG, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						else
							SendDlgItemMessage(hwnd, IDC_SWITCH, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						
						memset(buf, 0, sizeof(buf));
						itoa(map->header.maxPlayers, buf, 10);
						SendDlgItemMessage(hwnd, IDC_MAXPLAY, WM_SETTEXT, 0, (LPARAM)buf);
						memset(buf, 0, sizeof(buf));
						itoa(map->header.maxSimulPowerups, buf, 10);
						SendDlgItemMessage(hwnd, IDC_POWUPS, WM_SETTEXT, 0, (LPARAM)buf);
						memset(buf, 0, sizeof(buf));
						itoa(map->header.numTeams, buf, 10);
						SendDlgItemMessage(hwnd, IDC_TEAMS, WM_SETTEXT, 0, (LPARAM)buf);
						memset(buf, 0, sizeof(buf));
						itoa(map->header.holdingTime, buf, 10);
						SendDlgItemMessage(hwnd, IDC_DEFHOLD, WM_SETTEXT, 0, (LPARAM)buf);
						memset(buf, 0, sizeof(buf));
						
						SendDlgItemMessage(hwnd, IDC_LDAMAGE, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,4));
						SendDlgItemMessage(hwnd, IDC_SDAMAGE, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,4));
						SendDlgItemMessage(hwnd, IDC_RECRATE, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,4));
						SendDlgItemMessage(hwnd, IDC_LDAMAGE, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)map->header.laserDamage);
						SendDlgItemMessage(hwnd, IDC_SDAMAGE, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)map->header.specialDamage);
						SendDlgItemMessage(hwnd, IDC_RECRATE, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)map->header.rechargeRate);
						SetDlgItemText(hwnd, IDC_LASERD, infosettings[map->header.laserDamage]);
						SetDlgItemText(hwnd, IDC_SPECIALD, infosettings[map->header.specialDamage]);
						SetDlgItemText(hwnd, IDC_RECHARGER, infosettings[map->header.rechargeRate]);
					}
					break;    
					
				case WM_COMMAND:
					switch (HIWORD(wparm)) {
					case 1: 
						{
							switch (LOWORD(wparm)) {
							case COMM_TAB:
							case COMM_ENTER:
								{
									HWND ctrl = GetFocus();
									
									SetFocus(GetNextDlgTabItem(hwnd, ctrl, FALSE));
									
									if (LOWORD(wparm) == COMM_ENTER && ctrl == GetDlgItem(hwnd, IDC_SETSET))
										return SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SETSET, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_SETSET));
									return 1;
								}
								
							default:
								return 0;
							}
							break;            
						}
						
					case BN_CLICKED:
						switch (LOWORD(wparm)) {
						case IDC_SETSET:
							map->modify = TRUE;
							map->header.nameLength = (short)SendDlgItemMessage(hwnd, IDC_MAPNAME,
								WM_GETTEXTLENGTH, 0, 0);
							map->header.name = (char *)malloc(map->header.nameLength+1);
							memset(map->header.name, 0, map->header.nameLength + 1);
							SendDlgItemMessage(hwnd, IDC_MAPNAME, WM_GETTEXT,
								(WPARAM)map->header.nameLength+1, (LPARAM)map->header.name);
							map->header.descriptionLength = (short)SendDlgItemMessage(hwnd, IDC_DESC,
								WM_GETTEXTLENGTH, 0, 0);
							map->header.description = (char *)malloc(map->header.descriptionLength+1);
							memset(map->header.description, 0, map->header.descriptionLength+1);
							SendDlgItemMessage(hwnd, IDC_DESC, WM_GETTEXT,
								(WPARAM)map->header.descriptionLength+1, (LPARAM)map->header.description);
							memset(info, 0, sizeof(info));
							
							
							map->header.laserDamage = (char)SendDlgItemMessage(hwnd, IDC_LDAMAGE, TBM_GETPOS, 0, 0);
							map->header.specialDamage = (char)SendDlgItemMessage(hwnd, IDC_SDAMAGE, TBM_GETPOS, 0, 0);
							map->header.rechargeRate = (char)SendDlgItemMessage(hwnd, IDC_RECRATE, TBM_GETPOS, 0, 0);
							
							map->header.bounciesEnabled = SendDlgItemMessage(hwnd, IDC_BOUNCIES, BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
							map->header.misslesEnabled = SendDlgItemMessage(hwnd, IDC_MISSLES, BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
							map->header.bombsEnabled = SendDlgItemMessage(hwnd, IDC_BOMBS, BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
							
							memset(buf, 0, sizeof(buf));
							i = SendDlgItemMessage(hwnd, IDC_MAXPLAY, WM_GETTEXTLENGTH, 0, 0);
							SendDlgItemMessage(hwnd, IDC_MAXPLAY, WM_GETTEXT, (WPARAM)i+1, (LPARAM)buf);
							map->header.maxPlayers = string_len(buf)>0?atoi(buf):64;
							memset(buf, 0, sizeof(buf));
							i = SendDlgItemMessage(hwnd, IDC_POWUPS, WM_GETTEXTLENGTH, 0, 0);
							SendDlgItemMessage(hwnd, IDC_POWUPS, WM_GETTEXT, (WPARAM)i+1, (LPARAM)buf);
							map->header.maxSimulPowerups = string_len(buf)>0?atoi(buf):4;
							memset(buf, 0, sizeof(buf));
							i = SendDlgItemMessage(hwnd, IDC_DEFHOLD, WM_GETTEXTLENGTH, 0, 0);
							SendDlgItemMessage(hwnd, IDC_DEFHOLD, WM_GETTEXT, (WPARAM)i+1, (LPARAM)buf);
							map->header.holdingTime = string_len(buf)>0?atoi(buf):12;
							memset(buf, 0, sizeof(buf));
							i = SendDlgItemMessage(hwnd, IDC_TEAMS, WM_GETTEXTLENGTH, 0, 0);
							SendDlgItemMessage(hwnd, IDC_TEAMS, WM_GETTEXT, (WPARAM)i+1, (LPARAM)buf);
							map->header.numTeams = string_len(buf)>0?atoi(buf):2;
							if (SendDlgItemMessage(hwnd, IDC_FRAG, BM_GETCHECK, 0, 0) == BST_CHECKED)
								map->header.objective = 0;
							else if (SendDlgItemMessage(hwnd, IDC_FLAG, BM_GETCHECK, 0, 0) == BST_CHECKED)
								map->header.objective = 1;
							else
								map->header.objective = 2;
							
							SetWindowText(map->hwnd, map->header.name);
							
							EndDialog(hwnd, 1);
							break;
							
						case IDC_CANSET:
							EndDialog(hwnd, 0);
							break;

						case IDC_FRAG:
							MessageBox(NULL, "Setting the game for \"frag game\" renders all switches and flags in a map useless.", "Warning", MB_OK);
							break;
							
						default:
							break;
						}
						break;
						
						case EN_UPDATE:
							if (LOWORD(wparm) == IDC_TEAMS) {
								memset(buf, 0, sizeof(buf));
								SendDlgItemMessage(hwnd, IDC_TEAMS, WM_GETTEXT, (WPARAM)sizeof(buf)+1, (LPARAM)buf);
								i = string_len(buf)>0?atoi(buf):2;
								
								if (i > 4 || i < 2) {
									memset(buf, 0, sizeof(buf));
									if (i < 2) i = 2;
									if (i > 4) i = 4;
									itoa(i, buf, 10);
									SendDlgItemMessage(hwnd, IDC_TEAMS, WM_SETTEXT, (WPARAM)2, (LPARAM)buf);
									MessageBeep(0xFFFFFFFF);
								}
								break;
							}
							if (LOWORD(wparm) == IDC_MAXPLAY) {
								memset(buf, 0, sizeof(buf));
								SendDlgItemMessage(hwnd, IDC_MAXPLAY, WM_GETTEXT, (WPARAM)sizeof(buf)+1, (LPARAM)buf);
								i = string_len(buf)>0?atoi(buf):2;
								
								if (i > 64 || i < 2) {
									memset(buf, 0, sizeof(buf));
									if (i < 2) i = 2;
									if (i > 64) i = 64;
									itoa(i, buf, 10);
									SendDlgItemMessage(hwnd, IDC_MAXPLAY, WM_SETTEXT, (WPARAM)string_len(buf), (LPARAM)buf);
									MessageBeep(0xFFFFFFFF);
								}
								break;
							}
							if (LOWORD(wparm) == IDC_DEFHOLD) {
								memset(buf, 0, sizeof(buf));
								SendDlgItemMessage(hwnd, IDC_DEFHOLD, WM_GETTEXT, (WPARAM)sizeof(buf)+1, (LPARAM)buf);
								i = string_len(buf)>0?atoi(buf):1;
								
								if (i < 1) {
									memset(buf, 0, sizeof(buf));
									if (i < 1) i = 1;
									itoa(i, buf, 10);
									SendDlgItemMessage(hwnd, IDC_DEFHOLD, WM_SETTEXT, (WPARAM)string_len(buf), (LPARAM)buf);
									MessageBeep(0xFFFFFFFF);
								}
								break;
							}                               
							break;
							
						default:
							break;
							
      }
      break;
	  
	case WM_SYSCOMMAND:
		switch (wparm) {
		case SC_CLOSE:
			EndDialog(hwnd, 0);
			break;
			
		default:
			return 0;
		}
		break;
		
		case WM_HSCROLL:
			{
				int nScrollCode = (int)LOWORD(wparm);
				HWND bar = (HWND)lparm;
				int nPos, id = -1;
				
				
				nPos = SendMessage(bar, TBM_GETPOS, 0, 0);
				
				
				switch (nScrollCode) {
				case TB_TOP:
				case TB_BOTTOM:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
					
					/* slider movement */
					switch (GetDlgCtrlID(bar)) {
					case IDC_LDAMAGE:
						id = IDC_LASERD;
						break;
						
					case IDC_SDAMAGE:
						id = IDC_SPECIALD;
						break;
						
					case IDC_RECRATE:
						id = IDC_RECHARGER;
						break;
						
					default:
						break;
					}
					
					if (id != -1) 
						SetDlgItemText(hwnd, id, infosettings[nPos % 5]);
					
					
					break;
					
					default:
						break;
						
				}
				break;
			}
			
			
		default:
			return 0;
  }
  return 1;
}

int CALLBACK SettingsAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map;
	
	switch(msg) {
    case WM_DESTROY:
		for (map = map_list; map; map = map->next)
			RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		RedrawWindow(GetDlgItem(frameWnd, ID_TILEAREA), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		RedrawWindow(GetDlgItem(frameWnd, ID_ANIMATIONS), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		
		SetWindowLong(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), GWL_STYLE,
			GetWindowLong(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), GWL_STYLE) & ~WS_DISABLED);
		SetWindowPos(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), NULL, 0, 0, 0, 0,
			SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOCOPYBITS);
		
		break;
		
    case WM_INITDIALOG:
		if (minSysTray) SendDlgItemMessage(hwnd, IDC_SYSTRAY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (autoPan) SendDlgItemMessage(hwnd, IDC_AUTOPAN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (autoAssociate) SendDlgItemMessage(hwnd, IDC_ASSOC, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (saveArrange) SendDlgItemMessage(hwnd, IDC_ARRANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (analWallMode) SendDlgItemMessage(hwnd, IDC_WALLMODE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (miniOverview) SendDlgItemMessage(hwnd, IDC_MINIVIEW, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (displayAnims) SendDlgItemMessage(hwnd, IDC_ANIMFRAME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		
		SetWindowLong(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), GWL_STYLE,
			GetWindowLong(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), GWL_STYLE) | WS_DISABLED);
		SetWindowPos(GetDlgItem(GetDlgItem(frameWnd, ID_INFOAREA), ID_SETTINGS), NULL, 0, 0, 0, 0,
			SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOCOPYBITS);
		
		break;
		
    case WM_COMMAND:
		{
			int notify = HIWORD(wparm);
			int id = LOWORD(wparm);
			HWND ctrl = (HWND)lparm;
			
			switch(notify) {
			case 1:
				{
					switch (id) {
					case COMM_TAB:
						SetFocus(GetNextDlgTabItem(hwnd, GetFocus(), FALSE));
						break;
						
					case COMM_ENTER:
						return SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SETSET, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_SETSET));
						
					default:
						break;
					}
					break;
				}
				
			case BN_CLICKED:
				switch(id) {
				case IDC_CANSET:
					DestroyWindow(hwnd);
					break;
					
				case IDC_SETSET:
					{
						int result;
						
						result = SendDlgItemMessage(hwnd, IDC_SYSTRAY, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) minSysTray = TRUE;
						else minSysTray = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_ASSOC, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) autoAssociate = TRUE;
						else autoAssociate = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_AUTOPAN, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) autoPan = TRUE;
						else autoPan = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_ARRANGE, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) saveArrange = TRUE;
						else saveArrange = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_WALLMODE, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) analWallMode = TRUE;
						else analWallMode = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_MINIVIEW, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) miniOverview = TRUE;
						else miniOverview = FALSE;
						result = SendDlgItemMessage(hwnd, IDC_ANIMFRAME, BM_GETCHECK, 0, 0);
						if (result == BST_CHECKED) displayAnims = TRUE;
						else displayAnims = FALSE;


						{
							RECT rc;
							int width, height;
							struct MapInfo *m;
							
							GetClientRect(frameWnd, &rc);
							width = rc.right - rc.left;
							height = rc.bottom - rc.top;
							
							
							if (!miniOverview) {
								SetWindowPos(GetDlgItem(frameWnd, ID_ANIMATIONS), NULL, width-130, 32, 130, height-130-20-32,
									SWP_SHOWWINDOW|SWP_NOREPOSITION);
								KillTimer(GetDlgItem(frameWnd, ID_MINIVIEW), TIMER_OVUPDATE);
							}
							else {
								SetWindowPos(GetDlgItem(frameWnd, ID_ANIMATIONS), NULL, width-130, 130+32, 130, height-130-20-32-130,
									SWP_SHOWWINDOW|SWP_NOREPOSITION);
								SetWindowPos(GetDlgItem(frameWnd, ID_MINIVIEW), NULL, width-130, 32, 130, 130, SWP_SHOWWINDOW|SWP_NOREPOSITION);
								SetTimer(GetDlgItem(frameWnd, ID_MINIVIEW), TIMER_OVUPDATE, 3000, NULL);
							}

							if (!displayAnims) {
								for (m = map_list; m; m = m->next) {
									KillTimer(m->hwnd, TIMER_ANIMFRAME);
									m->anim_frame = 0;
								}
							}
							else {
								for (m = map_list; m; m = m->next) {
									SetTimer(m->hwnd, TIMER_ANIMFRAME, ANIMFRAME_DURATION, NULL);
									m->anim_frame = 0;
								}
							}
								
							
						}
						
						
						
						DestroyWindow(hwnd);
						break;
					}
					
				case IDC_RESTORE:
					{
						int x, result;
						
						result = MessageBox(NULL, "Doing this will erase any changes you made\n"
							"to your arrangement. Do you wish to continue?",
							"Undoable Operation", MB_YESNO);
						if (result == IDNO) break;
						
						if (tileArrange) free(tileArrange);
						tileArrange = (WORD *)malloc(8000);
						numTiles = 4000;
						for (x = 0; x < numTiles; x++)
							tileArrange[x] = x;
						
						RedrawWindow(GetDlgItem(frameWnd, ID_TILEAREA), NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
						DestroyWindow(hwnd);
						break;
					}
					
				case IDC_CUSTOMIZE:
					{
#ifdef DONTALLOWCUSTOMIZE
						MessageBox(NULL, "Customization has been disabled in this build!", "No", MB_OK);
						break;
#else
						HWND customizeDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CUSTOMIZE), frameWnd, CustomizeProc, (LPARAM)hwnd);
						ShowWindow(customizeDlg, SW_SHOW);
						ShowWindow(hwnd, SW_HIDE);
#endif
						break;
					}             
					
				default:
					break;
				}
				break;
				
				default:
					break;
			}
			break;
		}
		
    default:
		break;
  }
  return 0;
}

int CALLBACK InfoAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	
	switch(msg) {
    case WM_CREATE:
		{
			HWND tmp;
			
			tmp = CreateWindow("BUTTON", "Tile",
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				2, 2, 50, 20,
				hwnd, (HMENU)ID_TILE, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			SendMessage(tmp, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			tmp = CreateWindow("BUTTON", "Anim",
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				54, 2, 60, 20,
				hwnd, (HMENU)ID_ANIM, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL,
				WS_CHILD|WS_VISIBLE|ES_NUMBER,
				64, 25, 50, 20,
				hwnd, (HMENU)ID_OFFSET, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Settings",
				WS_CHILD|BS_PUSHBUTTON|BS_NOTIFY,
				25, 110, 60, 20,
				hwnd, (HMENU)ID_SETTINGS, hInstance, NULL);
			ShowWindow(tmp, SW_SHOW);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Green", 
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				2, 47, 60, 20, 
				hwnd, (HMENU)ID_GREEN, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Red", 
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				2, 69, 60, 20, 
				hwnd, (HMENU)ID_RED, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Blue", 
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				64, 47, 60, 20, 
				hwnd, (HMENU)ID_BLUE, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Yellow", 
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				64, 69, 65, 20, 
				hwnd, (HMENU)ID_YELLOW, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			tmp = CreateWindow("BUTTON", "Neutral", 
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
				30, 91, 65, 10, 
				hwnd, (HMENU)ID_NEUTRAL, hInstance, NULL);
			SendMessage(tmp, WM_SETFONT, (WPARAM)arialFont, MAKELPARAM(TRUE, 0));
			SendDlgItemMessage(hwnd, ID_GREEN, BM_SETCHECK, BST_CHECKED, 0);
			
			break;
		}
		
    case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		{
			HDC dc = (HDC)wparm;

			SetBkMode(dc, TRANSPARENT);
			
			switch (GetDlgCtrlID((HWND)lparm)) {
			case ID_GREEN:
				(void)SelectObject(dc, arialFont);
				SetTextColor(dc, 0x00006600);
				break;
			case ID_RED:
				(void)SelectObject(dc, arialFont);
				SetTextColor(dc, 0x00000066);
				break;
			case ID_BLUE:
				(void)SelectObject(dc, arialFont);
				SetTextColor(dc, 0x00FF0000);
				break;
			case ID_YELLOW:
				(void)SelectObject(dc, arialFont);
				SetTextColor(dc, 0x0000CCCC);
				break;
			default:
				break;
			}
			
			return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
		}
		break;
		
    case WM_PAINT:
		{
			HDC dc;
			PAINTSTRUCT ps;
			
			BeginPaint(hwnd, &ps);
			dc = ps.hdc;
			
			SetBkMode(dc, TRANSPARENT);
			
			arialFont = (HFONT)SelectObject(dc, arialFont);
			TextOut(dc, 0, 25, "Offset:", 7);
			arialFont = (HFONT)SelectObject(dc, arialFont);
			EndPaint(hwnd, &ps);
			break;
		}
		
    case WM_COMMAND:
		{
			int id = LOWORD(wparm);
			int notify = HIWORD(wparm);
			HWND ctrl = (HWND)lparm;
			int style;
			
			switch (notify) {
			case EN_SETFOCUS:
				if (id == ID_OFFSET) {
					style = SendDlgItemMessage(hwnd, ID_ANIM, BM_GETCHECK, 0, 0);
					if (style == BST_UNCHECKED) {
						DestroyCaret();
						SetFocus(hwnd);
					}
				}
				break;
				
			case EN_CHANGE:
				{
					char offsetBuf[4];
					int os;
					
					memset(offsetBuf, 0, sizeof(offsetBuf));
					if (id == ID_OFFSET) {
						GetWindowText(ctrl, offsetBuf, sizeof(offsetBuf));
						os = atoi(offsetBuf);
						if (os > 127 || os < 0) {
							os = MIN(127, MAX(0, os));
							
							sprintf(offsetBuf, "%d", os);
							SetWindowText(ctrl, offsetBuf);
							MessageBeep(0xFFFFFFFF);
						}
					}
					break;
				}
				
			case BN_CLICKED:
				{
					
					switch (id) {
					case ID_TILE:
						SendMessage(ctrl, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						SendDlgItemMessage(hwnd, ID_ANIM, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						useAnim = FALSE;
						break;
						
					case ID_ANIM:
						SendMessage(ctrl, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						SendDlgItemMessage(hwnd, ID_TILE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						useAnim = TRUE;
						break;
						
					case ID_SETTINGS:
						{
							HWND dlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SETTINGS),
								frameWnd, SettingsAreaProc);
							ShowWindow(dlg, SW_SHOW);
							break;
						}
						
					case ID_GREEN:
					case ID_RED:
					case ID_BLUE:
					case ID_YELLOW:
					case ID_NEUTRAL:
						{
							int n;
							
							if (id == ID_NEUTRAL) {
								if (currentTool == TOOL_HOLDING_PEN) {
									MessageBox(NULL, "You cannot make NEUTRAL holding pens.\nSelect another team.", "Holding Pen Tool", MB_OK);
									break;
								}
								if (currentTool == TOOL_SPAWN) {
									MessageBox(NULL, "You cannot make NEUTRAL spawns.\nSelect another team.", "Spawn Tool", MB_OK);
									break;
								}
							}
							
							hTeamSelected = id - ID_GREEN;
							
							
							
							for (n = 0; n < 5; n++) {
								SendDlgItemMessage(hwnd, n+ID_GREEN, BM_SETCHECK, hTeamSelected==n?BST_CHECKED:BST_UNCHECKED, 0);
							}
							break;
						}
						
					default:
						break;
					}
					
					break;
				}
				
			default:
				return DefWindowProc(hwnd, msg, wparm, lparm);
			}
			break;
		}
		
    default:
		return DefWindowProc(hwnd,msg,wparm,lparm);
  }
  return 0;
}

int CALLBACK MalletEditMDISubclassProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	WNDPROC oldproc = (WNDPROC)GetWindowLong(hwnd, GWL_USERDATA);

	switch(msg) {
	case WM_NCLBUTTONDOWN:
		{
			int nchit = (int)wparm;
			POINTS pts = MAKEPOINTS(lparm);
			HCURSOR curs = LoadCursor(NULL, IDC_SIZENS);
			RECT rc;

			GetWindowRect(hwnd, &rc);

			if (nchit == HTBORDER && abs(rc.bottom - pts.y) < 10) {
				if (GetCursor() != curs)
					SetCursor(curs);

				ResizeTileArea();
				break;
			}
			else {
				if (GetCursor() == curs)
					SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			
			return DefWindowProc(hwnd, msg, wparm, lparm);
		}

	case WM_NCMOUSEMOVE:
		{
			int nchit = (int)wparm;
			POINTS pts = MAKEPOINTS(lparm);
			HCURSOR curs = LoadCursor(NULL, IDC_SIZENS);
			RECT rc;

			GetWindowRect(hwnd, &rc);

			if (nchit == HTBORDER && abs(rc.bottom - pts.y) < 10) {
				if (GetCursor() != curs)
					SetCursor(curs);
			}
			else {
				if (GetCursor() == curs)
					SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			break;
		}

	default:
		break;
	}
	return CallWindowProc(oldproc, hwnd, msg, wparm, lparm);
}

int CALLBACK MalletEditFrameProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map = NULL;
	HWND mdi = GetDlgItem(hwnd, ID_MDICLIENT);
	BOOL foo;
	
	if (map_list) {
		map = get_map((HWND)SendDlgItemMessage(hwnd, ID_MDICLIENT, WM_MDIGETACTIVE, 0, (LPARAM)&foo));
		if (!map && map_list)
			MessageBox(NULL, "No map active, but map list is not empty", "Report bug immediately", MB_OK);
			
	}
	
	
    
	switch(msg) {
		// each case will have its own scope
    case WM_CREATE:
		{
			WNDCLASS wc;
			HWND tmp;
			CLIENTCREATESTRUCT ccs;
			HDC screen = GetDC(NULL);
			
			frameWnd = hwnd;
			create_shell_toolbar();
			
			arialFont = CreateFont(-MulDiv(10, GetDeviceCaps(screen, LOGPIXELSY), 72),0,0,0,0,0,0,0,0,0,0,0,0,"Arial");
			ReleaseDC(NULL, screen);
			
			
			memset(&wc, 0, sizeof(wc));
			wc.style = CS_OWNDC|CS_BYTEALIGNCLIENT|CS_BYTEALIGNWINDOW;
			wc.lpfnWndProc = (WNDPROC)MapAreaProc;
			wc.lpszClassName = "MalletEdit_maparea";
			wc.hCursor = NULL;
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
			wc.hInstance = hInstance;
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)TileAreaProc;
			wc.lpszClassName = "MalletEdit_tilearea";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpfnWndProc = (WNDPROC)AnimAreaProc;
			wc.lpszClassName = "MalletEdit_animarea";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			/*
			wc.lpfnWndProc = (WNDPROC)OverviewProc;
			wc.lpszClassName = "MalletEdit_ov";
			wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
			if (!RegisterClass(&wc)) DestroyWindow(hwnd);
			*/
			wc.lpfnWndProc = (WNDPROC)InfoAreaProc;
			wc.lpszClassName = "MalletEdit_infoarea";
			wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)CustomizeListProc;
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.lpszClassName = "MalletEdit_customizel";
			wc.style |= CS_DBLCLKS;
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.style &= ~CS_DBLCLKS;
			wc.lpfnWndProc = (WNDPROC)CustomizeEditProc;
			wc.lpszClassName = "MalletEdit_customizee";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)CustomizeEditFrameProc;
			wc.lpszClassName = "MalletEdit_customizeframe";
			wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			memset(&wc, 0, sizeof(wc));
			wc.style = CS_OWNDC|CS_BYTEALIGNCLIENT|CS_BYTEALIGNWINDOW;
			wc.cbClsExtra = wc.cbWndExtra = 0;
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wc.hInstance = hInstance;
			wc.lpfnWndProc = (WNDPROC)WarpToolAreaProc;
			wc.lpszClassName = "MalletEdit_warptoolarea";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)WarpAnimsProc;
			wc.lpszClassName = "MalletEdit_warpanims";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)MiniOverviewProc;
			wc.lpszClassName = "MalletEdit_miniview";
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}
			wc.lpfnWndProc = (WNDPROC)OVDataProc;
			wc.lpszClassName = "MalletEdit_ovdata";
			wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
			if (!RegisterClass(&wc)) {
				MessageBox(NULL, "Failed to register child classes.", "Error", MB_OK);
				DestroyWindow(hwnd);
			}

			
			
			// MDI Area
			memset(&ccs, 0, sizeof(ccs));
			ccs.idFirstChild = 20000;
			ccs.hWindowMenu = GetSubMenu(hMenu, 2);
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "MDICLIENT", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_HSCROLL|WS_VSCROLL|MDIS_ALLCHILDSTYLES,
				0, 32, 1, 1,
				hwnd, (HMENU)ID_MDICLIENT, hInstance, (LPVOID)&ccs);
			ShowWindow(tmp, SW_SHOW);

			{ // subclass scope
				WNDPROC oldproc;

				oldproc = (WNDPROC)SetWindowLong(GetDlgItem(hwnd, ID_MDICLIENT), GWL_WNDPROC, (LONG)MalletEditMDISubclassProc);
				SetWindowLong(GetDlgItem(hwnd, ID_MDICLIENT), GWL_USERDATA, (LONG)oldproc);
			}
			
			// Customize Frame Area
			
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_customizeframe", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
				0, 32, 1, 1,
				hwnd, (HMENU)ID_CFRAME, hInstance, (LPVOID)&ccs);
			ShowWindow(tmp, SW_HIDE);
			
			
			// Anim Area 
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_animarea", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VSCROLL, 0, 32, 200, 300,
				hwnd, (HMENU)ID_ANIMATIONS, hInstance, NULL);
			ShowWindow(tmp, SW_SHOW);
			
			// Tile Area 
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_tilearea", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_HSCROLL|WS_VSCROLL, 0, 32, 200, 300,
				hwnd, (HMENU)ID_TILEAREA, hInstance, NULL);
			ShowWindow(tmp, SW_SHOW);
			
			// Info Area
			tmp = CreateWindow("MalletEdit_infoarea", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 32, 200, 300,
				hwnd, (HMENU)ID_INFOAREA, hInstance, NULL);
			ShowWindow(tmp, SW_SHOW);

			// Mini overview
			tmp = CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_miniview", NULL,
				WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 0, 0, 0,
				hwnd, (HMENU)ID_MINIVIEW, hInstance, NULL);
			ShowWindow(tmp, (miniOverview? SW_SHOW : SW_HIDE));
			if (miniOverview) SetTimer(tmp, TIMER_OVUPDATE, 3000, NULL);

			
			// Status Bar
			tmp = CreateWindow(STATUSCLASSNAME, NULL,
				WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
				0, 0, 0, 0, 
				hwnd, (HMENU)ID_STATUS, hInstance, NULL);
			
			// note: don't worry about x,y,w,h values; set in WM_SIZE
			
			break;
		}
		
    case WM_SIZE:
		{
			int width = LOWORD(lparm);
			int height = HIWORD(lparm);
			
			SetWindowPos(GetDlgItem(hwnd, ID_TOOLBAR), NULL, 0, 0, width, 32,
				SWP_SHOWWINDOW|SWP_NOREPOSITION);
			SetWindowPos(GetDlgItem(hwnd, ID_TILEAREA), NULL, 0, height-applayout.tw_height-20, width-130, applayout.tw_height,
				SWP_SHOWWINDOW|SWP_NOREPOSITION);
			SetWindowPos(GetDlgItem(hwnd, ID_INFOAREA), NULL, width-130, height-130-20, 130, 130,
				SWP_SHOWWINDOW|SWP_NOREPOSITION);
			if (!miniOverview)
				SetWindowPos(GetDlgItem(hwnd, ID_ANIMATIONS), NULL, width-130, 32, 130, height-130-20-32,
					SWP_SHOWWINDOW|SWP_NOREPOSITION);
			else {
				SetWindowPos(GetDlgItem(hwnd, ID_ANIMATIONS), NULL, width-130, 130+32, 130, height-130-20-32-130,
					SWP_SHOWWINDOW|SWP_NOREPOSITION);
				SetWindowPos(GetDlgItem(hwnd, ID_MINIVIEW), NULL, width-130, 32, 130, 130, SWP_SHOWWINDOW|SWP_NOREPOSITION);
			}
				

			SetWindowPos(GetDlgItem(hwnd, ID_STATUS), NULL, 0, height-20, width, 20,
				SWP_SHOWWINDOW|SWP_NOREPOSITION);
			SetWindowPos(GetDlgItem(hwnd, ID_MDICLIENT), HWND_TOP, 0, 32, width-130, height-20-32-applayout.tw_height,
				0);
			SetWindowPos(GetDlgItem(hwnd, ID_CFRAME), HWND_TOP, 0, 32, width-130, height-20-32-applayout.tw_height,
				SWP_NOREPOSITION);
			
			{
				int sizes[5] = {
					width/5, width/5*2, width/5*3, width/5*4, -1
				};
				
				SendDlgItemMessage(hwnd, ID_STATUS, SB_SETPARTS, (WPARAM)5, (LPARAM)sizes);
			}


			GetAppLayout(hwnd);

			break;
		}
		
    case WM_DESTROY:
		{
			char f[256];
			GetCurrentDirectory(sizeof(f), f);
			strcpy(lastdir, f);
			GetAppLayout(hwnd);
			frameKilled = TRUE;
			PostQuitMessage(0);
			break;
		}
		
    case WM_TBCOMMAND:
		{
			int command = wparm;
			
			
			
			switch (command) {
			case COMM_NEW:
				CreateNewMap();
				break;
				
			case COMM_SAVE:
			case COMM_SAVEAS:
				{
					BOOL saveas = FALSE, result = FALSE;
					OPENFILENAME ofn;
					char file[256];
					char dir[256];
					
					if (!map) break;
					
					if (command == COMM_SAVEAS || !map->file) saveas = TRUE;
					
					memset(&ofn, 0, sizeof(ofn));
					memset(file, 0, sizeof(file));
					memset(dir, 0, sizeof(dir));
					
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
					
					if (saveas) {
						if (GetSaveFileName(&ofn))
							result = SaveMap(file, map);
					}
					else
						result = SaveMap(map->file, map);
					
					break;
				}
				
				
			case COMM_OPEN:
				{
					OPENFILENAME ofn;
					char file[256];
					char dir[256];
					
					memset(file, 0, sizeof(file));
					memset(&ofn, 0, sizeof(ofn));
					memset(dir, 0, sizeof(dir));
					
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
					
					if (GetOpenFileName(&ofn))
						OpenMap(file);

					break;
				}
				
			case COMM_INFO:
				{			
					if (!map) break;
					infomap = map;
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAPINFO), hwnd, MapInfoProc);				
					break;
				}
				
			case COMM_EDIT:
				currentTool = TOOL_EDIT;         
				break;
				
			case COMM_SELECT:
				currentTool = TOOL_SELECT;
				break;
				
			case COMM_FILL:
				currentTool = TOOL_FILL;
				break;

			case COMM_PICKER:
				currentTool = TOOL_PICKER;
				break;
				
			case COMM_HOLDING_PEN:
				{
					HMENU popup;
					MENUITEMINFO mii;
					POINT pt;
					int result;
					
					if (hTeamSelected == 4) {
						MessageBox(NULL, "You may not make a NEUTRAL holding pen.\nSelect a different team in the info box.", "Holding Pen Tool", MB_OK);
						break;
					}
					
					GetCursorPos(&pt);
					
					popup = CreatePopupMenu();
					
					memset(&mii, 0, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID|MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = "Normal tile";
					mii.cch = strlen("Normal tile");
					mii.wID = 1;
					InsertMenuItem(popup, 0, TRUE, &mii);
					mii.dwTypeData = "Animated tiles";
					mii.cch = strlen("Animated tiles");
					mii.wID = 2;
					InsertMenuItem(popup, 1, TRUE, &mii);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					if (result < 1 || result > MAX_HOLDING_PEN_TYPES)
						break;
					
					DestroyMenu(popup);
					
					currentTool = TOOL_HOLDING_PEN;
					hHoldingType = result - 1;
				}
				break;
				
			case COMM_BRIDGE:
				{
					MENUITEMINFO mii;
					HMENU popup;
					HBITMAP *bmp;
					HDC dc;
					RECT rc;
					POINT pt;
					int x, z, result;
					
					
					popup = CreatePopupMenu();
					
					memset(&mii, 0, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID|MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = "Left and Right";
					mii.cch = strlen("Left and Right");
					mii.wID = 1;
					if (MAX_BRIDGE_LR_TYPES)
						InsertMenuItem(popup, 0, TRUE, &mii);
					mii.dwTypeData = "Up and Down";
					mii.cch = strlen("Up and Down");
					mii.wID = 2;
					if (MAX_BRIDGE_UD_TYPES)
						InsertMenuItem(popup, 1, TRUE, &mii);
					
					if (MAX_BRIDGE_UD_TYPES + MAX_BRIDGE_LR_TYPES == 0)
						break;
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					DestroyMenu(popup);
					
					if (result < 1)
						break;
					
					hBridgeDir = result-1;
					
					popup = CreatePopupMenu();
					dc = CreateCompatibleDC(tilesDC);
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES));
					for (x = 0; x < (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES); x++) {
						if (hBridgeDir) { /* up and down */
							bmp[x] = CreateCompatibleBitmap(tilesDC, 80, 48);
							bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
							
							rc.left = rc.top = 0;
							rc.right = 80;
							rc.bottom = 48;
							
							FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
							
							for (z = 0; z < 15; z++) {
								WORD tile = bridge_ud_data[x][z];
								if (bridge_ud_data[x][z] < 0)
									continue;
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%5)*16, (z/5)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%5)*16, (z/5)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%5)*16, (z/5)*16, "a", 1);
								}
							}
						}
						else {
							bmp[x] = CreateCompatibleBitmap(tilesDC, 48, 80);
							bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
							
							rc.left = rc.top = 0;
							rc.right = 48;
							rc.bottom = 80;
							
							FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
							
							for (z = 0; z < 15; z++) {
								WORD tile = bridge_lr_data[x][z];
								if (bridge_lr_data[x][z] < 0)
									continue;
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%3)*16, (z/3)*16, "a", 1);
								}
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
					}
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES); x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1 || result > (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES))
						break;
					
					hBridgeType = result-1;
					currentTool = TOOL_BRIDGE;
					
					DrawToolbar();
					break;
				}

			case COMM_CONV:
				{
					MENUITEMINFO mii;
					HMENU popup;
					HBITMAP *bmp;
					HDC dc;
					RECT rc;
					POINT pt;
					int x, z, result;
					
					
					popup = CreatePopupMenu();
					
					memset(&mii, 0, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID|MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = "Left and Right";
					mii.cch = strlen("Left and Right");
					mii.wID = 1;
					if (MAX_CONV_LR_TYPES)
						InsertMenuItem(popup, 0, TRUE, &mii);
					mii.dwTypeData = "Up and Down";
					mii.cch = strlen("Up and Down");
					mii.wID = 2;
					if (MAX_CONV_UD_TYPES)
						InsertMenuItem(popup, 1, TRUE, &mii);
					
					if (MAX_CONV_LR_TYPES + MAX_CONV_UD_TYPES == 0)
						break;
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					DestroyMenu(popup);
					
					if (result < 1)
						break;

					hConvDir = result-1;
					
					popup = CreatePopupMenu();
					dc = CreateCompatibleDC(tilesDC);
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * (hConvDir?MAX_CONV_UD_TYPES:MAX_CONV_LR_TYPES));
					for (x = 0; x < (hConvDir?MAX_CONV_UD_TYPES:MAX_CONV_LR_TYPES); x++) {
						if (hConvDir) { /* up and down */
							bmp[x] = CreateCompatibleBitmap(tilesDC, 32, 64);
							bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
							
							rc.left = rc.top = 0;
							rc.right = 32;
							rc.bottom = 64;
							
							FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
							
							for (z = 0; z < 15; z++) {
								WORD tile = conv_ud_data[x][z];
								if (conv_ud_data[x][z] < 0)
									continue;
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%2)*16, (z/2)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%2)*16, (z/2)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%2)*16, (z/2)*16, "a", 1);
								}
							}
						}
						else {
							bmp[x] = CreateCompatibleBitmap(tilesDC, 64, 32);
							bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
							
							rc.left = rc.top = 0;
							rc.right = 64;
							rc.bottom = 32;
							
							FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
							
							for (z = 0; z < 15; z++) {
								WORD tile = conv_lr_data[x][z];
								if (conv_lr_data[x][z] < 0)
									continue;
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%4)*16, (z/4)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%4)*16, (z/4)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%4)*16, (z/4)*16, "a", 1);
								}
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
					}
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES); x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1 || result > (hBridgeDir?MAX_BRIDGE_UD_TYPES:MAX_BRIDGE_LR_TYPES))
						break;
					
					hConvType = result-1;
					currentTool = TOOL_CONV;
					
					DrawToolbar();
					break;
				}
				
			case COMM_WARP:
				{
					int returnval; /* if 0, they clicked cancel */

					if (map) infomap = map;
					else break;
				
					returnval = DialogBox(hInstance, MAKEINTRESOURCE(IDD_WARPTOOL), hwnd, (DLGPROC)WarpToolDlgProc);
					if (returnval)
						currentTool = TOOL_WARP;
					else
						BringWindowToTop(hwnd);

					infomap = NULL;
					
					DrawToolbar();
					
					break;
				}

			case COMM_SWITCH:
				{
					HMENU popup;
					MENUITEMINFO mii;
					POINT pt;
					int result, x, z, tot;
					HBITMAP *bmp;
					HDC dc;

					if (!map) break;

					if (MAX_SWITCH_TYPES < 1) {
						MessageBox(NULL, "There are currently no switch types defined.\nUpdate your custom data in settings", "MalletEdit Switch Tool", MB_OK);
						break;
					}

					LookOverFlags(map);

					for (tot = map->header.neutralCount, x = 0; x < map->header.numTeams; x++)
						tot += map->header.flagCount[x] + map->header.flagPoleCount[x];

					if (tot > 0 && map->header.objective != OBJECTIVE_SWITCH) {
						MessageBox(NULL, "There are currently flags placed on the map.\nIf you decide to place a switch in the map area, MalletEdit will automatically change\n"
							             "the game objective to \"switch game.\" Switches and flags are not compatible\n"
										 "If a game is of flag type, switches are ignored, and if a game is of switch type, flags are ignored.", "MalletEdit Switch Tool", MB_OK);
					}
				
					
					GetCursorPos(&pt);
					
					popup = CreatePopupMenu();
												
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * MAX_SWITCH_TYPES);
					dc = CreateCompatibleDC(tilesDC);
					for (x = 0; x < MAX_SWITCH_TYPES; x++) {
						RECT rc;
						rc.left = rc.top = 0;
						rc.right = rc.bottom = 48;
						bmp[x] = CreateCompatibleBitmap(tilesDC, 48, 48);
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
						
						for (z = 0; z < 9; z++) {
							WORD tile = switch_data[x][z];
							if (switch_data[x][z] > -1) {
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%3)*16, (z/3)*16, "a", 1);
								}
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
						
					}		
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < MAX_SWITCH_TYPES; x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1 || result > MAX_SWITCH_TYPES) 
						currentTool = TOOL_EDIT;
					else {
						currentTool = TOOL_SWITCH;
						hSwitchType = result - 1;
					}
					
					DrawToolbar();		
				}
				
				break;
				
				
			case COMM_BUNKER:
				{
					HMENU popup;
					MENUITEMINFO mii;
					POINT pt;
					int result, x, z;
					HBITMAP *bmp;
					HDC dc;
					
					GetCursorPos(&pt);
					
					popup = CreatePopupMenu();
					
					memset(&mii, 0, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID|MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = "Left";
					mii.cch = strlen("Left");
					mii.wID = 1;
					InsertMenuItem(popup, 0, TRUE, &mii);
					mii.dwTypeData = "Right";
					mii.cch = strlen("Right");
					mii.wID = 2;
					InsertMenuItem(popup, 1, TRUE, &mii);
					mii.dwTypeData = "Up";
					mii.cch = strlen("Up");
					mii.wID = 3;
					InsertMenuItem(popup, 2, TRUE, &mii);
					mii.dwTypeData = "Down";
					mii.cch = strlen("Down");
					mii.wID = 4;
					InsertMenuItem(popup, 3, TRUE, &mii);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					DestroyMenu(popup);
					
					if (result < 1)
						break;
					
					
					popup = CreatePopupMenu();
					
					hBunkerDir = result - 1;
					
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * MAX_BUNKER_TYPES/4);
					dc = CreateCompatibleDC(tilesDC);
					for (x = 0; x < MAX_BUNKER_TYPES/4; x++) {
						RECT rc;
						rc.left = rc.top = 0;
						rc.right = rc.bottom = 64;
						bmp[x] = CreateCompatibleBitmap(tilesDC, 64, 64);
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
						
						for (z = 0; z < 16; z++) {
							WORD tile = bunker_data[x*4+hBunkerDir][z];
							if (bunker_data[x*4+hBunkerDir][z] > -1) {
								if (!(tile & 0x8000))
									DrawTileBlock(tile, dc, (z%4)*16, (z/4)*16, 1, 1, FALSE);
								else {
									struct Animation *anim;
									anim = get_anim((unsigned char)((tile & 0xFF)));
									if (anim && anim->frames)
										DrawTileBlock(anim->frames[0], dc, (z%4)*16, (z/4)*16, 1, 1, FALSE);
									else
										TextOut(dc, (z%4)*16, (z/4)*16, "a", 1);
								}
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
						
					}		
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < MAX_BUNKER_TYPES/4; x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1 || result > MAX_BUNKER_TYPES/4) 
						currentTool = TOOL_EDIT;
					else {
						currentTool = TOOL_BUNKER;
						hBunkerType = result - 1;
					}
					
					DrawToolbar();		
				}
				
				break;
				
			case COMM_SPAWN:
				{
					HMENU popup;
					HBITMAP *bmp;
					MENUITEMINFO mii;
					POINT pt;
					RECT rc;
					int x, z, result;
					HDC dc;
					
					if (hTeamSelected > 3) {
						MessageBox(NULL, "You cannot make NEUTRAL Spawns.\nSelect a different team.", "Spawn Tool", MB_OK);
						break;
					}
					
					
					dc = CreateCompatibleDC(tilesDC);
					
					popup = CreatePopupMenu();
					
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * MAX_SPAWN_TYPES/4);
					for (x = 0; x < MAX_SPAWN_TYPES/4; x++) {
						bmp[x] = CreateCompatibleBitmap(tilesDC, 48, 48);
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						rc.left = rc.top = 0;
						rc.bottom = rc.right = 48;
						
						FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
						
						for (z = 0; z < 9; z++) {
							WORD tile = spawn_data[hTeamSelected+x*4][z];
							if (spawn_data[hTeamSelected+x*4][z] < 0) continue;
							if (!(tile & 0x8000))
								DrawTileBlock(tile, dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
							else {
								struct Animation *anim;
								anim = get_anim((unsigned char)(tile & 0xFF));
								if (anim && anim->frames)
									DrawTileBlock(anim->frames[0], dc, (z%3)*16, (z/3)*16, 1, 1, FALSE);
								else
									TextOut(dc, (z%3)*16, (z/3)*16, "a", 1);
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);				
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
						
					}
					
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < MAX_SPAWN_TYPES/4; x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1 || result > MAX_SPAWN_TYPES/4)
						break;
					else {
						currentTool = TOOL_SPAWN;
						hSpawnType = result-1;
					}
					
					DrawToolbar();
					break;
				}				
				
			case COMM_WALL:
			case COMM_LINE:
				{
					HMENU popup;
					HBITMAP *bmp;
					MENUITEMINFO mii;
					POINT pt;
					int x, z, result, tool;
					HDC dc = CreateCompatibleDC(tilesDC);

					switch (command) {
						case COMM_WALL:
							tool = TOOL_WALL;
							break;

						case COMM_LINE:
							tool = TOOL_LINE;
							break;
							
						default:
							tool = -1;
					}
					if (tool < 0) break;

					
					popup = CreatePopupMenu();
					
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * MAX_WALL_TYPES);
					
					for (x = 0; x < MAX_WALL_TYPES; x++) {
						RECT rc;
						rc.left = rc.top = 0;
						rc.right = MAX_WALL_CONNS*16;
						rc.bottom = 16;
						bmp[x] = CreateCompatibleBitmap(tilesDC, MAX_WALL_CONNS*16, 16);
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);
						
						FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
						
						for (z = 0; z < MAX_WALL_CONNS; z++) {
							WORD tile = wall_data[x][z];
							if (wall_data[x][z] < 0) continue;
							if (!(tile & 0x8000))
								DrawTileBlock(tile, dc, z*16, 0, 1, 1, FALSE);
							else {
								struct Animation *anim;
								anim = get_anim((unsigned char)((tile & 0xFF)));
								if (anim && anim->frames)
									DrawTileBlock(anim->frames[0], dc, z*16, 0, 1, 1, FALSE);
								else
									TextOut(dc, z*16, 0, "a", 1);
							}
						}
						
						bmp[x] = (HBITMAP)SelectObject(dc, bmp[x]);				
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[x];
						mii.wID = x + 1;
						
						InsertMenuItem(popup, x, TRUE, &mii);
						
					}
					
					
					
					GetCursorPos(&pt);
					
					result = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
					for (x = 0; x < MAX_WALL_TYPES; x++)
						DeleteObject(bmp[x]);
					free(bmp);
					
					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (result < 1|| result > MAX_WALL_TYPES) 
						break;
					else {
						currentTool = tool;
						hWallType = result - 1;
					}
					
					DrawToolbar();		
				}
				break;
				
			case COMM_FLAG:
			case COMM_POLE:
				{
					HMENU popup;
					MENUITEMINFO mii;
					HBITMAP *bmp;
					HDC dc;
					int n, z, tile, result, max;
					struct Animation *anim;
					POINT pt;
					int **data;
					RECT rc;

					if (!map) break;
					
					if (command == COMM_POLE) {
						max = MAX_POLE_TYPES;
						data = pole_data;
					}
					else {
						max = MAX_FLAG_TYPES;
						data = flag_data;
					}

					LookOverFlags(map);
					if (map->header.switchCount > 0 && map->header.objective != OBJECTIVE_FLAG) {
						MessageBox(NULL, "There are currently switches placed on the map.\nIf you decide to place a switch in the map area, MalletEdit will automatically change\n"
							             "the game objective to \"flag game.\" Switches and flags are not compatible\n"
										 "If a game is of flag type, switches are ignored, and if a game is of switch type, flags are ignored.", "MalletEdit Switch Tool", MB_OK);
					}

					
					bmp = (HBITMAP *)malloc(sizeof(HBITMAP) * max);
					
					popup = CreatePopupMenu();
					dc = CreateCompatibleDC(tilesDC);
					
					for (n = 0; n < max; n++) {
						bmp[n] = CreateCompatibleBitmap(tilesDC, 48, 48);
						bmp[n] = (HBITMAP)SelectObject(dc, bmp[n]);
						
						rc.top = 0; rc.bottom = 48;
						rc.left = 0; rc.right = 48;
						FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
						
						
						for (z = 0; z < 9; z++) {
							tile = data[n][z];
							anim = NULL;
							if (data[n][z] < 0) continue;
							if (tile & 0x8000)
								anim = get_anim((unsigned char)(tile & 0xFF));
							if (anim && anim->frameCount > 0)
								DrawTileBlock(anim->frames[0], dc, (z%3) * 16, (z/3) * 16, 1, 1, FALSE);
							else if (anim && !(tile & 0x80000000))
								TextOut(dc, (z%3) * 16, (z/3) * 16, "a", 1);
							else if (tile > 0)
								DrawTileBlock(tile, dc, (z%3) * 16, (z/3) * 16, 1, 1, FALSE);
						}
						
						bmp[n] = (HBITMAP)SelectObject(dc, bmp[n]);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_TYPE;
						mii.fType = MFT_BITMAP;
						mii.dwTypeData = (char *)bmp[n];
						mii.wID = n+1;
						
						InsertMenuItem(popup, n, TRUE, &mii);
					}
					
					
					GetCursorPos(&pt);
					result = TrackPopupMenu(popup, TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTALIGN|TPM_TOPALIGN, pt.x, pt.y, 0, hwnd, NULL);
					for (n = 0; n < max; n++)
						DeleteObject(bmp[n]);
					free(bmp);

					DestroyMenu(popup);
					DeleteDC(dc);
					
					if (command == COMM_FLAG) {
						if (result == 0) 
							currentTool = TOOL_EDIT;
						else {
							currentTool = TOOL_FLAG;
							hFlagType = result - 1;
						}
					}
					else {
						if (result == 0)
							currentTool = TOOL_EDIT;
						else {
							currentTool = TOOL_POLE;
							hPoleType = result - 1;
						}
					}
					
					DrawToolbar();
					
					break;
				}
				
			case COMM_SELECTALL:
				if (map) {
					map->selection.tile = 0;
					map->selection.vert = map->header.height;
					map->selection.horz = map->header.width;
					RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				break;
				
			case COMM_ROTATE:
				if (map) {
					RotateBits(map);
					RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				break;
				
			case COMM_MIRROR:
				{
					HMENU popupMenu = CreatePopupMenu();
					POINT curPos;
					MENUITEMINFO mii;
					int selection;
					
					if (!popupMenu && map) 
						MirrorBits(0, map);
					else if (map) {
						GetCursorPos(&curPos);
						
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_TYPE|MIIM_ID;
						mii.dwTypeData = "Left";
						mii.wID = 1;
						mii.cch = 4;
						InsertMenuItem(popupMenu, 0, TRUE, &mii);
						mii.dwTypeData = "Right";
						mii.wID = 2;
						mii.cch = 5;
						InsertMenuItem(popupMenu, 1, TRUE, &mii);
						mii.dwTypeData = "Up";
						mii.wID = 3;
						mii.cch = 2;
						InsertMenuItem(popupMenu, 2, TRUE, &mii);
						mii.dwTypeData = "Down";
						mii.wID = 4;
						mii.cch = 4;
						InsertMenuItem(popupMenu, 3, TRUE, &mii);
						
						selection = TrackPopupMenu(popupMenu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NONOTIFY,
                            curPos.x, curPos.y,
                            0, hwnd, NULL);
						if (!selection) {
							currentTool = TOOL_EDIT;
							DrawToolbar();
							break;
						}
						
						MirrorBits(selection-1, map);
						DestroyMenu(popupMenu);
					}
					
					if (map) RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					break;
				}
				
			case COMM_COPY:
				if (map) copyBits(FALSE, map);
				break;
				
			case COMM_CUT:
				if (map) {
					copyBits(TRUE, map);
					RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				break;
				
			case COMM_PASTE:
				if (map) {
					pasteBits(map);
					RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				}
				break;
				
				
			case COMM_HELP:
				{
					char dir[256];
					memset(dir, 0, sizeof(dir));
					
					GetCurrentDirectory(sizeof(dir), dir);
					
					ShellExecute(hwnd, "open", "MalletEdit.hlp", NULL, dir, SW_SHOW);
					break;
				}
				
			case COMM_DEBUG:
				MessageBox(NULL, lastdir, "Current Working Directory", MB_OK);
				break;
				
			default:
				break;
      }
      break;
    }
	
    case WM_INITMENUPOPUP:
		{
			HMENU subMenu = (HMENU)wparm;
			int nPos = LOWORD(lparm);
			BOOL sysMenu = (BOOL)HIWORD(lparm);
			
			MENUITEMINFO mii;
			
			if (sysMenu || GetMenuItemCount(hMenu) > 4) 
				nPos--;
			
			switch (nPos) {
			case 0: // File Menu
				memset(&mii, 0, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				
				if (map)
					mii.fState = MFS_ENABLED;
				else
					mii.fState = MFS_GRAYED;
					
				
				SetMenuItemInfo(subMenu, COMM_SAVE, FALSE, &mii);
				SetMenuItemInfo(subMenu, COMM_SAVEAS, FALSE, &mii);
				
				break;
				
			case 1: // Edit Menu
				memset(&mii, 0, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				if (map)
					mii.fState = MFS_ENABLED;
				else 
					mii.fState = MFS_GRAYED;
					

				{
					int x;
					for (x = 0; x < GetMenuItemCount(subMenu); x++) {
						if (GetMenuState(subMenu, x, MF_BYPOSITION) == MF_SEPARATOR) continue;
						SetMenuItemInfo(subMenu, x, TRUE, &mii);
					}
				}
				
				if (map && !map->redo_list) {
					char buf[100];
					mii.fState = MFS_GRAYED;
					mii.fMask = MIIM_STATE|MIIM_TYPE;
					mii.cch = 5;
					sprintf(buf, "&Redo");
					mii.dwTypeData = buf;
					mii.fType = MFT_STRING;
					SetMenuItemInfo(subMenu, COMM_REDO, FALSE, &mii);
				}
				else if (map) {
					char buf[100];
					sprintf(buf, "&Redo <%s>\t(%d)", undo_types[map->redo_list->type], count_list(map->redo_list));
					mii.fState = MFS_ENABLED;
					mii.dwTypeData = buf;
					mii.fMask |= MIIM_TYPE;
					mii.cch = string_len(buf);
					mii.fType = MFT_STRING;
					SetMenuItemInfo(subMenu, COMM_REDO, FALSE, &mii);
				}
				if (map && !map->undo_list) {
					char buf[100];
					mii.fState = MFS_GRAYED;
					mii.fMask = MIIM_STATE|MIIM_TYPE;
					mii.cch = 5;
					sprintf(buf, "&Undo");
					mii.dwTypeData = buf;
					mii.fType = MFT_STRING;
					SetMenuItemInfo(subMenu, COMM_UNDO, FALSE, &mii);
				}
				else if (map) {
					char buf[100];
					sprintf(buf, "&Undo <%s>\t(%d)", undo_types[map->undo_list->type], count_list(map->undo_list));
					mii.cch = string_len(buf);
					mii.fState = MFS_ENABLED;
					mii.fMask |= MIIM_TYPE;
					mii.dwTypeData = buf;
					mii.fType = MFT_STRING;
					SetMenuItemInfo(subMenu, COMM_UNDO, FALSE, &mii);
				}
				mii.fMask = MIIM_STATE;
				
				// gray stuff that requires selection if there isn't one
				if (map && (map->selection.tile == -1 || !map->selection.horz || !map->selection.vert)) {
					memset(&mii, 0, sizeof(mii));
					mii.fMask = MIIM_STATE;
					mii.fState = MFS_GRAYED;
					mii.cbSize = sizeof(mii);
					
					SetMenuItemInfo(subMenu, COMM_DELETE, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_ROTATE, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_MIRROR, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_COPY, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_CUT, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_PASTE, FALSE, &mii);
					SetMenuItemInfo(subMenu, COMM_FILLSEL, FALSE, &mii);
				}
				
				break;
				
			case 2: // window menu
				if (map) {
					memset(&mii, 0, sizeof(mii));
					mii.fMask = MIIM_STATE;
					mii.fState = MFS_ENABLED;
					mii.cbSize = sizeof(mii);
				}
				else {
					memset(&mii, 0, sizeof(mii));
					mii.fMask = MIIM_STATE;
					mii.fState = MFS_GRAYED;
					mii.cbSize = sizeof(mii);
				}
				
				SetMenuItemInfo(subMenu, COMM_TILEH, FALSE, &mii);
				SetMenuItemInfo(subMenu, COMM_TILEV, FALSE, &mii);
				SetMenuItemInfo(subMenu, COMM_CASCADE, FALSE, &mii);
				break;
				
			case 3: // help menu
				break;
				
				
			default:
				break;
      }
      break;
    }
	
    case WM_STMESSAGE:
		{
			UINT mouseMsg = (UINT)lparm;
			
			if (mouseMsg == WM_LBUTTONDBLCLK) {
				ShowWindow(hwnd, SW_SHOW);
				OpenIcon(hwnd);
				
				Shell_NotifyIcon(NIM_DELETE, &nid);
				break;
			}
			return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
		}
		
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
			case SC_MINIMIZE:
				CloseWindow(hwnd);
				if (minSysTray) {
					memset(&nid, 0, sizeof(nid));
					nid.cbSize = sizeof(nid);
					sprintf(nid.szTip, "MalletEdit v2");
					nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
					nid.uCallbackMessage = WM_STMESSAGE;
					nid.uID = 604;
					nid.uFlags = NIF_MESSAGE|NIF_TIP|NIF_ICON;
					nid.hWnd = hwnd;
					
					Shell_NotifyIcon(NIM_ADD, &nid);
					ShowWindow(hwnd, SW_HIDE);
				}
				break;            
				
			case SC_CLOSE:
				{
					struct MapInfo *next;
					BOOL break2 = FALSE;
					for (map = map_list; map; map = next) {
						next = map->next;
						if (map->modify) {
							char buf[256];
							sprintf(buf, "Would you like to save your changes to %s?", map->header.name);
							result = MessageBox(NULL, buf,
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
								
								break;
								
							case IDCANCEL:
								break2 = TRUE;
								break;
								
							case IDNO:
							default:
								break;
							}
							if (break2) break;
						}
					}
					if (!break2) {
						while (map_list) killMap(map_list);
						return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
					}
					else return 0;
				}
				
				
			default:
				return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
			}
			return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
		}
		
    case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wparm);
			WORD wId = LOWORD(wparm);
			HWND hwndCtl = (HWND)lparm;


			switch (wId) {
				case COMM_NEW:
					CreateNewMap();
					break;

				case COMM_INFO:
					SendMessage(hwnd, WM_TBCOMMAND, (WPARAM)COMM_INFO, 0);
					break;

				case COMM_SAVE:
				case COMM_SAVEAS:
					{
						BOOL saveas = FALSE, result = FALSE;
						OPENFILENAME ofn;
						char file[256];
						char dir[256];

						if (!map) break;

						if (wId == COMM_SAVEAS || !map->file) saveas = TRUE;

						memset(&ofn, 0, sizeof(ofn));
						memset(file, 0, sizeof(file));
						memset(dir, 0, sizeof(dir));

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

						if (saveas) {
							if (GetSaveFileName(&ofn))
								result = SaveMap(file, map);
						}
						else
							result = SaveMap(map->file, map);

						if (result) map->modify = FALSE;
						break;
					}

				case COMM_OPEN:
					{
						OPENFILENAME ofn;
						char file[256];
						char dir[256];

						memset(file, 0, sizeof(file));
						memset(&ofn, 0, sizeof(ofn));
						memset(dir, 0, sizeof(dir));

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
						if (GetOpenFileName(&ofn))
							OpenMap(file);

						break;
					}

				case COMM_EXIT:
					PostMessage(hwnd, WM_SYSCOMMAND, (WPARAM)SC_CLOSE, 0);
					break;

				case COMM_FILLSEL:
					if (map) {
						FillSelection(map);
						RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				case COMM_SELECTALL:
					if (map) {
						map->selection.tile = 0;
						map->selection.vert = map->header.height;
						map->selection.horz = map->header.width;
						RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				case COMM_OVERVIEW:

					if (!map) break;


					if (overDlg) {
						SetFocus(overDlg);
						BringWindowToTop(overDlg);
						SetForegroundWindow(overDlg);
					}
					else
						overDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_OVERVIEW), hwnd, (DLGPROC)OverviewProc);
					break;

				case COMM_UNDO:
					if (map) {
						if (performUndo(map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					}
					break;

				case COMM_REDO:
					if (map) {
						if (performRedo(map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					}
					break;

				case COMM_DELETE:
					if (map) {
						if (performDelete(map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					}
					break;

				case COMM_MIRROR:
					if (map) {
						if (MirrorBits(0, map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
					}
					break;

				case COMM_ROTATE:
					if (map) {
						if (RotateBits(map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				case COMM_COPY:
					if (map) copyBits(FALSE, map);
					break;

				case COMM_CUT:
					if (map) {
						if (copyBits(TRUE, map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				case COMM_PASTE:
					if (map) {
						if (pasteBits(map))
							RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}
					break;

				case COMM_TILEV:
					SendDlgItemMessage(hwnd, ID_MDICLIENT, WM_MDITILE, MDITILE_VERTICAL, 0);
					break;

				case COMM_TILEH:
					SendDlgItemMessage(hwnd, ID_MDICLIENT, WM_MDITILE, MDITILE_HORIZONTAL, 0);
					break;

				case COMM_CASCADE:
					SendDlgItemMessage(hwnd, ID_MDICLIENT, WM_MDICASCADE, 0, 0);
					break;

				case COMM_HELP:
					{
						char dir[256];
						memset(dir, 0, sizeof(dir));

						GetCurrentDirectory(sizeof(dir), dir);

						ShellExecute(hwnd, "open", "MalletEdit.hlp", NULL, dir, SW_SHOW);
						break;
					}

				case COMM_ABOUT:
					{

						DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutDlgProc);
						BringWindowToTop(hwnd);
						break;
					}

				default:
					break;
			} // switch(wId)

			return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
		}
	
	case WM_SYSCOLORCHANGE:
		// reload the buttons
		{
			struct toolbar_button *btn, *next;
			
			for (btn = button_list; btn; btn = next) {
				next = btn->next;
				if (btn->text) free(btn->text);
				if (btn->bmp) DeleteObject(btn->bmp);
				if (btn->disabled) DeleteObject(btn->disabled);
				free(btn);
			}
			button_list = NULL;
			init_toolbar(GetDlgItem(frameWnd, ID_TOOLBAR));
			
			DrawToolbar();
			break;
		}
		
		
    default:
		return DefFrameProc(hwnd, mdi, msg, wparm, lparm);
  }
  return 0;
}


int initMalletEdit() 
{
	if (!read_tiles("tiles.bmp")) 
	{
		MessageBox(NULL, "Failed to find tiles.bmp in the local directory.", "Error", MB_OK);
		DestroyWindow(frameWnd);
		return -1;
	}

	if (!load_anims("Gfx.dll")) {
		MessageBox(NULL, "Failed to find Gfx.dll in the local directory.", "Error", MB_OK);
		DestroyWindow(frameWnd);
		return -1;
	}
	
	InitCommonControls();
	
	load_custom_data();
	
	make_overview();
	
	memset(&tileSel, 0, sizeof(tileSel));
	tileSel.tile = -1;
	animSel.tile = -1;
	
	memset(arcdir, 0, sizeof(arcdir));
	GetCurrentDirectory(sizeof(arcdir), arcdir);

	applayout.dimensions.cx = 800;
	applayout.dimensions.cy = 580;
	applayout.position.x = 10;
	applayout.position.y = 10;
	applayout.showtype = SW_NORMAL;
	applayout.tw_height = 160;
	GetSettings();

	if (!strchr(lastdir, ':')) strcpy(lastdir, arcdir);

	if (autoAssociate) associateWithMap();

	return 0;
}

void freeMem() {
	HBITMAP bmp = NULL;
	
	bmp = (HBITMAP)SelectObject(tilesDC, bmp);
	DeleteObject(bmp);
	DeleteDC(tilesDC);
	
	DeleteObject(tilesPal);
	DestroyMenu(hMenu);
	DestroyAcceleratorTable(hAccel);
	
	while (map_list)
		killMap(map_list);
	killAnims();
	
	SaveSettings();
	
	free_custom_data(FALSE);
	
	if (arialFont) DeleteObject(arialFont);
	
	if (overViewBmpInfo) free(overViewBmpInfo);
	if (tileArrange) free(tileArrange);
}

void SaveSettings() {
	HKEY key, tmp;
	DWORD val;

#ifndef DONTALLOWCUSTOMIZE
	save_custom_data();
#endif	
	
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0, KEY_ALL_ACCESS, &tmp);
	RegCreateKeyEx(tmp, "MalletEdit v2.0", 0, "string", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &key, &val);
	
	RegSetValueEx(key, "lastdir", 0, REG_SZ, (BYTE *)lastdir, string_len(lastdir));
	
	RegSetValueEx(key, "MinimizeSystray", 0, REG_DWORD, (BYTE *)&minSysTray, 4);
	RegSetValueEx(key, "AutoPan", 0, REG_DWORD, (BYTE *)&autoPan, 4);
	RegSetValueEx(key, "AutoAssociate", 0, REG_DWORD, (BYTE *)&autoAssociate, 4);
	RegSetValueEx(key, "SaveArrangement", 0, REG_DWORD, (BYTE *)&saveArrange, 4);
	RegSetValueEx(key, "AnalWallMode", 0, REG_DWORD, (BYTE *)&analWallMode, 4);
	RegSetValueEx(key, "MiniOverview", 0, REG_DWORD, (BYTE *)&miniOverview, 4);
	RegSetValueEx(key, "AnimationsMove", 0, REG_DWORD, (BYTE *)&displayAnims, 4);
	RegSetValueEx(key, "Layout", 0, REG_BINARY, (BYTE *)&applayout, sizeof(applayout));
	
	if (saveArrange)
		RegSetValueEx(key, "ARRANGEMENT", 0, REG_BINARY, (BYTE *)tileArrange, numTiles * 2);
	
	RegCloseKey(key);
	RegCloseKey(tmp);
}

void GetSettings() {
	HKEY key, tmp;
	long type, res;
	DWORD val, sz;
	char buffer[256];
	
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0, KEY_ALL_ACCESS, &tmp);
	res = RegCreateKeyEx(tmp, "MalletEdit v2.0", 0, "string", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &key, &val);

	if (res == ERROR_SUCCESS && val != REG_CREATED_NEW_KEY) {
		sz = sizeof(lastdir);
		memset(buffer, 0, sizeof(buffer));
		res = RegQueryValueEx(key, "lastdir", 0, NULL, (BYTE *)buffer, &sz);
		if (res == ERROR_SUCCESS) {
			strcpy(lastdir, buffer);
			SetCurrentDirectory(buffer);

		}
		else *lastdir = 0;
		
		type = REG_DWORD;
		sz = 4;
		
		val = 0;
		
		if (RegQueryValueEx(key, "MinimizeSystray", 0, NULL, (BYTE*)&val, &sz) == ERROR_SUCCESS)
			minSysTray = val;
		if (RegQueryValueEx(key, "AutoPan", 0, NULL, (BYTE*)&val, &sz) == ERROR_SUCCESS)
			autoPan = val;
		if (RegQueryValueEx(key, "AutoAssociate", 0, NULL, (BYTE*)&val, &sz) == ERROR_SUCCESS)
			autoAssociate = val;
		if (RegQueryValueEx(key, "SaveArrangement", 0, NULL, (BYTE*)&val, &sz) == ERROR_SUCCESS)
			saveArrange = val;
		if (RegQueryValueEx(key, "AnalWallMode", 0, NULL, (BYTE*)&val, &sz) == ERROR_SUCCESS)
			analWallMode = val;
		if (RegQueryValueEx(key, "MiniOverview", 0, NULL, (BYTE *)&val, &sz) == ERROR_SUCCESS)
			miniOverview = val;
		if (RegQueryValueEx(key, "AnimationsMove", 0, NULL, (BYTE *)&val, &sz) == ERROR_SUCCESS)
			displayAnims = val;

		sz = sizeof(applayout);
		if (RegQueryValueEx(key, "Layout", 0, NULL, NULL, &sz) == ERROR_SUCCESS)
			RegQueryValueEx(key, "Layout", 0, NULL, (BYTE *)&applayout, &sz);
	
		if (saveArrange) {
			sz = 0;
			if (RegQueryValueEx(key, "ARRANGEMENT", 0, NULL, NULL, &sz) == ERROR_SUCCESS) {
				if (sz) {
					if (tileArrange) free(tileArrange);
					tileArrange = (WORD *)malloc(sz);
					numTiles = sz/2;
					RegQueryValueEx(key, "ARRANGEMENT", 0, NULL, (BYTE *)tileArrange, &sz);
				}
			}
		}
		
	}
	
	RegCloseKey(key);
	RegCloseKey(tmp);
}  

int associateWithMap() {
	HKEY key, temp;
	
	char *mapClass = "MalletEdit.Document";
	char *mapExt = ".map";
	
	char buf[256], buffer[256], *c, binbuf[9];
	
	memset(&binbuf, 0, sizeof(binbuf));
	binbuf[1] = 1;
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, mapExt, 0,
		KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
		
		RegSetValueEx(key, NULL, 0, REG_SZ, (BYTE *)mapClass, strlen(mapClass)+1);
		RegCloseKey(key);
		
		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, mapClass, 0,
			KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
			// just make sure it's pointing to the right place.
			
			memset(buffer, 0, sizeof(buffer));
			strncpy((char *)buffer, GetCommandLine(), sizeof(buffer));
			if ((c = strchr((char *)buffer, ' ')) != NULL)
				*c = 0;
			if (*buffer == '\"')
				strcpy((char *)buf, (char *)buffer+1);
			else
				strcpy((char *)buf, (char *)buffer);
			if (buf[MAX(0,string_len((char *)buf)-1)] == '\"')
				buf[MAX(0,string_len((char *)buf)-1)] = 0;
			strcat((char *)buf, ",0");
			RegSetValue(key, "DefaultIcon", REG_SZ,
				(LPCSTR)buf, string_len((char *)buf)+1);
			memset(&buf, 0, sizeof(buf));
			sprintf((char *)buf, "Map File");
			RegSetValueEx(key, NULL, 0, REG_SZ, (BYTE *)buf, strlen((char *)buf)+1);
			
			RegCreateKey(key, "Shell", &temp);
			RegCloseKey(key);
			RegCreateKey(temp, "open", &key);
			
			RegSetValueEx(key, "EditFlags", 0, REG_BINARY, (BYTE *)binbuf, 8);
			
			memset(buffer, 0, sizeof(buffer));
			strncpy((char *)buffer, GetCommandLine(), sizeof(buffer));
			if ((c = strchr((char *)buffer, ' ')) != NULL)
				*c = 0;
			if (*buffer == '\"')
				strcpy((char *)buf, buffer+1);
			else
				strcpy(buf, buffer);
			if (buf[MAX(0,string_len(buf)-1)] == '\"')
				buf[MAX(0,string_len(buf)-1)] = 0;
			strcat(buf, " \"%1\"");
			
			RegSetValue(key, "command", REG_SZ, buf, strlen(buf)+1);
			RegCloseKey(temp);
			RegCloseKey(key);
		}
		else {
			RegCreateKey(HKEY_CLASSES_ROOT, mapClass, &key);
			
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, GetCommandLine(), sizeof(buffer));
			if ((c = strchr(buffer, ' ')) != NULL)
				*c = 0;
			if (*buffer == '\"')
				strcpy(buf, buffer+1);
			else
				strcpy(buf, buffer);
			if (buf[MAX(0,string_len(buf)-1)] == '\"')
				buf[MAX(0,string_len(buf)-1)] = 0;
			strcat(buf, ",1");
			
			RegSetValue(key, "DefaultIcon", REG_SZ,
				buf, string_len(buf)+1);
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "Map File");
			RegSetValueEx(key, NULL, 0, REG_SZ, (BYTE *)buf, string_len(buf)+1);
			
			RegCreateKey(key, "Shell", &temp);
			RegCloseKey(key);
			RegCreateKey(temp, "open", &key);
			
			RegSetValueEx(key, "EditFlags", 0, REG_BINARY, (BYTE *)binbuf, 8);
			
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, GetCommandLine(), sizeof(buffer));
			if ((c = strchr(buffer, ' ')) != NULL)
				*c = 0;
			if (*buffer == '\"')
				strcpy(buf, buffer+1);
			else
				strcpy(buf, buffer);
			if (buf[MAX(0,string_len(buf)-1)] == '\"')
				buf[MAX(0,string_len(buf)-1)] = 0;
			strcat(buf, " \"%1\"");
			
			RegSetValue(key, "command", REG_SZ, buf, strlen(buf)+1);
			RegCloseKey(temp);
			RegCloseKey(key);
		}
	}
	else {
		RegCreateKey(HKEY_CLASSES_ROOT, mapExt, &key);
		
		RegSetValueEx(key, NULL, 0, REG_SZ, (BYTE *)mapClass, strlen(mapClass)+1);
		RegCloseKey(key);
		
		RegCreateKey(HKEY_CLASSES_ROOT, mapClass, &key);
		
		memset(buffer, 0, sizeof(buffer));
		strncpy(buffer, GetCommandLine(), sizeof(buffer));
		if ((c = strchr(buffer, ' ')) != NULL)
			*c = 0;
		if (*buffer == '\"')
			strcpy(buf, buffer+1);
		else
			strcpy(buf, buffer);
		if (buf[MAX(0,string_len(buf)-1)] == '\"')
			buf[MAX(0,string_len(buf)-1)] = 0;
		strcat(buf, ",1");
		
		RegSetValue(key, "DefaultIcon", REG_SZ,
			buf, string_len(buf)+1);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "Map File");
		RegSetValueEx(key, NULL, 0, REG_SZ, (BYTE *)buf, string_len(buf)+1);
		
		RegCreateKey(key, "Shell", &temp);
		RegCloseKey(key);
		RegCreateKey(temp, "open", &key);
		
		RegSetValueEx(key, "EditFlags", 0, REG_BINARY, (BYTE *)binbuf, 8);
		
		memset(buffer, 0, sizeof(buffer));
		strncpy(buffer, GetCommandLine(), sizeof(buffer));
		if ((c = strchr(buffer, ' ')) != NULL)
			*c = 0;
		if (*buffer == '\"')
			strcpy(buf, buffer+1);
		else
			strcpy(buf, buffer);
		if (buf[MAX(0,string_len(buf)-1)] == '\"')
			buf[MAX(0,string_len(buf)-1)] = 0;
		strcat(buf, " \"%1\"");
		
		RegSetValue(key, "command", REG_SZ, buf, strlen(buf)+1);
		RegCloseKey(temp);
		RegCloseKey(key);
		
	}
	
	return 1;
}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR lpCommandLine, int nShow) {
	WNDCLASS wc;
	MSG msg;
	HWND hwnd;
	long err;
	char buf[256], *p, mapfile[256];
		
	// first we need to get the directory straight
	
	memset(buf, 0,sizeof(buf));
	strncpy(buf, GetCommandLine(), sizeof(buf));
	if ((p = strchr(buf, ' ')) != NULL)
		*p = 0;
	if ((p = strrchr(buf, '\\')) != NULL)
		*p = 0;
	if (*buf) SetCurrentDirectory(buf);
	
	memset(mapfile, 0, sizeof(mapfile));
	if (lpCommandLine) {
		if (*lpCommandLine == '\"')
			strcpy(mapfile, lpCommandLine+1);
		else
			strcpy(mapfile, lpCommandLine);
	}
	if ((p = strrchr(mapfile, '\"')) != NULL)
		*p = 0; 
	
	if ((hwnd = FindWindowEx(NULL, NULL, "MalletEdit_frame", NULL))) {
		// don't start a new instance - open up the old one
		if (IsIconic(hwnd)) OpenIcon(hwnd);
		BringWindowToTop(hwnd);
		SetActiveWindow(hwnd);
		return 0;
	}
	
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_OWNDC|CS_BYTEALIGNCLIENT|CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = (WNDPROC)MalletEditFrameProc;
	wc.lpszClassName = "MalletEdit_frame";
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.cbWndExtra = wc.cbClsExtra = 0;
	wc.hInstance = hInstance = hinst;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	
	if (!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed to register main window frame.", "Error", MB_OK);
		return FALSE;
	}
	
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	if (initMalletEdit()) return 0;
	
	hwnd = CreateWindow("MalletEdit_frame", "MalletEdit v2",
		WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
		applayout.position.x, applayout.position.y,
		applayout.dimensions.cx, applayout.dimensions.cy,
		NULL, hMenu, hinst, NULL);
	
	if (applayout.showtype == SW_MAXIMIZE || applayout.showtype == SW_SHOWMAXIMIZED) {
		ShowWindow(hwnd, SW_SHOWMAXIMIZED);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOCOPYBITS);
	}
	else
		ShowWindow(hwnd, nShow);


	
	// now if we're opening a map from command line, do it now.
	if (*mapfile)
		OpenMap(mapfile);
	else
		*lastdir = 0;
	
	while (1) {
		err = GetMessage(&msg, NULL, 0, 0);
		if (!err && frameKilled) break;
		else if (err < 1) continue;
		
		if (!TranslateMDISysAccel(GetDlgItem(frameWnd, ID_MDICLIENT), &msg) &&
			!TranslateAccelerator(GetFocus(), hAccel, &msg)) {		
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}
	freeMem();
	return msg.wParam;
}    
