package codemallet.spark.server.player;

import java.nio.channels.ClosedChannelException;

import org.xsocket.connection.INonBlockingConnection;
import org.xsocket.datagram.UserDatagram;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.map.Flag;
import codemallet.spark.server.map.FlagPole;
import codemallet.spark.server.map.MapSpawn;
import codemallet.spark.server.map.Switch;
import codemallet.spark.server.map.Warp;
import codemallet.spark.server.net.NetCode;


public final class Player
{
	private final SparkServer server;

	public String IPAddress;

	public INonBlockingConnection connection;

	public String Name;

	public int Index;

	public float X;

	public float Y;

	public int Direction;

	public int Team;

	public boolean inUse;

	public boolean Verified;

	public int State;

	public long timeDead;

	public long timeSpawn;

	public long timeSendRecharge;

	public long timeUDPAck;

	public long timeUDPSendAck;

	public int MissileCharges;

	public int GrenadeCharges;

	public int BouncyCharges;

	public int MissileRecharge;

	public int GrenadeRecharge;

	public int BouncyRecharge;

	public int MaxMissileRecharge = 21;

	public int MaxGrenadeRecharge = 21;

	public int MaxBouncyRecharge = 21;

	public long timeMissile;

	public long timeGrenade;

	public long timeBouncy;

	public int Score;

	public boolean hasFlag = false;

	public int FlagIndex;

	public boolean MovedInPen = false;

	public UserDatagram packet = null;

	public boolean EstablishedUDP;

	public boolean DisableUDP;

	public long SendCounter;

	public long ReceiveCounter;

	public long GoodPackets;

	public long timeWarp;

	public long timeRewarp;

	public long timeTeamSwitch;

	public long timeCorrectPosition;

	public long timeMaxClickSpeed;

	public int movementCorrectionStrikes;

	public int WarpIndex;

	public int Health;

	public int Energy;

	public long timestampLastTick;

	public long timestampLastEpoch;

	public int TotalDamageTaken;

	public int TotalHitsTaken;

	public int TotalDamageGiven;

	public int TotalHitsGiven;

	public int Clan;

	public Player(SparkServer server, INonBlockingConnection connection)
	{
		this.server = server;
		this.connection = connection;
	}

	public final void Cycle()
	{
		try
		{
			if (inUse == false || State != 2)
				return;

			if (EstablishedUDP == true && DisableUDP == false
					&& server.Tick > timeUDPSendAck)
			{
				server.players.sendUDPAck(Index);
				timeUDPSendAck = server.Tick + 5000;
			}

			if (timeDead > 0)
			{
				if (server.Tick > timeDead)
				{
					server.players.holdPen(Index);
				}
			}

			if (timeSpawn > 0 && MovedInPen == true)
			{
				if (server.Tick > timeSpawn)
				{
					if (server.engine.Locked == false)
					{
						checkForSpawnTiles();
					}
				}
			}

			if (timeDead == 0 && timeSpawn == 0)
			{
				if (MissileCharges < 3)
				{
					if (server.Tick > timeMissile)
					{
						timeMissile = server.Tick
								+ server.map.GameMap.MissileTime;
						MissileRecharge++;
						if (MissileRecharge > MaxMissileRecharge
								|| server.map.GameMap.MissileTime == 0)
						{
							MissileRecharge = 0;
							MissileCharges++;
							server.players.sendTo(Index, "addCharge " + 0);
							timeSendRecharge = 0;
						}
					}
				}
				if (GrenadeCharges < 2)
				{
					if (server.Tick > timeGrenade)
					{
						timeGrenade = server.Tick
								+ server.map.GameMap.GrenadeTime;
						GrenadeRecharge++;
						if (GrenadeRecharge > MaxGrenadeRecharge
								|| server.map.GameMap.GrenadeTime == 0)
						{
							GrenadeRecharge = 0;
							GrenadeCharges++;
							server.players.sendTo(Index, "addCharge " + 1);
							timeSendRecharge = 0;
						}
					}
				}
				if (BouncyCharges < 3)
				{
					if (server.Tick > timeBouncy)
					{
						timeBouncy = server.Tick
								+ server.map.GameMap.BouncyTime;
						BouncyRecharge++;
						if (BouncyRecharge > MaxBouncyRecharge
								|| server.map.GameMap.BouncyTime == 0)
						{
							BouncyRecharge = 0;
							BouncyCharges++;
							server.players.sendTo(Index, "addCharge " + 2);
							timeSendRecharge = 0;
						}
					}
				}
				if (server.Tick > timeSendRecharge)
				{
					server.players.sendTo(Index, "recharge " + MissileRecharge
							+ " " + GrenadeRecharge + " " + BouncyRecharge);
					timeSendRecharge = server.Tick + 1000;
				}
				if (timeWarp > 0)
				{
					if (server.Tick > timeWarp)
					{
						timeWarp = 0;
						Warp warp = server.map.Warps.get(WarpIndex);
						X = (warp.X * 16 - 8);
						Y = (warp.Y * 16 - 8);
						server.players.sendTo(Index, "move " + (int) X + " "
								+ (int) Y);
					}
				}
				else
				{
					if (timeDead == 0 && Team >= 0)
					{
						checkForFlags();
						checkForSwitches();
						checkForSpawnTiles();
						if (hasFlag == false)
							checkForWarps();
					}
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private final void checkForWarps()
	{
		int MyX = (int) ((X + 16) / 16);
		int MyY = (int) ((Y + 16) / 16);
		boolean Warp = false;
		int WarpDestination = 0;
		int WarpX = 0;
		int WarpY = 0;
		for (int i = 0; i < server.map.Warps.size(); i++)
		{
			Warp warp = server.map.Warps.get(i);
			if (warp.X == MyX && warp.Y == MyY && server.Tick > timeRewarp)
			{
				WarpIndex = i;
				WarpDestination = warp.Destination;
				WarpX = warp.X;
				WarpY = warp.Y;
				Warp = true;
				break;
			}
		}

		if (Warp == true)
		{
			int DestCount = 0;
			for (int i = 0; i < server.map.Warps.size(); i++)
			{
				Warp warp = server.map.Warps.get(i);
				if (warp.Address == WarpDestination && i != WarpIndex)
				{
					DestCount++;
				}
			}
			DestCount = server.rand.nextInt(DestCount);
			for (int i = 0; i < server.map.Warps.size(); i++)
			{
				Warp warp = server.map.Warps.get(i);
				if (warp.Address == WarpDestination && i != WarpIndex)
				{
					if (DestCount == 0)
					{
						WarpIndex = i;
						timeWarp = server.Tick + 1000;
						timeRewarp = server.Tick + 5000;
						server.players.sendGame(-1, "warp " + Index + " "
								+ WarpX + " " + WarpY);
						break;
					}
					else
					{
						DestCount--;
					}
				}
			}
		}
	}

	private final void checkForFlags()
	{
		boolean CheckForWin = false;

		int MyX = (int) ((X + 16) / 16);
		int MyY = (int) ((Y + 16) / 16);

		for (int i = 0; i < server.map.FlagPoles.size(); i++)
		{
			FlagPole pole = (FlagPole) server.map.FlagPoles.get(i);
			if (hasFlag == false)
			{
				if (pole.hasFlag == true && pole.Team != Team && pole.X == MyX
						&& pole.Y == MyY)
				{
					if (pole.FlagType == Team)
					{
						for (int j = 0; j < server.map.FlagPoles.size(); j++)
						{
							FlagPole pole2 = server.map.FlagPoles.get(j);
							if (pole2.hasFlag == false && pole2.Team == Team
									&& pole2.FlagType == Team)
							{
								pole2.hasFlag = true;
								pole2.FlagIndex = pole.FlagIndex;

								pole.hasFlag = false;

								Flag flag = server.map.Flags
										.get(pole2.FlagIndex);
								flag.PoleIndex = j;
								flag.onPole = true;
								flag.onPlayer = false;
								flag.onGround = false;

								CheckForWin = true;

								server.players.sendGame(-1, "flag "
										+ pole2.FlagIndex + " " + 7 + " "
										+ Index + " " + j + " " + i);
								break;
							}
						}
					}
					else
					{
						pole.hasFlag = false;
						hasFlag = true;
						FlagIndex = (char) pole.FlagIndex;
						Flag flag = server.map.Flags.get(FlagIndex);
						flag.onGround = false;
						flag.onPole = false;
						flag.onPlayer = true;
						flag.PlayerIndex = Index;
						server.players.sendGame(-1, "flag " + FlagIndex + " "
								+ 0 + " " + Index + " " + i);
						break;
					}
				}
			}
			else
			{
				Flag flag = server.map.Flags.get(FlagIndex);
				if (pole.hasFlag == false && pole.Team == Team
						&& pole.FlagType == flag.Team && pole.X == MyX
						&& pole.Y == MyY)
				{
					pole.hasFlag = true;
					hasFlag = false;
					pole.FlagIndex = FlagIndex;
					flag.onGround = false;
					flag.onPole = true;
					flag.onPlayer = false;
					flag.PoleIndex = i;
					server.players.sendGame(-1, "flag " + FlagIndex + " " + 1
							+ " " + Index + " " + i);
					CheckForWin = true;
					break;
				}
			}
		}

		if (CheckForWin == true)
		{
			checkForWin();
		}
		else
		{
			if (hasFlag == false)
			{
				for (int i = 0; i < server.map.Flags.size(); i++)
				{
					Flag flag = server.map.Flags.get(i);
					if (flag.onGround == true)
					{
						int FlagDistanceX = Math.abs((int) (X + 16 - flag.X));
						int FlagDistanceY = Math.abs((int) (Y + 16 - flag.Y));
						if (flag.Team == Team)
						{
							if (FlagDistanceX < 16 && FlagDistanceY < 16)
							{
								for (int j = 0; j < server.map.FlagPoles.size(); j++)
								{
									FlagPole pole = server.map.FlagPoles.get(j);
									if (pole.hasFlag == false
											&& pole.Team == Team
											&& pole.FlagType == Team)
									{
										pole.hasFlag = true;
										pole.FlagIndex = i;

										flag.onPole = true;
										flag.onGround = false;
										flag.onPlayer = false;
										flag.PoleIndex = j;

										CheckForWin = true;

										server.players.sendGame(-1, "flag " + i
												+ " " + 9 + " " + Index + " "
												+ j);
										break;
									}
								}
							}
						}
						else if (FlagDistanceX < 16 && FlagDistanceY < 16)
						{
							if (flag.lastTeam == Team)
							{
								if (server.Tick > flag.timeDrop)
								{
									flag.onGround = false;
									flag.onPlayer = true;
									flag.onPole = false;
									flag.PlayerIndex = Index;
									hasFlag = true;
									FlagIndex = (char) i;
									server.players.sendGame(-1, "flag " + i
											+ " " + 6 + " " + Index);
									break;
								}
							}
							else
							{
								flag.onGround = false;
								flag.onPlayer = true;
								flag.onPole = false;
								flag.PlayerIndex = Index;
								hasFlag = true;
								FlagIndex = (char) i;
								server.players.sendGame(-1, "flag " + i + " "
										+ 6 + " " + Index);
								break;
							}
						}
					}
				}
			}
		}

		if (CheckForWin == true)
		{
			checkForWin();
		}
	}

	private final void checkForWin()
	{
		boolean Won = true;

		for (int i = 0; i < server.map.Flags.size(); i++)
		{
			Flag flag = (Flag) server.map.Flags.get(i);
			if (flag.onPole == true)
			{
				if (server.map.FlagPoles.get(flag.PoleIndex).Team == Team)
				{

				}
				else
				{
					Won = false;
					break;
				}
			}
			else
			{
				Won = false;
				break;
			}
		}

		for (int i = 0; i < server.map.Switches.size(); i++)
		{
			Switch Switch = server.map.Switches.get(i);
			if (Switch.Team == Team)
			{

			}
			else
			{
				Won = false;
				break;
			}
		}

		if (Won == true)
		{
			Score++;
			server.players.Won(Team);
		}
	}

	private final void checkForSwitches()
	{
		boolean CheckForWin = false;

		int MyX = (int) ((X + 16) / 16);
		int MyY = (int) ((Y + 16) / 16);
		for (int i = 0; i < server.map.Switches.size(); i++)
		{
			Switch Switch = server.map.Switches.get(i);
			if (server.Tick > Switch.timeFlipWait)
			{
				int SwitchDistanceX = Math.abs(MyX - Switch.X);
				int SwitchDistanceY = Math.abs(MyY - Switch.Y);
				if (SwitchDistanceX <= 1 && SwitchDistanceY <= 1
						&& Switch.Team != Team)
				{
					Switch.timeFlipWait = server.Tick + 1000;
					Switch.Team = Team;
					server.players.sendGame(-1, "switch " + i + " " + 0 + " "
							+ Index);
					CheckForWin = true;
					break;
				}
			}
		}

		if (CheckForWin == true)
			checkForWin();
	}

	private final void checkForSpawnTiles()
	{
		int MyX = (int) ((X + 16) / 16);
		int MyY = (int) ((Y + 16) / 16);

		if (MyX > 0 && MyY > 0 && MyX < server.map.GameMap.width
				&& MyY < server.map.GameMap.height)
		{
			int tile = server.map.GameMap.MapData[(MyY * server.map.GameMap.height)
					+ MyX];

			if ((tile & 0x8000) != 0) // Is tile animation?
			{
				tile = (char) ((tile & 0x00FF));
				switch (tile)
				{
					case 139:
					case 141:
					case 142:
					case 143:
					case 144:
						int randomspawn = 0;
						MapSpawn spawn = null;
						switch (Team)
						{
							case 0:
								randomspawn = server.rand
										.nextInt(server.map.spawnGreen.size());
								spawn = server.map.spawnGreen.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 1:
								randomspawn = server.rand
										.nextInt(server.map.spawnRed.size());
								spawn = server.map.spawnRed.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 2:
								randomspawn = server.rand
										.nextInt(server.map.spawnBlue.size());
								spawn = server.map.spawnBlue.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 3:
								randomspawn = server.rand
										.nextInt(server.map.spawnYellow.size());
								spawn = server.map.spawnYellow.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
						}
						if (server.engine.Locked == false)
						{
							timeSpawn = 0;
							server.players.sendGame(-1, "spawn " + Index);
							server.players.sendTo(Index, "move " + (int) X
									+ " " + (int) Y);
						}
						break;
				}
			}
			else
			{
				switch (tile)
				{
					case 146:
					case 186:
					case 276:
					case 316:
					case 320:
						int randomspawn = 0;
						MapSpawn spawn = null;
						switch (Team)
						{
							case 0:
								randomspawn = server.rand
										.nextInt(server.map.spawnGreen.size());
								spawn = server.map.spawnGreen.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 1:
								randomspawn = server.rand
										.nextInt(server.map.spawnRed.size());
								spawn = server.map.spawnRed.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 2:
								randomspawn = server.rand
										.nextInt(server.map.spawnBlue.size());
								spawn = server.map.spawnBlue.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
							case 3:
								randomspawn = server.rand
										.nextInt(server.map.spawnYellow.size());
								spawn = server.map.spawnYellow.get(randomspawn);
								X = spawn.X * 16 - 8;
								Y = spawn.Y * 16 - 8;
								break;
						}
						if (server.engine.Locked == false)
						{
							timeSpawn = 0;
							server.players.sendGame(-1, "spawn " + Index);
							server.players.sendTo(Index, "move " + (int) X
									+ " " + (int) Y);
						}
						break;
				}
			}
		}

	}

	public final void sendTCP(String msg)
	{
		try
		{
			if (connection != null)
			{
				connection.write(msg + "\r\n" + NetCode.Delim);
			}
		}
		catch (ClosedChannelException e)
		{
			try
			{
				connection.close();
			}
			catch (Exception ee)
			{

			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public final void sendUDP(String send)
	{
		if (EstablishedUDP == true && DisableUDP == false && packet != null
				&& server.Tick < timeUDPAck)
		{
			send = SendCounter + " " + send;

			SendCounter++;

			final UserDatagram newPacket = new UserDatagram(packet
					.getRemoteAddress(), packet.getRemotePort(), send.length());
			try
			{
				newPacket.write(send);
				server.netServer.connectionUDP.send(newPacket);
			}
			catch (Exception e)
			{
				e.printStackTrace();
				DisableUDP = true;
				sendTCP(send);
			}
		}
		else
		{
			sendTCP(send);
		}
	}

	public void sendUDPAck()
	{
		String send = SendCounter + " udpACK";
		if (EstablishedUDP == true && DisableUDP == false && packet != null)
		{
			SendCounter++;

			final UserDatagram newPacket = new UserDatagram(packet
					.getRemoteAddress(), packet.getRemotePort(), send.length());
			try
			{
				newPacket.write(send);
				server.netServer.connectionUDP.send(newPacket);
			}
			catch (Exception e)
			{
				DisableUDP = true;
				e.printStackTrace();
			}
		}
	}

	public void sendEstablishUDP()
	{
		String send = SendCounter + " establishUDP";
		SendCounter++;

		final UserDatagram newPacket = new UserDatagram(packet
				.getRemoteAddress(), packet.getRemotePort(), send.length());
		try
		{
			newPacket.write(send);
			server.netServer.connectionUDP.send(newPacket);
		}
		catch (Exception e)
		{
			DisableUDP = true;
			e.printStackTrace();
		}
	}
}
