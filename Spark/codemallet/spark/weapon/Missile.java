package codemallet.spark.weapon;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.engines.GameResources;
import codemallet.spark.player.PlayerList;

public final class Missile
{
	private final Spark game;

	char Type;

	double X;

	double Y;

	double FreeX;

	double FreeY;

	double LastX;

	double LastY;

	double StartX;

	double StartY;

	double Angle;

	int Owner;

	int Team;

	float MaxLife;

	float Life;

	float Magnitude = 0.30f;

	float ReverseMagnitude = 0.08f;

	int SlowMove = 8;

	long TrailTimer;

	Missile(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		Life -= (Magnitude * delta);
		if (Life < 0)
		{
			return;
		}

		LastX = X;
		LastY = Y;
		for (int a = 1; a <= SlowMove; a++)
		{
			X += ((double) (Math.cos(Angle) * Magnitude) / SlowMove) * delta;
			Y += ((double) (Math.sin(Angle) * Magnitude) / SlowMove) * delta;

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

				checkCollision();

				Life = -1;

				break;
			}

			if (game.Tick > TrailTimer)
			{
				TrailTimer = game.Tick + 16;
				game.gameTrails.addTrail((int) X, (int) Y, Owner, Team, 0);
			}
		}
	}

	public final void Render()
	{
		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX) - 3;
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY) - 3;

		game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 4, DrawY + 4, 6, 586,
				6 + 4, 586 + 4);
	}

	private final boolean testCollision()
	{
		int TX = (int) (X / 16);
		int TY = (int) (Y / 16);

		if ((TX > 0) & (TY > 0) & (TX < 256) & (TY < 256))
		{
			for (int i = 0; i < PlayerList.MaxPlayers; i++)
			{
				if (game.gamePlayers.Players[i].inUse == true && i != Owner
						&& game.gamePlayers.Players[i].Dead == false
						&& game.gamePlayers.getTeam(i) >= 0)
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
					return true;
				}
			}
		}

		return false;
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
						if (game.gamePlayer.Index == i)
						{
							game.gameWeapons.missileHit(Owner, Team, (int) X,
									(int) Y);
						}
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
					game.gameSound.play3DSound(GameResources.sndHitMissile,
							(float) X, (float) Y);
					for (double i = -Math.PI; i < Math.PI; i += (Math.PI / 6))
					{
						game.gameShrapnel.addShrapnel((int) FreeX, (int) FreeY,
								Owner, Team, 2 * Math.PI - i, 0);
					}
					game.gameExplosions.addExplosion((int) (X - 10),
							(int) (Y - 10), 0);
					return true;
				}
			}
		}

		return false;
	}
}