package codemallet.spark.weapon;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.engines.GameResources;
import codemallet.spark.player.PlayerList;

public final class Laser
{
	private final Spark game;
	
	boolean Active;

	char Type;

	double X;

	double Y;

	double FreeX;

	double FreeY;

	double LastX;

	double LastY;

	double StartX;

	double StartY;

	double EndX;

	double EndY;

	double Angle;

	int Owner;

	int Team;

	float MaxLife;

	float Life;

	float TrailLength = 50;

	float Magnitude = 0.41f;

	int SlowMove = 5;

	float ReverseMagnitude = 0.08f;

	float Red;

	float Green;

	float Blue;

	Laser(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		Life -= (Magnitude * delta);
		if (Life < 0)
		{
			EndX = X;
			EndY = Y;
		}

		if (X == StartX && Y == StartY)
		{
			if (checkCollision() == true)
			{
				EndX = X;
				EndY = Y;
			}
		}

		if (EndX > 0 || EndY > 0)
		{
			X = EndX;
			Y = EndY;
			if (MaxLife - Life < TrailLength)
			{
				TrailLength = MaxLife - Life;
				Life = 0;
			}
			TrailLength -= (Magnitude * delta);
		}

		else
		{
			LastX = X;
			LastY = Y;
			for (int a = 1; a <= SlowMove; a++)
			{
				X += (double) ((Math.cos(Angle) * Magnitude) / SlowMove)
						* delta;
				Y += (double) ((Math.sin(Angle) * Magnitude) / SlowMove)
						* delta;

				if (testCollision() == true)
				{
					while (true)
					{
						if (LastX != StartX || LastY != StartY)
						{
							X -= (double) (Math.cos(Angle) * ReverseMagnitude)
									* delta;
							Y -= (double) (Math.sin(Angle) * ReverseMagnitude)
									* delta;
						}
						else
						{
							break;
						}

						if (testCollision() == false)
						{
							FreeX = X;
							FreeY = Y;
							X += (double) (Math.cos(Angle) * ReverseMagnitude)
									* delta;
							Y += (double) (Math.sin(Angle) * ReverseMagnitude)
									* delta;
							break;
						}
					}

					EndX = FreeX;
					EndY = FreeY;

					checkCollision();

					break;
				}
			}
		}
	}

	public final void Render()
	{
		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);

		int DrawX2 = 0;
		int DrawY2 = 0;

		if (Life >= (MaxLife - TrailLength))
		{
			DrawX2 = (int) StartX
					- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
			DrawY2 = (int) StartY
					- (int) (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);
		}
		else
		{
			DrawX2 = (int) X
					- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX)
					- (int) (Math.cos(Angle) * TrailLength);
			DrawY2 = (int) Y
					- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY)
					- (int) (Math.sin(Angle) * TrailLength);
		}

		game.gameEngine.drawGradientLine(DrawX, DrawY, Red, Green, Blue, 1.0f,
				DrawX2, DrawY2, Red, Green, Blue, 0.0f);
	}

	private final boolean checkCollision()
	{
		boolean ReturnValue = false;

		int TX = (int) (X / 16);
		int TY = (int) (Y / 16);

		if ((TX > 0) & (TY > 0) & (TX < 256) & (TY < 256))
		{
			for (int i = 0; i < PlayerList.MaxPlayers; i++)
			{
				if (game.gamePlayers.Players[i].inUse == true && i != Owner
						&& game.gamePlayers.Players[i].Dead == false
						&& game.gamePlayers.Players[i].Team >= 0)
				{
					Rectangle intersect = Collision.PixelPointWeaponCollision(
							(int) X, (int) Y, game.imgShipCollision,
							(int) game.gamePlayers.getX(i),
							(int) game.gamePlayers.getY(i), 31, 31, 8 * 32, 0,
							Weapons.Tolerance);
					if (intersect != null)
					{
						if (ReturnValue == false)
						{
							ReturnValue = true;
						}
						if (i == game.gamePlayer.Index)
							game.gameWeapons.laserHit(Owner, Team, (int) X,
									(int) Y, (float) Angle);
					}
				}
			}

			if (ReturnValue == true)
				return ReturnValue;

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
					game.gameSound.play3DSound(GameResources.sndSpark,
							(float) EndX, (float) EndY);
					for (int i = 0; i < 30; i++)
					{
						game.gameSparks.addSpark(LastX, LastY, Angle);
					}
					return true;
				}
			}
		}

		return false;
	}

	private final boolean testCollision()
	{
		final int TX = (int) (X / 16);
		final int TY = (int) (Y / 16);

		if ((TX > 0) & (TY > 0) & (TX < 256) & (TY < 256))
		{
			for (int i = 0; i < PlayerList.MaxPlayers; i++)
			{
				if (game.gamePlayers.Players[i].inUse == true && i != Owner
						&& game.gamePlayers.Players[i].Dead == false
						&& game.gamePlayers.Players[i].Team >= 0)
				{
					Rectangle intersect = Collision.PixelPointWeaponCollision(
							(int) X, (int) Y, game.imgShipCollision,
							(int) game.gamePlayers.getX(i),
							(int) game.gamePlayers.getY(i), 31, 31, 8 * 32, 0,
							Weapons.Tolerance);
					if (intersect != null)
					{
						return true;
					}
				}
			}

			int tile = game.gameMap.GameMap.MapData[(TY * game.gameMap.GameMap.width)
					+ TX];

			if ((tile & 0x8000) != 0) // Is it an animation?
			{
				final char idx = (char) ((tile & 0x00FF));
				tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
			}

			if (GameCollision.checkWeaponCollision(tile, Angle) == true)
			{
				final int TXO = TX * 16;
				final int TYO = TY * 16;
				final int cy = tile / 40 * 16;
				final int cx = tile % 40 * 16;
				Rectangle intersect = Collision.PixelPointWeaponCollision(
						(int) X, (int) Y, game.imgCollision, TXO, TYO, 16, 16,
						cx, cy, 0);
				if (intersect != null)
				{
					return true;
				}
			}
		}

		return false;
	}
}
