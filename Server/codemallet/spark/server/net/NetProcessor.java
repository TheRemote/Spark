package codemallet.spark.server.net;

import java.io.FileWriter;
import java.util.Date;
import java.util.StringTokenizer;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.map.Flag;
import codemallet.spark.server.player.Player;

public final class NetProcessor
{
	private final SparkServer server;

	private Player localPlayer;

	private static final int MaxUpdateDistance = 256;

	public NetProcessor(SparkServer server)
	{
		this.server = server;
	}

	public final void doRequest(String line, Player localPlayer)
	{
		this.localPlayer = localPlayer;
		if (line.startsWith("ping"))
		{
			server.players.sendUDP(localPlayer.Index, "pong");
		}
		else if (line.startsWith("tcpping"))
		{
			server.players.sendTo(localPlayer.Index, "tcppong");
		}
		else if (line.startsWith("m "))
		{
			processMovement(line);
		}
		else if (line.startsWith("laser"))
		{
			processLaser(line);
		}
		else if (line.startsWith("missile"))
		{
			processMissile(line);
		}
		else if (line.startsWith("grenade"))
		{
			processGrenade(line);
		}
		else if (line.startsWith("bouncy"))
		{
			processBouncy(line);
		}
		else if (line.startsWith("death"))
		{
			processDeath(line);
		}
		else if (line.startsWith("hit "))
		{
			processHit(line);
		}
		else if (line.startsWith("smoking"))
		{
			processSmoking(line);
		}
		else if (line.startsWith("drop"))
		{
			processDropFlag();
		}
		else if (line.startsWith("joinGame"))
		{
			processJoinGame(line);
		}
		else if (line.startsWith("version"))
		{
			processVersion(line);
		}
		else if (line.startsWith("disableUDP"))
		{
			processDisableUDP(line);
		}
		else if (line.startsWith("changeTeam"))
		{
			processChangeTeam(line);
		}
		else if (line.startsWith("broadcast"))
		{
			processBroadcast(line);
		}
		else if (line.startsWith("teamchat"))
		{
			processTeamChat(line);
		}
		else if (line.startsWith("shutdown"))
		{
			processShutdown();
		}
		else if (line.startsWith("rgba "))
		{
			processRGBA(line);
		}
		else if (line.equals("establishUDP"))
		{
			processEstablishUDP();
		}
		else if (line.equals("udpACK"))
		{
			localPlayer.timeUDPAck = server.Tick + 10000;
		}
		else if (line.startsWith("nadespeed "))
		{
			processNadeSpeed(line);
		}
		else if ("crazyweapons".equals(line))
		{
			processCrazyWeapons();
		}
		else if (line.startsWith("timestamp "))
		{
			processTimestamp(line);
		}
		else if (line.startsWith("LagCompensation"))
		{
			processLagCompensation(line);
		}
		else
			System.err.println("Command unrecognized - " + line);
	}

	private final void processMovement(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		final float X = Float.parseFloat(st.nextToken());
		final float Y = Float.parseFloat(st.nextToken());
		final int MoveX = Integer.parseInt(st.nextToken());
		final int MoveY = Integer.parseInt(st.nextToken());
		final int Health = Integer.parseInt(st.nextToken());
		final int Energy = Integer.parseInt(st.nextToken());

		if (Math.abs(localPlayer.X - X) < MaxUpdateDistance
				&& Math.abs(localPlayer.Y - Y) < MaxUpdateDistance)
		{
			localPlayer.X = X;
			localPlayer.Y = Y;
			localPlayer.Health = Health;
			localPlayer.Energy = Energy;

			localPlayer.movementCorrectionStrikes = 0;

			if (localPlayer.timeSpawn > 0)
			{
				if (MoveX != 0 || MoveY != 0)
					localPlayer.MovedInPen = true;
			}

			server.players.sendToSpectatorsUDP(localPlayer.Index, "m "
					+ localPlayer.Index + " " + (float) X + " " + (float) Y
					+ " " + MoveX + " " + MoveY + " " + Health + " " + Energy);
			server.players.sendToNonSpectatorsUDP(localPlayer.Index, "m "
					+ localPlayer.Index + " " + (float) X + " " + (float) Y
					+ " " + MoveX + " " + MoveY);
		}
		else
		{
			localPlayer.movementCorrectionStrikes++;
			if (localPlayer.movementCorrectionStrikes > 3
					&& server.Tick > localPlayer.timeCorrectPosition)
			{
				localPlayer.timeCorrectPosition = server.Tick + 500;
				server.players.sendTo(localPlayer.Index, "move "
						+ (int) localPlayer.X + " " + (int) localPlayer.Y);
			}
		}
	}

	private final void processBroadcast(String line)
	{
		if (line != null && line.length() > 10)
		{
			String Message = line.substring(10, line.length());
			server.players.sendGame(localPlayer.Index, "broadcast "
					+ localPlayer.Index + " " + Message);
		}
	}

	private final void processTeamChat(String line)
	{
		if (line != null && line.length() > 9)
		{
			String Message = line.substring(9, line.length());
			server.players.sendToTeam(localPlayer.Index, localPlayer.Team,
					"teamchat " + localPlayer.Index + " " + Message);
		}
	}

	private final void processJoinGame(final String line)
	{
		if (localPlayer.Verified == true)
		{
			server.players.joinGame(localPlayer.Index);
		}
	}

	private final void processLaser(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			float X = Float.parseFloat(st.nextToken());
			float Y = Float.parseFloat(st.nextToken());
			double Angle = Double.parseDouble(st.nextToken());

			if (Math.abs(localPlayer.X - (X + 16)) < MaxUpdateDistance
					&& Math.abs(localPlayer.Y - (Y + 16)) < MaxUpdateDistance)
			{
				if (server.Tick > localPlayer.timeMaxClickSpeed)
				{
					localPlayer.timeMaxClickSpeed = server.Tick + 92;
					server.players.sendGameUDP(localPlayer.Index, "laser "
							+ localPlayer.Index + " " + X + " " + Y + " "
							+ Angle);
				}
			}
		}
	}

	private final void processMissile(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			float X = Float.parseFloat(st.nextToken());
			float Y = Float.parseFloat(st.nextToken());
			double Angle = Double.parseDouble(st.nextToken());

			if (localPlayer.MissileCharges > 0)
			{
				localPlayer.MissileCharges--;
				if (Math.abs(localPlayer.X - (X + 16)) < MaxUpdateDistance
						&& Math.abs(localPlayer.Y - (Y + 16)) < MaxUpdateDistance)
				{
					server.players.sendGameUDP(localPlayer.Index, "missile "
							+ localPlayer.Index + " " + X + " " + Y + " "
							+ Angle);
				}
			}
		}
	}

	private final void processGrenade(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			float X = Float.parseFloat(st.nextToken());
			float Y = Float.parseFloat(st.nextToken());
			double Angle = Double.parseDouble(st.nextToken());
			float TargetX = Float.parseFloat(st.nextToken());
			float TargetY = Float.parseFloat(st.nextToken());

			if (localPlayer.GrenadeCharges > 0)
			{
				localPlayer.GrenadeCharges--;
				if (Math.abs(localPlayer.X - (X + 16)) < MaxUpdateDistance
						&& Math.abs(localPlayer.Y - (Y + 16)) < MaxUpdateDistance)
				{
					server.players.sendGameUDP(localPlayer.Index, "grenade "
							+ localPlayer.Index + " " + X + " " + Y + " "
							+ Angle + " " + TargetX + " " + TargetY);
				}
			}
		}
	}

	private final void processBouncy(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			float X = Float.parseFloat(st.nextToken());
			float Y = Float.parseFloat(st.nextToken());
			double Angle = Double.parseDouble(st.nextToken());

			if (localPlayer.BouncyCharges > 0)
			{
				localPlayer.BouncyCharges--;
				if (Math.abs(localPlayer.X - (X + 16)) < MaxUpdateDistance
						&& Math.abs(localPlayer.Y - (Y + 16)) < MaxUpdateDistance)
				{
					server.players.sendGameUDP(localPlayer.Index, "bouncy "
							+ localPlayer.Index + " " + X + " " + Y + " "
							+ Angle);
				}
			}
		}
	}

	private final void processDeath(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			int Killer = Integer.parseInt(st.nextToken());
			server.players.Death(localPlayer.Index, Killer);
		}
	}

	private final void processSmoking(String line)
	{
		if (localPlayer.timeDead == 0 && localPlayer.Team >= 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			int Smoking = Integer.parseInt(st.nextToken());
			server.players.sendGameUDP(localPlayer.Index, "smoking "
					+ localPlayer.Index + " " + Smoking);
		}
	}

	private final void processVersion(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		if (st.nextToken().equals(SparkServer.Version) == true)
		{
			localPlayer.Name = st.nextToken();
			if (st.nextToken().compareTo(server.Password) == 0)
			{
				localPlayer.Verified = true;
				localPlayer.sendTCP("beginJoining " + server.map.MapFile);
			}
			else
			{
				localPlayer.Verified = false;
				localPlayer.sendTCP("badpassword");
			}
			localPlayer.Clan = Integer.parseInt(st.nextToken());
		}
		else
		{
			localPlayer.sendTCP("version");
		}
	}

	private final void processDisableUDP(String line)
	{
		System.err.println(localPlayer.Name + " disabled UDP");
		localPlayer.DisableUDP = true;
	}

	private final void processChangeTeam(String line)
	{
		if (server.Tick > localPlayer.timeTeamSwitch)
		{
			localPlayer.timeTeamSwitch = server.Tick + 5000;

			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			int Team = Integer.parseInt(st.nextToken());

			if (Team < server.map.GameMap.numTeams)
			{
				if (localPlayer.Team == -1)
				{
					if (server.players.getPlayerCount() < server.MaxPlayers)
					{
						server.players.setTeam(localPlayer.Index, Team);
					}
					else
					{
						server.players
								.sendTo(
										localPlayer.Index,
										"message The game is full.  Please wait for a player to leave, or join a different game!");
					}
				}
				else
				{
					server.players.setTeam(localPlayer.Index, Team);
				}
			}
		}
	}

	private final void processDropFlag()
	{
		if (localPlayer.hasFlag == true)
		{
			Flag flag = server.map.Flags.get(localPlayer.FlagIndex);
			flag.onGround = true;
			flag.onPlayer = false;
			flag.onPole = false;
			flag.timeDrop = server.Tick + 5000;
			flag.timeReturn = server.Tick + 60000;
			flag.lastTeam = localPlayer.Team;
			flag.X = localPlayer.X + 16;
			flag.Y = localPlayer.Y + 16;
			localPlayer.hasFlag = false;
			server.players.sendGame(-1, "flag " + localPlayer.FlagIndex + " "
					+ 5 + " " + (int) flag.X + " " + (int) flag.Y);

		}
	}

	private final void processShutdown()
	{
		if (localPlayer.Name.toLowerCase().compareTo("remote") == 0
				|| localPlayer.Name.toLowerCase().compareTo("k0rrupt") == 0)
		{
			if (localPlayer != null && localPlayer.Name != null)
			{
				System.err.println(localPlayer.Name
						+ " is shutting down the server...");
			}
			else
			{
				System.err.println("Shutting down the server...");
			}

			server.Close();
		}
	}

	private final void processRGBA(String line)
	{
		if (localPlayer.Name.toLowerCase().compareTo("remote") == 0
				|| localPlayer.Name.toLowerCase().compareTo("k0rrupt") == 0)
		{
			StringTokenizer st = new StringTokenizer(line);
			st.nextToken();

			int Red = Integer.parseInt(st.nextToken());
			int Green = Integer.parseInt(st.nextToken());
			int Blue = Integer.parseInt(st.nextToken());
			int Alpha = Integer.parseInt(st.nextToken());

			server.players.sendGame(-1, "rgba " + localPlayer.Index + " " + Red
					+ " " + Green + " " + Blue + " " + Alpha);
		}
	}

	private final void processNadeSpeed(String line)
	{
		if (localPlayer.Name.toLowerCase().compareTo("remote") == 0
				|| localPlayer.Name.toLowerCase().compareTo("k0rrupt") == 0)
		{
			server.players.sendGame(-1, line);
		}
	}

	private final void processCrazyWeapons()
	{
		if (localPlayer.Name.toLowerCase().compareTo("remote") == 0
				|| localPlayer.Name.toLowerCase().compareTo("k0rrupt") == 0)
		{
			server.map.GameMap.MissileTime = 0;
			server.map.GameMap.GrenadeTime = 0;
			server.map.GameMap.BouncyTime = 0;

			server.players.sendGame(-1, "crazyweapons");
		}
	}

	private final void processLagCompensation(String line)
	{
		if (localPlayer.Name.toLowerCase().compareTo("remote") == 0
				|| localPlayer.Name.toLowerCase().compareTo("k0rrupt") == 0)
		{
			server.players.sendGame(-1, line);
		}
	}

	private final void processTimestamp(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		long timestamp = Long.parseLong(st.nextToken());
		long epoch = Long.parseLong(st.nextToken());

		if (localPlayer.timestampLastTick != 0
				&& localPlayer.timestampLastEpoch != 0)
		{
			long EpochDifference = Math.abs(epoch
					- localPlayer.timestampLastEpoch);
			long TickDifference = Math.abs(timestamp
					- localPlayer.timestampLastTick);
			if ((TickDifference - EpochDifference) > 1000)
			{
				String CheatString = server.dateFormatting.dateFormatMinute
						.format(new Date())
						+ " - "
						+ localPlayer.Name
						+ " was detected speedhacking.  Epoch: "
						+ EpochDifference + "   Tick: " + TickDifference;
				System.err.println(CheatString);
				try
				{
					FileWriter fw = new FileWriter("CheatLog.txt", true);
					fw.write(CheatString + "\r\n");
					fw.flush();
					fw.close();
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
			}
			localPlayer.timestampLastEpoch = epoch;
			localPlayer.timestampLastTick = timestamp;
		}
		else
		{
			localPlayer.timestampLastTick = timestamp;
			localPlayer.timestampLastEpoch = epoch;
		}
	}

	private final void processEstablishUDP()
	{
		localPlayer.EstablishedUDP = true;
		localPlayer.timeUDPAck = server.Tick + 20000;
		localPlayer.timeUDPSendAck = server.Tick + 5000;
	}

	private final void processHit(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		final int Attacker = Integer.parseInt(st.nextToken());
		final int Weapon = Integer.parseInt(st.nextToken());
		final int Damage = Integer.parseInt(st.nextToken());
		final int Life = Integer.parseInt(st.nextToken());
		final int X = Integer.parseInt(st.nextToken());
		final int Y = Integer.parseInt(st.nextToken());
		final float Angle = Float.parseFloat(st.nextToken());

		final Player attacker = server.players.getPlayer(Attacker);
		if (attacker.Team != localPlayer.Team)
		{
			if (attacker.inUse == true)
			{
				attacker.TotalDamageGiven = attacker.TotalDamageGiven + Damage;
				attacker.TotalHitsGiven++;
			}

			localPlayer.TotalDamageTaken = localPlayer.TotalDamageTaken
					+ Damage;
			localPlayer.TotalHitsTaken++;
		}

		server.players.sendGameUDP(localPlayer.Index, "hit "
				+ localPlayer.Index + " " + Attacker + " " + Weapon + " "
				+ Damage + " " + Life + " " + X + " " + Y + " " + Angle);
	}
}
