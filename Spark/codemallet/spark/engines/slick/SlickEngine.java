package codemallet.spark.engines.slick;

import javax.swing.JOptionPane;

import org.newdawn.slick.AppGameContainer;
import org.newdawn.slick.Color;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameEngine;
import codemallet.spark.engines.GameResources;

public final class SlickEngine implements GameEngine
{
	private final Spark game;

	private AppGameContainer container;

	private SlickGame slickGame;

	protected SlickResources slickResources;

	// Drawing
	private int CurrentTexture;

	private int CurrentColor;

	private boolean Blending;

	private int Red;

	private int Green;

	private int Blue;

	private int Alpha;

	private int Font;

	public SlickEngine(Spark game)
	{
		this.game = game;
	}

	public final void Initialize()
	{
		// Renderer.setRenderer(Renderer.VERTEX_ARRAY_RENDERER);
		try
		{
			if (AppGameContainer.getBuildVersion() < 206)
			{
				System.err
						.println("Your slick.jar is outdated!  Please reinstall the game.");
				JOptionPane
						.showMessageDialog(null,
								"Your slick.jar is outdated!  Please reinstall the game.");
				System.exit(0);
				return;
			}

			slickResources = new SlickResources(game);
			slickGame = new SlickGame(game, this);

			container = new AppGameContainer(slickGame);

			// Setup the display
			if (game.gameOptions.displayWindowed == true)
				container.setDisplayMode(game.ResolutionX, game.ResolutionY,
						false);
			else
				container.setDisplayMode(game.ResolutionX, game.ResolutionY,
						true);

			container.start();
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public final void Destroy()
	{
		container.exit();
	}

	public final boolean hasFocus()
	{
		return true;
	}

	public final void drawImage(int Image, int DrawX, int DrawY, int DrawX2,
			int DrawY2, int SrcX, int SrcY, int SrcX2, int SrcY2)
	{
		if (CurrentTexture == 0)
		{
			if (slickGame.g != null)
			{
				slickGame.g.drawImage(slickResources.images[Image], DrawX,
						DrawY, DrawX2, DrawY2, SrcX, SrcY, SrcX2, SrcY2);
			}
		}
	}

	public final void drawInUse(int DrawX, int DrawY, int DrawX2, int DrawY2,
			int SrcX, int SrcY, int SrcX2, int SrcY2)
	{
		if (CurrentTexture > 0)
		{
			if (slickGame.g != null)
			{
				if (Blending == false)
				{
					slickResources.images[CurrentTexture].drawEmbedded(DrawX,
							DrawY, DrawX2, DrawY2, SrcX, SrcY, SrcX2, SrcY2,
							Color.white);
				}
				else
				{
					slickResources.images[CurrentTexture].drawEmbedded(DrawX,
							DrawY, DrawX2, DrawY2, SrcX, SrcY, SrcX2, SrcY2,
							slickGame.g.getColor());
				}
			}
		}
	}

	public final void beginUse(int Resource)
	{
		CurrentTexture = Resource;

		switch (Resource)
		{
			case GameResources.imgTuna:
				slickResources.images[GameResources.imgTuna].startUse();
				break;
			case GameResources.imgTiles:
				slickResources.images[GameResources.imgTiles].startUse();
				break;
			case GameResources.imgFarplane:
				slickResources.images[GameResources.imgFarplane].startUse();
				break;
			case GameResources.imgMrBandwidth:
				slickResources.images[GameResources.imgMrBandwidth].startUse();
				break;
		}
	}

	public final void endUse()
	{
		switch (CurrentTexture)
		{
			case GameResources.imgTuna:
				slickResources.images[GameResources.imgTuna].endUse();
				break;
			case GameResources.imgTiles:
				slickResources.images[GameResources.imgTiles].endUse();
				break;
			case GameResources.imgFarplane:
				slickResources.images[GameResources.imgFarplane].endUse();
				break;
			case GameResources.imgMrBandwidth:
				slickResources.images[GameResources.imgMrBandwidth].endUse();
				break;
		}

		CurrentTexture = 0;
	}

	public final void setColor(int Color)
	{
		if (slickGame.g != null && CurrentColor != Color)
		{
			CurrentColor = Color;
			slickGame.g.setColor(slickResources.colors[Color]);
		}
	}

	public final void setAntiAlias(boolean Enabled)
	{
		if (slickGame.g != null)
		{
			slickGame.g.setAntiAlias(Enabled);
		}
	}

	public final void setLineWidth(float Width)
	{
		if (slickGame.g != null)
		{
			slickGame.g.setLineWidth(Width);
		}
	}

	public final void drawRect(float X, float Y, float Width, float Height)
	{
		if (CurrentTexture == 0)
		{
			if (slickGame.g != null)
			{
				slickGame.g.drawRect(X, Y, Width, Height);
			}
		}
	}

	public final void fillRect(float X, float Y, float Width, float Height)
	{
		if (CurrentTexture == 0)
		{
			if (slickGame.g != null)
			{
				slickGame.g.fillRect(X, Y, Width, Height);
			}
		}
	}

	public final void setFont(int Font)
	{
		this.Font = Font;
	}

	public final void drawString(String DrawString, int X, int Y)
	{
		if (DrawString == null || DrawString.length() == 0)
			return;

		if (slickGame.g != null)
		{
			if (CurrentTexture == 0)
			{
				if (slickResources.fonts[Font] != null)
				{
					slickResources.fonts[Font].drawString(X, Y, DrawString,
							slickResources.colors[CurrentColor]);
				}
				else
				{
					Y = Y + getFontHeight() - 2;
					slickGame.g.drawString(DrawString, X, Y);
				}
			}
		}
	}

	public final int getStringWidth(String WidthString)
	{
		if (WidthString == null || WidthString.length() == 0)
			return 0;

		if (slickGame.g != null)
		{
			if (slickResources.fonts[Font] != null)
				return (int) slickResources.fonts[Font].getWidth(WidthString);
			else
				return slickGame.g.getFont().getWidth(WidthString);
		}
		else
		{
			return -1;
		}
	}

	public final int getFontHeight()
	{
		if (slickGame.g != null)
		{
			if (slickResources.fonts[Font] != null)
				return slickResources.fonts[Font].getLineHeight();
			else
				return slickGame.g.getFont().getLineHeight() / 2;
		}
		else
		{
			return -1;
		}
	}

	public final void drawOval(float X, float Y, float Width, float Height)
	{
		if (slickGame.g != null)
		{
			if (CurrentTexture == 0)
			{
				slickGame.g.drawOval(X, Y, Width, Height);
			}
		}
	}

	public final void fillOval(float X, float Y, float Width, float Height)
	{
		if (slickGame.g != null)
		{
			if (CurrentTexture == 0)
			{
				slickGame.g.fillOval(X, Y, Width, Height);
			}
		}
	}

	public final void drawLine(float X1, float Y1, float X2, float Y2)
	{
		if (slickGame.g != null)
		{
			if (CurrentTexture == 0)
			{
				slickGame.g.drawLine(X1, Y1, X2, Y2);
			}
		}
	}

	public final void drawGradientLine(float X1, float Y1, float Red1,
			float Green1, float Blue1, float Alpha1, float X2, float Y2,
			float Red2, float Green2, float Blue2, float Alpha2)
	{
		slickGame.g.drawGradientLine(X1, Y1, Red1, Green1, Blue1, Alpha1, X2,
				Y2, Red2, Green2, Blue2, Alpha2);
	}

	public final void setColorRGBA(int Red, int Green, int Blue, int Alpha)
	{
		if (slickGame.g != null)
		{
			CurrentColor = 0;
			slickGame.g.setColor(new Color(Red, Green, Blue, Alpha));
			this.Red = Red;
			this.Green = Green;
			this.Blue = Blue;
			this.Alpha = Alpha;
		}
	}

	public final void setColorBlending(boolean Blending)
	{
		if (Blending == true)
		{
			setColorRGBA(Red, Green, Blue, Alpha);
		}
		else
		{
			setColor(GameResources.colorWhite);
		}
		this.Blending = Blending;
	}

}
