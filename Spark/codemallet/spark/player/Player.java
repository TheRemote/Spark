package codemallet.spark.player;

import static codemallet.spark.engines.GameResources.colorBlack;

import java.awt.Rectangle;

import codemallet.spark.Spark;
import codemallet.spark.clan.Clan;
import codemallet.spark.collision.Collision;
import codemallet.spark.collision.GameCollision;
import codemallet.spark.engines.GameEngine;
import codemallet.spark.engines.GameResources;
import codemallet.spark.map.Flag;

public final class Player
{
	private final Spark game;

	public String Name;

	public String Points;

	public int Index;

	public boolean inUse;

	public float X;

	public float Y;

	public int Team;

	public int Direction;

	public int MoveXInput;

	public int MoveYInput;

	public float MoveX;

	public float MoveY;

	public float DriftX;

	public float DriftY;

	public boolean Dead;

	public boolean HoldPen;

	public boolean Smoking;

	public long timeSmoke;

	public int SmokeAnimation;

	public int Ping;

	public boolean hasFlag;

	public int FlagIndex;

	public int Score;

	public int Kills;

	public int Deaths;

	public int WarpEffect;

	public float WarpStartX;

	public float WarpStartY;

	public long WarpTimer;

	public int Clan;

	public boolean Ignore;

	public int Health;

	public int Energy;

	public int blendRed = 255;

	public int blendGreen = 255;

	public int blendBlue = 255;

	public int blendAlpha = 255;

	public long lastUpdate;

	private static final float MoveSpeed = 0.0713f;

	private static final float ReducedSpeed = 0.0535f;

	private static final float Drift = 0.0535f;

	private static final String LagString = "(Lagging)";

	public Player(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		if (inUse == true && Dead == false)
		{
			if (Team >= 0)
			{
				if (Smoking == true && game.Tick > timeSmoke)
				{
					timeSmoke = game.Tick + 400;
					game.gameSmoke.addSmoke((int) X
							+ game.gameRandom.nextInt(25), (int) Y
							+ game.gameRandom.nextInt(25), 0, 0);

					SmokeAnimation = 1 - SmokeAnimation;
				}

				MoveX = 0;
				MoveY = 0;

				if (MoveXInput == 0 && MoveYInput == 0)
				{
					Direction = 8;
				}
				else
				{
					float CurrentMoveSpeed = 0;
					if (hasFlag == false)
						CurrentMoveSpeed = MoveSpeed;
					else
						CurrentMoveSpeed = ReducedSpeed;
					if (MoveYInput > 0)
					{
						MoveY = CurrentMoveSpeed * delta;
					}

					if (MoveYInput < 0)
					{
						MoveY -= CurrentMoveSpeed * delta;
					}

					if (MoveXInput > 0)
					{
						MoveX = CurrentMoveSpeed * delta;
					}

					if (MoveXInput < 0)
					{
						MoveX -= CurrentMoveSpeed * delta;
					}

					if (MoveX != 0 && MoveY != 0)
					{
						float CurrentReducedSpeed = ReducedSpeed;
						if (hasFlag == true)
							CurrentReducedSpeed = ReducedSpeed * 0.75f;
						if (MoveX < 0)
							MoveX = -CurrentReducedSpeed * delta;
						else
							MoveX = CurrentReducedSpeed * delta;
						if (MoveY < 0)
							MoveY = -CurrentReducedSpeed * delta;
						else
							MoveY = CurrentReducedSpeed * delta;
					}

					int MoveXDir = 0;
					int MoveYDir = 0;
					if (MoveX < 0)
						MoveXDir = -1;
					if (MoveX > 0)
						MoveXDir = 1;
					if (MoveY < 0)
						MoveYDir = -1;
					if (MoveY > 0)
						MoveYDir = 1;

					switch (MoveXDir)
					{
						case -1: // Left
						{
							switch (MoveYDir)
							{
								case -1: // Left + Up
									Direction = 3;
									break;
								case 0: // Left
									Direction = 4;
									break;
								case 1: // Left + Down
									Direction = 5;
									break;
							}
							break;
						}
						case 0: // Not moving
						{
							switch (MoveYDir)
							{
								case -1: // Up
									Direction = 2;
									break;
								case 0: // Not Moving
									break;
								case 1: // Down
									Direction = 6;
									break;
							}
							break;
						}
						case 1: // Right
						{
							switch (MoveYDir)
							{
								case -1: // Right + Up
									Direction = 1;
									break;
								case 0: // Right
									Direction = 0;
									break;
								case 1: // Right + Down
									Direction = 7;
									break;
							}
							break;
						}
					}
				}

				final float OldX = X;
				final float OldY = Y;

				DriftX = 0;
				DriftY = 0;

				checkDrift();
				if (hasFlag)
				{
					if (DriftX > 0 && MoveX < 0)
					{
						MoveX = 0;
						DriftX = 0;
					}
					else if (DriftX < 0 && MoveX > 0)
					{
						MoveX = 0;
						DriftX = 0;
					}
					if (DriftY > 0 && MoveY < 0)
					{
						MoveY = 0;
						DriftY = 0;
					}
					else if (DriftY < 0 && MoveY > 0)
					{
						MoveY = 0;
						DriftY = 0;
					}
				}
				final float OldMoveX = MoveX;
				final float OldMoveY = MoveY;

				MoveX += DriftX * delta;
				MoveY += DriftY * delta;

				X += MoveX;
				if (checkCollision() == true)
				{
					X = OldX;
				}

				Y += MoveY;
				if (checkCollision() == true)
				{
					Y = OldY;
				}

				MoveX = OldMoveX;
				MoveY = OldMoveY;

				tryToMove(delta, OldX, OldY);

				MoveX = OldMoveX;
				MoveY = OldMoveY;

				if (WarpEffect != 0)
				{
					if (game.Tick > WarpTimer)
					{
						if (WarpEffect > 1)
						{
							WarpEffect--;
						}
						else
						{
							if (WarpStartX != X || WarpStartY != Y)
								WarpEffect = 0;
						}
						WarpTimer = game.Tick + 30;
					}

				}

				if (X < 0)
					X = 0;
				if (Y < 0)
					Y = 0;
				if (X > game.gameMap.MapWidthPixels)
					X = game.gameMap.MapWidthPixels;
				if (Y > game.gameMap.MapHeightPixels)
					Y = game.gameMap.MapHeightPixels;

				if (game.gamePlayer.Index == Index)
				{
					game.gamePlayer.X = X;
					game.gamePlayer.Y = Y;
				}
			}
			else
			{
				if (Index == game.gamePlayer.Index)
				{
					MoveX = 0;
					MoveY = 0;

					if (MoveXInput == 0 && MoveYInput == 0)
					{

					}
					else
					{
						if (MoveYInput > 0)
						{
							MoveY = MoveSpeed * delta;
						}

						if (MoveYInput < 0)
						{
							MoveY -= MoveSpeed * delta;
						}

						if (MoveXInput > 0)
						{
							MoveX = MoveSpeed * delta;
						}

						if (MoveXInput < 0)
						{
							MoveX -= MoveSpeed * delta;
						}

						if (MoveX != 0 && MoveY != 0)
						{
							if (MoveX < 0)
								MoveX = -ReducedSpeed * delta;
							else
								MoveX = ReducedSpeed * delta;
							if (MoveY < 0)
								MoveY = -ReducedSpeed * delta;
							else
								MoveY = ReducedSpeed * delta;
						}
					}

					if (MoveX != 0 || MoveY != 0)
					{
						game.gamePlayer.SpectatingPlayer = -1;
						MoveX = MoveX * 10;
						MoveY = MoveY * 10;
						X += MoveX;
						Y += MoveY;
						if (X < 0)
							X = 0;
						if (Y < 0)
							Y = 0;
						if (X > game.gameMap.MapWidthPixels)
							X = game.gameMap.MapWidthPixels;
						if (Y > game.gameMap.MapHeightPixels)
							Y = game.gameMap.MapHeightPixels;
					}
					else
					{
						if (game.gamePlayer.SpectatingPlayer >= 0)
						{
							X = game.gamePlayers
									.getX(game.gamePlayer.SpectatingPlayer);
							Y = game.gamePlayers
									.getY(game.gamePlayer.SpectatingPlayer);
						}
					}

					game.gamePlayer.X = X;
					game.gamePlayer.Y = Y;
				}
			}
		}
	}

	public final void drawPlayer()
	{
		if (inUse == true && Dead == false)
		{
			if (Name != null)
			{
				if (blendRed != 255 || blendBlue != 255 || blendGreen != 255
						|| blendAlpha != 255)
				{
					game.gameEngine.setColorRGBA(blendRed, blendBlue,
							blendGreen, blendAlpha);
					game.gameEngine.setColorBlending(true);
				}

				int tX = (int) ((int) X - (int) game.gamePlayer.X + game.gamePlayer.OffsetX);
				int tY = (int) ((int) Y - (int) game.gamePlayer.Y + game.gamePlayer.OffsetY);

				if (WarpEffect == 0)
				{
					game.gameEngine.drawInUse(tX, tY, tX + 32, tY + 32,
							(Direction * 32), 292 + (Team * 32),
							(Direction * 32) + 32, 292 + (Team * 32) + 32);
				}

				else
				{
					int Difference = 32 - WarpEffect;
					int Subtract = 0;
					if (Difference > 0)
						Subtract = Difference / 2;
					game.gameEngine.drawInUse(tX + Subtract, tY + Subtract, tX
							+ 32 - Subtract, tY + 32 - Subtract,
							(Direction * 32), 292 + (Team * 32),
							(Direction * 32) + 32, 292 + (Team * 32) + 32);
				}

				if (hasFlag == true)
				{
					Flag flag = (Flag) game.gameMap.Flags.get(FlagIndex);
					game.gameEngine.drawInUse(tX + 15, tY + 5, tX + 15 + 16,
							tY + 5 + 16, 288, 311 + flag.Team * 32, 288 + 16,
							311 + flag.Team * 32 + 16);
				}

				if (blendRed != 255 || blendBlue != 255 || blendGreen != 255
						|| blendAlpha != 255)
					game.gameEngine.setColorBlending(false);
			}
		}
	}

	public final void drawPlayerName()
	{
		if (inUse == true && Dead == false)
		{
			if (Name != null)
			{
				int tX = (int) ((int) X - (int) game.gamePlayer.X + game.gamePlayer.OffsetX);
				int tY = (int) ((int) Y - (int) game.gamePlayer.Y + game.gamePlayer.OffsetY);

				Points = " [" + Score + "]";

				String DrawName = Name;

				if (Clan > -1)
				{
					if (game.gameClans.inUse(Clan) == true)
					{
						Clan theClan = game.gameClans.getClan(Clan);
						if (theClan.TagAfterName == true)
						{
							if (theClan.TagSpaceBetween == true)
							{
								DrawName = DrawName + " " + theClan.Tag;
							}
							else
							{
								DrawName = DrawName + theClan.Tag;
							}
						}
						else
						{
							if (theClan.TagSpaceBetween == true)
							{
								DrawName = theClan.Tag + " " + DrawName;
							}
							else
							{
								DrawName = theClan.Tag + DrawName;
							}
						}
					}
				}

				int drawx = tX + 16;
				int drawy = tY + 34;
				int NameLength = game.gameEngine.getStringWidth(DrawName);
				int PointsLength = game.gameEngine.getStringWidth(Points);
				int TotalLength = (NameLength + PointsLength) / 2;
				drawx -= TotalLength;

				if (game.gameOptions.fontNameShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString(DrawName, drawx + 1, drawy + 1);
				}
				game.gameEngine.setColor(GameResources.colorName);
				game.gameEngine.drawString(DrawName, drawx, drawy);
				drawx += NameLength;
				if (game.gameOptions.fontNameShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString(Points, drawx + 1, drawy + 1);
				}
				game.gameEngine.setColor(GameResources.colorPoints);
				game.gameEngine.drawString(Points, drawx, drawy);

				if (Index != game.gamePlayer.Index
						&& game.Tick - lastUpdate > 2000)
				{
					int LagLength = game.gameEngine.getStringWidth(LagString);
					TotalLength = LagLength / 2;
					drawx = tX + 16 - TotalLength;
					drawy = drawy + game.gameEngine.getFontHeight() + 1;
					if (game.gameOptions.fontNameShadow == true)
					{
						game.gameEngine.setColor(GameResources.colorBlack);
						game.gameEngine.drawString(LagString, drawx + 1,
								drawy + 1);
					}
					game.gameEngine.setColor(GameResources.colorRed);
					game.gameEngine.drawString(LagString, drawx, drawy);
				}
			}
		}
	}

	public final void drawRadar()
	{
		if (inUse == true)
		{
			if (Name != null && Dead == false && HoldPen == false)
			{
				int tX = (int) (X - game.gamePlayer.X);
				int tY = (int) (Y - game.gamePlayer.Y);

				int RadarX = (int) (game.ResolutionX - 89 + ((tX * 5) / 100)) + 1;
				int RadarY = (int) (66 + ((tY * 5) / 100)) - 4;

				if (RadarX > game.ResolutionX - 140
						&& RadarX < game.ResolutionX - 35 && RadarY > 12
						&& RadarY < 112)
				{

					game.gameEngine.setColor(game.gameChat.getColor(Team));
					game.gameEngine.fillOval(RadarX, RadarY, 6, 6);
					game.gameEngine.setColor(colorBlack);
					game.gameEngine.drawOval(RadarX, RadarY, 6, 6);
					if (hasFlag == true && game.gameRadar.Blink == true)
					{
						Flag flag = game.gameMap.Flags.get(FlagIndex);
						game.gameEngine.setColor(game.gameChat
								.getColor(flag.Team));
						game.gameEngine.drawOval(RadarX - 1, RadarY - 1, 8, 8);
					}

					if (hasFlag == true)
					{
						Flag flag = game.gameMap.Flags.get(FlagIndex);

						int StartX = (int) (X - game.gamePlayer.X);
						int StartY = (int) (Y - game.gamePlayer.Y);

						RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 5;
						RadarY = (int) (66 + ((StartY * 5) / 100)) - 6;

						game.gameEngine.drawImage(GameResources.imgTuna,
								RadarX, RadarY, RadarX + 8, RadarY + 8, 288,
								311 + flag.Team * 32, 288 + 16,
								311 + flag.Team * 32 + 16);
					}
				}
			}
		}
	}

	public final void checkDrift()
	{
		if (Team < 0)
			return;

		for (int i = -1; i < 3; i++)
		{
			for (int j = -1; j < 3; j++)
			{
				int TX = (int) (X / 16) + i;
				int TY = (int) (Y / 16) + j;
				if (TX > 0 && TY > 0 && TX < game.gameMap.GameMap.width
						&& TY < game.gameMap.GameMap.height)
				{
					int tile = game.gameMap.GameMap.MapData[(TY * game.gameMap.GameMap.width)
							+ TX];

					if ((tile & 0x8000) != 0)
					{
						char idx = (char) ((tile & 0x00FF));
						switch (idx)
						{
							// Down
							case 148:
							case 149:
							case 150:
							case 151:
							case 152:
							case 153:
							{
								tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
								TX *= 16;
								TY *= 16;
								int cy = tile / 40 * 16;
								int cx = tile % 40 * 16;
								Rectangle intersect = Collision
										.PixelPointCollision((int) X + 15,
												(int) Y + 15,
												game.imgCollision, TX, TY, 16,
												16, cx, cy);
								if (intersect != null)
								{
									DriftY = Drift;
									return;
								}
							}
								break;
							// Up
							case 167:
							case 168:
							case 169:
							case 170:
							case 171:
							case 172:
							{
								tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
								TX *= 16;
								TY *= 16;
								int cy = tile / 40 * 16;
								int cx = tile % 40 * 16;
								Rectangle intersect = Collision
										.PixelPointCollision((int) X + 15,
												(int) Y + 15,
												game.imgCollision, TX, TY, 16,
												16, cx, cy);
								if (intersect != null)
								{
									DriftY = -Drift;
									return;
								}
							}
								break;
							// Left
							case 177:
							case 178:
							case 179:
							case 180:
							case 181:
							case 182:
							{
								tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
								TX *= 16;
								TY *= 16;
								int cy = tile / 40 * 16;
								int cx = tile % 40 * 16;
								Rectangle intersect = Collision
										.PixelPointCollision((int) X + 15,
												(int) Y + 15,
												game.imgCollision, TX, TY, 16,
												16, cx, cy);
								if (intersect != null)
								{
									DriftX = -Drift;
									return;
								}
							}
								break;
							// Right
							case 183:
							case 184:
							case 185:
							case 186:
							case 187:
							case 188:
							{
								tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
								TX *= 16;
								TY *= 16;
								int cy = tile / 40 * 16;
								int cx = tile % 40 * 16;
								Rectangle intersect = Collision
										.PixelPointCollision((int) X + 15,
												(int) Y + 15,
												game.imgCollision, TX, TY, 16,
												16, cx, cy);
								if (intersect != null)
								{
									DriftX = Drift;
									return;
								}
							}
								break;
						}
					}
				}
			}
		}
	}

	public final boolean checkCollision()
	{
		if (Team < 0)
			return false;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int TX = (int) (X / 16) + i;
				int TY = (int) (Y / 16) + j;
				if (TX > 0 && TY > 0 && TX <= 256 && TY <= 256)
				{
					int tile = game.gameMap.GameMap.MapData[(TY * game.gameMap.GameMap.width)
							+ TX];

					if ((tile & 0x8000) != 0) // Is it an animation?
					{
						char idx = (char) ((tile & 0x00FF));
						tile = game.gameMap.anim_list[idx].frames[game.gameMap.anim_list[idx].Frame];
					}

					if (GameCollision.TileTankCollision[tile] == true)
					{
						TX *= 16;
						TY *= 16;
						int cy = tile / 40 * 16;
						int cx = tile % 40 * 16;
						if (Collision.PixelTankCollision(game.imgShipCollision,
								(int) X, (int) Y, 31, 31, 8 * 32, 0, 1,
								game.imgCollision, TX, TY, 16, 16, cx, cy, 1))
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	public final void tryToMove(int delta, float OldX, float OldY)
	{
		float MoveAxisTest = ReducedSpeed * delta;
		float OriginalMoveSpeed = ReducedSpeed * delta;

		if ((MoveX != 0 || MoveY != 0) && X == OldX && Y == OldY)
		{
			int Tries = 0;
			float OriginalX = OldX;
			float OriginalY = OldY;
			float OriginalMoveX = MoveX;
			float OriginalMoveY = MoveY;
			while (true)
			{
				if (Tries > GameEngine.MaxDelta)
				{
					break;
				}

				MoveX = OriginalMoveX;
				MoveY = OriginalMoveY;
				OldX = OriginalX;
				OldY = OriginalY;

				if (MoveX != 0 && MoveY == 0)
				{
					if (tryX(MoveAxisTest, OriginalMoveSpeed, OldX, OldY) == true)
						break;
				}
				else if (MoveY != 0 && MoveX == 0)
				{
					if (tryY(MoveAxisTest, OriginalMoveSpeed, OldX, OldY) == true)
						break;
				}
				else
				{

				}

				Tries = Tries + delta;
				MoveAxisTest = ReducedSpeed * Tries;
			}
		}

	}

	public final boolean tryX(float MoveAxisTest, float OriginalMoveSpeed,
			float OldX, float OldY)
	{
		if (MoveX > 0)
			MoveX = MoveAxisTest;
		else
			MoveX = -MoveAxisTest;

		MoveY = MoveAxisTest;
		Y += MoveY;
		if (checkCollision() == true)
		{
			Y = OldY;
		}
		else
		{
			if (MoveX > 0)
				X += OriginalMoveSpeed;
			else
				X -= OriginalMoveSpeed;
			if (checkCollision() == true)
			{
				X = OldX;
				Y = OldY;
			}
			else
			{
				Y = OldY;
				Y += OriginalMoveSpeed;

				X = OldX;
				if (MoveX > 0)
					X += OriginalMoveSpeed;
				else
					X -= OriginalMoveSpeed;
				if (checkCollision() == true)
					X = OldX;
				return true;
			}
		}

		MoveY = -MoveAxisTest;
		Y += MoveY;
		if (checkCollision() == true)
		{
			Y = OldY;
		}
		else
		{
			if (MoveX > 0)
				X += OriginalMoveSpeed;
			else
				X -= OriginalMoveSpeed;
			if (checkCollision() == true)
			{
				X = OldX;
				Y = OldY;
			}
			else
			{
				Y = OldY;
				Y -= OriginalMoveSpeed;

				X = OldX;
				if (MoveX > 0)
					X += OriginalMoveSpeed;
				else
					X -= OriginalMoveSpeed;
				if (checkCollision() == true)
					X = OldX;
				return true;
			}
		}

		return false;
	}

	public final boolean tryY(float MoveAxisTest, float OriginalMoveSpeed,
			float OldX, float OldY)
	{
		if (MoveY > 0)
			MoveY = MoveAxisTest;
		else
			MoveY = -MoveAxisTest;

		MoveX = MoveAxisTest;
		X += MoveX;
		if (checkCollision() == true)
		{
			X = OldX;
		}
		else
		{
			if (MoveY > 0)
				Y += OriginalMoveSpeed;
			else
				Y -= OriginalMoveSpeed;
			if (checkCollision() == true)
			{
				X = OldX;
				Y = OldY;
			}
			else
			{
				X = OldX;
				X += OriginalMoveSpeed;

				Y = OldY;
				if (MoveY > 0)
					Y += OriginalMoveSpeed;
				else
					Y -= OriginalMoveSpeed;
				if (checkCollision() == true)
					Y = OldY;
				return true;
			}
		}

		MoveX = -MoveAxisTest;
		X += MoveX;
		if (checkCollision() == true)
		{
			X = OldX;
		}
		else
		{
			if (MoveY > 0)
				Y += OriginalMoveSpeed;
			else
				Y -= OriginalMoveSpeed;
			if (checkCollision() == true)
			{
				X = OldX;
				Y = OldY;
			}
			else
			{
				X = OldX;
				X -= OriginalMoveSpeed;

				Y = OldY;

				Y = OldY;
				if (MoveY > 0)
					Y += OriginalMoveSpeed;
				else
					Y -= OriginalMoveSpeed;
				if (checkCollision() == true)
					Y = OldY;
				return true;
			}
		}

		return false;
	}
}
