package codemallet.spark.engines.java2d;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;

import javax.imageio.ImageIO;

import org.lwjgl.opengl.Display;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameEngine;
import codemallet.spark.engines.GameResources;

public final class Java2DGame
{
	private final Spark game;

	private final Java2DEngine java2DEngine;

	private final Java2DFrame java2DFrame;

	private final Java2DResources java2DResources;

	protected Graphics2D g;

	private Graphics2D g2;

	private BufferedImage imgBackBuffer;

	public Java2DGame(Spark game, Java2DEngine java2DEngine)
	{
		this.game = game;

		this.java2DEngine = java2DEngine;
		this.java2DFrame = java2DEngine.java2DFrame;
		this.java2DResources = java2DEngine.java2DResources;
	}

	public void run()
	{
		long cycles = 0;
		long currentTime = 0;
		long lastTime = 0;
		int delta = 0;

		LoadResources();

		lastTime = game.getTime();

		int SleepThreshold = 0;

		while (java2DEngine.Running == true)
		{
			currentTime = game.getTime();
			delta = (int) (currentTime - lastTime);
			lastTime = currentTime;

			cycles = delta / GameEngine.MaxDelta;
			for (int i = 0; i < cycles; i++)
			{
				updateGame((int) GameEngine.MaxDelta, currentTime);
			}
			updateGame((int) (delta % GameEngine.MaxDelta), currentTime);

			drawGame();

			try
			{
				SleepThreshold++;
				if (SleepThreshold >= 3)
				{
					Thread.sleep(1);
					SleepThreshold = 0;
				}
			}
			catch (Exception e)
			{

			}

			if (game.gameOptions.displayLimitFPS == true)
			{
				Display.sync(GameEngine.TargetFPS);
			}
		}
	}

	public void LoadResources()
	{
		game.gameNetCode.SendTCPPing();

		game.LoadTotal = 5;
		game.LoadProgress = 0;
		game.LoadString = "Loading Farplane...";
		drawGame();

		if (game.gameOptions.displayDisableFarplane == false)
		{
			java2DResources.images[GameResources.imgFarplane] = LoadImage("spark/images/imgFarplane.png");
		}

		game.gameNetCode.SendTCPPing();

		game.LoadProgress = 1;
		game.LoadString = "Loading Tiles...";
		drawGame();

		java2DResources.images[GameResources.imgTiles] = LoadImage("spark/images/imgTiles.png");

		game.gameNetCode.SendTCPPing();

		game.LoadProgress = 2;
		game.LoadString = "Loading Tuna...";
		drawGame();

		java2DResources.images[GameResources.imgTuna] = LoadImage("spark/images/imgTuna.png");

		game.gameNetCode.SendTCPPing();

		game.LoadProgress = 3;
		game.LoadString = "Loading Mr. Bandwidth...";
		drawGame();

		java2DResources.images[GameResources.imgMrBandwidth] = LoadImage("spark/images/imgMrBandwidth.png");

		game.gameNetCode.SendTCPPing();

		game.LoadProgress = 4;
		game.LoadString = "Loading Fonts...";
		drawGame();

		try
		{
			if (game.gameOptions.fontChatBold == false)
			{
				java2DResources.fonts[GameResources.fontChat] = new Font(
						game.gameOptions.fontChat, Font.PLAIN,
						game.gameOptions.fontChatSize);
			}
			else
			{
				java2DResources.fonts[GameResources.fontChat] = new Font(
						game.gameOptions.fontChat, Font.BOLD,
						game.gameOptions.fontChatSize);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			try
			{
				java2DResources.fonts[GameResources.fontChat] = Font
						.createFont(Font.TRUETYPE_FONT,
								new FileInputStream("portal/fonts/verdana.ttf"))
						.deriveFont(11.0f);
			}
			catch (Exception ee)
			{

			}
		}

		try
		{
			if (game.gameOptions.fontNameBold == false)
			{
				java2DResources.fonts[GameResources.fontName] = new Font(
						game.gameOptions.fontName, Font.PLAIN,
						game.gameOptions.fontNameSize);
			}
			else
			{
				java2DResources.fonts[GameResources.fontName] = new Font(
						game.gameOptions.fontName, Font.BOLD,
						game.gameOptions.fontNameSize);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			try
			{
				java2DResources.fonts[GameResources.fontName] = Font
						.createFont(Font.TRUETYPE_FONT,
								new FileInputStream("portal/fonts/verdana.ttf"))
						.deriveFont(9.0f);
			}
			catch (Exception ee)
			{

			}
		}

		game.LoadingResources = false;
		game.gameNetCode.SendTCP("joinGame");
	}

	public BufferedImage LoadImage(String Path)
	{
		try
		{
			File file = new File(Path);
			BufferedImage im = ImageIO.read(file);
			// int transparency = im.getColorModel().getTransparency();
			BufferedImage copy = java2DFrame.gc.createCompatibleImage(im
					.getWidth(), im.getHeight(), BufferedImage.BITMASK);

			Graphics2D g2d = (Graphics2D) copy.getGraphics();
			g2d.drawImage(im, 0, 0, null);
			g2d.dispose();
			return copy;
		}
		catch (Exception e)
		{
			game.throwException(e);
			return null;
		}
	}

	public void updateGame(int delta, long nanoTick)
	{
		try
		{
			Point mousePosition = java2DFrame.drawCanvas.getMousePosition();
			if (mousePosition != null)
			{
				game.gameInput.MouseX = mousePosition.x;
				game.gameInput.MouseY = mousePosition.y;
			}
		}
		catch (Exception e)
		{

		}

		game.updateGame(delta);
	}

	public void drawGame()
	{
		if (java2DFrame.Iconified == true)
			return;

		if (g == null)
		{
			imgBackBuffer = java2DFrame.gc.createCompatibleImage(
					game.ResolutionX, game.ResolutionY);
			g = (Graphics2D) imgBackBuffer.getGraphics();
		}

		if (g2 == null)
		{
			g2 = (Graphics2D) java2DFrame.drawCanvas.getGraphics();
		}

		game.drawGame();

		try
		{
			g2.drawImage(imgBackBuffer, 0, 0, null);
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}
}
