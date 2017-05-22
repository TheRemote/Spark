#include "main.h"
#include "extern.h"

size_t string_len(char *string) {
	char *c;
	
	for (c = string; c && *c; c++);
	return (size_t)(c - string);
}

char *dupestr(char *string) {
	char *buffer;
	
	if (!string || !*string) return NULL;
	buffer = (char *)malloc(strlen(string)+1);
	if (buffer)
		strcpy(buffer, string);
	return buffer;
}

void DrawSelection(HDC dc, HPEN pen, int x, int y, int w, int h) {
	pen = (HPEN)SelectObject(dc, pen);
	MoveToEx(dc, x, y, NULL);
	LineTo(dc, x+w-1, y);
	LineTo(dc, x+w-1, y+h-1);
	LineTo(dc, x, y+h-1);
	LineTo(dc, x, y);
	pen = (HPEN)SelectObject(dc, pen);
}

#define NewRgn(a)			(a) = CreateRectRgn(0,0,0,0)

void NewSelection(HDC dc, HPEN pen, struct seldata *oldsel, struct seldata *newsel, struct MapInfo *map) {
	HRGN oldrgn, newrgn, /*oldfr,*/ newfr, temp;
	RECT rc;
	POINT pt;
	int scx, scy;
	
	scx = GetScrollPos(map->hwnd, SB_HORZ);
	scy = GetScrollPos(map->hwnd, SB_VERT);
	
	SetWindowOrgEx(dc, scx, scy, &pt); /* save old position */
	
	rc.left = oldsel->tile % map->header.width * 16 - scx;
	rc.right = rc.left + oldsel->horz * 16;
	rc.top = oldsel->tile / map->header.width * 16 - scy;
	rc.bottom = rc.top + oldsel->vert * 16;
	oldrgn = CreateRectRgnIndirect(&rc);
/*
	rc.left++; rc.top++;
	rc.right--; rc.bottom--;
	oldfr = CreateRectRgnIndirect(&rc);
	CombineRgn(oldfr, oldrgn, oldfr, RGN_DIFF);
*/
	
	rc.left = newsel->tile % map->header.width * 16 - scx;
	rc.right = rc.left + newsel->horz * 16;
	rc.top = newsel->tile / map->header.width * 16 - scy;
	rc.bottom = rc.top + newsel->vert * 16;
	newrgn = CreateRectRgnIndirect(&rc);
	rc.left++; rc.top++;
	rc.right--; rc.bottom--;
	newfr = CreateRectRgnIndirect(&rc);
	CombineRgn(newfr, newrgn, newfr, RGN_DIFF);
	
	
	NewRgn(temp);
	CombineRgn(temp, oldrgn, newfr, RGN_DIFF);
	
	map->selection = *newsel;
	RedrawWindow(map->hwnd, NULL, temp, RDW_UPDATENOW|RDW_INVALIDATE);
	DeleteObject(temp);
	
	if (GetRgnBox(newrgn, &rc) != NULLREGION)
		DrawSelection(dc, pen, newsel->tile % map->header.width * 16, newsel->tile / map->header.width * 16,
		newsel->horz * 16, newsel->vert * 16);
	
	DeleteObject(newrgn); DeleteObject(newfr);
	DeleteObject(oldrgn); //DeleteObject(oldfr);
	
	
	SetWindowOrgEx(dc, pt.x, pt.y, NULL); /* reset position */
}





void BufferCompare(WORD *one, WORD *two, int w, int h, struct undo_buf **ub) {
/* compare the two buffers, returning the "block"
that has been changed.

  [][][][][][][][][]             [][][][][][][][][]
  [][][][][][][][][]             [][][][][][][][][]
  [][][][][][][][][]             [][][X[X[X[][][][]
  [][][][][][][][][]     and     [][][][][X[X[X[][] is
  [][][][][][][][][]             [][][][][][][][][]
  [][][][][][][][][]             [][][][][][][][][]
  
	[X[X[X[][]
	[][][X[X[X
	*/
	
	int x, y, n, count = 0;
	int firstX=w+1, lastX=-1;
	int firstY=h+1, lastY=-1;
	WORD *newBuf;
	struct undo_buf *newUndoBlock;
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (one[y*h+x] == two[y*h+x]) continue;
			
			count++;
			
			if (x < firstX) firstX = x;
			if (x > lastX) lastX = x;
			if (y < firstY) firstY = y;
			if (y > lastY) lastY = y;
		}
	}
	
	if (count) {
		newBuf = (WORD *)malloc((lastY - firstY + 1)*(lastX - firstX + 1)*2);
		for (n = 0, y = firstY; y < lastY+1; y++) {
			for (x = firstX; x < lastX+1; x++) {
				if (one[y*h+x] == two[y*h+x]) newBuf[n++] = 0xFFFF; // transparency thing :)
				else newBuf[n++] = two[y*h+x];
			}
		}
		
		newUndoBlock = (struct undo_buf *)malloc(sizeof(struct undo_buf));
		newUndoBlock->buffer = newBuf;
		newUndoBlock->width = lastX - firstX + 1;
		newUndoBlock->height = lastY - firstY + 1;
		newUndoBlock->x = firstX;
		newUndoBlock->y = firstY;
	}
	else newUndoBlock = NULL;
	
	*ub = newUndoBlock;
}

int count_list(struct undo_blocks *list) {
	int p = 0;
	struct undo_blocks *d;
	
	for (d = list; d; p++, d = d->next);
	return p;
}

BOOL chop_list(struct undo_blocks **list) {
	struct undo_blocks *loop, *prev;
	
	loop = prev = NULL;
	
	for (loop = *list; loop && loop->next; loop = loop->next)
		prev = loop;
	
	if (!prev) return FALSE;
	if (loop && loop->undo.ptr) {
		if (loop->type == UNDO_NORMAL && loop->undo.ub->buffer) free(loop->undo.ub->buffer);
		free(loop->undo.ptr);
		free(loop);
		prev->next = NULL;
	}
	return TRUE;
}

rdTbl rotTbl[512] = {
  /* Tiles First :) */
  
  { 4, 5, 4, 5 },
  { 6, 7, 6, 7 },
  { 8, 50, 10, 9 },
  { 11, 52, 51, 12 },
  { 14, 15, 14, 15 },
  { 16, 19, 17, 18 },
  { 24, 25, 64, 26 },
  { 46, 48, 49, 47 },
  { 54, 53, 55, 56 },
  { 57, 59, 60, 58 },
  { 84, 85, 87, 86 },
  { 88, 126, 168, 124 },
  { 92, 133, 132, 93 },
  { 99, 139, 179, 140 },
  { 127, 170, 169, 171 },
  { 162, 163, 203, 202 },
  { 164, 286, 247, 404 },
  { 165, 326, 246, 364 },
  { 166, 366, 245, 324 },
  { 167, 406, 244, 284 },
  { 204, 285, 207, 405 },
  { 205, 325, 206, 365 },
  { 208, 290, 250, 369 },
  { 209, 330, 249, 329 },
  { 210, 370, 248, 289 },
  { 211, 213, 293, 291 },
  { 212, 253, 292, 251 },
  { 216, 217, 216, 217 },
  { 218, 256, 258, 257 },
  { 219, 220, 259, 260 },
  { 221, 222, 261, 262 },
  { 214, 215, 255, 254 },
  { 269, 270, 269, 270 },
  { 271, 310, 309, 311 },
  { 272, 273, 312, 313 },
  { 274, 275, 314, 315 },
  { 283, 322, 281, 242 },
  { 287, 288, 327, 328 },
  { 331, 332, 372, 371 },
  { 422, 345, 303, 388 },
  { 301, 308, 424, 425 },
  { 302, 348, 423, 385 },
  { 303, 308, 424, 425 },
  { 304, 305, 467, 466 },
  { 306, 344, 427, 381 },
  { 307, 384, 426, 341 },
  { 333, 334, 374, 373 }, 
  { 386, 342, 347, 383 },
  { 387, 382, 346, 343 },
  { 349, 350, 349, 350 },
  { 351, 390, 389, 391 },
  { 352, 353, 392, 393 },
  { 354, 355, 394, 395 },
  { 434, 435, 434, 435 },
  { 436, 475, 474, 476 },
  { 437, 514, 477, 554 },
  { 515, 516, 555, 556 },
  { 419, 420, 459, 421 },
  { 410, 411, 451, 450 },
  { 458, 460, 500, 499 },
  { 461, 463, 543, 541 },
  { 462, 503, 542, 501 },
  { 504, 627, 584, 625 },
  { 544, 626, 544, 626 },
  { 505, 507, 587, 585 },
  { 506, 547, 586, 545 },
  { 537, 660, 617, 658 },
  { 538, 540, 620, 618 },
  { 539, 580, 619, 578 },
  { 444, 606, 486, 566 },
  { 484, 526, 446, 646 },
  { 577, 659, 577, 659 },
  { 686, 693, 809, 810 },
  { 687, 733, 808, 770 },
  { 688, 773, 807, 730 },
  { 726, 692, 769, 811 },
  { 766, 691, 729, 812 },
  { 728, 772, 767, 731 },
  { 768, 771, 727, 732 },
  { 806, 689, 690, 813 },
  { 694, 696, 776, 774 },
  { 695, 736, 775, 734 },
  { 697, 816, 777, 814 },
  { 815, 737, 815, 737 },
  { 698, 700, 780, 778 },
  { 699, 740, 779, 738 },
  { 742, 703, 744, 783 },
  { 702, 704, 784, 782 },
  { 701, 820, 781, 818 },
  { 819, 741, 819, 741 },
  { 1766, 1767, 1807, 1806 },
  { 1768, 1809, 1808, 1769 },
  { 1840, 1843, 1963, 1960 },
  { 1841, 1883, 1962, 1920 },
  { 1842, 1923, 1961, 1880 },
  { 1881, 1882, 1922, 1921 },
  { 2680, 2681, 2721, 2720 },
  { 2768, 2771, 2808, 2772 },
  { 2848, 2773, 2848, 2773 },
  { 2769, 2811, 2809, 2812 },
  { 2849, 2813, 2849, 2813 },
  { 2762, 2763, 2760, 2761 },
  { 2682, 2683, 2723, 2722 },
  { 2845, 2767, 2845, 2767 },
  { 2727, 2846, 2807, 2844 },
  { 2725, 2766, 2805, 2764 },
  { 2724, 2726, 2806, 2804 },
  { 2892, 2893, 2892, 2893 },
  { 3085, 3092, 3208, 3209 },
  { 3087, 3172, 3206, 3129 },
  { 3086, 3132, 3207, 3169 },
  { 3088, 3089, 3212, 3205 },
  { 3160, 3043, 3161, 3083 },
  { 3162, 3123, 3162, 3123 },
  { 3163, 3084, 3164, 3124 },
  { 3040, 3042, 3122, 3120 },
  { 3041, 3082, 3121, 3080 },
  { 3125, 3091, 3168, 3210 },
  { 3127, 3171, 3166, 3130 },
  { 3165, 3090, 3128, 3211 },
  { 3167, 3170, 3126, 3131 },

  /* Anims Now */

  { ANIM(0), ANIM(1), ANIM(3), ANIM(2) },
  { ANIM(4), ANIM(5), ANIM(7), ANIM(6) },
  { ANIM(8), ANIM(9), ANIM(11), ANIM(10) },
  { ANIM(0xC), ANIM(0xD), ANIM(0xC), ANIM(0xD) },
  { ANIM(0x32), ANIM(0x33), ANIM(0x35), ANIM(0x34) },
  { ANIM(0x36), ANIM(0x37), ANIM(0x39), ANIM(0x38) },
  { ANIM(0x42), ANIM(0x44), ANIM(0x4A), ANIM(0x48) },
  { ANIM(0x43), ANIM(0x47), ANIM(0x49), ANIM(0x45) },
  { ANIM(0x4B), ANIM(0x4D), ANIM(0x53), ANIM(0x51) },
  { ANIM(0x4C), ANIM(0x50), ANIM(0x52), ANIM(0x4E) },
  { ANIM(0x54), ANIM(0x56), ANIM(0x5C), ANIM(0x5A) },
  { ANIM(0x55), ANIM(0x59), ANIM(0x5B), ANIM(0x57) },
  { ANIM(0x5D), ANIM(0x5F), ANIM(0x65), ANIM(0x60) },
  { ANIM(0x66), ANIM(0x68), ANIM(0x6E), ANIM(0x6C) },
  { ANIM(0x67), ANIM(0x6B), ANIM(0x6D), ANIM(0x69) },
  { ANIM(0x6F), ANIM(0x76), ANIM(0x6F), ANIM(0x76) },
  { ANIM(0x70), ANIM(0x74), ANIM(0x71), ANIM(0x75) },
  { ANIM(0x72), ANIM(0x77), ANIM(0x72), ANIM(0x77) },
  { ANIM(0x79), ANIM(0x77), ANIM(0x79), ANIM(0x77) },
  { ANIM(0x78), ANIM(0x89), ANIM(0x7A), ANIM(0x8A) },
  { ANIM(0x94), ANIM(0xB1), ANIM(0xA7), ANIM(0xB7) },
  { ANIM(0x95), ANIM(0xB2), ANIM(0xA8), ANIM(0xB8) },
  { ANIM(0x96), ANIM(0xB3), ANIM(0xA9), ANIM(0xB9) },
  { ANIM(0x97), ANIM(0xB4), ANIM(0xAA), ANIM(0xBA) },
  { ANIM(0x98), ANIM(0xB5), ANIM(0xAB), ANIM(0xBB) },
  { ANIM(0x99), ANIM(0xB6), ANIM(0xAC), ANIM(0xBC) },
  { ANIM(0x9A), ANIM(0x9C), ANIM(0xA2), ANIM(0xA0) },
  { ANIM(0x9B), ANIM(0x9F), ANIM(0xA1), ANIM(0x9D) },
  { ANIM(0xAD), ANIM(0xAE), ANIM(0xB0), ANIM(0xAF) },
  { ANIM(0xD6), ANIM(0xD9), ANIM(0xD8), ANIM(0xD7) },
  { ANIM(0xDA), ANIM(0xDD), ANIM(0xDC), ANIM(0xDB) },
  { ANIM(0xDE), ANIM(0xE1), ANIM(0xE0), ANIM(0xDF) },
  { ANIM(0xE2), ANIM(0xE5), ANIM(0xE4), ANIM(0xE3) },
  { ANIM(0xE6), ANIM(0xE7), ANIM(0xE9), ANIM(0xE8) },
  { ANIM(0xEA), ANIM(0xEB), ANIM(0xED), ANIM(0xEC) },
  { ANIM(0xEE), ANIM(0xEF), ANIM(0xF1), ANIM(0xF0) },
  { ANIM(0xF2), ANIM(0xF3), ANIM(0xF5), ANIM(0xF4) },
  { -1, -1, -1, FALSE }
};
rdTbl NoRd;

mdTbl mirTbl[512] = {
  /* Tiles First */

  { 8, 10, 8, 10 },
  { 9, 9, 50, 50 },
  { 11, 51, 11, 51 },
  { 12, 12, 52, 52 },
  { 16, 16, 17, 17 },
  { 18, 19, 18, 19 },
  { 46, 47, 48, 49 },
  { 53, 55, 54, 56 },
  { 57, 60, 57, 60 },
  { 58, 58, 59, 59 },
  { 88, 168, 88, 168 },
  { 124, 124, 126, 126 },
  { 162, 202, 163, 203 },
  { 164, 244, 167, 247 },
  { 165, 245, 166, 246 },
  { 204, 204, 207, 207 },
  { 205, 205, 206, 206 },
  { 256, 258, 218, 257 },
  { 259, 219, 259, 219 },
  { 260, 260, 220, 220 },
  { 221, 221, 261, 261 },
  { 222, 262, 222, 262 },
  { 285, 405, 285, 405 },
  { 309, 310, 271, 311 },
  { 312, 272, 312, 272 },
  { 313, 313, 273, 273 },
  { 274, 274, 314, 314 },
  { 275, 315, 275, 315 },
  { 301, 422, 303, 424 },
  { 302, 423, 302, 423 },
  { 341, 382, 343, 384 },
  { 381, 342, 383, 344 },
  { 304, 466, 305, 467 },
  { 308, 388, 345, 425 },
  { 348, 348, 385, 385 },
  { 307, 387, 346, 426 },
  { 306, 386, 347, 427 },
  { 389, 391, 351, 390 },
  { 392, 352, 392, 352 },
  { 393, 393, 353, 353 },
  { 354, 354, 394, 394 },
  { 355, 395, 355, 395 },
  { 474, 476, 436, 475 },
  { 515, 515, 555, 555 },
  { 516, 556, 516, 556 },
  { 437, 477, 437, 477 },
  { 514, 514, 554, 554 },
  { 444, 484, 446, 486 },
  { 526, 566, 606, 646 },
  { 458, 499, 460, 500 },
  { 459, 421, 420, 419 },
  { 461, 541, 463, 543 },
  { 462, 542, 462, 542 },
  { 501, 501, 503, 503 },
  { 504, 584, 504, 584 },
  { 625, 625, 627, 627 },
  { 505, 585, 507, 587 },
  { 506, 586, 506, 586 },
  { 545, 545, 547, 547 },
  { 537, 617, 537, 617 },
  { 658, 658, 660, 660 },
  { 538, 618, 540, 620 },
  { 539, 619, 539, 619 },
  { 578, 578, 580, 580 },
  { 686, 807, 688, 809 },
  { 687, 808, 687, 808 },
  { 726, 767, 728, 769 },
  { 766, 727, 768, 729 },
  { 689, 806, 690, 813 },
  { 693, 773, 730, 810 },
  { 733, 733, 770, 770 },
  { 692, 772, 731, 811 },
  { 691, 771, 732, 812 },
  { 694, 774, 696, 776 },
  { 695, 775, 695, 775 },
  { 734, 734, 736, 736 },
  { 697, 777, 697, 777 },
  { 814, 814, 816, 816 },
  { 698, 778, 700, 780 },
  { 699, 779, 699, 779 },
  { 738, 738, 740, 740 },
  { 818, 818, 820, 820 },
  { 701, 781, 701, 781 },
  { 702, 782, 704, 784 },
  { 703, 783, 703, 783 },
  { 742, 742, 744, 744 },
  { 2680, 2720, 2681, 2721 },
  { 2682, 2722, 2683, 2723 },
  { 2760, 2762, 2760, 2762 },
  { 2761, 2761, 2763, 2763 },
  { 2724, 2804, 2726, 2806 },
  { 2725, 2805, 2725, 2805 },
  { 2764, 2764, 2766, 2766 },
  { 2727, 2807, 2727, 2807 },
  { 2844, 2844, 2846, 2846 },
  { 2800, 2840, 2801, 2841 },
  { 2920, 2960, 2921, 2961 },
  { 2929, 2969, 2930, 2970 },
  { 3040, 3120, 3042, 3122 },
  { 3041, 3121, 3041, 3121 },
  { 3080, 3080, 3082, 3082 },
  { 3084, 3124, 3084, 3124 },
  { 3163, 3163, 3164, 3164 },
  { 3085, 3026, 3087, 3028 },
  { 3086, 3207, 3086, 3207 },
  { 3125, 3166, 3127, 3168 },
  { 3165, 3126, 3167, 3128 },
  { 3088, 3205, 3089, 3212 },
  { 3092, 3172, 3129, 3209 },
  { 3132, 3132, 3169, 3169 },
  { 3091, 3171, 3130, 3210 },
  { 3090, 3170, 3131, 3211 },

  /* Now the Anims */

  { ANIM(0x04), ANIM(0x07), ANIM(0x04), ANIM(0x07) },
  { ANIM(0x05), ANIM(0x05), ANIM(0x06), ANIM(0x06) },
  { ANIM(0x08), ANIM(0x0B), ANIM(0x08), ANIM(0x0B) },
  { ANIM(0x09), ANIM(0x09), ANIM(0x0A), ANIM(0x0A) },
  { ANIM(0x32), ANIM(0x35), ANIM(0x32), ANIM(0x35) },
  { ANIM(0x33), ANIM(0x33), ANIM(0x34), ANIM(0x34) },
  { ANIM(0x36), ANIM(0x39), ANIM(0x36), ANIM(0x39) },
  { ANIM(0x37), ANIM(0x37), ANIM(0x38), ANIM(0x38) },
  { ANIM(0x42), ANIM(0x48), ANIM(0x44), ANIM(0x4A) },
  { ANIM(0x43), ANIM(0x49), ANIM(0x43), ANIM(0x49) },
  { ANIM(0x45), ANIM(0x45), ANIM(0x47), ANIM(0x47) },
  { ANIM(0x4b), ANIM(0x51), ANIM(0x4d), ANIM(0x53) },
  { ANIM(0x4c), ANIM(0x52), ANIM(0x4c), ANIM(0x52) },
  { ANIM(0x4e), ANIM(0x4e), ANIM(0x50), ANIM(0x50) },
  { ANIM(0x54), ANIM(0x5a), ANIM(0x56), ANIM(0x5c) },
  { ANIM(0x55), ANIM(0x5b), ANIM(0x55), ANIM(0x5b) },
  { ANIM(0x57), ANIM(0x57), ANIM(0x59), ANIM(0x59) },
  { ANIM(0x5d), ANIM(0x63), ANIM(0x5f), ANIM(0x65) },
  { ANIM(0x5e), ANIM(0x64), ANIM(0x5e), ANIM(0x64) },
  { ANIM(0x60), ANIM(0x60), ANIM(0x62), ANIM(0x62) },
  { ANIM(0x66), ANIM(0x6c), ANIM(0x68), ANIM(0x6e) },
  { ANIM(0x67), ANIM(0x6d), ANIM(0x67), ANIM(0x6d) },
  { ANIM(0x69), ANIM(0x6b), ANIM(0x69), ANIM(0x6b) },
  { ANIM(0x70), ANIM(0x70), ANIM(0x71), ANIM(0x71) },
  { ANIM(0x74), ANIM(0x75), ANIM(0x74), ANIM(0x75) },
  { ANIM(0x78), ANIM(0x78), ANIM(0x7a), ANIM(0x7a) },
  { ANIM(0x89), ANIM(0x8a), ANIM(0x89), ANIM(0x8a) },
  { ANIM(0x94), ANIM(0xa7), ANIM(0x94), ANIM(0xa7) },
  { ANIM(0x95), ANIM(0xa8), ANIM(0x95), ANIM(0xa8) },
  { ANIM(0x96), ANIM(0xa9), ANIM(0x96), ANIM(0xa9) },
  { ANIM(0x97), ANIM(0xaa), ANIM(0x97), ANIM(0xaa) },
  { ANIM(0x98), ANIM(0xab), ANIM(0x98), ANIM(0xab) },
  { ANIM(0x99), ANIM(0xac), ANIM(0x99), ANIM(0xac) },
  { ANIM(0x9a), ANIM(0xa0), ANIM(0x9c), ANIM(0xa2) },
  { ANIM(0x9b), ANIM(0xa1), ANIM(0x9b), ANIM(0xa1) },
  { ANIM(0x9d), ANIM(0x9d), ANIM(0x9f), ANIM(0x9f) },
  { ANIM(0xad), ANIM(0xaf), ANIM(0xae), ANIM(0xb0) },
  { ANIM(0xb1), ANIM(0xb1), ANIM(0xbb), ANIM(0xbb) },
  { ANIM(0xb2), ANIM(0xb2), ANIM(0xbc), ANIM(0xbc) },
  { ANIM(0xb3), ANIM(0xb3), ANIM(0xb9), ANIM(0xb9) },
  { ANIM(0xb4), ANIM(0xb4), ANIM(0xba), ANIM(0xba) },
  { ANIM(0xb5), ANIM(0xb5), ANIM(0xb7), ANIM(0xb7) },
  { ANIM(0xb6), ANIM(0xb6), ANIM(0xb8), ANIM(0xb8) },
  { ANIM(0xd6), ANIM(0xd8), ANIM(0xd6), ANIM(0xd8) },
  { ANIM(0xd7), ANIM(0xd7), ANIM(0xd9), ANIM(0xd9) },
  { ANIM(0xda), ANIM(0xdc), ANIM(0xda), ANIM(0xdc) },
  { ANIM(0xdb), ANIM(0xdb), ANIM(0xdd), ANIM(0xdd) },
  { ANIM(0xde), ANIM(0xe0), ANIM(0xde), ANIM(0xe0) },
  { ANIM(0xdf), ANIM(0xdf), ANIM(0xe1), ANIM(0xe1) },
  { ANIM(0xe2), ANIM(0xe4), ANIM(0xe2), ANIM(0xe4) },
  { ANIM(0xe3), ANIM(0xe3), ANIM(0xe5), ANIM(0xe5) },
  { ANIM(0xe6), ANIM(0xe8), ANIM(0xe6), ANIM(0xe8) },
  { ANIM(0xe7), ANIM(0xe7), ANIM(0xe9), ANIM(0xe9) },
  { ANIM(0xea), ANIM(0xec), ANIM(0xea), ANIM(0xec) },
  { ANIM(0xeb), ANIM(0xeb), ANIM(0xed), ANIM(0xed) },
  { ANIM(0xee), ANIM(0xf0), ANIM(0xee), ANIM(0xf0) },
  { ANIM(0xef), ANIM(0xef), ANIM(0xf1), ANIM(0xf1) },
  { ANIM(0xf2), ANIM(0xf4), ANIM(0xf2), ANIM(0xf4) },
  { ANIM(0xf3), ANIM(0xf3), ANIM(0xf5), ANIM(0xf5) },

  { -1, -1, -1, -1 }
};
mdTbl NoMd;

int ReadMRData() {
	int i, fd;
	unsigned short sz;
	char dir[256];

	GetCurrentDirectory(sizeof(dir), dir);
	SetCurrentDirectory(arcdir);
	if ((fd = _open("mrdata.dat", _O_RDONLY|_O_BINARY)) == -1) {
		return -1;
	}
	
	for (i = 0;i < 1024;i++) {
		if (mirTbl[i].tile == -1) break;
		NoMd.tile = NoMd.horz = NoMd.vert = NoMd.hv = -1;
		memcpy(&mirTbl[i], &NoMd, sizeof(mdTbl));
	}
	for (i = 0;i < 1024;i++) {
		if (rotTbl[i].tile == -1) break;
		NoRd.tile = NoRd.deg90 = NoRd.deg180 = NoRd.deg270 = -1;
		memcpy(&rotTbl[i], &NoRd, sizeof(rdTbl));
	}
	
	_read(fd, &sz, 2);
	for (i = 0; sz > 0; i++)
		sz -= _read(fd, &mirTbl[i], sizeof(mdTbl));
	NoMd.tile = NoMd.horz = NoMd.vert = NoMd.hv = -1;
	memcpy(&mirTbl[i], &NoMd, sizeof(mdTbl));
	
	_read(fd, &sz, 2);
	for (i = 0; sz > 0; i++)
		sz -= _read(fd, &rotTbl[i], sizeof(rdTbl));
	NoRd.tile = NoRd.deg90 = NoRd.deg180 = NoRd.deg270 = -1;
	memcpy(&rotTbl[i], &NoMd, sizeof(rdTbl));
	
	_close(fd);

	SetCurrentDirectory(dir);
	return 0;
}

rdTbl PseudoRD(int tile, BOOL animfind) {
	rdTbl *boo = &rotTbl[0];
	rdTbl pseudo;
	int i;
	int idx = animfind == TRUE?0xFF:0x7FFF;
	
	for (i = 0; boo->tile != -1; boo = &rotTbl[++i]) {
		
		if ((animfind && !(boo->tile & 0x8000)) || (!animfind && (boo->tile & 0x8000)))
			continue;
		
		if ((boo->tile & idx) == tile) {
			memcpy(&pseudo, boo, sizeof(pseudo));
			return pseudo;
		}
		if ((boo->deg90 & idx) == tile) {
			pseudo.tile = boo->deg90;
			pseudo.deg90 = boo->deg180;
			pseudo.deg180 = boo->deg270;
			pseudo.deg270 = boo->tile;
			return pseudo;
		}
		else if ((boo->deg180 & idx) == tile) {
			pseudo.tile = boo->deg180;
			pseudo.deg90 = boo->deg270;
			pseudo.deg180 = boo->tile;
			pseudo.deg270 = boo->deg90;
			return pseudo;
		}
		else if ((boo->deg270 & idx) == tile) {
			pseudo.tile = boo->deg270;
			pseudo.deg90 = boo->tile;
			pseudo.deg180 = boo->deg90;
			pseudo.deg270 = boo->deg180;
			return pseudo;
		}
	}
	NoRd.tile = NoRd.deg90 = NoRd.deg180 = NoRd.deg270 = -1;
	return NoRd;
}

mdTbl PseudoMD(int tile, BOOL animfind) {
	mdTbl *boo = &mirTbl[0];
	mdTbl pseudo;
	int i;
	int idx = animfind == TRUE?0xFF:0x7FFF;
	
	for (i = 0; boo->tile != -1; boo = &mirTbl[++i]) {
		
		if ((animfind && !(boo->tile & 0x8000)) || (!animfind && (boo->tile & 0x8000)))
			continue;
		
		if ((boo->tile & idx) == tile) {
			pseudo.tile = boo->tile;
			pseudo.horz = boo->horz;
			pseudo.vert = boo->vert;
			pseudo.hv = boo->hv;
			return pseudo;
		}
		else if ((boo->horz & idx) == tile) {
			pseudo.tile = boo->horz;
			pseudo.horz = boo->tile;
			pseudo.vert = boo->hv;
			pseudo.hv = boo->vert;
			return pseudo;
		}
		else if ((boo->vert & idx) == tile) {
			pseudo.tile = boo->vert;
			pseudo.horz = boo->hv;
			pseudo.vert = boo->tile;
			pseudo.hv = boo->horz;
			return pseudo;
		}
		else if ((boo->hv & idx) == tile) {
			pseudo.tile = boo->hv;
			pseudo.horz = boo->vert;
			pseudo.vert = boo->horz;
			pseudo.hv = boo->tile;
			return pseudo;
		}
	}
	NoMd.tile = NoMd.horz = NoMd.vert = NoMd.hv = -1;
	return NoMd;
}

void ClearStatus() {
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, 0, 0);
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, 1, 0);
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, 2, 0);
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, 3, 0);
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, 4, 0);
}

void SetStatusText(int part, char *string) {
	SendDlgItemMessage(frameWnd, ID_STATUS, SB_SETTEXT, part, (LPARAM)string);
}

void swapTiles(int sx, int sy, int w, int h, int x, int y) {
	WORD *tooldData, *fromoldData;
	int i, j, n = 0;
	int dy, dx, stx, sty;
	
	tooldData = (WORD *)malloc(w * h * 2);
	fromoldData = (WORD *)malloc(w * h * 2);
	for (j = y; j < (y+h); j++) {
		for (i = x; i < (x+w); i++, n++) {
			tooldData[n] = tileArrange[j*40+i];
			fromoldData[n] = tileArrange[(j-y + sy)*40+(i-x+sx)];
		}
	}
	// ok the "old" stuff is in a buffer, now shift
	for (j = y; j < (y+h); j++) {
		for (i = x; i < (x+w); i++) {
			tileArrange[j*40+i] = fromoldData[(j-y)*w+(i-x)];
		}
	}    
	
	// and our shift is complete, now just put back remainder tiles
	
	dy = sy - y;
	dx = sx - x;
	
	if (sy > y) {
		// block shifted upwards.
		
		sty = y + h;
		stx = x + w - 1; // backwards shift
		
		for (j = sty; j > y; j--) {
			for (i = stx; i >= x; i--) {
				if ((sty - j) < dy ||
					(stx - i) < dx)
					tileArrange[j*40+i] = tooldData[(sty-j)*w + (stx-i)];
			}
		}
	}
	else if (sy < y) {
		// block shifted downwards.
		
		sty = y - 1;
		stx = x;  // fowards shift
		
		for (j = sty; j < (sty+h); j++) {
			for (i = stx; i < (stx+w); i++) {
				if ((i - stx) < ABS(dx) ||
					(j - sty) < ABS(dy))
					tileArrange[j*40+i] = tooldData[w*h - 1 - (j - sty)*w - (i - stx)];
			}
		}
	}
	else if (dy == 0) {
		if (sx > x) {
			// block shifted left
			sty = y + h - 1;
			stx = x + w; // backwards shift
			
			for (j = sty; j >= y; j--) {
				for (i = stx; i > x; i--) {
					if ((sty - j) < dy ||
						(stx - i) < dx)
						tileArrange[j*40+i] = tooldData[(sty-j)*w + (stx-i)];
				}
			}
		}
		else if (sx < x) {
			// block shifted right
			
			sty = y;
			stx = x - 1;  // fowards shift
			
			for (j = sty; j < (sty+h); j++) {
				for (i = stx; i < (stx+w); i++) {
					if ((i - stx) < ABS(dx) ||
						(j - sty) < ABS(dy))
						tileArrange[j*40+i] = tooldData[w*h - 1 - (j - sty)*w - (i - stx)];
				}
			}
		}
	}    
	
	free(tooldData);
	free(fromoldData);
}

void swapSwap(int sx, int sy, int w, int h, int x, int y) {
	int i, j;
	
	// AREAS MAY NOT OVERLAP OR YOU LOSE TILES
	
	for (j = sy; j < (sy + h); j++) {
		for (i = sx; i < (sx + w); i++) {
			tileArrange[j*40+i] ^=
				tileArrange[(j-sy+y)*40+(i-sx+x)] ^=
				tileArrange[j*40+i] ^=
				tileArrange[(j-sy+y)*40+(i-sx+x)];
		}
	}
}

// WALLS

int get_wall_type(int tile) {
	int x, y;

	for (x = 0; x < MAX_WALL_TYPES; x++) {
		for (y = 0; y < MAX_WALL_CONNS; y++) {
			if (wall_data[x][y] == tile) return x;
		}
	}
	return -1;
}

BOOL is_type(WORD tile, int type) {
	int x;

	for (x = 0; x < MAX_WALL_CONNS; x++) {
		if (wall_data[type][x] == tile) return TRUE;
	}
	return FALSE;
}

int wall_index_data[MAX_WALL_CONNS] = {	15, 14, 11, 10, 12, 13, 8, 9, 3, 2, 7, 6, 0, 1, 4, 5 };

void set_wall_tile(struct MapInfo *map, int x, int y, int type) {
	long flags = 0, oflags = 0, wt;
	int tile;

	// first find out what kind of wall piece we need
	if (!map || y >= map->header.height || x >= map->header.width || x < 0 || y < 0) return;
	if (type > MAX_WALL_TYPES || type < 0) return;

	if (x > 0)
		if ((wt = get_wall_type(map->mapData[y*map->header.width+x-1])) > -1 && !analWallMode || (analWallMode && is_type(map->mapData[y*map->header.width+x-1], type)))
			flags |= WALL_LEFT;
	if (x < map->header.width)
		if ((wt = get_wall_type(map->mapData[y*map->header.width+x+1])) > -1 && !analWallMode || (analWallMode && is_type(map->mapData[y*map->header.width+x+1], type)))
			flags |= WALL_RIGHT;
	if (y > 0)
		if ((wt = get_wall_type(map->mapData[(y-1)*map->header.width+x])) > -1 && !analWallMode || (analWallMode && is_type(map->mapData[(y-1)*map->header.width+x], type)))
			flags |= WALL_UP;
	if (y < map->header.height)
		if ((wt = get_wall_type(map->mapData[(y+1)*map->header.width+x])) > -1 && !analWallMode || (analWallMode && is_type(map->mapData[(y+1)*map->header.width+x], type)))
			flags |= WALL_DOWN;

	tile = wall_data[type][wall_index_data[flags]];
	if (tile != -1)
		map->mapData[y*map->header.width+x] = tile;
	if (flags & WALL_LEFT) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->mapData[y*map->header.width+x-1])][wall_index_data[oflags]] == map->mapData[y*map->header.width+x-1]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_RIGHT;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->mapData[y*map->header.width+x-1] = tile;
		}
	}
	if (flags & WALL_RIGHT) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->mapData[y*map->header.width+x+1])][wall_index_data[oflags]] == map->mapData[y*map->header.width+x+1]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_LEFT;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->mapData[y*map->header.width+x+1] = tile;
		}
	}
	if (flags & WALL_UP) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->mapData[(y-1)*map->header.width+x])][wall_index_data[oflags]] == map->mapData[(y-1)*map->header.width+x]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_DOWN;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->mapData[(y-1)*map->header.width+x] = tile;
		}
	}
	if (flags & WALL_DOWN) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->mapData[(y+1)*map->header.width+x])][wall_index_data[oflags]] == map->mapData[(y+1)*map->header.width+x]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_UP;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->mapData[(y+1)*map->header.width+x] = tile;
		}
	}
}
/*
void primitive_wall_tile(struct MapInfo *map, int x, int y, int type) {
	long flags = 0, oflags = 0, wt;
	int tile, width, height;

	width = abs(map->lineTool.start.x - map->lineTool.stop.x );
	height = abs(map->lineTool.start.y - map->lineTool.stop.y);

	// first find out what kind of wall piece we need
	if (!map || y >= height || x >= width || x < 0 || y < 0) return;
	if (type > MAX_WALL_TYPES || type < 0) return;

	

	if (x > 0)
		if ((wt = get_wall_type(map->lineTool.buffer[y*width+x-1])) > -1 && !analWallMode || (analWallMode && is_type(map->lineTool.buffer[y*width+x-1], type)))
			flags |= WALL_LEFT;
	if (x < width-1)
		if ((wt = get_wall_type(map->lineTool.buffer[y*width+x+1])) > -1 && !analWallMode || (analWallMode && is_type(map->lineTool.buffer[y*width+x+1], type)))
			flags |= WALL_RIGHT;
	if (y > 0)
		if ((wt = get_wall_type(map->lineTool.buffer[(y-1)*width+x])) > -1 && !analWallMode || (analWallMode && is_type(map->lineTool.buffer[(y-1)*width+x], type)))
			flags |= WALL_UP;
	if (y < height-1)
		if ((wt = get_wall_type(map->lineTool.buffer[(y+1)*width+x])) > -1 && !analWallMode || (analWallMode && is_type(map->lineTool.buffer[(y+1)*width+x], type)))
			flags |= WALL_DOWN;

	tile = wall_data[type][wall_index_data[flags]];
	if (tile != -1)
		map->lineTool.buffer[y*width+x] = tile;
	if (flags & WALL_LEFT) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->lineTool.buffer[y*width+x-1])][wall_index_data[oflags]] == map->lineTool.buffer[y*width+x-1]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_RIGHT;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->lineTool.buffer[y*width+x-1] = tile;
		}
	}
	if (flags & WALL_RIGHT) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->lineTool.buffer[y*width+x+1])][wall_index_data[oflags]] == map->lineTool.buffer[y*width+x+1]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_LEFT;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->lineTool.buffer[y*width+x+1] = tile;
		}
	}
	if (flags & WALL_UP) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->lineTool.buffer[(y-1)*width+x])][wall_index_data[oflags]] == map->lineTool.buffer[(y-1)*width+x]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_DOWN;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->lineTool.buffer[(y-1)*width+x] = tile;
		}
	}
	if (flags & WALL_DOWN) {
		for (oflags = 0; oflags < MAX_WALL_CONNS; oflags++)
			if (wall_data[get_wall_type(map->lineTool.buffer[(y+1)*width+x])][wall_index_data[oflags]] == map->lineTool.buffer[(y+1)*width+x]) break;
		if (oflags != MAX_WALL_CONNS)  {
			oflags |= WALL_UP;
			tile = wall_data[type][wall_index_data[oflags]];
			if (tile != -1)
				map->lineTool.buffer[(y+1)*width+x] = tile;
		}
	}
}
*/

void MyFocusRect(HDC dc, RECT *rc) {
	int i;
	RECT rcold = *rc;

	for (i = 0; i < 4; i++) {
		DrawFocusRect(dc, rc);
		rc->left++; rc->right--;
		rc->top++; rc->bottom--;
	}

	*rc = rcold;
}

size_t flength(FILE *f) {
	long pos = ftell(f);
	size_t sz;

	fseek(f, 0, SEEK_END);
	sz = (size_t)ftell(f);
	fseek(f, pos, SEEK_SET);
	return sz;
}

BITMAPINFO *jpeg_2_bmp(FILE *jpdata) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	size_t jpsz = flength(jpdata);
	int rowsz, pad, j, i, z;
	BYTE *inptr, *p;
	JSAMPARRAY buffer;
	BITMAPINFO *bmi;
	
	rewind(jpdata);
	
	memset(&cinfo, 0, sizeof(cinfo));
	memset(&jerr, 0, sizeof(jerr));
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	jpeg_stdio_src(&cinfo, jpdata);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	rowsz = cinfo.output_width * cinfo.output_components;
	while (rowsz & 3) rowsz++;
	pad = rowsz - cinfo.output_width * cinfo.output_components;
	
	bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + rowsz * cinfo.output_height + 1);
	memset(bmi, 0, sizeof(BITMAPINFO) + rowsz * cinfo.output_height + 1);
    bmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth       = cinfo.output_width;
    bmi->bmiHeader.biHeight      = (int)(cinfo.output_height); // bottom-up image
    bmi->bmiHeader.biPlanes      = 1;
    bmi->bmiHeader.biBitCount    = cinfo.output_components << 3;
    bmi->bmiHeader.biCompression = BI_RGB;
    
	p = (BYTE *)bmi + sizeof(BITMAPINFO);

	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_components * cinfo.output_width, 1);
	
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		inptr = buffer[0];

		for (j = 0; j < (int)cinfo.output_width; j++) {
			for (z = cinfo.output_components; z--; )
				p[z] = *inptr++;
			p += cinfo.output_components;
		}
		for (j = pad; j--; ) *p++ = 0;
	}

	p = (BYTE *)bmi + sizeof(BITMAPINFO);
	for (j = 0; j < (int)((float)cinfo.output_height/2.0); j++) {
		for (i = 0; i < (int)cinfo.output_width*cinfo.output_components; i++)
			p[i+j*rowsz] ^= p[i+(cinfo.output_height-j-1)*rowsz] ^= p[i+j*rowsz] ^= p[i+(cinfo.output_height-j-1)*rowsz];
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return bmi;
}

BITMAPINFO *j2bres(int res) {
	HGLOBAL loaded;
	HRSRC found;
	BYTE *p, *end;
	FILE *infile;
	BITMAPINFO *bmi;
	int sz;

	found = FindResource(hInstance, MAKEINTRESOURCE(res), "JPEG");
	if (found) loaded = LoadResource(hInstance, found);
	if (found && loaded) {
		sz = SizeofResource(hInstance, found);
        p = (BYTE *)LockResource(loaded);
		end = p + sz;

		infile = fopen("nak.jpg", "w+b");
		while (p < end) {
			size_t x = fwrite(p, 1, (end - p > 16384)?16384:(int)(end - p), infile);
			p += (int)x;
		}
		fclose(infile);

		if ((infile = fopen("nak.jpg", "rb")) != NULL) {
			bmi = jpeg_2_bmp(infile);
			fclose(infile);
			unlink("nak.jpg");	
			return bmi;
		}
	}
	return NULL;
}

BITMAPINFO *merge_fpov(BITMAPINFO *ip_bmi, BYTE *bmp) {
	BITMAPINFO *bmi;
	BYTE *foo;
	int x, y, r, g, b;


    bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + fpBmpInfo->bmiHeader.biHeight * fpBmpInfo->bmiHeader.biWidth * 3);
	foo = (BYTE *)bmi + sizeof(BITMAPINFO);
	memcpy(bmi, fpBmpInfo, sizeof(BITMAPINFO) + fpBmpInfo->bmiHeader.biHeight * fpBmpInfo->bmiHeader.biWidth * 3);
/*
	for (y = 0; y < abs(ip_bmi->bmiHeader.biHeight); y++) {
		for (x = 0; x < ip_bmi->bmiHeader.biWidth * 3; x += 3) {
			b= *bmp++;
			g = *bmp++;
			r = *bmp++;

			d = (abs(199 - r) + abs(44 - g) + abs(88 - b))/3;
			d2 = (abs(150 - r) + abs(40 - g) + abs(210 - b))/3;
			if (d2 < 20 || d < 20)
				foo += 3;
			else {
				*foo++ = b;
				*foo++ = g;
				*foo++ = r;				
			}
			
		}
	}
*/
	int height = abs(ip_bmi->bmiHeader.biHeight);
	int width = ip_bmi->bmiHeader.biWidth;
	__asm {
		mov y, 0
		mov ebx, foo
		mov ecx, bmp
loop1:
		mov eax, height
		cmp y, eax
		jge end
		mov x, 0
loop2:
		mov eax, width
		cmp x, eax
		jge loop1inc
		push ebx
		push ecx
		mov eax, [ecx]
		mov b, eax
		and b, 0xFF
		mov g, eax
		shr g, 8
		and g, 0xFF
		mov r, eax
		shr r, 16
		and r, 0xFF

		mov eax, r
		mov edx, 150
		cmp eax, edx
		jg rfirst
		sub edx, eax
		mov ebx, edx
		jmp green
rfirst:
		sub eax, edx
		mov ebx, eax
green:
		mov eax, g
		mov edx, 40
		cmp eax, edx
		jg gfirst
		sub edx, eax
		add ebx, edx
		jmp blue
gfirst:
		sub eax, edx
		add ebx, eax
blue:
		mov eax, b
		mov edx, 210
		cmp eax, edx
		jg bfirst
		sub edx, eax
		add ebx, edx
		jmp trans
bfirst:
		sub eax, edx
		add ebx, eax
trans:
		xor edx, edx
		mov eax, ebx
		mov ebx, 3
		div bx
		pop ecx
		pop ebx
		cmp eax, 20
        jle nextpix
		push ebx
		push ecx

		mov eax, r
		mov edx, 199
		cmp eax, edx
		jg rfirst2
		sub edx, eax
		mov ebx, edx
		jmp green2
rfirst2:
		sub eax, edx
		mov ebx, eax
green2:
		mov eax, g
		mov edx, 44
		cmp eax, edx
		jg gfirst2
		sub edx, eax
		add ebx, edx
		jmp blue2
gfirst2:
		sub eax, edx
		add ebx, eax
blue2:
		mov eax, b
		mov edx, 88
		cmp eax, edx
		jg bfirst2
		sub edx, eax
		add ebx, edx
		jmp trans2
bfirst2:
		sub eax, edx
		add ebx, eax
trans2:
		xor edx, edx
		mov eax, ebx
		mov ebx, 3
		div bx
		pop ecx
		pop ebx
		cmp eax, 20
        jle nextpix
		mov eax, [ecx]
		mov [ebx], ax
		mov eax, [ecx+2]
		mov [ebx+2], al

nextpix:
		add ecx, 3
		add ebx, 3
		inc x
		jmp loop2
loop1inc:
		inc y
		jmp loop1
end:

	}

	return (BITMAPINFO *)bmi;
}