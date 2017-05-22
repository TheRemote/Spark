package codemallet.spark.server.map;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.zip.Inflater;

import codemallet.spark.server.SparkServer;

public final class Map
{
	private final SparkServer server;

	public String MapFile;

	public MapStructure GameMap;

	public ArrayList<MapSpawn> spawnGreen = new ArrayList<MapSpawn>();

	public ArrayList<MapSpawn> spawnRed = new ArrayList<MapSpawn>();

	public ArrayList<MapSpawn> spawnBlue = new ArrayList<MapSpawn>();

	public ArrayList<MapSpawn> spawnYellow = new ArrayList<MapSpawn>();

	public ArrayList<MapPen> holdpenGreen = new ArrayList<MapPen>();

	public ArrayList<MapPen> holdpenRed = new ArrayList<MapPen>();

	public ArrayList<MapPen> holdpenBlue = new ArrayList<MapPen>();

	public ArrayList<MapPen> holdpenYellow = new ArrayList<MapPen>();

	public ArrayList<Flag> Flags = new ArrayList<Flag>();

	public ArrayList<FlagPole> FlagPoles = new ArrayList<FlagPole>();

	public ArrayList<Warp> Warps = new ArrayList<Warp>();

	public ArrayList<Switch> Switches = new ArrayList<Switch>();

	public Map(SparkServer server, String MapFile)
	{
		this.server = server;

		this.MapFile = MapFile;
		GameMap = new MapStructure();

		LoadMap();
	}

	private final void LoadMap()
	{
		InputStream in = null;
		try
		{
			in = new FileInputStream("spark/maps/" + MapFile);

			char MapBuffer[] = new char[23];
			for (int i = 0; i < 23; i++)
			{
				MapBuffer[i] = (char) in.read();
			}

			GameMap.id = (MapBuffer[1] * 256 + MapBuffer[0]);
			GameMap.header = (MapBuffer[3] * 256 + MapBuffer[2]);
			GameMap.version = MapBuffer[4];
			GameMap.width = (MapBuffer[6] * 256 + MapBuffer[5]);
			GameMap.height = (MapBuffer[8] * 256 + MapBuffer[7]);
			GameMap.maxPlayers = MapBuffer[9];
			GameMap.holdingTime = MapBuffer[10];
			GameMap.numTeams = MapBuffer[11];
			GameMap.objective = MapBuffer[12];
			GameMap.laserDamage = MapBuffer[13];
			GameMap.specialDamage = MapBuffer[14];
			GameMap.rechargeRate = MapBuffer[15];
			GameMap.misslesEnabled = MapBuffer[16];
			GameMap.bombsEnabled = MapBuffer[17];
			GameMap.bounciesEnabled = MapBuffer[18];
			GameMap.powerupCount = (MapBuffer[20] * 256 + MapBuffer[19]);
			GameMap.maxSimulPowerups = MapBuffer[21];
			GameMap.switchCount = MapBuffer[22];

			GameMap.MissileTime = 315 * (5 - GameMap.rechargeRate);
			GameMap.GrenadeTime = 650 * (5 - GameMap.rechargeRate);
			GameMap.BouncyTime = 255 * (5 - GameMap.rechargeRate);

			for (int i = 0; i < GameMap.numTeams; i++)
			{
				GameMap.flagCount[i] = (char) in.read();
			}
			for (int i = 0; i < GameMap.numTeams; i++)
			{
				GameMap.flagPoleCount[i] = (char) in.read();
			}

			for (int i = 0; i < GameMap.numTeams; i++)
			{
				if (GameMap.flagPoleCount[i] > 0)
				{
					for (int j = 0; j < GameMap.flagPoleCount[i]; j++)
					{
						GameMap.flagPoleData[i] = (char) in.read();
					}
				}
			}

			char Length1 = (char) in.read();
			char Length2 = (char) in.read();
			GameMap.nameLength = (Length2 * 256 + Length1);

			if (GameMap.nameLength > 0)
			{
				GameMap.name = "";
				for (int i = 0; i < GameMap.nameLength; i++)
				{
					GameMap.name += (char) in.read();
				}
			}

			System.err.println(GameMap.name);

			Length1 = (char) in.read();
			Length2 = (char) in.read();
			GameMap.descriptionLength = (Length2 * 256 + Length1);

			if (GameMap.descriptionLength > 0)
			{
				GameMap.description = "";
				for (int i = 0; i < GameMap.descriptionLength; i++)
				{
					GameMap.description += (char) in.read();
				}
			}

			System.err.println(GameMap.description);

			GameMap.neutralCount = (char) in.read();

			int TotalSize = GameMap.width * GameMap.height * 2;
			if (GameMap.version == 3) // Compressed
			{
				byte[] Buffer = new byte[TotalSize];

				int i = 0;
				while (true)
				{
					try
					{
						if (i == TotalSize)
							break;
						byte read = (byte) (in.read());
						Buffer[i] = read;
						i++;
					}
					catch (IOException e)
					{
						e.printStackTrace();
						break;
					}
				}

				Inflater decompresser = new Inflater();
				byte[] NewBuffer = new byte[TotalSize];
				decompresser.setInput(Buffer, 0, i);
				try
				{
					decompresser.inflate(NewBuffer);

				}
				catch (Exception e)
				{
					System.err.println("Error decompressing map");
				}

				decompresser.end();

				GameMap.MapData = new int[TotalSize / 2];
				for (int x = 0; x < (TotalSize / 2); x++)
				{
					int V = x * 2;
					int Result = ((NewBuffer[V + 1] & 0xff) * 256 + (NewBuffer[V] & 0xff));
					GameMap.MapData[x] = Result;
				}
			}

			in.close();

			LoadMapData();
		}
		catch (Exception e)
		{
			try
			{
				in.close();
			}
			catch (Exception ee)
			{

			}

			e.printStackTrace();

			System.err.println("Could not load the map " + MapFile);
			server.Close();
		}
	}

	private final void LoadMapData()
	{
		int FlagCount = 0;
		// Load Spawns, Holding Pens, Flagpoles, Flags
		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				int tile = GameMap.MapData[(y * GameMap.width) + x];

				if ((tile & 0x8000) != 0)
				{
					char idx = (char) ((tile & 0x00FF));
					switch (idx)
					{
						case 25: // Green Flag Pole (Red Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 0;
							NewFlagPole.FlagType = 1;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 26: // Green Flag Pole (Blue Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 0;
							NewFlagPole.FlagType = 2;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 27: // Green Flag Pole (Yellow Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 0;
							NewFlagPole.FlagType = 3;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 28: // Green Flag
						{
							Flag NewFlag = new Flag();
							NewFlag.X = x;
							NewFlag.Y = y;
							NewFlag.Team = 0;
							NewFlag.onPole = true;
							NewFlag.PoleIndex = FlagPoles.size();
							Flags.add(NewFlag);

							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 0;
							NewFlagPole.hasFlag = true;
							NewFlagPole.FlagType = 0;
							NewFlagPole.FlagIndex = FlagCount;
							FlagPoles.add(NewFlagPole);

							FlagCount++;
						}
							break;
						case 32: // Red Flag Pole (Green Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 1;
							NewFlagPole.FlagType = 0;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 34: // Red Flag Pole (Blue Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 1;
							NewFlagPole.FlagType = 2;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 35: // Red Flag Pole (Yellow Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 1;
							NewFlagPole.FlagType = 3;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 37: // Red Flag
						{
							Flag NewFlag = new Flag();
							NewFlag.X = x;
							NewFlag.Y = y;
							NewFlag.Team = 1;
							NewFlag.onPole = true;
							NewFlag.PoleIndex = FlagPoles.size();
							Flags.add(NewFlag);

							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 1;
							NewFlagPole.hasFlag = true;
							NewFlagPole.FlagType = 1;
							NewFlagPole.FlagIndex = FlagCount;
							FlagPoles.add(NewFlagPole);

							FlagCount++;
						}
							break;
						case 40: // Blue Flag Pole (Green Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 2;
							NewFlagPole.FlagType = 0;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 41: // Blue Flag Pole (Red Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 2;
							NewFlagPole.FlagType = 1;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 43: // Blue Flag Pole (Yellow Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 2;
							NewFlagPole.FlagType = 3;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 46: // Blue Flag
						{
							Flag NewFlag = new Flag();
							NewFlag.X = x;
							NewFlag.Y = y;
							NewFlag.Team = 2;
							NewFlag.onPole = true;
							NewFlag.PoleIndex = FlagPoles.size();
							Flags.add(NewFlag);

							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 2;
							NewFlagPole.hasFlag = true;
							NewFlagPole.FlagType = 2;
							NewFlagPole.FlagIndex = FlagCount;
							FlagPoles.add(NewFlagPole);

							FlagCount++;
						}
							break;
						case 58: // Yellow Flag Pole (Green Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 3;
							NewFlagPole.FlagType = 0;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 59: // Yellow Flag Pole (Red Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 3;
							NewFlagPole.FlagType = 1;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 60: // Yellow Flag Pole (Blue Team)
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 3;
							NewFlagPole.FlagType = 2;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 65: // Yellow Flag
						{
							Flag NewFlag = new Flag();
							NewFlag.X = x;
							NewFlag.Y = y;
							NewFlag.Team = 3;
							NewFlag.onPole = true;
							NewFlag.PoleIndex = FlagPoles.size();
							Flags.add(NewFlag);

							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 3;
							NewFlagPole.hasFlag = true;
							NewFlagPole.FlagType = 3;
							NewFlagPole.FlagIndex = FlagCount;
							FlagPoles.add(NewFlagPole);

							FlagCount++;
						}
							break;
						case 123: // Switch
						{
							Switch NewSwitch = new Switch();
							NewSwitch.X = x;
							NewSwitch.Y = y;
							NewSwitch.Team = -1;
							Switches.add(NewSwitch);
						}
							break;
						case 128: // Green Neutral Flag Pole
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 0;
							NewFlagPole.FlagType = 4;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 129: // Red Neutral Flag Pole
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 1;
							NewFlagPole.FlagType = 4;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 130: // Blue Neutral Flag Pole
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 2;
							NewFlagPole.FlagType = 4;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 131: // Yellow Neutral Flag Pole
						{
							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 3;
							NewFlagPole.FlagType = 4;
							NewFlagPole.hasFlag = false;
							FlagPoles.add(NewFlagPole);
						}
							break;
						case 140: // Neutral Flag
						{
							Flag NewFlag = new Flag();
							NewFlag.X = x;
							NewFlag.Y = y;
							NewFlag.Team = 4;
							NewFlag.onPole = true;
							NewFlag.PoleIndex = FlagPoles.size();
							Flags.add(NewFlag);

							FlagPole NewFlagPole = new FlagPole();
							NewFlagPole.X = x;
							NewFlagPole.Y = y;
							NewFlagPole.Team = 4;
							NewFlagPole.hasFlag = true;
							NewFlagPole.FlagType = 4;
							NewFlagPole.FlagIndex = FlagCount;
							FlagPoles.add(NewFlagPole);

							FlagCount++;
						}
							break;
						case 141: // Green Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenGreen.add(NewHoldingPen);
						}
							break;
						case 142: // Red Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenRed.add(NewHoldingPen);
						}
							break;
						case 143: // Blue Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenBlue.add(NewHoldingPen);
						}
							break;
						case 144: // Yellow Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenYellow.add(NewHoldingPen);
						}
							break;
						case 158: // Warp
						case 250:
						{
							Warp NewWarp = new Warp();
							NewWarp.Destination = (((tile & 0x7F00) >> 8) / 10);
							NewWarp.Address = (char) ((tile & 0x7F00) >> 8)
									- NewWarp.Destination * 10;
							NewWarp.X = x;
							NewWarp.Y = y;
							Warps.add(NewWarp);
						}
							break;
						case 163: // Green Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnGreen.add(NewSpawn);
						}
							break;
						case 164: // Red Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnRed.add(NewSpawn);
						}
							break;
						case 165: // Blue Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnBlue.add(NewSpawn);
						}
							break;
						case 166: // Yellow Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnYellow.add(NewSpawn);
						}
							break;
					}
				}
				else
				// Non animated tiles
				{
					switch (tile)
					{
						case 146: // Green Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenGreen.add(NewHoldingPen);
						}
							break;
						case 147: // Green Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnGreen.add(NewSpawn);
						}
							break;
						case 186: // Red Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenRed.add(NewHoldingPen);
						}
							break;
						case 187: // Red Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnRed.add(NewSpawn);
						}
							break;
						case 276: // Blue Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenBlue.add(NewHoldingPen);
						}
							break;
						case 277: // Blue Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnBlue.add(NewSpawn);
						}
							break;
						case 316: // Yellow Holding Pen
						{
							MapPen NewHoldingPen = new MapPen();
							NewHoldingPen.X = x;
							NewHoldingPen.Y = y;
							holdpenYellow.add(NewHoldingPen);
						}
							break;
						case 317: // Yellow Spawn
						{
							MapSpawn NewSpawn = new MapSpawn();
							NewSpawn.X = x;
							NewSpawn.Y = y;
							spawnYellow.add(NewSpawn);
						}
							break;
					}
				}
			}
		}
	}
}
