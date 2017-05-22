extern "C" {


	extern HWND             frameWnd, overDlg;
	extern HINSTANCE        hInstance;
	extern HMENU            hMenu;
	extern HACCEL           hAccel;
	extern HFONT			arialFont;
	extern button           *button_list;
	extern HDC              tilesDC;
	extern HPALETTE         tilesPal;
	extern struct Animation anim_list[256];
	extern struct MapInfo   *map_list, *infomap, *overmap;
	extern struct seldata   tileSel, animSel;
	extern struct ApplicationLayout applayout;

	extern BITMAPINFO       *overViewBmpInfo, *fpBmpInfo;
	extern BYTE             *overViewBmp, *fpBmp;

	extern int              currentTool, hWallType, hFlagType, hPoleType, hHoldingType;
	extern int				hTeamSelected, hSpawnType, hBunkerType, hBunkerDir, hSwitchType;
	extern int				hBridgeDir, hBridgeType, hWarpSrc, hWarpDest, hWarpIdx, hConvDir, hConvType;
	extern int				warps[], NUM_WARPS;
	extern char				*undo_types[];


	extern BOOL             useAnim;
	extern int				constrict;

	extern rdTbl            rotTbl[];
	extern mdTbl            mirTbl[];
	extern char             arcdir[256], lastdir[256];

	extern BOOL             minSysTray, autoAssociate, autoPan, saveArrange, analWallMode, miniOverview, displayAnims;

	extern WORD             *tileArrange;
	extern int              numTiles;

	extern int				MAX_WALL_TYPES, MAX_BUNKER_TYPES, MAX_POLE_TYPES, MAX_FLAG_TYPES, MAX_HOLDING_PEN_TYPES, MAX_SPAWN_TYPES, MAX_BRIDGE_UD_TYPES, MAX_BRIDGE_LR_TYPES, MAX_SWITCH_TYPES, MAX_CONV_UD_TYPES, MAX_CONV_LR_TYPES;
	extern int				**wall_data, **bunker_data, **pole_data, **flag_data, **holding_pen_data, **spawn_data;
	extern int				**bridge_lr_data, **bridge_ud_data, **switch_data, **conv_lr_data, **conv_ud_data;

	/* anim.c */
	extern int CALLBACK     AnimAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int              load_anims(char *);
	extern void             killAnims();
	extern struct Animation *get_anim(unsigned char);

	/* customize.c */
	extern int CALLBACK		CustomizeProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK		CustomizeEditProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK		CustomizeListProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK		CustomizeEditFrameProc(HWND,UINT,WPARAM,LPARAM);
	extern void				load_custom_data();
	extern void				save_custom_data();
	extern void				free_custom_data(BOOL);
	extern void				free_custom_edits(BOOL);

	/* frame.c */
	extern int CALLBACK     MalletEditFrameProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK     InfoAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK     SettingsAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK		WarpToolDlgProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK		WarpToolAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int              initMalletEdit();
	extern void             freeMem();
	extern void             SaveSettings();
	extern void             GetSettings();
	extern int              associateWithMap();

	/* map.c */
	extern int CALLBACK     MapAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int              CreateNewMap();
	extern void             killMap(struct MapInfo *);
	extern void             EditMap(int,int,BOOL, struct MapInfo *);
	extern void             killUndoBlocks(struct undo_blocks **);
	extern BOOL             performUndo(struct MapInfo *);
	extern BOOL             performRedo(struct MapInfo *);
	extern BOOL             performDelete(struct MapInfo *);
	extern BOOL             MirrorBits(int, struct MapInfo *);
	extern BOOL             RotateBits(struct MapInfo *);
	extern void				addSelectUndo(struct undo_blocks **,struct seldata *,struct MapInfo *);
	extern void             addUndo(struct undo_blocks **,struct undo_buf *, struct MapInfo *);
	extern void             LookOverFlags(struct MapInfo *);
	extern BOOL             SaveMap(char *, struct MapInfo *);
	extern int				OpenMap(char *);
	extern BOOL             copyBits(BOOL, struct MapInfo *);
	extern BOOL             pasteBits(struct MapInfo *);
	extern BOOL				FillBits(WORD, int, int, struct MapInfo *);
	extern BOOL				FillSelection(struct MapInfo *);
	extern struct MapInfo   *get_map(HWND);
	extern BOOL				iswarp(WORD);
	extern int				select_check(struct MapInfo *, POINT*);

	/* tile.c */
	extern int CALLBACK     TileAreaProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK     OVDataProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK     OverviewProc(HWND,UINT,WPARAM,LPARAM);
	extern int CALLBACK     MiniOverviewProc(HWND,UINT,WPARAM,LPARAM);
	extern BOOL             read_tiles(char *);
	extern void             DrawTileBlock(int,HDC,int,int,int,int, BOOL);
	extern void             make_overview();
	extern BYTE             *OverView(struct MapInfo *);
	extern void				ResizeTileArea();

	/* toolbar.c */
	extern button           *create_button(long,long,long,long);
	extern void             set_button_text(button *, char *);
	extern void				init_toolbar(HWND);
	extern void				obtain_disabled_bmp(button *, int);
	extern int CALLBACK     ToolbarWindowProc(HWND,UINT,WPARAM,LPARAM);
	extern void             create_shell_toolbar();
	extern void             DrawTBState(HDC,long,long,long,long,long);
	extern void             DrawToolbar();
	extern void				DisableSelectButtons();
	extern void				DisableButtons();
	extern void				EnableButtons();


	/* utils.c */
	extern size_t           string_len(char *);
	extern char             *dupestr(char *);
	extern void             DrawSelection(HDC,HPEN,int,int,int,int);
	extern void				NewSelection(HDC,HPEN,struct seldata *, struct seldata *,struct MapInfo *);
	extern void             BufferCompare(WORD*,WORD*,int,int,struct undo_buf**);
	extern int              count_list(struct undo_blocks *);
	extern BOOL             chop_list(struct undo_blocks **);
	extern rdTbl            PseudoRD(int,BOOL);
	extern mdTbl            PseudoMD(int,BOOL);
	extern void             ClearStatus();
	extern void             SetStatusText(int,char *);
	extern void             swapTiles(int,int,int,int,int,int);
	extern void             swapSwap(int,int,int,int,int,int);
	extern void				set_wall_tile(struct MapInfo *,int,int,int);
	extern void				primitive_wall_tile(struct MapInfo *, int, int, int);
	extern void				MyFocusRect(HDC,RECT *);
	extern BITMAPINFO		*jpeg_2_bmp(FILE *);
	extern BITMAPINFO		*j2bres(int);
	extern size_t			flength(FILE *);
	extern BITMAPINFO		*merge_fpov(BITMAPINFO *, BYTE *);

}