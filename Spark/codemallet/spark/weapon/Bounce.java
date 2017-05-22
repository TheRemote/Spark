package codemallet.spark.weapon;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.engines.GameResources;
import codemallet.spark.player.PlayerList;

public final class Bounce
{
	private final Spark game;

	char Type;

	double X;

	double Y;

	double FreeX;

	double FreeY;

	double StartX;

	double StartY;

	double EndX;

	double EndY;

	double Angle;

	int Owner;

	int Team;

	float MaxLife;

	float Life;

	float TrailLength;

	float MaxTrailLength = 85;

	float Magnitude = 0.52f;

	float ReverseMagnitude = 0.05f;

	int SlowMove = 8;

	float Red;

	float Green;

	float Blue;

	Bounce(Spark game)
	{
		this.game = game;

	}

	public final void Cycle(int delta)
	{
		if (EndX > 0 || EndY > 0)
		{
			X = EndX;
			Y = EndY;
			TrailLength -= (Magnitude * delta);
		}
		else
		{
			Life -= (Magnitude * delta);
			if (Life < 0)
			{
				EndX = X;
				EndY = Y;
				return;
			}

			for (int a = 1; a <= SlowMove; a++)
			{
				X += ((double) (Math.cos(Angle) * Magnitude) / SlowMove)
						* delta;
				Y += ((double) (Math.sin(Angle) * Magnitude) / SlowMove)
						* delta;

				if (testCollision() == true)
				{
					while (true)
					{

						X -= (double) (Math.cos(Angle) * ReverseMagnitude)
								* delta;
						Y -= (double) (Math.sin(Angle) * ReverseMagnitude)
								* delta;

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

					checkCollision(delta);
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

		float Brightness = 1.0f;

		if (EndX > 0 || EndY > 0)
		{
			Brightness = TrailLength / MaxTrailLength;
		}

		game.gameEngine.drawGradientLine(DrawX, DrawY, Red, Green, Blue,
				Brightness, DrawX2, DrawY2, Red, Green, Blue, 0);
	}

	protected final boolean testCollision()
	{
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

	private final boolean checkCollision(int delta)
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
							game.gameWeapons.bounceHit(Owner, Team, (int) X,
									(int) Y, (float) Angle);
						}
					}
				}
			}

			if (ReturnValue == true)
			{
				return ReturnValue;
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
					double NewAngle = 0;
					NewAngle = (Math.PI) - Angle;

					X = FreeX;
					Y = FreeY;
					X += ((double) (Math.cos(NewAngle) * Magnitude) / SlowMove)
							* delta;
					Y += ((double) (Math.sin(NewAngle) * Magnitude) / SlowMove)
							* delta;

					if (testCollision() == true)
					{
						NewAngle = (2 * Math.PI) - Angle;
					}

					if (NewAngle > (Math.PI))
					{
						NewAngle -= (2 * Math.PI);
					}

					game.gameBouncies.addBounce(FreeX, FreeY, NewAngle, Owner,
							(int) Life);
					game.gameSound.play3DSound(GameResources.sndBounce,
							(float) X, (float) Y);

					return true;
				}
			}
		}

		return false;
	}
}
