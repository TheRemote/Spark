package codemallet.spark.collision;

public final class GameCollision
{
	public static final int MaxTile = 4000;
	
	public static final boolean[] farPlaneOverride = new boolean[MaxTile];
	public static final boolean[] doNotDrawTile = new boolean[MaxTile];
	public static final boolean[] TileTankCollision = new boolean[MaxTile];
	
	public static final int[] TileSourceX = new int[MaxTile];
	public static final int[] TileSourceY = new int[MaxTile];
	
	static
	{
		//Farplane override
		farPlaneOverride[21] = true;
		farPlaneOverride[22] = true;
		farPlaneOverride[23] = true;
		farPlaneOverride[27] = true;
		farPlaneOverride[28] = true;
		farPlaneOverride[29] = true;
		farPlaneOverride[37] = true;
		farPlaneOverride[61] = true;
		farPlaneOverride[62] = true;
		farPlaneOverride[69] = true;
		farPlaneOverride[84] = true;
		farPlaneOverride[85] = true;
		farPlaneOverride[86] = true;
		farPlaneOverride[87] = true;
		farPlaneOverride[90] = true;
		farPlaneOverride[91] = true;
		farPlaneOverride[92] = true;
		farPlaneOverride[93] = true;
		farPlaneOverride[94] = true;
		farPlaneOverride[95] = true;
		farPlaneOverride[96] = true;
		farPlaneOverride[97] = true;
		farPlaneOverride[98] = true;
		farPlaneOverride[100] = true;
		farPlaneOverride[101] = true;
		farPlaneOverride[102] = true;
		farPlaneOverride[104] = true;
		farPlaneOverride[109] = true;
		farPlaneOverride[127] = true;
		farPlaneOverride[129] = true;
		farPlaneOverride[130] = true;
		farPlaneOverride[131] = true;
		farPlaneOverride[132] = true;
		farPlaneOverride[133] = true;
		farPlaneOverride[134] = true;
		farPlaneOverride[135] = true;
		farPlaneOverride[136] = true;
		farPlaneOverride[137] = true;
		farPlaneOverride[138] = true;
		farPlaneOverride[146] = true;
		farPlaneOverride[148] = true;
		farPlaneOverride[149] = true;
		farPlaneOverride[160] = true;
		farPlaneOverride[161] = true;
		farPlaneOverride[162] = true;
		farPlaneOverride[163] = true;
		farPlaneOverride[164] = true;
		farPlaneOverride[165] = true;
		farPlaneOverride[166] = true;
		farPlaneOverride[167] = true;
		farPlaneOverride[169] = true;
		farPlaneOverride[170] = true;
		farPlaneOverride[171] = true;
		farPlaneOverride[172] = true;
		farPlaneOverride[173] = true;
		farPlaneOverride[174] = true;
		farPlaneOverride[175] = true;
		farPlaneOverride[176] = true;
		farPlaneOverride[180] = true;
		farPlaneOverride[186] = true;
		farPlaneOverride[193] = true;
		farPlaneOverride[194] = true;
		farPlaneOverride[195] = true;
		farPlaneOverride[200] = true;
		farPlaneOverride[201] = true;
		farPlaneOverride[202] = true;
		farPlaneOverride[203] = true;
		farPlaneOverride[204] = true;
		farPlaneOverride[205] = true;
		farPlaneOverride[206] = true;
		farPlaneOverride[207] = true;
		farPlaneOverride[208] = true;
		farPlaneOverride[209] = true;
		farPlaneOverride[210] = true;
		farPlaneOverride[211] = true;
		farPlaneOverride[212] = true;
		farPlaneOverride[213] = true;
		farPlaneOverride[214] = true;
		farPlaneOverride[215] = true;
		farPlaneOverride[234] = true;
		farPlaneOverride[235] = true;
		farPlaneOverride[241] = true;
		farPlaneOverride[242] = true;
		farPlaneOverride[243] = true;
		farPlaneOverride[244] = true;
		farPlaneOverride[245] = true;
		farPlaneOverride[246] = true;
		farPlaneOverride[247] = true;
		farPlaneOverride[248] = true;
		farPlaneOverride[249] = true;
		farPlaneOverride[250] = true;
		farPlaneOverride[251] = true;
		farPlaneOverride[252] = true;
		farPlaneOverride[253] = true;
		farPlaneOverride[254] = true;
		farPlaneOverride[255] = true;
		farPlaneOverride[276] = true;
		farPlaneOverride[278] = true;
		farPlaneOverride[279] = true;
		farPlaneOverride[280] = true;
		farPlaneOverride[281] = true;
		farPlaneOverride[282] = true;
		farPlaneOverride[283] = true;
		farPlaneOverride[284] = true;
		farPlaneOverride[285] = true;
		farPlaneOverride[286] = true;
		farPlaneOverride[287] = true;
		farPlaneOverride[288] = true;
		farPlaneOverride[289] = true;
		farPlaneOverride[290] = true;
		farPlaneOverride[291] = true;
		farPlaneOverride[292] = true;
		farPlaneOverride[293] = true;
		farPlaneOverride[294] = true;
		farPlaneOverride[295] = true;
		farPlaneOverride[296] = true;
		farPlaneOverride[316] = true;
		farPlaneOverride[318] = true;
		farPlaneOverride[319] = true;
		farPlaneOverride[320] = true;
		farPlaneOverride[321] = true;
		farPlaneOverride[322] = true;
		farPlaneOverride[323] = true;
		farPlaneOverride[324] = true;
		farPlaneOverride[325] = true;
		farPlaneOverride[326] = true;
		farPlaneOverride[327] = true;
		farPlaneOverride[328] = true;
		farPlaneOverride[329] = true;
		farPlaneOverride[330] = true;
		farPlaneOverride[331] = true;
		farPlaneOverride[332] = true;
		farPlaneOverride[333] = true;
		farPlaneOverride[334] = true;
		farPlaneOverride[360] = true;
		farPlaneOverride[361] = true;
		farPlaneOverride[362] = true;
		farPlaneOverride[363] = true;
		farPlaneOverride[364] = true;
		farPlaneOverride[365] = true;
		farPlaneOverride[366] = true;
		farPlaneOverride[369] = true;
		farPlaneOverride[370] = true;
		farPlaneOverride[371] = true;
		farPlaneOverride[372] = true;
		farPlaneOverride[373] = true;
		farPlaneOverride[374] = true;
		farPlaneOverride[400] = true;
		farPlaneOverride[401] = true;
		farPlaneOverride[402] = true;
		farPlaneOverride[403] = true;
		farPlaneOverride[404] = true;
		farPlaneOverride[405] = true;
		farPlaneOverride[406] = true;
		farPlaneOverride[410] = true;
		farPlaneOverride[411] = true;
		farPlaneOverride[419] = true;
		farPlaneOverride[420] = true;
		farPlaneOverride[421] = true;
		farPlaneOverride[440] = true;
		farPlaneOverride[441] = true;
		farPlaneOverride[442] = true;
		farPlaneOverride[443] = true;
		farPlaneOverride[444] = true;
		farPlaneOverride[446] = true;
		farPlaneOverride[450] = true;
		farPlaneOverride[451] = true;
		farPlaneOverride[458] = true;
		farPlaneOverride[459] = true;
		farPlaneOverride[460] = true;
		farPlaneOverride[461] = true;
		farPlaneOverride[462] = true;
		farPlaneOverride[463] = true;
		farPlaneOverride[464] = true;
		farPlaneOverride[465] = true;
		farPlaneOverride[480] = true;
		farPlaneOverride[481] = true;
		farPlaneOverride[482] = true;
		farPlaneOverride[483] = true;
		farPlaneOverride[484] = true;
		farPlaneOverride[486] = true;
		farPlaneOverride[499] = true;
		farPlaneOverride[500] = true;
		farPlaneOverride[501] = true;
		farPlaneOverride[502] = true;
		farPlaneOverride[503] = true;
		farPlaneOverride[526] = true;
		farPlaneOverride[541] = true;
		farPlaneOverride[542] = true;
		farPlaneOverride[543] = true;
		farPlaneOverride[566] = true;
		farPlaneOverride[581] = true;
		farPlaneOverride[582] = true;
		farPlaneOverride[583] = true;
		farPlaneOverride[606] = true;
		farPlaneOverride[621] = true;
		farPlaneOverride[622] = true;
		farPlaneOverride[623] = true;
		farPlaneOverride[624] = true;
		farPlaneOverride[646] = true;
		farPlaneOverride[661] = true;
		farPlaneOverride[662] = true;
		farPlaneOverride[663] = true;
		farPlaneOverride[664] = true;
		farPlaneOverride[795] = true;
		farPlaneOverride[2084] = true;
		farPlaneOverride[2085] = true;
		farPlaneOverride[2086] = true;
		farPlaneOverride[2087] = true;
		farPlaneOverride[2127] = true;
		farPlaneOverride[2160] = true;
		farPlaneOverride[2169] = true;
		farPlaneOverride[2170] = true;
		farPlaneOverride[2171] = true;
		farPlaneOverride[2200] = true;
		farPlaneOverride[2201] = true;
		farPlaneOverride[2204] = true;
		farPlaneOverride[2207] = true;
		farPlaneOverride[2285] = true;
		farPlaneOverride[2287] = true;
		farPlaneOverride[2288] = true;
		farPlaneOverride[2327] = true;
		farPlaneOverride[2328] = true;
		farPlaneOverride[2405] = true;
		farPlaneOverride[3200] = true;
		farPlaneOverride[3201] = true;
		farPlaneOverride[3202] = true;
		farPlaneOverride[3203] = true;
		farPlaneOverride[3240] = true;
		farPlaneOverride[3241] = true;
		farPlaneOverride[3243] = true;
		farPlaneOverride[3245] = true;
		farPlaneOverride[3246] = true;
		farPlaneOverride[3247] = true;
		farPlaneOverride[3248] = true;
		farPlaneOverride[3249] = true;
		farPlaneOverride[3280] = true;
		farPlaneOverride[3281] = true;
		farPlaneOverride[3283] = true;
		farPlaneOverride[3320] = true;
		farPlaneOverride[3321] = true;
		farPlaneOverride[3322] = true;
		farPlaneOverride[3323] = true;
		
		// Do not draw tile
		doNotDrawTile[21] = true;
		doNotDrawTile[22] = true;
		doNotDrawTile[23] = true;
		doNotDrawTile[27] = true;
		doNotDrawTile[28] = true;
		doNotDrawTile[29] = true;
		doNotDrawTile[61] = true;
		doNotDrawTile[62] = true;
		doNotDrawTile[69] = true;
		doNotDrawTile[90] = true;
		doNotDrawTile[91] = true;
		doNotDrawTile[94] = true;
		doNotDrawTile[95] = true;
		doNotDrawTile[96] = true;
		doNotDrawTile[97] = true;
		doNotDrawTile[98] = true;
		doNotDrawTile[100] = true;
		doNotDrawTile[101] = true;
		doNotDrawTile[102] = true;
		doNotDrawTile[104] = true;
		doNotDrawTile[109] = true;
		doNotDrawTile[129] = true;
		doNotDrawTile[130] = true;
		doNotDrawTile[131] = true;
		doNotDrawTile[134] = true;
		doNotDrawTile[135] = true;
		doNotDrawTile[136] = true;
		doNotDrawTile[137] = true;
		doNotDrawTile[138] = true;
		doNotDrawTile[148] = true;
		doNotDrawTile[149] = true;
		doNotDrawTile[161] = true;
		doNotDrawTile[172] = true;
		doNotDrawTile[173] = true;
		doNotDrawTile[174] = true;
		doNotDrawTile[175] = true;
		doNotDrawTile[176] = true;
		doNotDrawTile[180] = true;
		doNotDrawTile[193] = true;
		doNotDrawTile[194] = true;
		doNotDrawTile[195] = true;
		doNotDrawTile[234] = true;
		doNotDrawTile[235] = true;
		doNotDrawTile[278] = true;
		doNotDrawTile[279] = true;
		doNotDrawTile[280] = true;
		doNotDrawTile[294] = true;
		doNotDrawTile[295] = true;
		doNotDrawTile[296] = true;
		doNotDrawTile[318] = true;
		doNotDrawTile[319] = true;
		doNotDrawTile[360] = true;
		doNotDrawTile[361] = true;
		doNotDrawTile[362] = true;
		doNotDrawTile[363] = true;
		doNotDrawTile[400] = true;
		doNotDrawTile[401] = true;
		doNotDrawTile[402] = true;
		doNotDrawTile[403] = true;
		doNotDrawTile[440] = true;
		doNotDrawTile[441] = true;
		doNotDrawTile[442] = true;
		doNotDrawTile[443] = true;
		doNotDrawTile[464] = true;
		doNotDrawTile[465] = true;
		doNotDrawTile[480] = true;
		doNotDrawTile[481] = true;
		doNotDrawTile[482] = true;
		doNotDrawTile[483] = true;
		doNotDrawTile[502] = true;
		doNotDrawTile[581] = true;
		doNotDrawTile[582] = true;
		doNotDrawTile[583] = true;
		doNotDrawTile[621] = true;
		doNotDrawTile[624] = true;
		doNotDrawTile[664] = true;
		doNotDrawTile[795] = true;
		
		// TileSourceX/Y
		for (int i = 0; i < MaxTile; i++)
		{
			TileSourceX[i] = (i % 40) * 16;
			TileSourceY[i] = (i / 40) * 16;
		}
		
		// Tank Collision
		TileTankCollision[4] = true;
		TileTankCollision[5] = true;
		TileTankCollision[6] = true;
		TileTankCollision[7] = true;
		TileTankCollision[8] = true;
		TileTankCollision[9] = true;
		TileTankCollision[10] = true;
		TileTankCollision[11] = true;
		TileTankCollision[12] = true;
		TileTankCollision[13] = true;
		TileTankCollision[14] = true;
		TileTankCollision[15] = true;
		TileTankCollision[16] = true;
		TileTankCollision[17] = true;
		TileTankCollision[18] = true;
		TileTankCollision[19] = true;
		TileTankCollision[20] = true;
		TileTankCollision[24] = true;
		TileTankCollision[25] = true;
		TileTankCollision[26] = true;
		TileTankCollision[46] = true;
		TileTankCollision[47] = true;
		TileTankCollision[48] = true;
		TileTankCollision[49] = true;
		TileTankCollision[50] = true;
		TileTankCollision[51] = true;
		TileTankCollision[52] = true;
		TileTankCollision[53] = true;
		TileTankCollision[54] = true;
		TileTankCollision[55] = true;
		TileTankCollision[56] = true;
		TileTankCollision[57] = true;
		TileTankCollision[58] = true;
		TileTankCollision[59] = true;
		TileTankCollision[60] = true;
		TileTankCollision[64] = true;
		TileTankCollision[89] = true;
		TileTankCollision[92] = true;
		TileTankCollision[93] = true;
		TileTankCollision[99] = true;
		TileTankCollision[132] = true;
		TileTankCollision[133] = true;
		TileTankCollision[139] = true;
		TileTankCollision[140] = true;
		TileTankCollision[164] = true;
		TileTankCollision[167] = true;
		TileTankCollision[177] = true;
		TileTankCollision[178] = true;
		TileTankCollision[179] = true;
		TileTankCollision[204] = true;
		TileTankCollision[207] = true;
		TileTankCollision[208] = true;
		TileTankCollision[209] = true;
		TileTankCollision[210] = true;
		TileTankCollision[211] = true;
		TileTankCollision[212] = true;
		TileTankCollision[213] = true;
		TileTankCollision[214] = true;
		TileTankCollision[215] = true;
		TileTankCollision[216] = true;
		TileTankCollision[217] = true;
		TileTankCollision[218] = true;
		TileTankCollision[219] = true;
		TileTankCollision[220] = true;
		TileTankCollision[221] = true;
		TileTankCollision[222] = true;
		TileTankCollision[232] = true;
		TileTankCollision[244] = true;
		TileTankCollision[247] = true;
		TileTankCollision[248] = true;
		TileTankCollision[249] = true;
		TileTankCollision[250] = true;
		TileTankCollision[251] = true;
		TileTankCollision[253] = true;
		TileTankCollision[254] = true;
		TileTankCollision[255] = true;
		TileTankCollision[256] = true;
		TileTankCollision[257] = true;
		TileTankCollision[258] = true;
		TileTankCollision[259] = true;
		TileTankCollision[260] = true;
		TileTankCollision[261] = true;
		TileTankCollision[262] = true;
		TileTankCollision[268] = true;
		TileTankCollision[269] = true;
		TileTankCollision[270] = true;
		TileTankCollision[271] = true;
		TileTankCollision[272] = true;
		TileTankCollision[273] = true;
		TileTankCollision[274] = true;
		TileTankCollision[275] = true;
		TileTankCollision[280] = true;
		TileTankCollision[284] = true;
		TileTankCollision[285] = true;
		TileTankCollision[286] = true;
		TileTankCollision[289] = true;
		TileTankCollision[290] = true;
		TileTankCollision[292] = true;
		TileTankCollision[291] = true;
		TileTankCollision[293] = true;
		TileTankCollision[301] = true;
		TileTankCollision[302] = true;
		TileTankCollision[303] = true;
		TileTankCollision[304] = true;
		TileTankCollision[305] = true;
		TileTankCollision[306] = true;
		TileTankCollision[307] = true;
		TileTankCollision[308] = true;
		TileTankCollision[309] = true;
		TileTankCollision[310] = true;
		TileTankCollision[311] = true;
		TileTankCollision[312] = true;
		TileTankCollision[313] = true;
		TileTankCollision[314] = true;
		TileTankCollision[315] = true;
		TileTankCollision[329] = true;
		TileTankCollision[330] = true;
		TileTankCollision[331] = true;
		TileTankCollision[332] = true;
		TileTankCollision[333] = true;
		TileTankCollision[334] = true;
		TileTankCollision[341] = true;
		TileTankCollision[342] = true;
		TileTankCollision[343] = true;
		TileTankCollision[344] = true;
		TileTankCollision[345] = true;
		TileTankCollision[346] = true;
		TileTankCollision[347] = true;
		TileTankCollision[348] = true;
		TileTankCollision[349] = true;
		TileTankCollision[350] = true;
		TileTankCollision[351] = true;
		TileTankCollision[352] = true;
		TileTankCollision[353] = true;
		TileTankCollision[354] = true;
		TileTankCollision[355] = true;
		TileTankCollision[356] = true;
		TileTankCollision[369] = true;
		TileTankCollision[370] = true;
		TileTankCollision[371] = true;
		TileTankCollision[372] = true;
		TileTankCollision[373] = true;
		TileTankCollision[374] = true;
		TileTankCollision[381] = true;
		TileTankCollision[382] = true;
		TileTankCollision[383] = true;
		TileTankCollision[384] = true;
		TileTankCollision[385] = true;
		TileTankCollision[386] = true;
		TileTankCollision[387] = true;
		TileTankCollision[388] = true;
		TileTankCollision[389] = true;
		TileTankCollision[390] = true;
		TileTankCollision[391] = true;
		TileTankCollision[392] = true;
		TileTankCollision[393] = true;
		TileTankCollision[394] = true;
		TileTankCollision[395] = true;
		TileTankCollision[396] = true;
		TileTankCollision[404] = true;
		TileTankCollision[405] = true;
		TileTankCollision[406] = true;
		TileTankCollision[410] = true;
		TileTankCollision[411] = true;
		TileTankCollision[412] = true;
		TileTankCollision[419] = true;
		TileTankCollision[420] = true;
		TileTankCollision[421] = true;
		TileTankCollision[422] = true;
		TileTankCollision[423] = true;
		TileTankCollision[424] = true;
		TileTankCollision[425] = true;
		TileTankCollision[426] = true;
		TileTankCollision[427] = true;
		TileTankCollision[434] = true;
		TileTankCollision[435] = true;
		TileTankCollision[436] = true;
		TileTankCollision[437] = true;
		TileTankCollision[444] = true;
		TileTankCollision[446] = true;
		TileTankCollision[450] = true;
		TileTankCollision[451] = true;
		TileTankCollision[452] = true;
		TileTankCollision[458] = true;
		TileTankCollision[459] = true;
		TileTankCollision[461] = true;
		TileTankCollision[462] = true;
		TileTankCollision[463] = true;
		TileTankCollision[466] = true;
		TileTankCollision[467] = true;
		TileTankCollision[474] = true;
		TileTankCollision[475] = true;
		TileTankCollision[476] = true;
		TileTankCollision[477] = true;
		TileTankCollision[484] = true;
		TileTankCollision[486] = true;
		TileTankCollision[492] = true;
		TileTankCollision[499] = true;
		TileTankCollision[500] = true;
		TileTankCollision[501] = true;
		TileTankCollision[503] = true;
		TileTankCollision[504] = true;
		TileTankCollision[505] = true;
		TileTankCollision[506] = true;
		TileTankCollision[507] = true;
		TileTankCollision[514] = true;
		TileTankCollision[515] = true;
		TileTankCollision[516] = true;
		TileTankCollision[517] = true;
		TileTankCollision[526] = true;
		TileTankCollision[532] = true;
		TileTankCollision[537] = true;
		TileTankCollision[538] = true;
		TileTankCollision[539] = true;
		TileTankCollision[540] = true;
		TileTankCollision[541] = true;
		TileTankCollision[542] = true;
		TileTankCollision[543] = true;
		TileTankCollision[544] = true;
		TileTankCollision[545] = true;
		TileTankCollision[546] = true;
		TileTankCollision[547] = true;
		TileTankCollision[554] = true;
		TileTankCollision[555] = true;
		TileTankCollision[556] = true;
		TileTankCollision[557] = true;
		TileTankCollision[566] = true;
		TileTankCollision[572] = true;
		TileTankCollision[577] = true;
		TileTankCollision[578] = true;
		TileTankCollision[579] = true;
		TileTankCollision[580] = true;
		TileTankCollision[584] = true;
		TileTankCollision[585] = true;
		TileTankCollision[586] = true;
		TileTankCollision[587] = true;
		TileTankCollision[588] = true;
		TileTankCollision[589] = true;
		TileTankCollision[590] = true;
		TileTankCollision[591] = true;
		TileTankCollision[592] = true;
		TileTankCollision[593] = true;
		TileTankCollision[594] = true;
		TileTankCollision[606] = true;
		TileTankCollision[612] = true;
		TileTankCollision[615] = true;
		TileTankCollision[616] = true;
		TileTankCollision[617] = true;
		TileTankCollision[618] = true;
		TileTankCollision[619] = true;
		TileTankCollision[620] = true;
		TileTankCollision[622] = true;
		TileTankCollision[623] = true;
		TileTankCollision[625] = true;
		TileTankCollision[626] = true;
		TileTankCollision[627] = true;
		TileTankCollision[628] = true;
		TileTankCollision[629] = true;
		TileTankCollision[630] = true;
		TileTankCollision[632] = true;
		TileTankCollision[633] = true;
		TileTankCollision[634] = true;
		TileTankCollision[646] = true;
		TileTankCollision[650] = true;
		TileTankCollision[655] = true;
		TileTankCollision[656] = true;
		TileTankCollision[657] = true;
		TileTankCollision[658] = true;
		TileTankCollision[659] = true;
		TileTankCollision[660] = true;
		TileTankCollision[661] = true;
		TileTankCollision[662] = true;
		TileTankCollision[663] = true;
		TileTankCollision[665] = true;
		TileTankCollision[666] = true;
		TileTankCollision[667] = true;
		TileTankCollision[668] = true;
		TileTankCollision[669] = true;
		TileTankCollision[670] = true;
		TileTankCollision[672] = true;
		TileTankCollision[673] = true;
		TileTankCollision[674] = true;
		TileTankCollision[686] = true;
		TileTankCollision[687] = true;
		TileTankCollision[688] = true;
		TileTankCollision[689] = true;
		TileTankCollision[690] = true;
		TileTankCollision[691] = true;
		TileTankCollision[692] = true;
		TileTankCollision[693] = true;
		TileTankCollision[694] = true;
		TileTankCollision[695] = true;
		TileTankCollision[696] = true;
		TileTankCollision[697] = true;
		TileTankCollision[698] = true;
		TileTankCollision[699] = true;
		TileTankCollision[700] = true;
		TileTankCollision[701] = true;
		TileTankCollision[671] = true;
		TileTankCollision[726] = true;
		TileTankCollision[727] = true;
		TileTankCollision[728] = true;
		TileTankCollision[729] = true;
		TileTankCollision[730] = true;
		TileTankCollision[731] = true;
		TileTankCollision[732] = true;
		TileTankCollision[733] = true;
		TileTankCollision[734] = true;
		TileTankCollision[735] = true;
		TileTankCollision[736] = true;
		TileTankCollision[737] = true;
		TileTankCollision[738] = true;
		TileTankCollision[739] = true;
		TileTankCollision[740] = true;
		TileTankCollision[741] = true;
		TileTankCollision[766] = true;
		TileTankCollision[767] = true;
		TileTankCollision[768] = true;
		TileTankCollision[769] = true;
		TileTankCollision[770] = true;
		TileTankCollision[771] = true;
		TileTankCollision[772] = true;
		TileTankCollision[773] = true;
		TileTankCollision[774] = true;
		TileTankCollision[775] = true;
		TileTankCollision[776] = true;
		TileTankCollision[777] = true;
		TileTankCollision[778] = true;
		TileTankCollision[779] = true;
		TileTankCollision[780] = true;
		TileTankCollision[781] = true;
		TileTankCollision[806] = true;
		TileTankCollision[807] = true;
		TileTankCollision[808] = true;
		TileTankCollision[809] = true;
		TileTankCollision[810] = true;
		TileTankCollision[811] = true;
		TileTankCollision[812] = true;
		TileTankCollision[813] = true;
		TileTankCollision[814] = true;
		TileTankCollision[815] = true;
		TileTankCollision[816] = true;
		TileTankCollision[817] = true;
		TileTankCollision[818] = true;
		TileTankCollision[819] = true;
		TileTankCollision[820] = true;
		TileTankCollision[821] = true;
		TileTankCollision[1562] = true;
		TileTankCollision[1563] = true;
		TileTankCollision[1564] = true;
		TileTankCollision[1565] = true;
		TileTankCollision[1566] = true;
		TileTankCollision[1567] = true;
		TileTankCollision[1568] = true;
		TileTankCollision[1569] = true;
		TileTankCollision[1602] = true;
		TileTankCollision[1603] = true;
		TileTankCollision[1604] = true;
		TileTankCollision[1605] = true;
		TileTankCollision[1606] = true;
		TileTankCollision[1607] = true;
		TileTankCollision[1608] = true;
		TileTankCollision[1609] = true;
		TileTankCollision[1680] = true;
		TileTankCollision[1681] = true;
		TileTankCollision[1682] = true;
		TileTankCollision[1683] = true;
		TileTankCollision[1684] = true;
		TileTankCollision[1685] = true;
		TileTankCollision[1686] = true;
		TileTankCollision[1687] = true;
		TileTankCollision[1688] = true;
		TileTankCollision[1689] = true;
		TileTankCollision[1690] = true;
		TileTankCollision[1691] = true;
		TileTankCollision[1692] = true;
		TileTankCollision[1693] = true;
		TileTankCollision[1694] = true;
		TileTankCollision[1695] = true;
		TileTankCollision[1696] = true;
		TileTankCollision[1697] = true;
		TileTankCollision[1698] = true;
		TileTankCollision[1699] = true;
		TileTankCollision[1720] = true;
		TileTankCollision[1721] = true;
		TileTankCollision[1722] = true;
		TileTankCollision[1723] = true;
		TileTankCollision[1724] = true;
		TileTankCollision[1725] = true;
		TileTankCollision[1726] = true;
		TileTankCollision[1727] = true;
		TileTankCollision[1728] = true;
		TileTankCollision[1729] = true;
		TileTankCollision[1730] = true;
		TileTankCollision[1731] = true;
		TileTankCollision[1732] = true;
		TileTankCollision[1733] = true;
		TileTankCollision[1734] = true;
		TileTankCollision[1735] = true;
		TileTankCollision[1736] = true;
		TileTankCollision[1737] = true;
		TileTankCollision[1738] = true;
		TileTankCollision[1739] = true;
		TileTankCollision[1840] = true;
		TileTankCollision[1841] = true;
		TileTankCollision[1842] = true;
		TileTankCollision[1843] = true;
		TileTankCollision[1844] = true;
		TileTankCollision[1845] = true;
		TileTankCollision[1846] = true;
		TileTankCollision[1847] = true;
		TileTankCollision[1848] = true;
		TileTankCollision[1849] = true;
		TileTankCollision[1850] = true;
		TileTankCollision[1851] = true;
		TileTankCollision[1852] = true;
		TileTankCollision[1853] = true;
		TileTankCollision[1854] = true;
		TileTankCollision[1855] = true;
		TileTankCollision[1856] = true;
		TileTankCollision[1857] = true;
		TileTankCollision[1880] = true;
		TileTankCollision[1881] = true;
		TileTankCollision[1882] = true;
		TileTankCollision[1883] = true;
		TileTankCollision[1884] = true;
		TileTankCollision[1885] = true;
		TileTankCollision[1886] = true;
		TileTankCollision[1887] = true;
		TileTankCollision[1888] = true;
		TileTankCollision[1889] = true;
		TileTankCollision[1890] = true;
		TileTankCollision[1891] = true;
		TileTankCollision[1892] = true;
		TileTankCollision[1893] = true;
		TileTankCollision[1894] = true;
		TileTankCollision[1895] = true;
		TileTankCollision[1896] = true;
		TileTankCollision[1897] = true;
		TileTankCollision[1920] = true;
		TileTankCollision[1921] = true;
		TileTankCollision[1922] = true;
		TileTankCollision[1923] = true;
		TileTankCollision[1924] = true;
		TileTankCollision[1925] = true;
		TileTankCollision[1926] = true;
		TileTankCollision[1927] = true;
		TileTankCollision[1928] = true;
		TileTankCollision[1929] = true;
		TileTankCollision[1930] = true;
		TileTankCollision[1931] = true;
		TileTankCollision[1932] = true;
		TileTankCollision[1933] = true;
		TileTankCollision[1934] = true;
		TileTankCollision[1935] = true;
		TileTankCollision[1936] = true;
		TileTankCollision[1937] = true;
		TileTankCollision[1945] = true;
		TileTankCollision[1946] = true;
		TileTankCollision[1947] = true;
		TileTankCollision[1948] = true;
		TileTankCollision[1949] = true;
		TileTankCollision[1950] = true;
		TileTankCollision[1951] = true;
		TileTankCollision[1952] = true;
		TileTankCollision[1960] = true;
		TileTankCollision[1961] = true;
		TileTankCollision[1962] = true;
		TileTankCollision[1963] = true;
		TileTankCollision[1964] = true;
		TileTankCollision[1965] = true;
		TileTankCollision[1966] = true;
		TileTankCollision[1967] = true;
		TileTankCollision[1968] = true;
		TileTankCollision[1969] = true;
		TileTankCollision[1970] = true;
		TileTankCollision[1971] = true;
		TileTankCollision[1972] = true;
		TileTankCollision[1973] = true;
		TileTankCollision[1974] = true;
		TileTankCollision[1975] = true;
		TileTankCollision[1976] = true;
		TileTankCollision[1977] = true;
		TileTankCollision[1985] = true;
		TileTankCollision[1986] = true;
		TileTankCollision[1987] = true;
		TileTankCollision[1988] = true;
		TileTankCollision[1989] = true;
		TileTankCollision[1990] = true;
		TileTankCollision[1991] = true;
		TileTankCollision[1992] = true;
		TileTankCollision[2025] = true;
		TileTankCollision[2026] = true;
		TileTankCollision[2027] = true;
		TileTankCollision[2028] = true;
		TileTankCollision[2029] = true;
		TileTankCollision[2030] = true;
		TileTankCollision[2031] = true;
		TileTankCollision[2032] = true;
		TileTankCollision[2065] = true;
		TileTankCollision[2066] = true;
		TileTankCollision[2067] = true;
		TileTankCollision[2068] = true;
		TileTankCollision[2069] = true;
		TileTankCollision[2070] = true;
		TileTankCollision[2071] = true;
		TileTankCollision[2072] = true;
		TileTankCollision[2089] = true;
		TileTankCollision[2109] = true;
		TileTankCollision[2110] = true;
		TileTankCollision[2111] = true;
		TileTankCollision[2112] = true;
		TileTankCollision[2149] = true;
		TileTankCollision[2150] = true;
		TileTankCollision[2151] = true;
		TileTankCollision[2152] = true;
		TileTankCollision[2189] = true;
		TileTankCollision[2190] = true;
		TileTankCollision[2191] = true;
		TileTankCollision[2192] = true;
		TileTankCollision[2229] = true;
		TileTankCollision[2230] = true;
		TileTankCollision[2231] = true;
		TileTankCollision[2232] = true;
		TileTankCollision[2680] = true;
		TileTankCollision[2681] = true;
		TileTankCollision[2682] = true;
		TileTankCollision[2683] = true;
		TileTankCollision[2720] = true;
		TileTankCollision[2721] = true;
		TileTankCollision[2722] = true;
		TileTankCollision[2723] = true;
		TileTankCollision[2724] = true;
		TileTankCollision[2725] = true;
		TileTankCollision[2726] = true;
		TileTankCollision[2727] = true;
		TileTankCollision[2728] = true;
		TileTankCollision[2729] = true;
		TileTankCollision[2730] = true;
		TileTankCollision[2731] = true;
		TileTankCollision[2760] = true;
		TileTankCollision[2761] = true;
		TileTankCollision[2762] = true;
		TileTankCollision[2763] = true;
		TileTankCollision[2764] = true;
		TileTankCollision[2765] = true;
		TileTankCollision[2766] = true;
		TileTankCollision[2767] = true;
		TileTankCollision[2800] = true;
		TileTankCollision[2801] = true;
		TileTankCollision[2802] = true;
		TileTankCollision[2803] = true;
		TileTankCollision[2804] = true;
		TileTankCollision[2805] = true;
		TileTankCollision[2806] = true;
		TileTankCollision[2807] = true;
		TileTankCollision[2840] = true;
		TileTankCollision[2841] = true;
		TileTankCollision[2842] = true;
		TileTankCollision[2843] = true;
		TileTankCollision[2844] = true;
		TileTankCollision[2845] = true;
		TileTankCollision[2846] = true;
		TileTankCollision[2847] = true;
		TileTankCollision[2880] = true;
		TileTankCollision[2881] = true;
		TileTankCollision[2882] = true;
		TileTankCollision[2883] = true;
		TileTankCollision[2884] = true;
		TileTankCollision[2889] = true;
		TileTankCollision[2890] = true;
		TileTankCollision[2891] = true;
		TileTankCollision[2892] = true;
		TileTankCollision[2893] = true;
		TileTankCollision[2920] = true;
		TileTankCollision[2921] = true;
		TileTankCollision[2922] = true;
		TileTankCollision[2923] = true;
		TileTankCollision[2924] = true;
		TileTankCollision[2960] = true;
		TileTankCollision[2961] = true;
		TileTankCollision[2962] = true;
		TileTankCollision[2963] = true;
		TileTankCollision[2964] = true;
		TileTankCollision[3000] = true;
		TileTankCollision[3001] = true;
		TileTankCollision[3002] = true;
		TileTankCollision[3003] = true;
		TileTankCollision[3004] = true;
		TileTankCollision[3040] = true;
		TileTankCollision[3041] = true;
		TileTankCollision[3042] = true;
		TileTankCollision[3043] = true;
		TileTankCollision[3044] = true;
		TileTankCollision[3045] = true;
		TileTankCollision[3046] = true;
		TileTankCollision[3047] = true;
		TileTankCollision[3048] = true;
		TileTankCollision[3080] = true;
		TileTankCollision[3081] = true;
		TileTankCollision[3082] = true;
		TileTankCollision[3083] = true;
		TileTankCollision[3084] = true;
		TileTankCollision[3085] = true;
		TileTankCollision[3086] = true;
		TileTankCollision[3087] = true;
		TileTankCollision[3088] = true;
		TileTankCollision[3089] = true;
		TileTankCollision[3090] = true;
		TileTankCollision[3091] = true;
		TileTankCollision[3092] = true;
		TileTankCollision[3120] = true;
		TileTankCollision[3121] = true;
		TileTankCollision[3122] = true;
		TileTankCollision[3123] = true;
		TileTankCollision[3124] = true;
		TileTankCollision[3125] = true;
		TileTankCollision[3126] = true;
		TileTankCollision[3127] = true;
		TileTankCollision[3128] = true;
		TileTankCollision[3129] = true;
		TileTankCollision[3130] = true;
		TileTankCollision[3131] = true;
		TileTankCollision[3132] = true;
		TileTankCollision[3160] = true;
		TileTankCollision[3161] = true;
		TileTankCollision[3162] = true;
		TileTankCollision[3163] = true;
		TileTankCollision[3164] = true;
		TileTankCollision[3165] = true;
		TileTankCollision[3166] = true;
		TileTankCollision[3167] = true;
		TileTankCollision[3168] = true;
		TileTankCollision[3169] = true;
		TileTankCollision[3170] = true;
		TileTankCollision[3171] = true;
		TileTankCollision[3172] = true;
		TileTankCollision[3205] = true;
		TileTankCollision[3206] = true;
		TileTankCollision[3207] = true;
		TileTankCollision[3208] = true;
		TileTankCollision[3210] = true;
		TileTankCollision[3211] = true;
		TileTankCollision[3212] = true;
		TileTankCollision[3245] = true;
		TileTankCollision[3246] = true;
		TileTankCollision[3247] = true;
		TileTankCollision[3248] = true;
		TileTankCollision[3249] = true;
	}
	
	public static final boolean checkWeaponCollision(int tile, double Angle)
	{
		switch (tile)
		{
			// Normal Walls
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 24:
			case 25:
			case 26:
			case 46:
			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 64:
			case 89:
			case 92:
			case 93:
			case 99:
			case 132:
			case 133:
			case 139:
			case 140:
			case 164:
			case 167:
			case 177:
			case 178:
			case 179:
			case 208:
			case 209:
			case 210:
			case 214:
			case 215:
			case 216:
			case 217:
			case 218:
			case 219:
			case 220:
			case 221:
			case 222:
			case 232:
			case 244:
			case 247:
			case 248:
			case 249:
			case 250:
			case 254:
			case 255:
			case 256:
			case 257:
			case 258:
			case 259:
			case 260:
			case 261:
			case 262:
			case 268:
			case 269:
			case 270:
			case 271:
			case 272:
			case 273:
			case 274:
			case 275:
			case 284:
			case 286:
			case 289:
			case 290:
			case 306:
			case 307:
			case 309:
			case 310:
			case 311:
			case 312:
			case 313:
			case 314:
			case 315:
			case 329:
			case 330:
			case 331:
			case 332:
			case 333:
			case 334:
			case 341:
			case 342:
			case 343:
			case 344:
			case 346:
			case 347:
			case 349:
			case 350:
			case 351:
			case 352:
			case 353:
			case 354:
			case 355:
			case 356:
			case 369:
			case 370:
			case 371:
			case 372:
			case 373:
			case 374:
			case 381:
			case 382:
			case 383:
			case 384:
			case 386:
			case 387:
			case 389:
			case 390:
			case 391:
			case 392:
			case 393:
			case 394:
			case 395:
			case 396:
			case 404:
			case 406:
			case 410:
			case 411:
			case 412:
			case 419:
			case 420:
			case 421:
			case 426:
			case 427:
			case 434:
			case 435:
			case 436:
			case 437:
			case 444:
			case 446:
			case 450:
			case 451:
			case 452:
			case 458:
			case 459:
			case 461:
			case 462:
			case 463:
			case 474:
			case 475:
			case 476:
			case 477:
			case 484:
			case 486:
			case 491:
			case 492:
			case 499:
			case 500:
			case 501:
			case 503:
			case 504:
			case 505:
			case 506:
			case 507:
			case 514:
			case 515:
			case 516:
			case 517:
			case 526:
			case 531:
			case 532:
			case 537:
			case 538:
			case 539:
			case 540:
			case 541:
			case 542:
			case 543:
			case 544:
			case 545:
			case 546:
			case 547:
			case 554:
			case 555:
			case 556:
			case 557:
			case 566:
			case 571:
			case 572:
			case 577:
			case 578:
			case 579:
			case 580:
			case 584:
			case 585:
			case 586:
			case 587:
			case 606:
			case 611:
			case 612:
			case 615:
			case 616:
			case 617:
			case 618:
			case 619:
			case 620:
			case 622:
			case 623:
			case 625:
			case 626:
			case 627:
			case 646:
			case 650:
			case 651:
			case 652:
			case 653:
			case 654:
			case 655:
			case 656:
			case 657:
			case 658:
			case 659:
			case 660:
			case 661:
			case 662:
			case 663:
			case 691:
			case 692:
			case 698:
			case 699:
			case 671:
			case 700:
			case 701:
			case 726:
			case 727:
			case 728:
			case 729:
			case 731:
			case 732:
			case 738:
			case 739:
			case 740:
			case 741:
			case 766:
			case 767:
			case 768:
			case 769:
			case 771:
			case 772:
			case 778:
			case 779:
			case 780:
			case 781:
			case 811:
			case 812:
			case 818:
			case 819:
			case 820:
			case 821:
			case 1680:
			case 1681:
			case 1682:
			case 1683:
			case 1684:
			case 1685:
			case 1686:
			case 1687:
			case 1688:
			case 1689:
			case 1690:
			case 1691:
			case 1692:
			case 1693:
			case 1694:
			case 1695:
			case 1696:
			case 1697:
			case 1698:
			case 1699:
			case 1720:
			case 1721:
			case 1722:
			case 1723:
			case 1724:
			case 1725:
			case 1726:
			case 1727:
			case 1728:
			case 1729:
			case 1730:
			case 1731:
			case 1732:
			case 1733:
			case 1734:
			case 1735:
			case 1736:
			case 1737:
			case 1738:
			case 1739:
			case 1840:
			case 1841:
			case 1842:
			case 1843:
			case 1844:
			case 1845:
			case 1846:
			case 1847:
			case 1848:
			case 1849:
			case 1850:
			case 1851:
			case 1852:
			case 1853:
			case 1854:
			case 1855:
			case 1856:
			case 1857:
			case 1880:
			case 1881:
			case 1882:
			case 1883:
			case 1884:
			case 1885:
			case 1886:
			case 1887:
			case 1888:
			case 1889:
			case 1890:
			case 1891:
			case 1892:
			case 1893:
			case 1894:
			case 1895:
			case 1896:
			case 1897:
			case 1920:
			case 1921:
			case 1922:
			case 1923:
			case 1924:
			case 1925:
			case 1926:
			case 1927:
			case 1928:
			case 1929:
			case 1930:
			case 1931:
			case 1932:
			case 1933:
			case 1934:
			case 1935:
			case 1936:
			case 1937:
			case 1945:
			case 1946:
			case 1947:
			case 1948:
			case 1949:
			case 1950:
			case 1951:
			case 1952:
			case 1960:
			case 1961:
			case 1962:
			case 1963:
			case 1964:
			case 1965:
			case 1966:
			case 1967:
			case 1968:
			case 1969:
			case 1970:
			case 1971:
			case 1972:
			case 1973:
			case 1974:
			case 1975:
			case 1976:
			case 1977:
			case 1979:
			case 1985:
			case 1986:
			case 1987:
			case 1988:
			case 1989:
			case 1990:
			case 1991:
			case 1992:
			case 2019:
			case 2025:
			case 2026:
			case 2027:
			case 2028:
			case 2029:
			case 2030:
			case 2031:
			case 2032:
			case 2059:
			case 2065:
			case 2066:
			case 2067:
			case 2068:
			case 2069:
			case 2070:
			case 2071:
			case 2072:
			case 2099:
			case 2109:
			case 2110:
			case 2111:
			case 2112:
			case 2139:
			case 2140:
			case 2141:
			case 2142:
			case 2149:
			case 2150:
			case 2151:
			case 2152:
			case 2189:
			case 2190:
			case 2191:
			case 2192:
			case 2229:
			case 2230:
			case 2231:
			case 2232:
			case 2680:
			case 2681:
			case 2682:
			case 2683:
			case 2720:
			case 2721:
			case 2722:
			case 2723:
			case 2724:
			case 2725:
			case 2726:
			case 2727:
			case 2728:
			case 2729:
			case 2730:
			case 2731:
			case 2760:
			case 2761:
			case 2762:
			case 2763:
			case 2764:
			case 2765:
			case 2766:
			case 2767:
			case 2800:
			case 2801:
			case 2802:
			case 2803:
			case 2804:
			case 2805:
			case 2806:
			case 2807:
			case 2840:
			case 2841:
			case 2842:
			case 2843:
			case 2844:
			case 2845:
			case 2846:
			case 2847:
			case 2880:
			case 2881:
			case 2882:
			case 2883:
			case 2920:
			case 2921:
			case 2922:
			case 2923:
			case 2960:
			case 2961:
			case 2962:
			case 2963:
			case 3000:
			case 3001:
			case 3002:
			case 3003:
			case 3040:
			case 3041:
			case 3042:
			case 3043:
			case 3044:
			case 3080:
			case 3081:
			case 3082:
			case 3083:
			case 3084:
			case 3090:
			case 3091:
			case 3120:
			case 3121:
			case 3122:
			case 3123:
			case 3124:
			case 3125:
			case 3126:
			case 3127:
			case 3128:
			case 3130:
			case 3131:
			case 3160:
			case 3161:
			case 3162:
			case 3163:
			case 3164:
			case 3165:
			case 3166:
			case 3167:
			case 3168:
			case 3170:
			case 3171:
			case 3210:
			case 3211:
			case 3245:
			case 3246:
			case 3247:
			case 3248:
			case 3249:
				return true;
		}

		switch (tile)
		{
			// Up Collision Tiles (Bunkers)
			case 301:
			case 302:
			case 303:
			case 304:
			case 305:
			case 308:
			case 345:
			case 686:
			case 687:
			case 688:
			case 689:
			case 690:
			case 3085:
			case 3086:
			case 3087:
			case 3088:
			case 3089:
			{
				if ((Angle * 180 / Math.PI) > 0
						&& (Angle * 180 / Math.PI) < 180)
				{
					return true;
				}
			}
		}

		switch (tile)
		{
			// Down Collision Tiles
			case 388:
			case 422:
			case 423:
			case 424:
			case 425:
			case 466:
			case 467:
			case 806:
			case 807:
			case 808:
			case 809:
			case 813:
			case 3205:
			case 3206:
			case 3207:
			case 3208:
			case 3212:
			{
				if ((Angle * 180 / Math.PI) > -180
						&& (Angle * 180 / Math.PI) < 0)
				{

					return true;

				}
			}
		}

		switch (tile)
		{
			// Left Collision Tiles
			case 301:
			case 345:
			case 385:
			case 422:
			case 425:
			case 730:
			case 770:
			case 810:
			case 3129:
			case 3169:
			case 3209:
			{
				if ((Angle * 180 / Math.PI) > -90
						&& (Angle * 180 / Math.PI) < 90)
				{
					return true;
				}
			}
		}

		switch (tile)
		{
			// Right Collision Tiles
			case 303:
			case 308:
			case 348:
			case 388:
			case 424:
			case 693:
			case 733:
			case 773:
			case 3092:
			case 3132:
			case 3172:
			{
				if ((Angle * 180 / Math.PI) < -90
						|| (Angle * 180 / Math.PI) > 90)
				{
					return true;

				}
			}
		}

		return false;
	}

	public static final boolean checkNadeCollision(int tile)
	{
		switch (tile)
		{
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 301:
			case 302:
			case 303:
			case 304:
			case 305:
			case 306:
			case 307:
			case 308:
			case 341:
			case 342:
			case 343:
			case 344:
			case 345:
			case 346:
			case 347:
			case 348:
			case 381:
			case 382:
			case 383:
			case 384:
			case 385:
			case 386:
			case 387:
			case 388:
			case 422:
			case 423:
			case 424:
			case 425:
			case 426:
			case 427:
			case 466:
			case 467:
			case 480:
			case 686:
			case 687:
			case 688:
			case 689:
			case 690:
			case 691:
			case 692:
			case 693:
			case 726:
			case 727:
			case 728:
			case 729:
			case 730:
			case 731:
			case 732:
			case 733:
			case 766:
			case 767:
			case 768:
			case 769:
			case 770:
			case 771:
			case 772:
			case 773:
			case 806:
			case 807:
			case 808:
			case 809:
			case 810:
			case 811:
			case 812:
			case 813:
			case 3040:
			case 3041:
			case 3042:
			case 3043:
			case 3080:
			case 3081:
			case 3082:
			case 3083:
			case 3084:
			case 3085:
			case 3086:
			case 3087:
			case 3088:
			case 3089:
			case 3090:
			case 3091:
			case 3092:
			case 3120:
			case 3121:
			case 3122:
			case 3123:
			case 3124:
			case 3125:
			case 3126:
			case 3127:
			case 3128:
			case 3129:
			case 3130:
			case 3131:
			case 3132:
			case 3160:
			case 3161:
			case 3162:
			case 3163:
			case 3164:
			case 3165:
			case 3166:
			case 3167:
			case 3168:
			case 3169:
			case 3170:
			case 3171:
			case 3172:
			case 3205:
			case 3206:
			case 3207:
			case 3208:
			case 3209:
			case 3210:
			case 3211:
			case 3212:
			case 3245:
			case 3246:
			case 3247:
			case 3248:
			case 3249:
				return true;
		}

		return false;
	}

	public static final boolean farPlaneOverrideAnim(int tile)
	{
		switch (tile)
		{
			case 139:
			case 141:
			case 142:
			case 143:
			case 144:
				return true;
		}
		return false;
	}
}
