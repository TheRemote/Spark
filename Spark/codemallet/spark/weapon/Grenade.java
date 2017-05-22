package codemallet.spark.weapon;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.engines.GameResources;

public final class Grenade
{
	private final Spark game;

	double X;

	double Y;

	double StartX;

	double StartY;

	double TargetX;

	double TargetY;

	double Angle;

	int Team;

	int TotalDistance;

	int TotalFrames;

	int Owner;

	int MaxLife;

	int Life;

	float Magnitude;

	long TrailTimer;

	Grenade(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		if (Life > -1)
		{
			if (StartX == TargetX && StartY == TargetY)
				Life = -1;

			X += (double) (Math.cos(Angle) * Magnitude) * delta;
			Y += (double) (Math.sin(Angle) * Magnitude) * delta;

			if (game.Tick > TrailTimer)
			{
				TrailTimer = game.Tick + 150;
				game.gameTrails.addTrail((int) X, (int) Y, Owner, Team, 1);
			}

			if (TargetX - StartX > 0)
			{
				if (TargetY - StartY > 0)
				{
					if (X > TargetX)
						Life = -1;
					if (Y > TargetY)
						Life = -1;
				}
				else
				{
					if (X > TargetX)
						Life = -1;
					if (Y < TargetY)
						Life = -1;
				}
			}
			else
			{
				if (TargetY - StartY > 0)
				{
					if (X < TargetX)
						Life = -1;
					if (Y > TargetY)
						Life = -1;
				}
				else
				{
					if (X < TargetX)
						Life = -1;
					if (Y < TargetY)
						Life = -1;
				}
			}

			if (Life == -1)
			{
				int mapx = (int) (X / 16);
				int mapy = (int) (Y / 16);

				if (mapx > 0 && mapy > 0 && mapx < game.gameMap.GameMap.width
						&& mapy < game.gameMap.GameMap.height)
				{
					int tile = game.gameMap.GameMap.MapData[(mapy * game.gameMap.GameMap.width)
							+ mapx];

					if ((tile & 0x8000) != 0) // Is it an animation?
					{
						char idx = (char) ((tile & 0x00FF));
						tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
					}

					if (GameCollision.checkNadeCollision(tile) == false)
					{
						int cy = tile / 40 * 16;
						int cx = tile % 40 * 16;

						if (tile != 280) // Purple blank tile
						{
							if (Collision.PixelPointWeaponCollision((int) X,
									(int) Y, game.imgCollision, mapx * 16,
									mapy * 16, 16, 16, cx, cy, 0) == null)
							{
								return;
							}
						}
						else
						{
							return;
						}
					}
				}
				else
					return;

				for (double i = -Math.PI; i < Math.PI; i += (Math.PI / 16))
				{
					game.gameShrapnel.addShrapnel((int) X, (int) Y, Owner,
							Team, 2 * Math.PI - i, 1);
				}
				game.gameExplosions.addExplosion((int) (X - 22),
						(int) (Y - 22), 1);
				game.gameSound.play3DSound(GameResources.sndHitGrenade,
						(float) X, (float) Y);
			}
		}
	}

	public final void Render()
	{
		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);

		int XDistance = (int) (X - TargetX);
		int YDistance = (int) (Y - TargetY);
		double Distance = Math.sqrt(Math.pow(XDistance, 2)
				+ Math.pow(YDistance, 2));

		if (Distance > (TotalDistance / 2))
			Distance = Math.abs(Distance - (TotalDistance));
		Distance = Distance * 2;

		if (TotalDistance == 0)
			TotalDistance = 1;
		int FinalDistance = (int) ((double) (TotalFrames * (Distance / TotalDistance)));

		int SrcX = FinalDistance * 16;
		int SrcY = 580;
		int Size = 16;

		DrawX -= 8;
		DrawY -= 8;

		game.gameEngine.drawInUse(DrawX, DrawY, DrawX + Size, DrawY + Size,
				SrcX, SrcY, SrcX + Size, SrcY + Size);
	}
}
