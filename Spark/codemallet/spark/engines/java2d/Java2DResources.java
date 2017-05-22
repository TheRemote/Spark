package codemallet.spark.engines.java2d;

import java.awt.Color;
import java.awt.Font;
import java.awt.image.BufferedImage;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class Java2DResources implements GameResources
{
	private final Spark game;
	
	public final BufferedImage[] images;

	public final Color[] colors;

	public final Font[] fonts;

	public Java2DResources(Spark game)
	{
		this.game = game;
		
		images = new BufferedImage[GameResources.imgCount];
		colors = new Color[GameResources.colorCount];
		fonts = new Font[GameResources.fontCount];

		LoadColors();
	}

	public void LoadColors()
	{
		colors[GameResources.colorWhite] = Color.white;
		colors[GameResources.colorBlack] = Color.black;
		colors[GameResources.colorGreen] = new Color(game.gameOptions.colorGreenChat);
		colors[GameResources.colorRed] = new Color(game.gameOptions.colorRedChat);
		colors[GameResources.colorBlue] =new Color(game.gameOptions.colorBlueChat);
		colors[GameResources.colorYellow] =new Color(game.gameOptions.colorYellowChat);
		colors[GameResources.colorGray] = Color.gray;
		colors[GameResources.colorMagenta] = Color.magenta;
		colors[GameResources.colorDarkGray] = Color.darkGray;
		colors[GameResources.colorOrange] = Color.orange;
		colors[GameResources.colorCyan] = Color.cyan;
		
		// Weapons
		colors[GameResources.colorGreenWeapons] = new Color(game.gameOptions.colorGreenWeapons);
		colors[GameResources.colorRedWeapons] = new Color(game.gameOptions.colorRedWeapons);
		colors[GameResources.colorBlueWeapons] =new Color(game.gameOptions.colorBlueWeapons);
		colors[GameResources.colorYellowWeapons] =new Color(game.gameOptions.colorYellowWeapons);

		colors[GameResources.colorHealth] = new Color(255, 0, 0, 150);
		colors[GameResources.colorEnergy] = new Color(0, 0, 255, 150);
		colors[GameResources.colorBackground] = new Color(0, 0, 0, 130);
		colors[GameResources.colorInterfaceShade] = new Color(0, 40, 0, 255);
		colors[GameResources.colorPrivateMessage] = new Color(255, 153, 51);
		colors[GameResources.colorName] = new Color(222, 170, 0);
		colors[GameResources.colorPoints] = new Color(222, 120, 0);
	}
}
