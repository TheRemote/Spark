package codemallet.spark.engines.java2d;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.GradientPaint;
import java.awt.RenderingHints;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameEngine;

public final class Java2DEngine implements GameEngine
{
	private final Spark game;

	boolean Running = true;

	public Java2DGame java2DGame;

	public Java2DResources java2DResources;

	public Java2DFrame java2DFrame;

	private int CurrentImage;

	public Java2DEngine(Spark game)
	{
		this.game = game;
	}

	public final void Initialize()
	{
		java2DResources = new Java2DResources(game);
		java2DFrame = new Java2DFrame(game);
		java2DGame = new Java2DGame(game, this);

		java2DGame.run();
	}

	public final void Destroy()
	{
		Running = false;
		if (java2DFrame != null)
		{
			java2DFrame.Destroying = true;
			java2DFrame.dispose();
			java2DFrame = null;
		}
	}

	public final boolean hasFocus()
	{
		if (java2DFrame.Iconified == false)
			return true;
		else
			return false;
	}

	public final void beginUse(int Resource)
	{
		CurrentImage = Resource;
	}

	public final void endUse()
	{
		CurrentImage = 0;
	}

	public final void drawImage(int Image, int DrawX, int DrawY, int DrawX2,
			int DrawY2, int SrcX, int SrcY, int SrcX2, int SrcY2)
	{
		java2DGame.g.drawImage(java2DResources.images[Image], DrawX, DrawY,
				DrawX2, DrawY2, SrcX, SrcY, SrcX2, SrcY2, null);
	}

	public final void drawInUse(int DrawX, int DrawY, int DrawX2, int DrawY2,
			int SrcX, int SrcY, int SrcX2, int SrcY2)
	{
		java2DGame.g.drawImage(java2DResources.images[CurrentImage], DrawX,
				DrawY, DrawX2, DrawY2, SrcX, SrcY, SrcX2, SrcY2, null);
	}

	public final void drawRect(float X, float Y, float Width, float Height)
	{
		java2DGame.g.drawRect((int) X, (int) Y, (int) Width, (int) Height);
	}

	public final void fillRect(float X, float Y, float Width, float Height)
	{
		java2DGame.g.fillRect((int) X, (int) Y, (int) Width, (int) Height);
	}

	public final void drawOval(float X, float Y, float Width, float Height)
	{
		java2DGame.g.drawOval((int) X, (int) Y, (int) Width, (int) Height);
	}

	public final void fillOval(float X, float Y, float Width, float Height)
	{
		java2DGame.g.fillOval((int) X, (int) Y, (int) Width, (int) Height);
	}

	public final void drawLine(float X1, float Y1, float X2, float Y2)
	{
		java2DGame.g.drawLine((int) X1, (int) Y1, (int) X2, (int) Y2);
	}

	public final void drawGradientLine(float X1, float Y1, float Red1,
			float Green1, float Blue1, float Alpha1, float X2, float Y2,
			float Red2, float Green2, float Blue2, float Alpha2)
	{
		GradientPaint gp = new GradientPaint(X1, Y1, new Color(Red1, Green1,
				Blue1, Alpha1), X2, Y2, new Color(Red2, Green2, Blue2, Alpha2),
				true);
		java2DGame.g.setPaint(gp);
		java2DGame.g.drawLine((int) X1, (int) Y1, (int) X2, (int) Y2);
		java2DGame.g.setPaint(null);
	}

	public final void setFont(int Font)
	{
		java2DGame.g.setFont(java2DResources.fonts[Font]);
	}

	public final void drawString(String DrawString, int X, int Y)
	{
		if (DrawString == null || DrawString.length() == 0)
			return;

		Y = Y + getFontHeight() - 2;
		java2DGame.g.drawString(DrawString, X, Y);
	}

	public final int getStringWidth(String WidthString)
	{
		if (WidthString == null || WidthString.length() == 0)
			return 0;

		return java2DGame.g.getFontMetrics().stringWidth(WidthString);
	}

	public final int getFontHeight()
	{
		return java2DGame.g.getFontMetrics().getHeight()
				- java2DGame.g.getFontMetrics().getMaxDescent();
	}

	// Drawing
	public final void setAntiAlias(boolean Enabled)
	{
		java2DGame.g.setRenderingHint(RenderingHints.KEY_ALPHA_INTERPOLATION,
				RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING,
				RenderingHints.VALUE_COLOR_RENDER_SPEED);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_DITHERING,
				RenderingHints.VALUE_DITHER_DISABLE);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
				RenderingHints.VALUE_FRACTIONALMETRICS_OFF);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
				RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_RENDERING,
				RenderingHints.VALUE_RENDER_SPEED);
		java2DGame.g.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,
				RenderingHints.VALUE_STROKE_NORMALIZE);

		if (Enabled == true)
		{
			java2DGame.g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					RenderingHints.VALUE_ANTIALIAS_ON);
		}
		else
		{
			java2DGame.g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					RenderingHints.VALUE_ANTIALIAS_OFF);
		}
	}

	public final void setLineWidth(float Width)
	{
		java2DGame.g.setStroke(new BasicStroke(Width));
	}

	public final void setColor(int Color)
	{
		java2DGame.g.setColor(java2DResources.colors[Color]);
	}

	public final void setColorRGBA(int Red, int Green, int Blue, int Alpha)
	{
		java2DGame.g.setColor(new Color(Red, Green, Blue, Alpha));
	}

	public final void setColorBlending(boolean Blending)
	{

	}
}
