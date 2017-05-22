package codemallet.spark.weapon;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;

public final class Sparks
{
	private final Spark game;

	double X;

	double Y;

	double Angle;

	double Magnitude;

	long Time;

	int Red;

	int Green;

	int Blue;
	
	boolean Active;

	Sparks(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		X += (double) (Math.sin(Angle) * Magnitude) * delta;
		Y += (double) (Math.cos(Angle) * Magnitude) * delta;

		checkCollision();
	}

	public final void Render()
	{
		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);

		game.gameEngine.setColorRGBA(Red, Green, Blue, 255);
		game.gameEngine.fillRect(DrawX, DrawY, 1.0f, 1.0f);
	}

	private boolean checkCollision()
	{
		int TX = (int) (X / 16);
		int TY = (int) (Y / 16);

		if ((TX > 0) & (TY > 0) & (TX < 256) & (TY < 256))
		{
			int tile = game.gameMap.GameMap.MapData[(TY * game.gameMap.GameMap.width)
					+ TX];

			if ((tile & 0x8000) != 0) // Is it an animation?
			{
				char idx = (char) ((tile & 0x00FF));
				tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
			}
			if (GameCollision.checkWeaponCollision(tile, Angle) == true)
			{
				TX *= 16;
				TY *= 16;
				int cy = tile / 40 * 16;
				int cx = tile % 40 * 16;
				Rectangle intersect = Collision.PixelPointWeaponCollision(
						(int) X, (int) Y, game.imgCollision, TX, TY, 16, 16,
						cx, cy, 0);
				if (intersect != null)
				{
					Time = 0;
					return true;
				}
			}
		}
		return false;
	}
}
