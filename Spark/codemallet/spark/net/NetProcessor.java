package codemallet.spark.net;

import static codemallet.spark.engines.GameResources.colorWhite;

import java.util.StringTokenizer;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;
import codemallet.spark.map.Flag;
import codemallet.spark.map.FlagPole;
import codemallet.spark.map.Map;
import codemallet.spark.map.Switch;
import codemallet.spark.weapon.Bounce;
import codemallet.spark.weapon.Grenade;
import codemallet.spark.weapon.Laser;
import codemallet.spark.weapon.Missile;
import codemallet.spark.weapon.Weapons;

public final class NetProcessor
{
	private final Spark game;

	NetProcessor(Spark game)
	{
		this.game = game;
	}

	public final void doCommand(final String line)
	{
		if (line.startsWith("create"))
		{
			createPlayer(line);
		}
		else if (line.equals("pong"))
		{
			processReceiveUDPPing();
		}
		else if (line.equals("tcppong"))
		{
			processReceiveTCPPing();
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
		else if (line.startsWith("hit "))
		{
			processHit(line);
		}
		else if (line.startsWith("move "))
		{
			processMove(line);
		}
		else if (line.startsWith("warp"))
		{
			processWarp(line);
		}
		else if (line.startsWith("holdpen"))
		{
			processHoldPen(line);
		}
		else if (line.startsWith("spawn"))
		{
			processSpawn(line);
		}
		else if (line.startsWith("death"))
		{
			processDeath(line);
		}
		else if (line.startsWith("smoking"))
		{
			processSmoking(line);
		}
		else if (line.startsWith("remove"))
		{
			removePlayer(line);
		}
		else if (line.startsWith("joinGame"))
		{
			processJoinGame(line);
		}
		else if (line.startsWith("joinedGame"))
		{
			joinedGame(line);
		}
		else if (line.startsWith("broadcast"))
		{
			processBroadcast(line);
		}
		else if (line.startsWith("recharge"))
		{
			processRecharge(line);
		}
		else if (line.startsWith("addCharge"))
		{
			processAddCharge(line);
		}
		else if (line.startsWith("version"))
		{
			processVersion();
		}
		else if (line.startsWith("flag"))
		{
			processFlag(line);
		}
		else if (line.startsWith("switch"))
		{
			processSwitch(line);
		}
		else if (line.startsWith("win"))
		{
			processWin(line);
		}
		else if (line.startsWith("score"))
		{
			processScore(line);
		}
		else if (line.startsWith("teamchat "))
		{
			processTeamChat(line);
		}
		else if (line.startsWith("team "))
		{
			processTeam(line);
		}
		else if (line.startsWith("reset"))
		{
			processReset();
		}
		else if (line.startsWith("message"))
		{
			processMessage(line);
		}
		else if (line.startsWith("rgba "))
		{
			processRGBA(line);
		}
		else if (line.startsWith("beginJoining "))
		{
			processBeginJoining(line);
		}
		else if (line.startsWith("changeMap "))
		{
			processChangeMap(line);
		}
		else if (line.equals("establishUDP"))
		{
			game.gameNetCode.SendUDP("establishUDP");
		}
		else if (line.equals("udpACK"))
		{
			game.gameNetCode.SendUDP("udpACK");
		}
		else if (line.startsWith("nadespeed "))
		{
			processNadeSpeed(line);
		}
		else if (line.equals("crazyweapons"))
		{
			processCrazyWeapons();
		}
		else if (line.equals("badpassword"))
		{
			processBadPassword();
		}
		else if (line.startsWith("LagCompensation "))
		{
			processLagCompensation(line);
		}
		else if (line.startsWith("TeamScores "))
		{
			processTeamScores(line);
		}
		else if (line.startsWith("Locked"))
		{
			processLocked();
		}
		else if (line.startsWith("Unlocked"))
		{
			processUnlocked();
		}
		else if (line.startsWith("kick "))
		{
			processKick(line);
		}
		else
		{
			System.err.println("Command unrecognized - " + line);
		}

	}

	private final void createPlayer(final String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		final String userName = st.nextToken();
		final int Index = Integer.parseInt(st.nextToken());
		final int Team = Integer.parseInt(st.nextToken());
		final int Score = Integer.parseInt(st.nextToken());
		final int Clan = Integer.parseInt(st.nextToken());
		game.gamePlayers.addPlayer(Index, userName, Team, Score, Clan);

		if (game.gameIgnoreList.IgnoreList.contains(game.gamePlayers.getName(
				Index).toLowerCase()))
		{
			game.gamePlayers.setIgnore(Index, true);
		}
		else
		{
			game.gamePlayers.setIgnore(Index, false);
		}

		if (Clan > -1 && game.gameClans.inUse(Clan) == false)
		{
			game.gameNetLinkLobby.SendTCP("getClan " + Clan);
		}
	}

	private final void removePlayer(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		game.gamePlayers.removePlayer(Index);
	}

	private final void processJoinGame(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int Team = Integer.parseInt(st.nextToken());

		game.gamePlayer.Index = Index;
		game.gamePlayer.Team = Team;
		game.gamePlayer.Name = game.gamePlayers.getName(Index);
		game.gamePlayers.setTeam(Index, Team);
		game.gamePlayer.hasFlag = false;

		if (game.gamePlayer.Team == -1)
		{
			game.gamePlayer.SpectatingPlayer = game.gamePlayers
					.getNextPlayer(game.gamePlayer.SpectatingPlayer);
			if (game.gamePlayer.SpectatingPlayer != -1)
			{
				game.gameWeapons.Health = game.gamePlayers
						.getHealth(game.gamePlayer.SpectatingPlayer);
				game.gameWeapons.Energy = game.gamePlayers
						.getEnergy(game.gamePlayer.SpectatingPlayer);
			}
			else
			{
				if (game.gameMap.FlagPoles.size() > 0)
				{
					FlagPole mapFlagPole = game.gameMap.FlagPoles.get(0);
					game.gamePlayers.setX(game.gamePlayer.Index,
							mapFlagPole.X * 16);
					game.gamePlayers.setY(game.gamePlayer.Index,
							mapFlagPole.Y * 16);
				}
				else if (game.gameMap.Switches.size() > 0)
				{
					Switch mapSwitch = game.gameMap.Switches.get(0);
					game.gamePlayers.setX(game.gamePlayer.Index,
							mapSwitch.X * 16);
					game.gamePlayers.setY(game.gamePlayer.Index,
							mapSwitch.Y * 16);
				}
			}
			game.gamePlayer.HoldingPen = false;
			game.gamePlayer.Dead = false;
		}

		game.gameInterface.TeamScores[0] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[1] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[2] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[3] = Integer.parseInt(st.nextToken());

		game.gameChat.AddMessage("Spark - Version "
				+ codemallet.spark.Spark.Version + " Beta",
				GameResources.colorRed);
		game.gameChat.AddMessage(
				"Copyright (C) 2017 James Chambers - CodeMallet Entertainment, Inc.",
				GameResources.colorCyan);
		game.gameChat.AddMessage("Map: " + game.gameMap.GameMap.name,
				GameResources.colorOrange);

		if (game.gameNetLinkLobby != null)
		{
			game.gameNetLinkLobby.updateStats();
		}

		game.Tick = game.getTime();
		game.LoadingGame = false;
		game.gameNetCode.Pings.clear();
		game.gameNetCode.AveragePing = 0;
		game.gameNetCode.TCPSpikeTimer = 0;
		game.gameNetCode.SendTCPPing();
		
		game.gameSound.playSound(GameResources.sndWelcome);
	}

	private final void joinedGame(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int Team = Integer.parseInt(st.nextToken());
		game.gamePlayers.setScore(Index, 0);
		game.gamePlayers.setTeam(Index, Team);
		game.gamePlayers.setSmoking(Index, false);
		game.gamePlayers.setX(Index, 0);
		game.gamePlayers.setY(Index, 0);

		game.gameChat.AddMessage(game.gamePlayers.getName(Index)
				+ " entering game . . .", GameResources.colorWhite);
	}

	private final void processMovement(String line)
	{
		final StringTokenizer st = new java.util.StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		float X = Float.parseFloat(st.nextToken());
		float Y = Float.parseFloat(st.nextToken());
		int MoveX = Integer.parseInt(st.nextToken());
		int MoveY = Integer.parseInt(st.nextToken());
		if (st.hasMoreTokens())
		{
			int Health = Integer.parseInt(st.nextToken());
			int Energy = Integer.parseInt(st.nextToken());
			game.gamePlayers.setHealth(Index, Health);
			game.gamePlayers.setEnergy(Index, Energy);
		}
		game.gamePlayers.setX(Index, (float) X);
		game.gamePlayers.setY(Index, (float) Y);
		game.gamePlayers.setMoveX(Index, MoveX);
		game.gamePlayers.setMoveY(Index, MoveY);
		game.gamePlayers.setLastUpdate(Index, game.Tick);

		int TotalLag = game.gameNetCode.TCPPing
				+ game.gamePlayers.getPing(Index);

		if (TotalLag > game.gameNetCode.LagCompensation)
			game.gamePlayers.Cycle(Index, game.gameNetCode.LagCompensation);
		else
			game.gamePlayers.Cycle(Index, TotalLag);
	}

	private final void processBroadcast(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		String token1 = st.nextToken();
		String token2 = st.nextToken();
		int Index = Integer.parseInt(token2);
		String Message = game.gamePlayers.getName(Index);
		Message = Message + " > ";
		Message = Message
				+ line.substring(token1.length() + token2.length() + 2);
		if (game.gamePlayers.getIgnore(Index) == false)
		{
			game.gameChat.AddMessage(Message, game.gameChat
					.getColor(game.gamePlayers.getTeam(Index)));
		}
	}

	private final void processTeamChat(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		String token1 = st.nextToken();
		String token2 = st.nextToken();
		int Index = Integer.parseInt(token2);
		String Message = game.gamePlayers.getName(Index);
		Message = Message + " (TO TEAM) > ";
		Message = Message
				+ line.substring(token1.length() + token2.length() + 2);
		if (game.gamePlayers.getIgnore(Index) == false)
		{
			game.gameChat.AddMessage(Message, game.gameChat
					.getColor(game.gamePlayers.getTeam(Index)));
		}
	}

	private final void processMessage(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		int length = st.nextToken().length() + 1;
		game.gameChat.AddMessage(line.substring(length),
				GameResources.colorWhite);
	}

	private final void processLaser(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		float X = Float.parseFloat(st.nextToken());
		float Y = Float.parseFloat(st.nextToken());
		double Angle = Double.parseDouble(st.nextToken());
		Laser laser = game.gameLasers.addLaser((double) X, (double) Y, Angle,
				Index);
		int TotalLag = game.gameNetCode.TCPPing
				+ game.gamePlayers.getPing(Index);
		if (TotalLag > game.gameNetCode.LagCompensation)
			laser.Cycle(game.gameNetCode.LagCompensation);
		else
			laser.Cycle(TotalLag);
		game.gameSound.play3DSound(GameResources.sndLaser, X, Y);
	}

	private final void processMissile(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		float X = Float.parseFloat(st.nextToken());
		float Y = Float.parseFloat(st.nextToken());
		double Angle = Double.parseDouble(st.nextToken());
		Missile missile = game.gameMissiles.addMissile((double) X, (double) Y,
				Angle, Index);
		int TotalLag = game.gameNetCode.TCPPing
				+ game.gamePlayers.getPing(Index);
		if (TotalLag > game.gameNetCode.LagCompensation)
			missile.Cycle(game.gameNetCode.LagCompensation);
		else
			missile.Cycle(TotalLag);
		game.gameSound.play3DSound(GameResources.sndMissile, X, Y);
	}

	private final void processGrenade(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		float X = Float.parseFloat(st.nextToken());
		float Y = Float.parseFloat(st.nextToken());
		double Angle = Double.parseDouble(st.nextToken());
		float TargetX = Float.parseFloat(st.nextToken());
		float TargetY = Float.parseFloat(st.nextToken());
		Grenade grenade = game.gameGrenades.addGrenade((double) ((float) X),
				(double) ((float) Y), (double) ((float) TargetX),
				(double) ((float) TargetY), Angle, Index);
		int TotalLag = game.gameNetCode.TCPPing
				+ game.gamePlayers.getPing(Index);
		if (TotalLag > game.gameNetCode.LagCompensation)
			grenade.Cycle(game.gameNetCode.LagCompensation);
		else
			grenade.Cycle(TotalLag);
		game.gameSound.play3DSound(GameResources.sndGrenade, X, Y);
	}

	private final void processBouncy(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		float X = Float.parseFloat(st.nextToken());
		float Y = Float.parseFloat(st.nextToken());
		double Angle = Double.parseDouble(st.nextToken());
		Bounce bouncy = game.gameBouncies.addBounce((double) X, (double) Y,
				Angle, Index);
		int TotalLag = game.gameNetCode.TCPPing
				+ game.gamePlayers.getPing(Index);
		if (TotalLag > game.gameNetCode.LagCompensation)
			bouncy.Cycle(game.gameNetCode.LagCompensation);
		else
			bouncy.Cycle(TotalLag);
		game.gameSound.play3DSound(GameResources.sndBouncy, X, Y);
	}

	private final void processHoldPen(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int X = Integer.parseInt(st.nextToken());
		int Y = Integer.parseInt(st.nextToken());
		game.gamePlayers.setX(Index, X);
		game.gamePlayers.setY(Index, Y);
		game.gamePlayers.setLastUpdate(Index, game.Tick);
		if (Index == game.gamePlayer.Index)
		{
			game.gamePlayer.Dead = false;
			game.gamePlayer.Frozen = false;
			game.gamePlayer.HoldingPen = true;
			game.gameWeapons.Energy = game.gameWeapons.MaxEnergy;
			game.gameWeapons.Health = game.gameWeapons.MaxHealth;
			game.gamePlayer.Smoking = false;
			game.gamePlayer.hasFlag = false;
			game.gamePlayer.timeHoldingPen = game.Tick
					+ ((long) (game.gameMap.GameMap.holdingTime * 1000));
		}

		game.gamePlayers.setDead(Index, false);
		game.gamePlayers.setSmoking(Index, false);
		game.gamePlayers.setHoldPen(Index, true);
		game.gamePlayers.setHasFlag(Index, false, 0);
	}

	private final void processSpawn(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		if (Index == game.gamePlayer.Index)
		{
			game.gamePlayer.HoldingPen = false;
			game.gamePlayer.Frozen = false;
			game.gameSound.playSound(GameResources.sndSpawn);
			game.gameWeapons.MissileRecharge = 0;
			game.gameWeapons.MissileCharges = 0;
			game.gameWeapons.GrenadeRecharge = 0;
			game.gameWeapons.GrenadeCharges = 0;
			game.gameWeapons.BouncyRecharge = 0;
			game.gameWeapons.BouncyCharges = 0;
		}

		game.gamePlayers.setHoldPen(Index, false);
	}

	private final void processDeath(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		int Killer = Integer.parseInt(st.nextToken());

		if (Index == game.gamePlayer.Index)
		{
			game.gamePlayer.Dead = true;
			game.gamePlayer.Frozen = false;
			game.gameExplosions.addExplosion((int) (game.gamePlayer.X - 16),
					(int) (game.gamePlayer.Y - 16), 2);
			game.gameSound.play3DSound(GameResources.sndExplosion,
					game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
			game.gameWeapons.Health = 0;
			game.gameWeapons.Energy = 0;
			game.gamePlayers.setDead(game.gamePlayer.Index, true);
		}
		if (Killer == game.gamePlayer.Index)
		{
			if (game.gamePlayer.Dead == false)
			{
				game.gameWeapons.addHealthBonus();
			}
		}
		if (Index != Killer)
		{
			game.gameChat.Add3PartMessage(game.gamePlayers.getName(Killer),
					" destroyed ", game.gamePlayers.getName(Index),
					game.gameChat.getColor(game.gamePlayers.getTeam(Killer)),
					GameResources.colorWhite, game.gameChat
							.getColor(game.gamePlayers.getTeam(Index)));

			game.gamePlayers.setKills(Killer,
					game.gamePlayers.getKills(Killer) + 1);
			game.gamePlayers.setDeaths(Index,
					game.gamePlayers.getDeaths(Index) + 1);
		}
		int X = (int) game.gamePlayers.getX(Index);
		int Y = (int) game.gamePlayers.getY(Index);
		game.gamePlayers.setDead(Index, true);
		game.gamePlayers.setSmoking(Index, false);
		game.gamePlayers.setHealth(Index, 0);
		game.gamePlayers.setEnergy(Index, 0);
		game.gameSound.play3DSound(GameResources.sndExplosion, X, Y);
		game.gameExplosions.addExplosion((int) (X - 16), (int) (Y - 16), 2);
	}

	private final void processRecharge(String line)
	{
		if (game == null)
			return;

		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int MissileRecharge = Integer.parseInt(st.nextToken());
		int GrenadeRecharge = Integer.parseInt(st.nextToken());
		int BouncyRecharge = Integer.parseInt(st.nextToken());
		game.gameWeapons.MissileRecharge = MissileRecharge;
		game.gameWeapons.GrenadeRecharge = GrenadeRecharge;
		game.gameWeapons.BouncyRecharge = BouncyRecharge;
	}

	private final void processAddCharge(String line)
	{
		if (game == null)
			return;

		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int ChargeType = Integer.parseInt(st.nextToken());
		switch (ChargeType)
		{
			case 0:
				game.gameWeapons.MissileCharges++;
				if (game.gameOptions.soundRechargedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectMissile);
				break;
			case 1:
				game.gameWeapons.GrenadeCharges++;
				if (game.gameOptions.soundRechargedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectGrenade);
				break;
			case 2:
				game.gameWeapons.BouncyCharges++;
				if (game.gameOptions.soundRechargedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectBouncy);
				break;
		}
	}

	private final void processSmoking(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int Smoking = Integer.parseInt(st.nextToken());
		if (Smoking == 1)
		{
			game.gamePlayers.setSmoking(Index, true);
		}
		else
		{
			game.gamePlayers.setSmoking(Index, false);
		}
	}

	private final void processVersion()
	{
		System.err.println("Version is outdated!  Shutting down..");
		if (game.gameNetLinkLobby != null)
		{
			game.gameNetLinkLobby.SendTCP("outdated");
		}
		game.CloseMessage = "Your version is outdated!\nThe game will now close and automatically update itself.\nYou do not need to do anything special on your part, just relaunch the game.";
		game.destroy();
	}

	private final void processBadPassword()
	{
		System.err.println("Incorrect password given!  Shutting down..");
		game.CloseMessage = "You entered an incorrect password!";
		game.destroy();
	}

	private final void processWin(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Team = Integer.parseInt(st.nextToken());
		if (game.gameSound != null)
			game.gameSound.addWin(Team);

		game.gameMap.RestoreMapTiles();

		game.gameInterface.TeamScores[0] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[1] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[2] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[3] = Integer.parseInt(st.nextToken());
	}

	private final void processFlag(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int FlagIndex = Integer.parseInt(st.nextToken());
		int EventType = Integer.parseInt(st.nextToken());
		Flag flag = game.gameMap.Flags.get(FlagIndex);
		FlagPole flagpole;
		int Index = 0;
		int PoleIndex = 0;
		{
			switch (EventType)
			{
				case 0: // Flag Captured
					Index = Integer.parseInt(st.nextToken());
					PoleIndex = Integer.parseInt(st.nextToken());
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = false;
					flag.onPlayer = true;
					flag.onGround = false;
					flag.onPole = false;
					flag.PlayerIndex = Index;
					game.gamePlayers.setHasFlag(Index, true, FlagIndex);
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getEmptyTile(
							flagpole.Team, flagpole.FlagType);
					switch (flag.Team)
					{
						case 0:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" grabbed the ", "green team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 1:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"red team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 2:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"blue team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 3:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"yellow team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 4:
							game.gameChat.Add2PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" grabs the not-a-team's flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite);
							break;
					}
					game.gameSound.addFlagCaptured(flag.Team, game.gamePlayers
							.getTeam(Index));
					break;
				case 1: // Flag Returned
					Index = Integer.parseInt(st.nextToken());
					PoleIndex = Integer.parseInt(st.nextToken());
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = true;
					flagpole.FlagIndex = FlagIndex;
					flag.onPlayer = false;
					flag.onGround = false;
					flag.onPole = true;
					flag.PoleIndex = PoleIndex;
					game.gamePlayers.setHasFlag(Index, false, FlagIndex);
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getFlagTile(
							flagpole.Team, flag.Team);
					switch (flag.Team)

					{
						case 0:
							game.gameChat
									.Add4PartMessage(
											game.gamePlayers
													.getName(flag.PlayerIndex),
											" bravely, through much torment, brought the ",
											"green team's ",
											"flag to it's base",
											game.gameChat
													.getColor(game.gamePlayers
															.getTeam(flag.PlayerIndex)),
											GameResources.colorWhite,
											game.gameChat.getColor(flag.Team),
											GameResources.colorWhite);
							break;
						case 1:
							game.gameChat
									.Add4PartMessage(
											game.gamePlayers
													.getName(flag.PlayerIndex),
											" bravely, through much torment, brought the ",
											"red team's ",
											"flag to it's base",
											game.gameChat
													.getColor(game.gamePlayers
															.getTeam(flag.PlayerIndex)),
											GameResources.colorWhite,
											game.gameChat.getColor(flag.Team),
											GameResources.colorWhite);
							break;
						case 2:
							game.gameChat
									.Add4PartMessage(
											game.gamePlayers
													.getName(flag.PlayerIndex),
											" bravely, through much torment, brought the ",
											"blue team's ",
											"flag to it's base",
											game.gameChat
													.getColor(game.gamePlayers
															.getTeam(flag.PlayerIndex)),
											GameResources.colorWhite,
											game.gameChat.getColor(flag.Team),
											GameResources.colorWhite);
							break;
						case 3:
							game.gameChat
									.Add4PartMessage(
											game.gamePlayers
													.getName(flag.PlayerIndex),
											" bravely, through much torment, brought the ",
											"yellow team's ",
											"flag to it's base",
											game.gameChat
													.getColor(game.gamePlayers
															.getTeam(flag.PlayerIndex)),
											GameResources.colorWhite,
											game.gameChat.getColor(flag.Team),
											GameResources.colorWhite);
							break;
						case 4:
							game.gameChat
									.Add2PartMessage(
											game.gamePlayers
													.getName(flag.PlayerIndex),
											" bravely, through much torment, brought the not-a-team's flag to it's base",
											game.gameChat
													.getColor(game.gamePlayers
															.getTeam(flag.PlayerIndex)),
											GameResources.colorWhite);
							break;
					}
					if (game.gameSound != null)
						game.gameSound.addFlagReturned(flag.Team,
								game.gamePlayers.getTeam(Index));
					break;
				case 2: // Flag on Pole
					PoleIndex = Integer.parseInt(st.nextToken());
					flagpole = (FlagPole) game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = true;
					flagpole.FlagIndex = FlagIndex;
					flag.onPole = true;
					flag.onGround = false;
					flag.onPlayer = false;
					flag.PoleIndex = PoleIndex;
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getFlagTile(
							flagpole.Team, flag.Team);
					break;
				case 3: // Flag on Ground
					flag.X = (float) Integer.parseInt(st.nextToken());
					flag.Y = (float) Integer.parseInt(st.nextToken());
					flag.onPole = false;
					flag.onGround = true;
					flag.onPlayer = false;
					break;
				case 4: // Flag on Player
					Index = Integer.parseInt(st.nextToken());
					flag.onPole = false;
					flag.onGround = false;
					flag.onPlayer = true;
					flag.PlayerIndex = Index;
					game.gamePlayers.setHasFlag(Index, true, FlagIndex);
					break;
				case 5: // Flag Dropped
					flag.X = (float) Integer.parseInt(st.nextToken());
					flag.Y = (float) Integer.parseInt(st.nextToken());
					flag.onPole = false;
					flag.onGround = true;
					flag.onPlayer = false;
					flag.timeTotalBlink = game.Tick + 5000;
					game.gamePlayers.setHasFlag(flag.PlayerIndex, false,
							FlagIndex);
					switch (flag.Team)
					{
						case 0:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" dropped the ", "green team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 1:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" dropped the ", "red team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 2:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" dropped the ", "blue team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 3:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" dropped the ", "yellow team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 4:
							game.gameChat.Add2PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" dropped the not-a-team's flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite);
							break;
					}

					game.gameSound.playSound(GameResources.sndDropFlag);
					break;
				case 6: // Flag Picked Up
					Index = Integer.parseInt(st.nextToken());
					flag.onPlayer = true;
					flag.onGround = false;
					flag.onPole = false;
					flag.PlayerIndex = Index;
					switch (flag.Team)
					{
						case 0:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" grabbed the ", "green team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 1:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"red team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 2:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"blue team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 3:
							game.gameChat.Add4PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex), " grabs the ",
									"yellow team's ", "flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite, game.gameChat
											.getColor(flag.Team),
									GameResources.colorWhite);
							break;
						case 4:
							game.gameChat.Add2PartMessage(game.gamePlayers
									.getName(flag.PlayerIndex),
									" grabs the not-a-team's flag",
									game.gameChat.getColor(game.gamePlayers
											.getTeam(flag.PlayerIndex)),
									GameResources.colorWhite);
							break;
					}
					if (game.gameSound != null)
						game.gameSound.addFlagCaptured(flag.Team,
								game.gamePlayers.getTeam(Index));
					game.gamePlayers.setHasFlag(Index, true, FlagIndex);
					break;
				case 7: // Person reclaims its team's flag from pole
					Index = Integer.parseInt(st.nextToken());
					PoleIndex = Integer.parseInt(st.nextToken());
					flag.onPole = true;
					flag.onGround = false;
					flag.onPlayer = false;
					flag.PoleIndex = PoleIndex;
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = true;
					flagpole.FlagIndex = FlagIndex;
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getFlagTile(
							flagpole.Team, flag.Team);

					PoleIndex = Integer.parseInt(st.nextToken());
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = false;
					flagpole.FlagIndex = 0;
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getEmptyTile(
							flagpole.Team, flag.Team);

					game.gameChat.Add2PartMessage(game.gamePlayers
							.getName(Index), " reclaims its team's flag",
							game.gameChat.getColor(game.gamePlayers
									.getTeam(Index)), GameResources.colorWhite);

					if (game.gameSound != null)
						game.gameSound.addFlagReturned(flag.Team,
								game.gamePlayers.getTeam(Index));
					break;
				case 8: // <Team> team's flag has returned to its base
					PoleIndex = Integer.parseInt(st.nextToken());
					flag.PoleIndex = PoleIndex;
					flag.onPole = true;
					flag.onGround = false;
					flag.onPlayer = false;
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = true;
					flagpole.FlagIndex = FlagIndex;
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getFlagTile(
							flagpole.Team, flag.Team);

					switch (flag.Team)
					{
						case 0:
							game.gameChat.Add2PartMessage("green team's",
									" flag returns to its base", game.gameChat
											.getColor(0),
									GameResources.colorWhite);
							break;
						case 1:
							game.gameChat.Add2PartMessage("red team's",
									" flag returns to its base", game.gameChat
											.getColor(1),
									GameResources.colorWhite);
							break;
						case 2:
							game.gameChat.Add2PartMessage("blue team's",
									" flag returns to its base", game.gameChat
											.getColor(2),
									GameResources.colorWhite);
							break;
						case 3:
							game.gameChat.Add2PartMessage("yellow team's",
									" flag returns to its base", game.gameChat
											.getColor(3),
									GameResources.colorWhite);
							break;
						case 4:
							game.gameChat.AddMessage(
									"neutral team's flag returns to its base",
									GameResources.colorWhite);
							break;
					}

					if (game.gameSound != null)
						game.gameSound.addFlagReturned(flag.Team, flag.Team);

					break;
				case 9: // Person reclaims its team's flag from ground
					Index = Integer.parseInt(st.nextToken());

					flag.onPole = true;
					flag.onGround = false;
					flag.onPlayer = false;

					PoleIndex = Integer.parseInt(st.nextToken());
					flag.PoleIndex = PoleIndex;
					flagpole = game.gameMap.FlagPoles.get(PoleIndex);
					flagpole.hasFlag = true;
					flagpole.FlagIndex = FlagIndex;
					game.gameMap.GameMap.MapData[(flagpole.Y * game.gameMap.GameMap.width)
							+ flagpole.X] = game.gameMap.getFlagTile(
							flagpole.Team, flag.Team);

					game.gameChat.Add2PartMessage(game.gamePlayers
							.getName(Index), " reclaims its team's flag",
							game.gameChat.getColor(game.gamePlayers
									.getTeam(Index)), GameResources.colorWhite);

					if (game.gameSound != null)
						game.gameSound.addFlagReturned(flag.Team,
								game.gamePlayers.getTeam(Index));

					break;
			}
		}
	}

	private final void processScore(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int Score = Integer.parseInt(st.nextToken());
		game.gamePlayers.setScore(Index, Score);
		if (Index == game.gamePlayer.Index)
			game.gameNetLinkLobby.updateStats();
	}

	private final void processTeam(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		int Index = Integer.parseInt(st.nextToken());
		int OldTeam = game.gamePlayers.getTeam(Index);
		int Team = Integer.parseInt(st.nextToken());

		if (Team >= 0)
		{
			game.gamePlayers.setDead(Index, true);
		}

		if (OldTeam >= 0)
		{
			int X = (int) game.gamePlayers.getX(Index);
			int Y = (int) game.gamePlayers.getY(Index);
			game.gameSound.play3DSound(GameResources.sndExplosion, X, Y);
			game.gameExplosions.addExplosion((int) (X - 16), (int) (Y - 16), 2);
		}

		game.gamePlayers.setSmoking(Index, false);

		switch (Team)
		{
			case -1:
				game.gameChat.Add2PartMessage(game.gamePlayers.getName(Index),
						" is now a spectator.", game.gameChat
								.getColor(game.gamePlayers.getTeam(Index)),
						GameResources.colorWhite);
				break;
			case 0:
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Index),
						" switched to the ", "green team", game.gameChat
								.getColor(game.gamePlayers.getTeam(Index)),
						GameResources.colorWhite, game.gameChat.getColor(0));
				break;
			case 1:
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Index),
						" switched to the ", "red team", game.gameChat
								.getColor(game.gamePlayers.getTeam(Index)),
						GameResources.colorWhite, game.gameChat.getColor(1));
				break;
			case 2:
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Index),
						" switched to the ", "blue team", game.gameChat
								.getColor(game.gamePlayers.getTeam(Index)),
						GameResources.colorWhite, game.gameChat.getColor(2));
				break;
			case 3:
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Index),
						" switched to the ", "yellow team", game.gameChat
								.getColor(game.gamePlayers.getTeam(Index)),
						colorWhite, game.gameChat.getColor(3));
				break;
		}
		game.gamePlayers.setTeam(Index, Team);
		game.gamePlayers.setScore(Index, 0);
		game.gamePlayers.setHoldPen(Index, false);
		if (Index == game.gamePlayer.Index)
		{
			if (game.gameNetLinkLobby != null)
				game.gameNetLinkLobby.updateStats();
			game.gamePlayer.Team = Team;
			game.gamePlayer.Dead = true;
			game.gamePlayer.HoldingPen = false;
		}
	}

	private final void processWarp(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		int X = Integer.parseInt(st.nextToken());
		int Y = Integer.parseInt(st.nextToken());
		if (Index == game.gamePlayer.Index)
		{
			game.gameSound.playSound(GameResources.sndWarp);
			game.gamePlayer.Frozen = true;
		}
		game.gamePlayers.setX(Index, ((X * 16) - 8));
		game.gamePlayers.setY(Index, ((Y * 16) - 8));
		game.gamePlayers.setWarpEffect(Index, 32);
	}

	private final void processMove(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int X = Integer.parseInt(st.nextToken());
		int Y = Integer.parseInt(st.nextToken());

		game.gamePlayer.Frozen = false;
		game.gamePlayers.setX(game.gamePlayer.Index, X);
		game.gamePlayers.setY(game.gamePlayer.Index, Y);
		game.gamePlayers.setWarpEffect(game.gamePlayer.Index, 0);
	}

	private final void processSwitch(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int SwitchIndex = Integer.parseInt(st.nextToken());
		int EventType = Integer.parseInt(st.nextToken());
		Switch Switch = game.gameMap.Switches.get(SwitchIndex);
		int Index = 0;
		int Team = 0;

		switch (EventType)
		{
			case 0: // Switch flipped by
				Index = Integer.parseInt(st.nextToken());
				Team = game.gamePlayers.getTeam(Index);
				Switch.Team = Team;
				game.gameMap.GameMap.MapData[(Switch.Y * game.gameMap.GameMap.width)
						+ Switch.X] = 32892 + Team;
				if (game.gameSound != null)
					game.gameSound.addSwitchFlipped(Index);
				break;
			case 1: // Switch team
				Team = Integer.parseInt(st.nextToken());
				Switch.Team = Team;
				game.gameMap.GameMap.MapData[(Switch.Y * game.gameMap.GameMap.width)
						+ Switch.X] = 32892 + Team;
				break;
		}

	}

	private final void processReset()
	{
		game.gamePlayers.ResetGame();
	}

	private final void processRGBA(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		int Red = Integer.parseInt(st.nextToken());
		int Green = Integer.parseInt(st.nextToken());
		int Blue = Integer.parseInt(st.nextToken());
		int Alpha = Integer.parseInt(st.nextToken());

		game.gamePlayers.setColor(Index, Red, Green, Blue, Alpha);
	}

	private final void processBeginJoining(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		String Map = st.nextToken();
		game.gameMap = new Map(game, Map);
	}

	private final void processChangeMap(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		String Map = st.nextToken();
		game.gameMap = new Map(game, Map);
	}

	private final void processReceiveUDPPing()
	{
		if (game.gameNetCode != null)
			game.gameNetCode.ReceiveUDPPing();
	}

	private final void processReceiveTCPPing()
	{
		if (game.gameNetCode != null)
			game.gameNetCode.ReceiveTCPPing();
	}

	private final void processNadeSpeed(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		game.gameGrenades.NadeSpeed = Float.parseFloat(st.nextToken());
		game.gameChat.AddMessage("Grenade speed: "
				+ game.gameGrenades.NadeSpeed, GameResources.colorWhite);
	}

	private final void processCrazyWeapons()
	{
		game.gameChat.AddMessage("Crazy weapons!", GameResources.colorWhite);
		game.gameWeapons.EnergyRechargeTime = 0;
	}

	private final void processLagCompensation(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		game.gameNetCode.LagCompensation = Integer.parseInt(st.nextToken());
		game.gameChat.AddMessage("Lag compensation: "
				+ game.gameNetCode.LagCompensation, GameResources.colorWhite);
	}

	private final void processHit(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int Index = Integer.parseInt(st.nextToken());
		// Attacker
		Integer.parseInt(st.nextToken());
		int Weapon = Integer.parseInt(st.nextToken());
		// Damage
		Integer.parseInt(st.nextToken());
		int Life = Integer.parseInt(st.nextToken());
		int X = Integer.parseInt(st.nextToken());
		int Y = Integer.parseInt(st.nextToken());
		float Angle = Float.parseFloat(st.nextToken());

		game.gamePlayers.setHealth(Index, Life);

		switch (Weapon)
		{
			case Weapons.weaponLaser:
				for (int i = 0; i < 30; i++)
				{
					game.gameSparks.addSpark(X, Y, Angle);
				}
				game.gameSound.play3DSound(GameResources.sndHitLaser,
						(float) X, (float) Y);
				break;
			case Weapons.weaponMissile:
				game.gameExplosions.addExplosion((int) (X - 10),
						(int) (Y - 10), 0);
				game.gameSound.play3DSound(GameResources.sndExplosion,
						(float) X, (float) Y);
				break;
			case Weapons.weaponBouncy:
				for (int i = 0; i < 30; i++)
				{
					game.gameSparks.addSpark(X, Y, Angle);
				}
				game.gameSound.play3DSound(GameResources.sndHitBouncy,
						(float) X, (float) Y);
				break;
		}
	}

	private final void processTeamScores(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		game.gameInterface.TeamScores[0] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[1] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[2] = Integer.parseInt(st.nextToken());
		game.gameInterface.TeamScores[3] = Integer.parseInt(st.nextToken());
	}

	private final void processLocked()
	{
		game.Locked = true;
	}

	private final void processUnlocked()
	{
		game.Locked = false;
	}

	private final void processKick(String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		String Admin = st.nextToken();
		String Reason = st.nextToken();
		while (st.hasMoreTokens())
		{
			Reason = Reason + " " + st.nextToken();
		}

		game.CloseMessage = "You were kicked by " + Admin + " for reason: "
				+ Reason;
	}
}
