package codemallet.spark.weapon;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.player.PlayerList;

public final class Shrapnel
{
	private final Spark game;
	
	boolean Active;

	double X;

	double Y;

	int Owner;

	int Team;

	int Type;

	float Life;

	double Angle;

	float Magnitude;

	Shrapnel(Spark game)
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

		X += (double) (Math.cos(Angle) * Magnitude) * delta;
		Y += (double) (Math.sin(Angle) * Magnitude) * delta;

		if (checkCollision() == true)
		{
			Life = -1;
			return;
		}
	}

	public final void Render()
	{

		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX) - 2;
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY) - 2;

		game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 5, DrawY + 5, 3, 756,
				3 + 5, 756 + 5);
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
						if (i == game.gamePlayer.Index)
						{
							switch (Type)
							{
								case 0:
									game.gameWeapons.missileShrapnelHit(Owner,
											Team);
									break;
								case 1:
									game.gameWeapons.grenadeHit(Owner, Team);
									break;
							}

						}
						ReturnValue = true;
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
					return true;
				}

			}
		}

		return false;
	}
}
