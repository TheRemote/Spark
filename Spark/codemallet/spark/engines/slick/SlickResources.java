package codemallet.spark.engines.slick;

import org.newdawn.slick.Color;
import org.newdawn.slick.Font;
import org.newdawn.slick.Image;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class SlickResources implements GameResources
{
	private final Spark game;

	public final Image[] images;

	public final Color[] colors;

	public final Font[] fonts;

	public SlickResources(Spark game)
	{
		this.game = game;

		images = new Image[GameResources.imgCount];
		colors = new Color[GameResources.colorCount];
		fonts = new Font[GameResources.fontCount];

		LoadColors();
	}

	public final void LoadColors()
	{
		java.awt.Color loadColor;

		colors[GameResources.colorWhite] = Color.white;
		colors[GameResources.colorBlack] = Color.black;
		loadColor = new java.awt.Color(game.gameOptions.colorGreenChat);
		colors[GameResources.colorGreen] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorRedChat);
		colors[GameResources.colorRed] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorBlueChat);
		colors[GameResources.colorBlue] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorYellowChat);
		colors[GameResources.colorYellow] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		colors[GameResources.colorGray] = Color.gray;
		colors[GameResources.colorMagenta] = Color.magenta;
		colors[GameResources.colorDarkGray] = Color.darkGray;
		colors[GameResources.colorOrange] = Color.orange;
		colors[GameResources.colorCyan] = Color.cyan;

		// Weapons
		loadColor = new java.awt.Color(game.gameOptions.colorGreenWeapons);
		colors[GameResources.colorGreenWeapons] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorRedWeapons);
		colors[GameResources.colorRedWeapons] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorBlueWeapons);
		colors[GameResources.colorBlueWeapons] = new Color(loadColor.getRed(),
				loadColor.getGreen(), loadColor.getBlue(), loadColor.getAlpha());
		loadColor = new java.awt.Color(game.gameOptions.colorYellowWeapons);
		colors[GameResources.colorYellowWeapons] = new Color(
				loadColor.getRed(), loadColor.getGreen(), loadColor.getBlue(),
				loadColor.getAlpha());

		colors[GameResources.colorHealth] = new Color(255, 0, 0, 150);
		colors[GameResources.colorEnergy] = new Color(0, 0, 255, 150);
		colors[GameResources.colorBackground] = new Color(0, 0, 0, 130);
		colors[GameResources.colorInterfaceShade] = new Color(0, 40, 0, 255);
		colors[GameResources.colorPrivateMessage] = new Color(255, 153, 51);
		colors[GameResources.colorName] = new Color(222, 170, 0);
		colors[GameResources.colorPoints] = new Color(222, 120, 0);
	}
}
