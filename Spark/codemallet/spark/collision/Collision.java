package codemallet.spark.collision;

// Collision.java
// Copyright James Chambers, May 2006, 05jchambers@gmail.com

import java.awt.Rectangle;
import java.awt.image.BufferedImage;

public final class Collision
{
	public static final boolean RectCollision(int X, int Y, int width, int height,
			int tolerance1, int X2, int Y2, int width2, int height2,
			int tolerance2)
	{
		Rectangle rct = new Rectangle();

		rct.x = X + tolerance1;
		rct.y = Y + tolerance1;
		rct.width = width - tolerance1 * 2;
		rct.height = height - tolerance1 * 2;

		Rectangle rct2 = new Rectangle();

		rct2.x = X2 + tolerance2;
		rct2.y = Y2 + tolerance2;
		rct2.width = width2 - tolerance2 * 2;
		rct2.height = height2 - tolerance2 * 2;

		if (rct.intersects(rct2) == true)
			return true;

		return false;
	}

	public static final boolean PixelCollision(BufferedImage Img1, int X, int Y,
			int width, int height, int SrcX, int SrcY, BufferedImage Img2,
			int X2, int Y2, int width2, int height2, int SrcX2, int SrcY2)
	{
		Rectangle rct = new Rectangle();

		rct.x = X;
		rct.y = Y;
		rct.width = width;
		rct.height = height;

		Rectangle rct2 = new Rectangle();

		rct2.x = X2;
		rct2.y = Y2;
		rct2.width = width2;
		rct2.height = height2;

		// If this is true, falls within bounds for pixel checking
		if (rct.intersects(rct2) == true)
		{
			Rectangle intersection = rct.intersection(rct2);

			int Image1StartX = SrcX + (intersection.x - X);
			int Image1StartY = SrcY + (intersection.y - Y);

			int Image2StartX = SrcX2 + (intersection.x - X2);
			int Image2StartY = SrcY2 + (intersection.y - Y2);

			for (int j = 0; j < intersection.height; j++)
			{
				for (int i = 0; i < intersection.width; i++)
				{
					int pixel1 = Img1
							.getRGB(Image1StartX + i, Image1StartY + j);
					int alpha1 = (pixel1 >> 24) & 0xff;

					int pixel2 = Img2
							.getRGB(Image2StartX + i, Image2StartY + j);
					int alpha2 = (pixel2 >> 24) & 0xff;

					if (alpha1 > 0 && alpha2 > 0)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	public static final Rectangle PixelPointCollision(int PointX, int PointY,
			BufferedImage Img1, int X, int Y, int width, int height, int SrcX,
			int SrcY)
	{
		Rectangle rct = new Rectangle();

		rct.x = PointX;
		rct.y = PointY;
		rct.width = 1;
		rct.height = 1;

		Rectangle rct2 = new Rectangle();

		rct2.x = X;
		rct2.y = Y;
		rct2.width = width;
		rct2.height = height;

		if (rct.intersects(rct2) == true)
		{
			Rectangle intersection = rct.intersection(rct2);

			int Image1StartX = SrcX + (intersection.x - X);
			int Image1StartY = SrcY + (intersection.y - Y);
			int pixel1 = Img1.getRGB(Image1StartX, Image1StartY);
			int alpha1 = (pixel1 >> 24) & 0xff;

			if (alpha1 > 0)
			{
				return intersection;
			}
		}

		return null;
	}

	public static final Rectangle PixelPointWeaponCollision(int PointX, int PointY,
			BufferedImage Img1, int X, int Y, int width, int height, int SrcX,
			int SrcY, int Tolerance)
	{
		Rectangle rct = new Rectangle();

		rct.x = PointX;
		rct.y = PointY;
		rct.width = 1;
		rct.height = 1;

		Rectangle rct2 = new Rectangle();

		rct2.x = X + Tolerance;
		rct2.y = Y + Tolerance;
		rct2.width = width - Tolerance;
		rct2.height = height - Tolerance;

		if (rct.intersects(rct2) == true)
		{
			Rectangle intersection = rct.intersection(rct2);

			int Image1StartX = SrcX + (intersection.x - rct2.x);
			int Image1StartY = SrcY + (intersection.y - rct2.y);
			int pixel1 = Img1.getRGB(Image1StartX, Image1StartY);
			int alpha = (pixel1 >> 24) & 0xff;
			int red = (pixel1 >> 16) & 0xff;
			int green = (pixel1 >> 8) & 0xff;
			int blue = (pixel1) & 0xff;

			if (alpha > 0 && !(red == 255 && green == 0 && blue == 0))
			{
				return intersection;
			}
		}

		return null;
	}
	
	public static final boolean PixelTankCollision(BufferedImage Img1, int X, int Y,
			int width, int height, int SrcX, int SrcY, int Tolerance1, BufferedImage Img2,
			int X2, int Y2, int width2, int height2, int SrcX2, int SrcY2, int Tolerance2)
	{
		Rectangle rct = new Rectangle();

		rct.x = X + Tolerance1;
		rct.y = Y + Tolerance1;
		rct.width = width - Tolerance1;
		rct.height = height - Tolerance1;

		Rectangle rct2 = new Rectangle();

		rct2.x = X2 + Tolerance2;
		rct2.y = Y2 + Tolerance2;
		rct2.width = width2 - Tolerance2;
		rct2.height = height2 - Tolerance2;

		// If this is true, falls within bounds for pixel checking
		if (rct.intersects(rct2) == true)
		{
			Rectangle intersection = rct.intersection(rct2);

			int Image1StartX = SrcX + (intersection.x - X);
			int Image1StartY = SrcY + (intersection.y - Y);

			int Image2StartX = SrcX2 + (intersection.x - X2);
			int Image2StartY = SrcY2 + (intersection.y - Y2);

			for (int j = 0; j < intersection.height; j++)
			{
				for (int i = 0; i < intersection.width; i++)
				{
					int pixel1 = Img1
							.getRGB(Image1StartX + i, Image1StartY + j);
					int alpha1 = (pixel1 >> 24) & 0xff;
					int green1 = (pixel1 >> 8) & 0xff;

					int pixel2 = Img2
							.getRGB(Image2StartX + i, Image2StartY + j);
					int alpha2 = (pixel2 >> 24) & 0xff;
					int green2 = (pixel2 >> 8) & 0xff;

					if (alpha1 > 0 && alpha2 > 0 && green1 != 255 && green2 != 255)
					{
						return true;
					}
				}
			}
		}

		return false;
	}
}
