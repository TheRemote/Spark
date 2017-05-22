package codemallet.spark.options;

import java.awt.Color;
import java.awt.event.KeyEvent;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Properties;

import codemallet.spark.input.KeyConversion;

public final class Options
{
	Properties optionsLoader = new Properties();

	// Display Options
	public String engineMode;

	public Boolean displayWindowed;

	public Boolean displayDisableFarplane;

	public Boolean displayDisableAntiAliasing;

	public Boolean displayDisableRadarGuideBox;

	public Boolean displayDisablePanel;

	public Boolean displayLimitFPS;

	public Boolean displayDisableMrBandwidth;

	// Sound Options
	public Boolean soundEnable;

	public int soundGameVolume;

	public int soundVoiceVolume;

	public int soundVoiceChatVolume;

	public int soundMusicVolume;

	public boolean soundSelectedVoiceNotification;

	public boolean soundRechargedVoiceNotification;

	// Control Options
	public int keyUp;

	public int keyDown;

	public int keyLeft;

	public int keyRight;

	public int keyCruiseUp;

	public int keyCruiseDown;

	public int keyCruiseLeft;

	public int keyCruiseRight;

	public int keyDropFlag;

	public int keyToggleSpecial;

	public int keyDisplayPlayers;

	public int keySwitchTeams;

	public boolean specialsPrevious;

	// Chat Options
	public boolean chatToggling;

	public int keyToggleChat;

	public int keySendChat;

	public int chatMessageLifetime;

	// Mouse Options
	public boolean mouseClassicMode;

	public boolean mouseGrabbed;

	// Color Options
	public int colorGreenChat;

	public int colorRedChat;

	public int colorBlueChat;

	public int colorYellowChat;

	public int colorGreenWeapons;

	public int colorRedWeapons;

	public int colorBlueWeapons;

	public int colorYellowWeapons;

	// Font Options
	public boolean fontAntiAliasing;

	public String fontChat;

	public int fontChatSize;

	public boolean fontChatBold;

	public boolean fontChatShadow;

	public String fontName;

	public int fontNameSize;

	public boolean fontNameBold;

	public boolean fontNameShadow;

	// Java2D Options

	public String java2DRenderer;

	// Weapon Options

	public Float weaponLaserThickness;

	public Float weaponBouncyThickness;

	public Options()
	{
		loadOptions();
	}

	public void loadOptions()
	{
		try
		{
			optionsLoader.loadFromXML(new FileInputStream("spark/Options.dat"));
		}
		catch (Exception e)
		{
			loadDefaults();
			e.printStackTrace();
		}

		String parseString = null;

		try
		{
			// Game Properties
			engineMode = optionsLoader.getProperty("engineMode");
			parseString = optionsLoader.getProperty("displayWindowed");
			displayWindowed = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("displayDisableFarplane");
			displayDisableFarplane = Boolean.parseBoolean(parseString);
			parseString = optionsLoader
					.getProperty("displayDisableAntiAliasing");
			displayDisableAntiAliasing = Boolean.parseBoolean(parseString);
			parseString = optionsLoader
					.getProperty("displayDisableRadarGuideBox");
			displayDisableRadarGuideBox = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("displayDisablePanel");
			displayDisablePanel = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("displayLimitFPS");
			displayLimitFPS = Boolean.parseBoolean(parseString);
			parseString = optionsLoader
					.getProperty("displayDisableMrBandwidth");
			displayDisableMrBandwidth = Boolean.parseBoolean(parseString);

			// Sound Options
			parseString = optionsLoader.getProperty("soundEnable");
			soundEnable = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("soundGameVolume");
			soundGameVolume = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("soundVoiceVolume");
			soundVoiceVolume = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("soundVoiceChatVolume");
			soundVoiceChatVolume = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("soundMusicVolume");
			soundMusicVolume = Integer.parseInt(parseString);
			parseString = optionsLoader
					.getProperty("soundSelectedVoiceNotification");
			soundSelectedVoiceNotification = Boolean.parseBoolean(parseString);
			parseString = optionsLoader
					.getProperty("soundRechargedVoiceNotification");
			soundRechargedVoiceNotification = Boolean.parseBoolean(parseString);

			// Control Options
			parseString = optionsLoader.getProperty("keyUp");
			keyUp = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyDown");
			keyDown = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyLeft");
			keyLeft = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyRight");
			keyRight = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyCruiseUp");
			keyCruiseUp = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyCruiseDown");
			keyCruiseDown = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyCruiseLeft");
			keyCruiseLeft = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyCruiseRight");
			keyCruiseRight = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyDropFlag");
			keyDropFlag = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyToggleSpecial");
			keyToggleSpecial = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keyDisplayPlayers");
			keyDisplayPlayers = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keySwitchTeams");
			keySwitchTeams = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("specialsPrevious");
			specialsPrevious = Boolean.parseBoolean(parseString);

			// Chat Options
			parseString = optionsLoader.getProperty("chatToggling");
			chatToggling = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("keyToggleChat");
			keyToggleChat = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("keySendChat");
			keySendChat = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("chatMessageLifetime");
			chatMessageLifetime = Integer.parseInt(parseString);

			// Mouse Options
			parseString = optionsLoader.getProperty("mouseClassicMode");
			mouseClassicMode = Boolean.parseBoolean(parseString);
			parseString = optionsLoader.getProperty("mouseGrabbed");
			mouseGrabbed = Boolean.parseBoolean(parseString);

			// Color Options
			parseString = optionsLoader.getProperty("colorGreenChat");
			colorGreenChat = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorRedChat");
			colorRedChat = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorBlueChat");
			colorBlueChat = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorYellowChat");
			colorYellowChat = Integer.parseInt(parseString);

			parseString = optionsLoader.getProperty("colorGreenWeapons");
			colorGreenWeapons = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorRedWeapons");
			colorRedWeapons = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorBlueWeapons");
			colorBlueWeapons = Integer.parseInt(parseString);
			parseString = optionsLoader.getProperty("colorYellowWeapons");
			colorYellowWeapons = Integer.parseInt(parseString);

			// Font Options
			fontAntiAliasing = Boolean.parseBoolean(optionsLoader
					.getProperty("fontAntiAliasing"));

			fontChat = optionsLoader.getProperty("fontChat");
			fontChatSize = Integer.parseInt(optionsLoader
					.getProperty("fontChatSize"));
			fontChatBold = Boolean.parseBoolean(optionsLoader
					.getProperty("fontChatBold"));
			fontChatShadow = Boolean.parseBoolean(optionsLoader
					.getProperty("fontChatShadow"));

			fontName = optionsLoader.getProperty("fontName");
			fontNameSize = Integer.parseInt(optionsLoader
					.getProperty("fontNameSize"));
			fontNameBold = Boolean.parseBoolean(optionsLoader
					.getProperty("fontNameBold"));
			fontNameShadow = Boolean.parseBoolean(optionsLoader
					.getProperty("fontNameShadow"));

			// Java2D Options
			java2DRenderer = optionsLoader.getProperty("java2DRenderer");

			// Weapon Options
			weaponLaserThickness = Float.parseFloat(optionsLoader
					.getProperty("weaponLaserThickness"));
			weaponBouncyThickness = Float.parseFloat(optionsLoader
					.getProperty("weaponBouncyThickness"));
		}
		catch (Exception e)
		{
			loadDefaults();
			e.printStackTrace();
		}

		saveOptions();

		convertKeys();
	}

	public void loadDefaults()
	{
		// Game Options
		engineMode = "lwjgl";
		displayWindowed = false;
		displayDisableFarplane = false;
		displayDisableAntiAliasing = false;
		displayDisableRadarGuideBox = false;
		displayDisablePanel = false;
		displayLimitFPS = true;
		displayDisableMrBandwidth = false;

		// Sound Options
		soundEnable = true;
		soundGameVolume = 100;
		soundVoiceVolume = 100;
		soundVoiceChatVolume = 100;
		soundMusicVolume = 100;
		soundSelectedVoiceNotification = true;
		soundRechargedVoiceNotification = false;

		// Control Options
		keyUp = KeyEvent.VK_UP;
		keyDown = KeyEvent.VK_DOWN;
		keyLeft = KeyEvent.VK_LEFT;
		keyRight = KeyEvent.VK_RIGHT;

		keyCruiseUp = KeyEvent.VK_HOME;
		keyCruiseDown = KeyEvent.VK_END;
		keyCruiseLeft = KeyEvent.VK_DELETE;
		keyCruiseRight = KeyEvent.VK_PAGE_DOWN;

		keyDropFlag = KeyEvent.VK_TAB;
		keyToggleSpecial = KeyEvent.VK_CONTROL;
		keyDisplayPlayers = KeyEvent.VK_F1;
		keySwitchTeams = KeyEvent.VK_F4;

		specialsPrevious = false;

		// Chat Options
		chatToggling = false;
		keyToggleChat = KeyEvent.VK_ENTER;
		keySendChat = KeyEvent.VK_ENTER;
		chatMessageLifetime = 30;

		// Mouse Options
		mouseClassicMode = false;
		mouseGrabbed = true;

		// Font Options
		fontAntiAliasing = false;

		fontChat = "Verdana";
		fontChatSize = 11;
		fontChatBold = false;
		fontChatShadow = true;

		fontName = "Verdana";
		fontNameSize = 9;
		fontNameBold = false;
		fontNameShadow = true;

		// Color options
		colorGreenChat = Color.green.getRGB();
		colorRedChat = Color.red.getRGB();
		colorBlueChat = Color.blue.getRGB();
		colorYellowChat = Color.yellow.getRGB();

		colorGreenWeapons = Color.green.getRGB();
		colorRedWeapons = Color.red.getRGB();
		colorBlueWeapons = Color.blue.getRGB();
		colorYellowWeapons = Color.yellow.getRGB();

		// Java2D Options
		java2DRenderer = "GDI";

		// Weapon Options
		weaponLaserThickness = 1.0f;
		weaponBouncyThickness = 1.2f;

		saveOptions();
	}

	public void saveOptions()
	{
		// Save
		try
		{
			// Game Options
			optionsLoader.setProperty("engineMode", engineMode.toString());
			optionsLoader.setProperty("displayWindowed", displayWindowed
					.toString());
			optionsLoader.setProperty("displayDisableFarplane",
					displayDisableFarplane.toString());
			optionsLoader.setProperty("displayDisableAntiAliasing",
					displayDisableAntiAliasing.toString());
			optionsLoader.setProperty("displayDisableRadarGuideBox",
					displayDisableRadarGuideBox.toString());
			optionsLoader.setProperty("displayDisablePanel",
					displayDisablePanel.toString());
			optionsLoader.setProperty("displayLimitFPS", displayLimitFPS
					.toString());
			optionsLoader.setProperty("displayDisableMrBandwidth",
					displayDisableMrBandwidth.toString());

			// Sound Options
			optionsLoader.setProperty("soundEnable", soundEnable.toString());
			optionsLoader.setProperty("soundGameVolume", "" + soundGameVolume);
			optionsLoader
					.setProperty("soundVoiceVolume", "" + soundVoiceVolume);
			optionsLoader.setProperty("soundVoiceChatVolume", ""
					+ soundVoiceChatVolume);
			optionsLoader
					.setProperty("soundMusicVolume", "" + soundMusicVolume);
			optionsLoader.setProperty("soundSelectedVoiceNotification", ""
					+ soundSelectedVoiceNotification);
			optionsLoader.setProperty("soundRechargedVoiceNotification", ""
					+ soundRechargedVoiceNotification);

			// Control Options
			optionsLoader.setProperty("keyUp", "" + keyUp);
			optionsLoader.setProperty("keyDown", "" + keyDown);
			optionsLoader.setProperty("keyLeft", "" + keyLeft);
			optionsLoader.setProperty("keyRight", "" + keyRight);

			optionsLoader.setProperty("keyCruiseUp", "" + keyCruiseUp);
			optionsLoader.setProperty("keyCruiseDown", "" + keyCruiseDown);
			optionsLoader.setProperty("keyCruiseLeft", "" + keyCruiseLeft);
			optionsLoader.setProperty("keyCruiseRight", "" + keyCruiseRight);

			optionsLoader.setProperty("keyDropFlag", "" + keyDropFlag);
			optionsLoader
					.setProperty("keyToggleSpecial", "" + keyToggleSpecial);
			optionsLoader.setProperty("keyDisplayPlayers", ""
					+ keyDisplayPlayers);
			optionsLoader.setProperty("keySwitchTeams", "" + keySwitchTeams);

			optionsLoader
					.setProperty("specialsPrevious", "" + specialsPrevious);

			// Chat Options
			optionsLoader.setProperty("chatToggling", "" + chatToggling);
			optionsLoader.setProperty("keyToggleChat", "" + keyToggleChat);
			optionsLoader.setProperty("keySendChat", "" + keySendChat);
			optionsLoader.setProperty("chatMessageLifetime", ""
					+ chatMessageLifetime);

			// Mouse Options
			optionsLoader
					.setProperty("mouseClassicMode", "" + mouseClassicMode);
			optionsLoader.setProperty("mouseGrabbed", "" + mouseGrabbed);

			// Color Options
			optionsLoader.setProperty("colorGreenChat", "" + colorGreenChat);
			optionsLoader.setProperty("colorRedChat", "" + colorRedChat);
			optionsLoader.setProperty("colorBlueChat", "" + colorBlueChat);
			optionsLoader.setProperty("colorYellowChat", "" + colorYellowChat);

			optionsLoader.setProperty("colorGreenWeapons", ""
					+ colorGreenWeapons);
			optionsLoader.setProperty("colorRedWeapons", "" + colorRedWeapons);
			optionsLoader
					.setProperty("colorBlueWeapons", "" + colorBlueWeapons);
			optionsLoader.setProperty("colorYellowWeapons", ""
					+ colorYellowWeapons);

			// Font Options
			optionsLoader
					.setProperty("fontAntiAliasing", "" + fontAntiAliasing);

			optionsLoader.setProperty("fontName", fontName);
			optionsLoader.setProperty("fontNameSize", "" + fontNameSize);
			optionsLoader.setProperty("fontNameBold", "" + fontNameBold);
			optionsLoader.setProperty("fontNameShadow", "" + fontNameShadow);

			optionsLoader.setProperty("fontChat", fontChat);
			optionsLoader.setProperty("fontChatSize", "" + fontChatSize);
			optionsLoader.setProperty("fontChatBold", "" + fontChatBold);
			optionsLoader.setProperty("fontChatShadow", "" + fontChatShadow);

			// Java2D Options
			optionsLoader.setProperty("java2DRenderer", java2DRenderer);

			// Weapon Options
			optionsLoader.setProperty("weaponLaserThickness",
					weaponLaserThickness.toString());
			optionsLoader.setProperty("weaponBouncyThickness",
					weaponBouncyThickness.toString());

			// Write the options

			optionsLoader.storeToXML(new FileOutputStream("spark/Options.dat"),
					null);
		}
		catch (Exception e)
		{
			loadDefaults();
			e.printStackTrace();
		}
	}

	private void convertKeys()
	{
		keyUp = KeyConversion.convertToLWJGL(keyUp);
		keyDown = KeyConversion.convertToLWJGL(keyDown);
		keyLeft = KeyConversion.convertToLWJGL(keyLeft);
		keyRight = KeyConversion.convertToLWJGL(keyRight);

		keyCruiseUp = KeyConversion.convertToLWJGL(keyCruiseUp);
		keyCruiseDown = KeyConversion.convertToLWJGL(keyCruiseDown);
		keyCruiseLeft = KeyConversion.convertToLWJGL(keyCruiseLeft);
		keyCruiseRight = KeyConversion.convertToLWJGL(keyCruiseRight);

		keyDropFlag = KeyConversion.convertToLWJGL(keyDropFlag);
		keyToggleSpecial = KeyConversion.convertToLWJGL(keyToggleSpecial);
		keyDisplayPlayers = KeyConversion.convertToLWJGL(keyDisplayPlayers);
		keySwitchTeams = KeyConversion.convertToLWJGL(keySwitchTeams);

		keyToggleChat = KeyConversion.convertToLWJGL(keyToggleChat);
		keySendChat = KeyConversion.convertToLWJGL(keySendChat);
	}
}
