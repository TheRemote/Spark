package codemallet.spark;

import java.awt.image.BufferedImage;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.Date;
import java.util.Random;
import java.util.concurrent.ConcurrentLinkedQueue;

import javax.imageio.ImageIO;
import javax.swing.JOptionPane;

import org.lwjgl.Sys;
import org.lwjgl.openal.AL;

import codemallet.spark.chat.Chat;
import codemallet.spark.clan.ClanList;
import codemallet.spark.dialog.Dialog;
import codemallet.spark.engines.GameEngine;
import codemallet.spark.engines.GameResources;
import codemallet.spark.engines.KeyboardEvent;
import codemallet.spark.engines.MousePressEvent;
import codemallet.spark.engines.SparkEvent;
import codemallet.spark.engines.SparkEvents;
import codemallet.spark.engines.java2d.Java2DEngine;
import codemallet.spark.engines.slick.SlickEngine;
import codemallet.spark.gameinterface.Interface;
import codemallet.spark.input.Input;
import codemallet.spark.map.Map;
import codemallet.spark.map.Radar;
import codemallet.spark.net.NetCode;
import codemallet.spark.net.NetLinkLobby;
import codemallet.spark.options.Options;
import codemallet.spark.player.IgnoreList;
import codemallet.spark.player.LocalPlayer;
import codemallet.spark.player.PlayerList;
import codemallet.spark.security.Security;
import codemallet.spark.sound.Sounds;
import codemallet.spark.util.WindowsHardwareUtil;
import codemallet.spark.weapon.BounceList;
import codemallet.spark.weapon.ExplosionList;
import codemallet.spark.weapon.GrenadeList;
import codemallet.spark.weapon.LaserList;
import codemallet.spark.weapon.MissileList;
import codemallet.spark.weapon.ShrapnelList;
import codemallet.spark.weapon.SmokeList;
import codemallet.spark.weapon.SparksList;
import codemallet.spark.weapon.TrailList;
import codemallet.spark.weapon.Weapons;

public final class Spark
{
	public static final String Version = "1.41";

	// Game Engine
	public final NetCode gameNetCode = new NetCode(this);

	public final NetLinkLobby gameNetLinkLobby = new NetLinkLobby(this);

	public final Options gameOptions = new Options();

	public final Sounds gameSound = new Sounds(this);

	public GameEngine gameEngine;

	public final ConcurrentLinkedQueue<SparkEvent> gameQueue = new ConcurrentLinkedQueue<SparkEvent>();

	// Game Classes

	public final LocalPlayer gamePlayer = new LocalPlayer(this);

	public final PlayerList gamePlayers = new PlayerList(this);

	public final IgnoreList gameIgnoreList = new IgnoreList();

	public final Interface gameInterface = new Interface(this);

	public final Input gameInput = new Input(this);

	public final Dialog gameDialog = new Dialog(this);

	public Map gameMap = null;

	public final Chat gameChat = new Chat(this);

	public final LaserList gameLasers = new LaserList(this);

	public final BounceList gameBouncies = new BounceList(this);

	public final MissileList gameMissiles = new MissileList(this);

	public final GrenadeList gameGrenades = new GrenadeList(this);

	public final SparksList gameSparks = new SparksList(this);

	public final ShrapnelList gameShrapnel = new ShrapnelList(this);

	public final TrailList gameTrails = new TrailList(this);

	public final ExplosionList gameExplosions = new ExplosionList(this);

	public final Weapons gameWeapons = new Weapons(this);

	public final SmokeList gameSmoke = new SmokeList(this);

	public final Radar gameRadar = new Radar(this);

	public final ClanList gameClans = new ClanList(this);

	public final Random gameRandom = new Random();

	// Variables
	public long Tick;

	public String Password;

	public boolean Destroy;

	public String CloseMessage;

	// FPS
	public float FPS;

	public float FPSCounter;

	public long timeFPS;

	public long timePing;

	public long timeAFKKick;

	public long timeUpdateTimestamp;

	public int ResolutionX;

	public int ResolutionY;

	public boolean Debug = false;

	public BufferedImage imgCollision;

	public BufferedImage imgShipCollision;

	public boolean Locked = false;

	// Loading
	public boolean LoadingResources = true;

	public boolean LoadingGame = true;

	public String LoadString = "";

	public int LoadTotal;

	public int LoadProgress;

	// System info
	public String OperatingSystem = null;

	public String JavaVersion = null;

	public String SystemArchitecture = null;

	public String HardwareDriverDate = null;

	public String HardwareDisplayDevice = null;

	public String HardwareDisplayService = null;

	public String HardwareCPUName = null;

	public String HardwareCPUSpeed = null;

	public String HardwareCPUVendor = null;

	public final int HardwareCPUCount = Runtime.getRuntime()
			.availableProcessors();

	public final static void main(String args[])
	{
		boolean DebugSpark = false;
		if (args.length == 8)
		{
			try
			{
				System.setErr(new PrintStream(new BufferedOutputStream(
						new FileOutputStream("SparkError.txt")), true));
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}
		else
		{
			DebugSpark = true;
		}

		String IPAddress = "";
		String Account = "";
		String Hash = "";
		String Password = "";
		boolean Windowed = false;
		int Resolution = 0;
		int ResolutionX = 0;
		int ResolutionY = 0;
		int Port = 0;
		int Clan = -1;

		try
		{
			Account = args[0];
			Hash = args[1];
			Windowed = Boolean.parseBoolean(args[2]);
			IPAddress = args[3];
			Port = Integer.parseInt(args[4]);
			Resolution = Integer.parseInt(args[5]);

			if (Resolution == 640)
			{
				ResolutionX = 640;
				ResolutionY = 480;
			}
			else if (Resolution == 800)
			{
				ResolutionX = 800;
				ResolutionY = 600;
			}
			else if (Resolution == 1024)
			{
				ResolutionX = 1024;
				ResolutionY = 768;
			}

			Password = args[6];
			Clan = Integer.parseInt(args[7]);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		Spark spark = null;
		try
		{
			spark = new Spark(Account, Hash, Windowed, IPAddress, Port,
					ResolutionX, ResolutionY, Password, Clan, DebugSpark);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			if (spark != null)
			{
				spark.throwException(e);
			}
			else
			{
				JOptionPane
						.showMessageDialog(
								null,
								"Spark has crashed (Unhandled Exception).\nReason: "
										+ e.getMessage()
										+ "\nThis error is being submitted to the Spark staff.");
			}
		}

		System.exit(0);
	}

	public Spark(String Account, String Hash, boolean Windowed,
			String IPAddress, int Port, int ResolutionX, int ResolutionY,
			String Password, int Clan, boolean Debug)
	{
		Tick = getTime();

		if (Security.CheckHash(Account, Hash) == false && Debug == false)
		{
			System.exit(0);
			return;
		}

		if (Security.CheckCriticalFilesModified() == true)
		{
			JOptionPane
					.showMessageDialog(null,
							"Critical Spark files have been modified!  Please reinstall the game.");
			System.exit(0);
			return;
		}

		gamePlayer.Name = Account;
		gamePlayer.Clan = Clan;
		gameOptions.displayWindowed = Windowed;

		this.Debug = Debug;

		// Resolution
		this.ResolutionX = ResolutionX;
		this.ResolutionY = ResolutionY;

		// Password
		this.Password = Password;

		OperatingSystem = System.getProperty("os.name");
		JavaVersion = System.getProperty("java.version");
		SystemArchitecture = System.getProperty("os.arch");
		System.err.println("Spark Version: " + Spark.Version);
		System.err.println("Java Version: " + JavaVersion);
		System.err.println("Operating System: " + OperatingSystem);
		System.err.println("System Architecture: " + SystemArchitecture);

		if (OperatingSystem.toLowerCase().contains("windows") == true)
		{
			WindowsHardwareUtil hardwareUtil = new WindowsHardwareUtil();

			HardwareDisplayDevice = hardwareUtil.VideoDisplayDevice;
			HardwareDisplayService = hardwareUtil.VideoDisplayService;
			HardwareDriverDate = hardwareUtil.VideoDriverDate;

			HardwareCPUName = hardwareUtil.CPUName;
			HardwareCPUSpeed = hardwareUtil.CPUSpeed;
			HardwareCPUVendor = hardwareUtil.CPUVendor;

			System.err.println("Hardware Display Device: "
					+ HardwareDisplayDevice);
			System.err.println("Hardware Display Service: "
					+ HardwareDisplayService);
			System.err.println("Hardware Display Driver Date: "
					+ HardwareDriverDate);
		}

		try
		{
			imgCollision = ImageIO.read(new File(
					"spark/images/imgCollision.png"));
			imgCollision.setAccelerationPriority(0);
			imgShipCollision = ImageIO.read(new File(
					"spark/images/imgShipCollision.png"));
			imgShipCollision.setAccelerationPriority(0);
		}
		catch (Exception e)
		{
			throwException(e);
		}

		Tick = getTime();
		gameNetCode.Initialize(IPAddress, Port);
		gameNetLinkLobby.Initialize();

		gameNetCode.SendTCPPing();

		gameSound.Initialize();
		if (gameOptions.engineMode.compareTo("lwjgl") == 0)
		{
			System.err.println("LWJGL Engine being used.");
			gameEngine = new SlickEngine(this);
		}
		else
		{
			System.err.println("Java2D Engine being used.");
			gameEngine = new Java2DEngine(this);
		}

		gameNetCode.SendTCPPing();

		gameEngine.Initialize();
	}

	public final void destroy()
	{
		gameNetCode.closeLink();
		gameNetLinkLobby.closeLink();

		if (gameEngine != null)
		{
			gameEngine.Destroy();
			gameEngine = null;
		}

		try
		{
			AL.destroy();
		}
		catch (Throwable t)
		{

		}

		if (CloseMessage != null)
		{
			JOptionPane.showMessageDialog(null, CloseMessage);
		}

		System.exit(0);
	}

	public final void Optimize()
	{
		// Precalculate variables for the game to increase speed

		// Sound
		if (gameSound != null)
			gameSound.MaxDistance = ResolutionX / 2 + ResolutionX / 4;

		// Player offset position
		gamePlayer.OffsetX = ResolutionX / 2 - 16;
		gamePlayer.OffsetY = ResolutionY / 2 - 16;

		// Map
		gameMap.EndX = ResolutionX / 16 + 1;
		gameMap.EndY = ResolutionY / 16 + 3;
		if (gameOptions.displayDisablePanel == true)
		{
			gameMap.EndX = gameMap.EndX + 1;
		}
		gameMap.MapWidthPixels = gameMap.GameMap.width * 16;
		gameMap.MapHeightPixels = gameMap.GameMap.height * 16;
		gameMap.FarPlaneMoveSizeX = 1280 - ResolutionX;
		gameMap.FarPlaneMoveSizeY = 960 - ResolutionY;

		// Interface
		gameInterface.InterfaceBlocks = (ResolutionY - 496) / 16 + 1;
	}

	public void throwException(Exception e)
	{
		gameNetLinkLobby.SendTCP("submitErrorReport");
		Destroy = true;
		e.printStackTrace();
		CloseMessage = "Spark has crashed.\nReason: "
				+ e.toString()
				+ "\nThis error is automatically being submitted to the Spark staff.";
		destroy();
	}

	public final void drawGame()
	{
		try
		{
			if (gameEngine.hasFocus() == false)
				return;
			if (LoadingResources == false && LoadingGame == false)
			{
				gameMap.drawMap();

				// Draw Lasers and Bouncies
				if (gameOptions.displayDisableAntiAliasing == false)
					gameEngine.setAntiAlias(true);
				gameEngine.setLineWidth(gameOptions.weaponLaserThickness);
				gameLasers.drawLasers();
				gameEngine.setLineWidth(gameOptions.weaponBouncyThickness);
				gameBouncies.drawBouncies();
				if (gameOptions.displayDisableAntiAliasing == false)
					gameEngine.setAntiAlias(false);
				gameEngine.setLineWidth(1.0f);

				gameEngine.setFont(GameResources.fontName);
				gameEngine.beginUse(GameResources.imgTuna);
				gameMissiles.drawMissiles();
				gamePlayers.drawPlayers();
				gameGrenades.drawGrenades();
				gameExplosions.drawExplosions();
				gameSmoke.drawSmoke();
				gameShrapnel.drawShrapnel();
				gameTrails.drawTrails();
				gameEngine.endUse();

				gameSparks.drawSparks();
				gamePlayers.drawPlayerNames();

				gameInterface.drawInterfaceGroundwork();

				gameEngine.setFont(GameResources.fontChat);
				gameChat.drawChat();

				gameEngine.beginUse(GameResources.imgTuna);
				gameInterface.drawInterface();
				if (gameDialog.DialogOpen == true)
				{
					gameEngine.endUse();
					gameDialog.drawDialog();
					gameEngine.beginUse(GameResources.imgTuna);
					gameDialog.drawCursor();
				}
				else
				{
					gameInterface.drawCursor();
				}
				gameEngine.endUse();
			}
			else
			{
				gameEngine.setColor(GameResources.colorBlack);
				gameEngine.fillRect(0, 0, ResolutionX, ResolutionY);

				gameEngine.setColor(GameResources.colorWhite);
				if (gameOptions.displayDisableFarplane == false)
				{
					gameEngine.drawImage(GameResources.imgFarplane, 0, 0,
							ResolutionX, ResolutionY, 0, 0, ResolutionX,
							ResolutionY);
				}

				gameEngine.drawString(LoadString, ResolutionX / 2
						- gameEngine.getStringWidth(LoadString) / 2,
						ResolutionY / 2 - 100);

				int Progress = (int) (((float) LoadProgress / (float) LoadTotal) * 150.0f);

				gameEngine.fillRect(ResolutionX / 2 - 75, ResolutionY / 2 - 60,
						Progress, 20);
				gameEngine.setColor(GameResources.colorBlack);
				gameEngine.drawRect(ResolutionX / 2 - 75, ResolutionY / 2 - 60,
						Progress, 20);
				gameEngine.drawRect(ResolutionX / 2 - 75, ResolutionY / 2 - 60,
						150, 20);
			}
		}
		catch (Exception e)
		{
			throwException(e);
		}
	}

	public final void updateGame(int delta)
	{
		if (Destroy == true)
		{
			destroy();
			return;
		}

		FPSCounter++;

		Tick = getTime();

		processQueue();

		if (gameNetCode != null)
		{
			gameNetCode.readTCPData();
			gameNetCode.readUDPData();
		}

		if (gameNetLinkLobby != null)
		{
			gameNetLinkLobby.readTCPData();
		}

		try
		{
			if (LoadingResources == false && LoadingGame == false)
			{
				Tick = getTime();

				if (Tick > timeUpdateTimestamp)
				{
					timeUpdateTimestamp = Tick + 30000;
					if (gameNetCode != null)
					{
						final long epoch = new Date().getTime();
						gameNetCode.SendTCP("timestamp " + Tick + " " + epoch);
					}
				}

				if (Tick > timePing)
				{
					timePing = Tick + 5000;
					gameNetCode.SendTCPPing();
					gameNetCode.SendUDPPing();
				}

				if (timeAFKKick != 0 && Tick > timeAFKKick)
				{
					System.err
							.println("AFK time limit has been reached, quitting.");
					if (gameNetLinkLobby != null)
					{
						gameNetLinkLobby.SendTCP("AFKKick");
					}
					destroy();
				}

				if (Tick > timeFPS)
				{
					timeFPS = Tick + 1000;
					FPS = FPSCounter;
					FPSCounter = 0;
				}

				gamePlayers.Cycle(delta);
				gamePlayer.Cycle(delta);
				gameInterface.Cycle();
				gameWeapons.Cycle(delta);
				gameMap.Cycle();
				gameSound.Cycle();
			}
		}
		catch (Exception e)
		{
			throwException(e);
		}

	}

	/**
	 * Get the accurate system time
	 * 
	 * @return The system time in milliseconds
	 */
	public final long getTime()
	{
		return (Sys.getTime() * 1000) / Sys.getTimerResolution();
	}

	/**
	 * Process the event queue
	 */
	public final void processQueue()
	{
		while (true)
		{
			final SparkEvent event = gameQueue.poll();
			if (event != null)
			{
				switch (event.Type)
				{
					case SparkEvents.E_KEYDOWN:
					{
						final KeyboardEvent keyEvent = (KeyboardEvent) event;
						gameInput.processKey(keyEvent.Key, keyEvent.c, true);
					}
						break;
					case SparkEvents.E_KEYUP:
					{
						final KeyboardEvent keyEvent = (KeyboardEvent) event;
						gameInput.processKey(keyEvent.Key, keyEvent.c, false);
					}
						break;
					case SparkEvents.E_MOUSEDOWN:
					{
						final MousePressEvent mouseEvent = (MousePressEvent) event;
						gameInput.MouseDown = true;
						gameInput.MouseUp = false;
						gameInput.processClick(mouseEvent.button);
					}
						break;
					case SparkEvents.E_MOUSEUP:
					{
						gameInput.MouseDown = false;
						gameInput.MouseUp = true;
					}
						break;
					case SparkEvents.E_DESTROY:
					{
						destroy();
					}
						break;
				}
			}
			else
			{
				break;
			}
		}
	}
}
