#include "main.h"
#include "extern.h"

/*
#define NUM_PIXELS				1
#define BMP_ROWSZ				768
#define BMP_HALFHT				128
#define BMP_HEIGHT				256			// this should be BMP_HALFHT * 2 ... constant = faster
*/

#define NUM_PIXELS				2
#define BMP_ROWSZ				1536		// NUM_PIXELS * 3 * map->header.width (256)
#define BMP_HALFHT				256			// NUM_PIXELS * map->header.height (256) / 2
#define BMP_HEIGHT				512			// BMP_HALFHT * 2



HDC tilesDC;
HBITMAP tilesBmp;
HPALETTE tilesPal;
HWND overDlg = NULL;
BYTE *overViewBmp = NULL, *fpBmp = NULL;
BITMAPINFO *overViewBmpInfo = NULL, *fpBmpInfo = NULL;
struct seldata tileSel;
struct seldata animSel;
struct MapInfo *overmap = NULL;

WORD *tileArrange = NULL;
int numTiles = 0;

BOOL arrangeDropping = FALSE;

int CALLBACK OVDataProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map;
	
	map = get_map((HWND)SendDlgItemMessage(frameWnd, ID_MDICLIENT, WM_MDIGETACTIVE, 0, 0));
	overmap = map;
	
	switch (msg) {

		case WM_ENTERSIZEMOVE:
			if (map) {
				BITMAPINFO bmi;
				BYTE *ptr = OverView(map);
				memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
				bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
				bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;

				map->overview = merge_fpov(&bmi, ptr);
				free(ptr);
			}
			break;

		case WM_EXITSIZEMOVE:
			if (!map) break;
			if (map->overview) free(map->overview);
			map->overview = NULL;
			break;

		case WM_SIZE:
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			break;

		case WM_PAINT:
			{
				HDC dc;
				PAINTSTRUCT ps;
				BYTE *bmp;
				RECT rc;
				BITMAPINFO bmi;
				int x, y;
				int w, h;
				double div_factorX = 0.0;
				double div_factorY = 0.0;

				BeginPaint(hwnd, &ps);
				if (!map) {
					EndPaint(hwnd, &ps);
					break;
				}
				dc = ps.hdc;
				if (!map->overview)
					bmp = OverView(map);
				else bmp = NULL;

				SetWindowText(hwnd, map->header.name);

				GetClientRect(hwnd, &rc);
				w = rc.right - rc.left;
				h = rc.bottom - rc.top;

				//FillRect(dc, &rc, (HBRUSH)(COLOR_BTNFACE+1));

				GetClientRect(map->hwnd, &rc);
				x = GetScrollPos(map->hwnd, SB_HORZ);
				y = GetScrollPos(map->hwnd, SB_VERT);

				if (bmp || map->overview) {
					double cx, cy;
					int fx, fy, bindw, bindh;
					if (!map->overview) {
						memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
						bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
						bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;
					}

					bindh = MIN(map->header.height*NUM_PIXELS, h);
					bindw = MIN(map->header.width*NUM_PIXELS, w);

					cx = (double)(w - MIN(bindh,bindw)) / 2.0;
					cy = (double)(h - MIN(bindh,bindw)) / 2.0;

					if (bindh > bindw) // maintain aspect ratio
						bindh = (int)((double)bindw * (double)map->header.height / (double)map->header.width);
					else
						bindw = (int)((double)bindh * (double)map->header.width / (double)map->header.height);

					div_factorX = 16.0 * (double)map->header.width / (double)bindw;
					div_factorY = 16.0 * (double)map->header.height / (double)bindh;

					SetStretchBltMode(dc, HALFTONE);
					//StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width, map->header.height, bmp, &bmi, DIB_RGB_COLORS, SRCCOPY);
					if (!map->overview)
					{ 
						BITMAPINFO *foo;

						foo = merge_fpov(&bmi, bmp);
						if (foo) {
							StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width*NUM_PIXELS, map->header.height*NUM_PIXELS, (BYTE *)foo + sizeof(BITMAPINFO), foo, DIB_RGB_COLORS, SRCCOPY);
							free(foo);
						}
					}
					else StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width*NUM_PIXELS, map->header.height*NUM_PIXELS, (BYTE *)map->overview + sizeof(BITMAPINFO), map->overview, DIB_RGB_COLORS, SRCCOPY);

					HRGN whole, bmpr, temp;
					temp = CreateRectRgn(0,0,0,0);
					whole = CreateRectRgnIndirect(&rc);
					bmpr = CreateRectRgn((int)cx, (int)cy, (int)cx + bindw, (int)cy + bindh);
					CombineRgn(temp, whole, bmpr, RGN_DIFF);
					SelectClipRgn(dc, temp);
					FillRect(dc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
					SelectClipRgn(dc, NULL);
					DeleteRgn(bmpr);
					DeleteRgn(whole);				
					DeleteRgn(temp);

					w = rc.right - rc.left;
					h = rc.bottom - rc.top;

					fx = (int)(cx + ((double)x+w)/div_factorX);
					fy = (int)(cy + ((double)y+h)/div_factorY);
					x = (int)(cx + ((double)x)/div_factorX);
					y = (int)(cy + ((double)y)/div_factorY);

					RECT rc; rc.left = x; rc.right = fx-1; rc.bottom = fy-1; rc.top = y;
					HBRUSH br = (HBRUSH)GetStockObject(WHITE_BRUSH);
					FrameRect(dc, &rc, br);
					DeleteObject(br);

					if (bmp) free(bmp);
				}
				else MessageBox(NULL, "No BMP", "...", MB_OK);

				EndPaint(hwnd, &ps);
				return 0;
			}

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
			{
				RECT rc;
				int sx, sy;
				int w, h;
				double div_factorX = 0.0;
				double div_factorY = 0.0;
				double cx, cy;
				int fx, fy, bindw, bindh;
				int x = LOWORD(lparm);
				int y = HIWORD(lparm);

				if (!(wparm & MK_LBUTTON) || !map) {
					if (map && map->overview) {
						free(map->overview);
						map->overview = NULL;
					}
					break;
				}

				if (msg == WM_LBUTTONDOWN && map->overview == NULL) {
					BITMAPINFO bmi;
					BYTE *ptr = OverView(map);
					memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
					bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
					bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;

					map->overview = merge_fpov(&bmi, ptr);
					free(ptr);
				}				

				if (x & 0x8000) x = 0;
				if (y & 0x8000) y = 0;

				SetCapture(hwnd);

				GetClientRect(hwnd, &rc);
				w = rc.right - rc.left;
				h = rc.bottom - rc.top;

				GetClientRect(map->hwnd, &rc);
				sx = GetScrollPos(map->hwnd, SB_HORZ);
				sy = GetScrollPos(map->hwnd, SB_VERT);

				bindh = MIN(map->header.height*NUM_PIXELS, h);
				bindw = MIN(map->header.width*NUM_PIXELS, w);

				cx = (double)(w - MIN(bindh,bindw)) / 2.0;
				cy = (double)(h - MIN(bindh,bindw)) / 2.0;

				if (bindh > bindw) // maintain aspect ratio
					bindh = (int)((double)bindw * (double)map->header.height / (double)map->header.width);
				else
					bindw = (int)((double)bindh * (double)map->header.width / (double)map->header.height);

				div_factorX = 16.0 * (double)map->header.width / (double)bindw;
				div_factorY = 16.0 * (double)map->header.height / (double)bindh;

				w = rc.right - rc.left;
				h = rc.bottom - rc.top;

				fx = (int)(cx + bindw);
				fy = (int)(cx + bindh);

				//      if (x > cx && y > cy && x < fx && y < fy) {
				// we want to center the window on the point of click
				x = (int)((double)(x - cx) * div_factorX - (double)w/2);
				y = (int)((double)(y - cy) * div_factorY - (double)h/2);

				x = MIN(MAX(0,x), map->header.width*16 - w/2);
				y = MIN(MAX(0,y), map->header.height*16 - h/2);

				sx = x*16 - sx;
				sy = y*16 - sy;

				SetScrollPos(map->hwnd, SB_VERT, y, TRUE);
				SetScrollPos(map->hwnd, SB_HORZ, x, TRUE);
				ScrollWindowEx(map->hwnd, -sx, -sy, NULL, NULL, NULL, NULL,
					0);

				RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				//      }

				return 0;
			}

		case WM_LBUTTONUP:
			ReleaseCapture();
			if (map && map->overview) {
				free(map->overview);
				map->overview = NULL;
			}
			return 0;


		default:
			return DefWindowProc(hwnd, msg, wparm, lparm);
		
	}
	return 0;
}

int CALLBACK OverviewProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map;
	RECT rc;
	
	map = get_map((HWND)SendDlgItemMessage(frameWnd, ID_MDICLIENT, WM_MDIGETACTIVE, 0, 0));
	overmap = map;
	
	switch(msg) {
		case WM_INITDIALOG:
			
			GetClientRect(hwnd, &rc);
			CreateWindowEx(WS_EX_CLIENTEDGE, "MalletEdit_ovdata", NULL, WS_VISIBLE|WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)ID_OVDATA, hInstance, NULL);

			SendMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
			SetWindowText(hwnd, map->header.name);
			break;

		case WM_DESTROY:
			overDlg = NULL;
			break;

		case WM_SIZE:
			int width, height;
			GetWindowRect(GetDlgItem(hwnd, IDC_EXPORT), &rc);
			width = MIN(512, LOWORD(lparm) - 10); height = MIN(512, HIWORD(lparm) - (rc.bottom - rc.top) - 15);

			SetWindowPos(GetDlgItem(hwnd, ID_OVDATA), NULL, (LOWORD(lparm) - width)/2, (HIWORD(lparm) - (rc.bottom - rc.top) - 10 - height)/2, width, height,
					SWP_SHOWWINDOW|SWP_NOREPOSITION|SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwnd, IDC_EXPORT), NULL, (LOWORD(lparm) - (rc.right - rc.left))/2, HIWORD(lparm) - (rc.bottom - rc.top) - 5, 0, 0, 
					SWP_SHOWWINDOW|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);
			break;

		case WM_SYSCOMMAND:
			switch (wparm) {
				case SC_CLOSE:
					DestroyWindow(hwnd);
					break;

				default:
					return 0;
			}
			break;

		case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wparm);
			int wID = LOWORD(wparm);
			HWND hwndCtl = (HWND)lparm;

			switch (wNotifyCode) {
			case BN_CLICKED:
				{
					switch(wID) {
						case IDC_EXPORT:
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
								ofn.hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
								ofn.lpstrFilter = "Windows Bitmaps (BMP)\0*.bmp\0JPEG\0*.jpg\0\0";
								ofn.nFilterIndex = 2;
								ofn.lpstrFile = file;
								ofn.nMaxFile = sizeof(file);
								ofn.lpstrInitialDir = dir;
								ofn.Flags = OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|
									OFN_NOLONGNAMES;
								ofn.lpstrDefExt = "jpg";

								if (GetSaveFileName(&ofn)) {
									BITMAPINFO bmi, *bmi2;
									RECT rc;
									BYTE *p;
									char *ext;

									char *support_ext[] = { "bmp", "jpg" };
									int num_sup = (int)(sizeof(support_ext)/sizeof(*support_ext));

									ext = strrchr(file, '.');
									if (ext) ext = strdup(ext+1);

									for (int n = 0; n < num_sup; n++)
										if (!stricmp(ext, support_ext[n])) break;
									if (n >= num_sup) {
										MessageBox(hwnd, "File name supplied has unknown extension", "Cannot save", MB_OK | MB_ICONINFORMATION);
										if (ext) free(ext);
										break;
									}
									if (ext) free(ext);
									ext = NULL;

									memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
									bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
									bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;
									p = OverView(map);
									bmi2 = merge_fpov(&bmi, p);
									free(p);
									p = (BYTE *)bmi2 + sizeof(BITMAPINFO);

									memset(&bmi, 0, sizeof(bmi));									
									bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
									bmi.bmiHeader.biBitCount = 24;
									bmi.bmiHeader.biClrImportant = bmi.bmiHeader.biClrUsed = 0;
									bmi.bmiHeader.biCompression = BI_RGB;
									bmi.bmiHeader.biSizeImage = 0;
									bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
									bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;
									bmi.bmiHeader.biPlanes = 1;
									
									switch (n) {
										case 0: // bmp
											{
												BITMAPFILEHEADER bfh;
												bfh.bfOffBits = sizeof(bfh) + sizeof(BITMAPINFOHEADER);
												bfh.bfReserved1 = bfh.bfReserved2 = 0;
												bfh.bfSize = sizeof(bfh) + sizeof(BITMAPINFOHEADER) + bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 3;
												bfh.bfType = MAKEWORD('B', 'M');

												int fd, r_width = bmi.bmiHeader.biWidth * 3;
												fd = _open(file, _O_RDWR|_O_BINARY|_O_CREAT, _S_IWRITE|_S_IREAD);
												_write(fd, &bfh, sizeof(bfh));
												_write(fd, &(bmi.bmiHeader), sizeof(BITMAPINFOHEADER));
												_write(fd, p, bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 3);
												_close(fd);
												break;
											}

										case 1: // jpg
											{
												struct jpeg_compress_struct cinfo;
												struct jpeg_error_mgr jerr;
												FILE *outfile;
												JSAMPROW rowPtr[1]; // write 1 scanline at a time
												int r_width = bmi.bmiHeader.biWidth * 3;

												for (int y = 0; y < bmi.bmiHeader.biHeight*bmi.bmiHeader.biWidth; y++)
													p[y*3] ^= p[y*3+2] ^= p[y*3] ^= p[y*3+2];
												for (int z = 0; z < bmi.bmiHeader.biHeight/2; z++) {
													for (int y = 0; y < r_width; y++) {
														p[z*r_width + y] ^= p[(bmi.bmiHeader.biHeight-1-z)*r_width + y] ^=
															p[z*r_width + y] ^= p[(bmi.bmiHeader.biHeight-1-z)*r_width + y];
													}
												}

												cinfo.err = jpeg_std_error(&jerr);
												jpeg_create_compress(&cinfo);

												outfile = fopen(file, "wb");
												if (!outfile) {
													MessageBox(hwnd, "Failed to open file", "Error", MB_OK | MB_ICONINFORMATION);
													break;
												}
												jpeg_stdio_dest(&cinfo, outfile);

												cinfo.image_height = bmi.bmiHeader.biHeight;
												cinfo.image_width = bmi.bmiHeader.biWidth;
												cinfo.input_components = 3;
												cinfo.in_color_space = JCS_RGB;
												jpeg_set_defaults(&cinfo);
												jpeg_set_quality(&cinfo, 100, TRUE);

												jpeg_start_compress(&cinfo, TRUE);

												while (cinfo.next_scanline < cinfo.image_height) {
													*rowPtr = &p[cinfo.next_scanline * r_width];
													jpeg_write_scanlines(&cinfo, rowPtr, 1);
												}

												jpeg_finish_compress(&cinfo);
												fclose(outfile);
												break;
											}
										default:
											MessageBox(hwnd, "This file format is not yet supported", "Export", MB_OK | MB_ICONINFORMATION);
											break;
									}

									free(bmi2);									
								}
								SetCurrentDirectory(dir);
								break;
							}


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

int CALLBACK MiniOverviewProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	struct MapInfo *map;
	
	map = get_map((HWND)SendDlgItemMessage(frameWnd, ID_MDICLIENT, WM_MDIGETACTIVE, 0, 0));
	
	switch(msg) {

	case WM_CREATE:
		break;
		
    case WM_DESTROY:
		break;
		
    case WM_SIZE:
		RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		break;
	
    case WM_PAINT:
		{
			HDC dc;
			PAINTSTRUCT ps;
			BYTE *bmp;
			HPEN pen;
			RECT rc;
			BITMAPINFO bmi;
			int x, y;
			int w, h;
			double div_factorX = 0.0;
			double div_factorY = 0.0;
			
			BeginPaint(hwnd, &ps);
			dc = ps.hdc;

			GetClientRect(hwnd, &rc);
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;
			
			if (!map) {
				FillRect(dc, &rc, (HBRUSH)(COLOR_BTNFACE+1));
				EndPaint(hwnd, &ps);
				break;
			}
	
			if (map->overview == NULL)
				bmp = OverView(map);
			else bmp = NULL;
			
			SetWindowText(hwnd, map->header.name);
					
			GetClientRect(map->hwnd, &rc);
			x = GetScrollPos(map->hwnd, SB_HORZ);
			y = GetScrollPos(map->hwnd, SB_VERT);
			
			if (bmp || map->overview) {
				double cx, cy;
				int fx, fy, bindw, bindh;
				if (!map->overview) {
					memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
					bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
					bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;
				}

				bindh = MIN(map->header.height, h);
				bindw = MIN(map->header.width, w);
				
				cx = (double)(w - MIN(bindh,bindw)) / 2.0;
				cy = (double)(h - MIN(bindh,bindw)) / 2.0;
				
				if (bindh > bindw) // maintain aspect ratio
					bindh = (int)((double)bindw * (double)map->header.height / (double)map->header.width);
				else
					bindw = (int)((double)bindh * (double)map->header.width / (double)map->header.height);
				
				div_factorX = 16.0 * (double)map->header.width / (double)bindw;
				div_factorY = 16.0 * (double)map->header.height / (double)bindh;
				
				SetStretchBltMode(dc, HALFTONE);
				//StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width * NUM_PIXELS, map->header.height * NUM_PIXELS, bmp, &bmi, DIB_RGB_COLORS, SRCCOPY);
				if (!map->overview)
				{ 
					BITMAPINFO *foo;

					foo = merge_fpov(&bmi, bmp);
					if (foo) {
						StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width*NUM_PIXELS, map->header.height*NUM_PIXELS, (BYTE *)foo + sizeof(BITMAPINFO), foo, DIB_RGB_COLORS, SRCCOPY);
						free(foo);
					}
				}
				else StretchDIBits(dc, (int)cx, (int)cy, bindw, bindh, 0, 0, map->header.width*NUM_PIXELS, map->header.height*NUM_PIXELS, (BYTE*)map->overview + sizeof(BITMAPINFO), map->overview, DIB_RGB_COLORS, SRCCOPY);
				
				
				w = rc.right - rc.left;
				h = rc.bottom - rc.top;
				
				fx = (int)(cx + ((double)x+w)/div_factorX);
				fy = (int)(cy + ((double)y+h)/div_factorY);
				x = (int)(cx + ((double)x)/div_factorX);
				y = (int)(cy + ((double)y)/div_factorY);
				/*
				pen = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
				pen = (HPEN)SelectObject(dc, pen);
				MoveToEx(dc, x, y, NULL);
				LineTo(dc, fx-1, y);
				LineTo(dc, fx-1, fy-1);
				LineTo(dc, x, fy-1);
				LineTo(dc, x, y);
				pen = (HPEN)SelectObject(dc, pen);
				DeleteObject(pen);
				*/
				RECT rc; rc.left = x; rc.right = fx-1; rc.bottom = fy-1; rc.top = y;
				HBRUSH br = (HBRUSH)GetStockObject(WHITE_BRUSH);
				FrameRect(dc, &rc, br);
				DeleteObject(br);
				
				if (bmp) free(bmp);
			}
			
			EndPaint(hwnd, &ps);
			return 0;
		}
	
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
		{
			RECT rc;
			int sx, sy;
			int w, h;
			double div_factorX = 0.0;
			double div_factorY = 0.0;
			double cx, cy;
			int fx, fy, bindw, bindh;
			int x = LOWORD(lparm);
			int y = HIWORD(lparm);
			
			if (!(wparm & MK_LBUTTON) || !map) {
				if (map && map->overview) {
					free(map->overview);
					map->overview = NULL;
				}
				break;
			}

			if (x & 0x8000) x = 0;
			if (y & 0x8000) y = 0;

			SetCapture(hwnd);

			if (msg == WM_LBUTTONDOWN && map->overview == NULL) {
				BITMAPINFO bmi;
				BYTE *ptr = OverView(map);
				memcpy(&bmi, overViewBmpInfo, sizeof(bmi));
				bmi.bmiHeader.biWidth = map->header.width * NUM_PIXELS;
				bmi.bmiHeader.biHeight = map->header.height * NUM_PIXELS;

				map->overview = merge_fpov(&bmi, ptr);
				free(ptr);
			}				
			
			GetClientRect(hwnd, &rc);
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;
			
			GetClientRect(map->hwnd, &rc);
			sx = GetScrollPos(map->hwnd, SB_HORZ);
			sy = GetScrollPos(map->hwnd, SB_VERT);
			
			bindh = MIN(map->header.height*NUM_PIXELS, h);
			bindw = MIN(map->header.width*NUM_PIXELS, w);
			
			cx = (double)(w - MIN(bindh,bindw)) / 2.0;
			cy = (double)(h - MIN(bindh,bindw)) / 2.0;
			
			if (bindh > bindw) // maintain aspect ratio
				bindh = (int)((double)bindw * (double)map->header.height / (double)map->header.width);
			else
				bindw = (int)((double)bindh * (double)map->header.width / (double)map->header.height);
			
			div_factorX = 16.0 * (double)map->header.width / (double)bindw;
			div_factorY = 16.0 * (double)map->header.height / (double)bindh;
			
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;
			
			fx = (int)(cx + bindw);
			fy = (int)(cx + bindh);
			
			//      if (x > cx && y > cy && x < fx && y < fy) {
			// we want to center the window on the point of click
			x = (int)((double)(x - cx) * div_factorX - (double)w/2);
			y = (int)((double)(y - cy) * div_factorY - (double)h/2);
			
			x = MIN(MAX(0,x), map->header.width*16 - w/2);
			y = MIN(MAX(0,y), map->header.height*16 - h/2);
			
			sx = x*16 - sx;
			sy = y*16 - sy;
	
			SetScrollPos(map->hwnd, SB_VERT, y, TRUE);
			SetScrollPos(map->hwnd, SB_HORZ, x, TRUE);
			ScrollWindowEx(map->hwnd, -sx, -sy, NULL, NULL, NULL, NULL,
				0);
			
			
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			RedrawWindow(map->hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
			//      }
			
			return 0;
		}

	case WM_LBUTTONUP:
		ReleaseCapture();
		if (map && map->overview) {
			free(map->overview);
			map->overview = NULL;
		}
		return 0;

	case WM_TIMER:
		RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		break;		
		
    default:
		return DefWindowProc(hwnd, msg, wparm, lparm);
  }
  return 0;
}


int CALLBACK TileAreaProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparm) {
	
	switch(msg) {
		// each case will have its own scope
    case WM_CREATE:
		break;
		
    case WM_SIZE:
		{
			int scr_range;
			int height = HIWORD(lparm);
			int width = LOWORD(lparm);
			SCROLLINFO si;
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_RANGE;
			
			
			scr_range = 1600 - height;
			si.nMax = scr_range;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			
			scr_range = 640 - width > -1?640 - width:0;
			si.nMax = scr_range;
			if (scr_range) ShowScrollBar(hwnd, SB_HORZ, TRUE);
			else ShowScrollBar(hwnd, SB_HORZ, FALSE);
			SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
			
			return DefWindowProc(hwnd, msg, wparm, lparm);
		}
		
		
    case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_PAINT:
		{
			HDC dc;
			PAINTSTRUCT ps;
			BITMAP bmp;
			HBITMAP obj = NULL;
			int scrollx, scrolly;
			int x, maxx;
			RECT rc;
			
			obj = (HBITMAP)SelectObject(tilesDC, obj);
			GetObject(obj, sizeof(bmp), &bmp);
			obj = (HBITMAP)SelectObject(tilesDC, obj);
			
			scrollx = GetScrollPos(hwnd, SB_HORZ);
			scrolly = GetScrollPos(hwnd, SB_VERT);
			
			BeginPaint(hwnd, &ps);
			dc = ps.hdc;
			
			SelectPalette(dc, tilesPal, FALSE);
			RealizePalette(dc);
			
			SetWindowOrgEx(dc, scrollx, scrolly, NULL);
			// get the right view
			
			/*
			BitBlt(dc, 0, 0, 640, 1600, tilesDC,
			0, 0, SRCCOPY);
			*/
			GetClientRect(hwnd, &rc);
			maxx = MIN(numTiles, (scrolly + rc.bottom - rc.top + 16)/16 * 40);
			
			for (x = scrolly/16*40; x < maxx; x++) {
				BitBlt(dc, x%40 * 16, x/40 * 16, 16, 16,
					tilesDC, tileArrange[x]%40 * 16, tileArrange[x]/40 * 16,
					SRCCOPY);
			}        
			
			if (tileSel.tile > -1) {
				HPEN pen = CreatePen(PS_SOLID, 1, 0xFFFFFF);
				DrawSelection(dc, pen, (tileSel.tile % 40)<<4,
					(tileSel.tile / 40)<<4, tileSel.horz << 4,
					tileSel.vert << 4);
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
			
			SCROLLINFO si;
			
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_HORZ, &si);
			
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
			
			si.fMask = SIF_POS;
			
			add_factor = MAX(si.nMin - si.nPos, MIN(si.nMax - si.nPos, add_factor));

			if (add_factor != 0) {
				si.nPos += add_factor;
				SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
				ScrollWindowEx(hwnd, -add_factor, 0, NULL, NULL, NULL, NULL, SW_ERASE);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
			}
			break;
		}

	case WM_NCLBUTTONDOWN:
		{
			int nchit = (int)wparm;
			POINTS pts = MAKEPOINTS(lparm);
			HCURSOR curs = LoadCursor(NULL, IDC_SIZENS);
			RECT rc;

			GetWindowRect(hwnd, &rc);

			if (nchit == HTBORDER && abs(rc.top - pts.y) < 10) {
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

			if (nchit != HTCLIENT)
				ClearStatus();

			if (nchit == HTBORDER && abs(rc.top - pts.y) < 10) {
				if (GetCursor() != curs)
					SetCursor(curs);
			}
			else if (nchit == HTCLIENT) {
				if (GetCursor() == curs)
					SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			
			break;
		}
				
		
    case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lparm) + GetScrollPos(hwnd, SB_HORZ);
			int y = GET_Y_LPARAM(lparm) + GetScrollPos(hwnd, SB_VERT);
			char buf[256];
			HCURSOR curs = LoadCursor(NULL, IDC_ARROW);

			if (GetCursor() != curs)
				SetCursor(curs);
			
			memset(buf, 0, sizeof(buf));
			ClearStatus();

			if (x > 640) break;
			
			sprintf(buf, "x: %-5d y: %-5d", x/16 + 1, y/16 + 1);
			SetStatusText(0, buf);
			sprintf(buf, "Tile: %-5d", tileArrange[y/16*40 + x/16]);
			SetStatusText(1, buf);
			break;
		}
		
    case WM_RBUTTONDOWN:
		{
			int nFlags = wparm;
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int scx, scy, rx, ry, shiftx, shifty;
			SCROLLINFO si;
			MSG mouseMsg;
			BOOL err, didShift = FALSE;
			
			memset(&si, 0, sizeof(si));
			si.fMask = SIF_ALL;
			si.cbSize = sizeof(si);
			GetScrollInfo(hwnd, SB_HORZ, &si);
			scx = si.nPos;
			x += si.nPos;
			GetScrollInfo(hwnd, SB_VERT, &si);
			scy = si.nPos;
			y += si.nPos;
			
			rx = x;
			ry = y;
			
			if (x >= (tileSel.tile%40 * 16) && x <= (tileSel.tile%40 * 16 + tileSel.horz * 16) &&
				y >= (tileSel.tile/40 * 16) && y <= (tileSel.tile/40 * 16 + tileSel.vert * 16))
			{
				SetCapture(hwnd);
				arrangeDropping = FALSE;
				
				while (1) {
					err = GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
					if (err < 1) break;
					
					if (mouseMsg.message == WM_RBUTTONUP || !(mouseMsg.wParam & MK_RBUTTON))
						break;
					
					x = LOWORD(mouseMsg.lParam) + scx;
					y = HIWORD(mouseMsg.lParam) + scy;
					
					if (x & 0x8000 || y & 0x8000) continue;
					
					shiftx = (x - rx)/16;
					shifty = (y - ry)/16;
					
					shiftx = MIN(MAX(-tileSel.tile % 40, shiftx),
						40 - tileSel.tile%40 - tileSel.horz);
					shifty = MIN(MAX(-tileSel.tile / 40, shifty),
						100 - tileSel.tile/40 - tileSel.vert);
					
					if (shiftx) {
						int k;
						
						rx = x;
						err = TRUE;
						
						while (shiftx) {
							if (shiftx > 0)
								k = tileSel.tile + 1;
							else
								k = tileSel.tile - 1;
							
							
							swapTiles(tileSel.tile % 40, tileSel.tile / 40,
								tileSel.horz, tileSel.vert,
								k % 40, k / 40);
							
							tileSel.tile = k;
							if (shiftx > 0) shiftx--;
							else shiftx++;
						}
					}
					if (shifty) {
						int k;
						
						ry = y;
						err = TRUE;
						
						while (shifty) {
							if (shifty > 0)
								k = tileSel.tile + 40;
							else
								k = tileSel.tile - 40;
							
							swapTiles(tileSel.tile % 40, tileSel.tile / 40,
								tileSel.horz, tileSel.vert,
								k % 40, k / 40);
							
							tileSel.tile = k;
							if (shifty > 0) shifty--;
							else shifty++;
						}
					}
					if (err) {
						didShift = TRUE;
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
					}          
				}
				if (!didShift) arrangeDropping = TRUE;
				ReleaseCapture();
			}
			else if (arrangeDropping) {
				int px = tileSel.tile % 40;
				int py = tileSel.tile / 40;
				int v = tileSel.vert;
				int h = tileSel.horz;
				char buf[256];
				
				x >>= 4; y >>= 4;
				
				x = MAX(0, MIN(x, 40 - h));
				y = MAX(0, MIN(y, 100 - v));
				
				
				if ((x >= px && x < (px+h) && y >= py && y < (py+v)) ||
					(px >= x && px < (x+h) && py >= y && py < (y+h)) ||
					(px >= x && px < (x+h) && y >= py && y < (py+h)) ||
					(x >= px && x < (px+h) && py >= y && py < (y+h))) {
					
					sprintf(buf, "%d %d %d %d -- %d %d",
						x, y, px, py, h, v);
					SetStatusText(3, buf);
					SetStatusText(4, "Rectangles overlap");
					break;
				}
				
				SetStatusText(4, "Swapped tiles");
				swapSwap(px, py, h, v, x, y);
				tileSel.tile = x + y*40;
				RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
				arrangeDropping = FALSE;
			}
			
			break;
    }         
	
    case WM_LBUTTONDOWN:
		{
			int nFlags = wparm;
			int x = GET_X_LPARAM(lparm);
			int y = GET_Y_LPARAM(lparm);
			int scx, scy, rx, ry;
			SCROLLINFO si;
			HDC dc;
			HPEN pen;
			int n;
			char buf[256];

			if (x > 640) break;
			
			memset(&si, 0, sizeof(si));
			si.fMask = SIF_ALL;
			si.cbSize = sizeof(si);
			GetScrollInfo(hwnd, SB_HORZ, &si);
			scx = si.nPos;
			x += si.nPos;
			GetScrollInfo(hwnd, SB_VERT, &si);
			scy = si.nPos;
			y += si.nPos;
			
			dc = GetDC(hwnd);
			
			
			rx = x;
			ry = y;

			
			arrangeDropping = FALSE;
			
			SetWindowOrgEx(dc, scx, scy, NULL);
			
			if (tileSel.tile > -1)
				DrawTileBlock(tileSel.tile, dc,
				(tileSel.tile % 40)<<4,
				(tileSel.tile / 40)<<4,
				tileSel.horz, tileSel.vert, TRUE);
			
			pen = CreatePen(PS_SOLID, 1, 0xFFFFFF);
			
			DrawSelection(dc, pen, x/16*16, y/16*16, 16, 16);
			DeleteObject(pen);
			ReleaseDC(hwnd, dc);
			
			tileSel.tile = x/16 + y/16*40;
			tileSel.horz = 1;
			tileSel.vert = 1;
			
			ClearStatus();
			sprintf(buf, "Start: %-5d", tileSel.tile);
			SetStatusText(0, buf);
			sprintf(buf, "Horizontal: %-5d", tileSel.horz);
			SetStatusText(1, buf);
			sprintf(buf, "Vertical: %-5d", tileSel.vert);
			SetStatusText(2, buf);
			
			while ((n = ShowCursor(FALSE)) > -1);
			
			SetCapture(hwnd);
			
			{
				MSG mouseMsg;
				BOOL err;
				int h, v, t;
				
				while (1) {
					err = GetMessage(&mouseMsg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST);
					if (err < 1) break;
					
					if (mouseMsg.message == WM_LBUTTONUP || !(mouseMsg.wParam & MK_LBUTTON))
						break;
					
					x = scx + LOWORD(mouseMsg.lParam);
					y = scy + HIWORD(mouseMsg.lParam);
					
					dc = GetDC(hwnd);
					
					SetWindowOrgEx(dc, scx, scy, NULL);

					t = MIN(y, ry)/16*40 + MIN(x, rx)/16;
					h = MIN(MAX(1, ABS(x - rx)/16), (40 - (tileSel.tile%40)));
					v = MIN(MAX(1, ABS(y - ry)/16), (100 - (tileSel.tile/40)));

					if (tileSel.tile > -1 && 
						(t != tileSel.tile || v != tileSel.vert || h != tileSel.horz))
					{

						DrawTileBlock(tileSel.tile, dc,
						(tileSel.tile % 40)<<4,
						(tileSel.tile / 40)<<4,
						tileSel.horz, tileSel.vert, TRUE);
					}
					else  {
						ReleaseDC(hwnd, dc);
						continue;
					}
						
				
					tileSel.tile = t;
					tileSel.horz = h;
					tileSel.vert = v;
					
					ClearStatus();
					sprintf(buf, "Start: %-5d", tileSel.tile);
					SetStatusText(0, buf);
					sprintf(buf, "Horizontal: %-5d", tileSel.horz);
					SetStatusText(1, buf);
					sprintf(buf, "Vertical: %-5d", tileSel.vert);
					SetStatusText(2, buf);
					
					
					pen = CreatePen(PS_SOLID, 1, 0xFFFFFF);
					
					DrawSelection(dc, pen, (tileSel.tile % 40)<<4,
                        (tileSel.tile / 40)<<4,
                        tileSel.horz << 4, tileSel.vert << 4);
					DeleteObject(pen);
					ReleaseDC(hwnd, dc);
				}
			}
			ReleaseCapture();
			while ((n = ShowCursor(TRUE)) < 0);
			
			if (useAnim) {
				useAnim = FALSE;
				SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_TILE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				SendDlgItemMessage(GetDlgItem(frameWnd, ID_INFOAREA), ID_ANIM, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			}
			
			if (currentTool != TOOL_FILL)
				currentTool = TOOL_EDIT;
			DrawToolbar();
			
			
			
			break;
    }
	
	
    default:
		return DefWindowProc(hwnd, msg, wparm, lparm);
  }
  return 0;
}

BOOL read_tiles(char *file) {
	int tmpFd, x;
	LOGPALETTE *lPal;
	RGBQUAD palEntries[256];
	BITMAP bm;
	
	tmpFd = _open(file, _O_RDONLY);
	if (tmpFd < 1) return FALSE;
	
	_read(tmpFd, palEntries, 54);
	_read(tmpFd, palEntries, sizeof(palEntries));
	_close(tmpFd);
	
	lPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*256);
	lPal->palNumEntries = 256;
	lPal->palVersion = 0x300;
	
	for (tmpFd = 0; tmpFd < 256; tmpFd++) {
		lPal->palPalEntry[tmpFd].peBlue = palEntries[tmpFd].rgbBlue;
		lPal->palPalEntry[tmpFd].peRed = palEntries[tmpFd].rgbRed;
		lPal->palPalEntry[tmpFd].peGreen = palEntries[tmpFd].rgbGreen;
	}
	
	tilesPal = CreatePalette(lPal);
	free(lPal);
	
	tilesBmp = (HBITMAP)LoadImage(hInstance, file, IMAGE_BITMAP, 640, 1600,
		LR_CREATEDIBSECTION|LR_LOADFROMFILE);
	if (!tilesBmp) return FALSE;
	
	GetObject(tilesBmp, sizeof(bm), &bm);
	tileArrange = (WORD *)malloc((bm.bmWidth * bm.bmHeight) / 256 * 2);
	numTiles = (bm.bmWidth * bm.bmHeight) / 256;
	for (x = 0; x < numTiles; x++)
		tileArrange[x] = x; // standard bitmap arrangement
	
	
	tilesDC = CreateCompatibleDC(NULL);
	(void)SelectObject(tilesDC, tilesBmp);
	return TRUE;
}

void DrawTileBlock(int tilePos, HDC dest, int x, int y, int horz, int vert, BOOL byPos) {
	int cx, cy;
	int tileNum;
	
	if (byPos)
		tileNum = tilePos;
	else {
		for (tileNum = 0; tileNum < numTiles; tileNum++)
			if (tileArrange[tileNum] == tilePos) break;
			if (tileNum == numTiles) tileNum = 0;
	}
	
	/* 40 tiles per line, 100 lines, tiles are 16x16 */
	
	/*
	cy = tilePos/40 * 16;
	cx = tilePos%40 * 16;
	
	  BitBlt(dest, x, y, horz*16, vert*16, tilesDC, cx, cy, SRCCOPY);
	*/
	
	for (cy = tileNum/40; cy < (tileNum/40 + vert); cy++) {
		for (cx = tileNum%40; cx < (tileNum%40 + horz); cx++) {
			if (cy*40+cx > numTiles) continue;
			BitBlt(dest, x + (cx - tileNum%40)*16,
				y + (cy - tileNum/40)*16, 16, 16, tilesDC,
				tileArrange[cy*40+cx] % 40 * 16,
				tileArrange[cy*40+cx] / 40 * 16,
				SRCCOPY);
		}
	}
	
}

void make_overview() {
	/*HBITMAP bmp;
	BITMAP dims;
	int nScanLines = 0;
	BITMAPINFO *bmi = &overViewBmpInfo;
	
	bmp = LoadImage(hInstance, MAKEINTRESOURCE(IDB_OVERVIEW), IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION);

	memset(bmi, 0, sizeof(BITMAPINFO));
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	GetObject(bmp, sizeof(dims), &dims);

	GetDIBits(tilesDC, bmp, 0, dims.bmHeight, NULL, bmi, DIB_RGB_COLORS);
	overViewBmp = (BYTE *)malloc(dims.bmHeight * dims.bmWidth * 3);
	GetDIBits(tilesDC, bmp, 0, dims.bmHeight, overViewBmp, bmi, DIB_RGB_COLORS);
	*/

	overViewBmpInfo = j2bres(IDR_OVERVIEW2);
	overViewBmp = (BYTE *)overViewBmpInfo + sizeof(BITMAPINFO);

	fpBmpInfo = j2bres(IDR_FARPLANE);
	fpBmp = (BYTE *)fpBmpInfo + sizeof(BITMAPINFO);
}

BYTE *OverView(struct MapInfo *map) {
	BYTE *lpBuf, *lpP; // buffer to create the bitmap in
	WORD *mapBuf;
	int x, tx, ty, tile; //, sz, ox, oy;
	int ox, oy, sz;
	int rx, ry;

	
	if (!map) return NULL;
	mapBuf = map->mapData;

/*		
	lpBuf = (BYTE *)malloc(map->header.width * map->header.height * 3);
	memset(lpBuf, 0, map->header.width * map->header.height * 3);

	
	__asm {
		mov ecx, lpBuf
			mov ebx, overViewBmp
			
			mov x, 0
			
loopX:
		cmp x, 0x10000
			je  end
			
			push ecx
			mov ecx, mapBuf
			add ecx, x
			add ecx, x
			mov eax, ecx
			pop ecx
			
			mov edx, [eax]
			and edx, 0x7FFF
			mov tile, edx
			
			xor edx, edx
			mov eax, tile
			mov ebx, 40
			div bx
			mov tx, edx
			mov ty, eax
			
			xor edx, edx
			mov eax, tx
			mov ebx, 3
			mul ebx
			mov tx, eax
			xor edx, edx
			mov eax, ty
			mov ebx, 120
			mul ebx
			mov ty, 11880
			sub ty, eax
			
			mov ebx, overViewBmp
			add ebx, ty
			add ebx, tx
			
			mov eax, [ebx]
			mov [ecx], ax
			add ecx, 2
			mov eax, [ebx+2]
			mov [ecx], al
			add ecx, 1
			
			inc x
			jmp loopX
			
end:
	}
	

*/	
	// new version uses NUM_PIXELS overviews and shrinks it = higher quality

	lpBuf = (BYTE *)malloc(map->header.width * map->header.height * NUM_PIXELS * NUM_PIXELS * 3);
	if (!lpBuf) return NULL;
	//memset(lpBuf, 0, map->header.width * map->header.height * NUM_PIXELS * NUM_PIXELS * 3);


	// optimize for speed
	sz = map->header.height * map->header.width;
	for (x = 0, lpP = lpBuf; x < sz; x++) {
		tile = map->mapData[x];
		oy = x / map->header.width;
		ox = x % map->header.width;
ntile:
		if (!(tile & 0x8000)) { // normal tile
			BYTE *ptr, *old;
			int w, h;

			w = overViewBmpInfo->bmiHeader.biWidth;
			h = overViewBmpInfo->bmiHeader.biHeight;
			ry = tile / 40;
			rx = tile % 40;

			ptr = &overViewBmp[(w*3*(h-1))- (ry*w*3)*NUM_PIXELS + rx*NUM_PIXELS*3];
			old = lpP;
			for (ty = 0; ty < NUM_PIXELS; ty++) {
				for (tx = 0; tx < NUM_PIXELS; tx++) {
					lpP[0+tx*3] = ptr[0+tx*3];
					lpP[1+tx*3] = ptr[1+tx*3];
					lpP[2+tx*3] = ptr[2+tx*3];					
				}
					
				ptr -= overViewBmpInfo->bmiHeader.biWidth * 3;
				lpP += map->header.width * NUM_PIXELS * 3;
			}
			if (ox+1 != map->header.width) lpP = old + NUM_PIXELS * 3;
			else lpP = old + (map->header.width+1) * NUM_PIXELS * 3;
		}
		else {
			struct Animation *anim = get_anim((unsigned char)tile);
			if (anim && anim->frameCount) {
				tile = anim->frames[0];
				goto ntile;
			}
			else continue;
		}
	}
	//*/

	
	for (ty = 0; ty < BMP_HALFHT; ty++)
		for (tx = 0; tx < BMP_ROWSZ; tx++) {
				lpBuf[ty*BMP_ROWSZ+tx] ^=
					lpBuf[(BMP_HEIGHT-ty-1)*BMP_ROWSZ+tx] ^=
					lpBuf[ty*BMP_ROWSZ+tx] ^=
					lpBuf[(BMP_HEIGHT-ty-1)*BMP_ROWSZ+tx];
		}
		
	
	return lpBuf;
}


void ResizeTileArea() {
	MSG msg; /* catch mouse messages */
	RECT rc;
	HDC screen = GetDC(NULL);
	HWND tile_area = GetDlgItem(frameWnd, ID_TILEAREA);
	int x, y, max_height, h, height, width, min_height;

	GetWindowRect(GetDlgItem(frameWnd, ID_TOOLBAR), &rc);
	max_height = rc.bottom;
	
	GetWindowRect(tile_area, &rc);
	h = rc.top;
	min_height = rc.bottom - 16;

	MyFocusRect(screen, &rc);

	SetCapture(tile_area);
	y = rc.top;
	while (1) {
		BOOL p = GetMessage(&msg, tile_area, WM_MOUSEFIRST, WM_MOUSELAST);
		if (p == 0) break;
		if (p < 1) continue;

		x = GET_X_LPARAM(msg.lParam);
		y = GET_Y_LPARAM(msg.lParam) + h;
		if (msg.message == WM_LBUTTONUP || !(msg.wParam & MK_LBUTTON))
			break;

		MyFocusRect(screen, &rc);
		
		rc.top = MAX(MIN(y, min_height), max_height);
		MyFocusRect(screen, &rc);
	}
	
	MyFocusRect(screen, &rc);
	h = rc.bottom - rc.top;
	ReleaseDC(NULL, screen);

	applayout.tw_height = MAX(-(min_height-rc.bottom),h);

	
	GetClientRect(frameWnd, &rc);
	height = rc.bottom - rc.top;
	width = rc.right - rc.left;

	SetWindowPos(GetDlgItem(frameWnd, ID_TILEAREA), NULL, 0, height-applayout.tw_height-20, width-130, applayout.tw_height,
				SWP_SHOWWINDOW|SWP_NOREPOSITION);
	SetWindowPos(GetDlgItem(frameWnd, ID_MDICLIENT), HWND_TOP, 0, 32, width-130, height-20-32-applayout.tw_height,
				0);
	SetWindowPos(GetDlgItem(frameWnd, ID_CFRAME), HWND_TOP, 0, 32, width-130, height-20-32-applayout.tw_height,
				SWP_NOREPOSITION);				
	
	ReleaseCapture();
}



			



	



	
