package codemallet.spark.server.player;

import org.xsocket.connection.INonBlockingConnection;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.map.Flag;
import codemallet.spark.server.map.Map;
import codemallet.spark.server.map.MapPen;
import codemallet.spark.server.map.Switch;

public final class PlayerList
{
	private final SparkServer server;

	private Player Players[];

	final int State_Game = 2;

	public PlayerList(SparkServer server)
	{
		this.server = server;
	}

	public final void Initialize()
	{
		Players = new Player[server.MaxTotalPlayers];

		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			Players[i] = new Player(server, null);
		}
	}

	public final void Cycle()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			Players[i].Cycle();
		}
	}

	public final Player getPlayer(int Index)
	{
		return Players[Index];
	}

	public final Player addPerson(INonBlockingConnection connection)
	{
		int newIndex = findIndex();

		if (newIndex != -1)
		{
			Players[newIndex].connection = connection;

			Players[newIndex].Index = newIndex;
			Players[newIndex].inUse = true;
			Players[newIndex].Verified = false;
			Players[newIndex].State = 0;
			Players[newIndex].Team = -1;

			Players[newIndex].packet = null;
			Players[newIndex].DisableUDP = false;
			Players[newIndex].EstablishedUDP = false;
			Players[newIndex].SendCounter = 0;
			Players[newIndex].ReceiveCounter = -1;
			Players[newIndex].GoodPackets = 0;

			Players[newIndex].Score = 0;
			Players[newIndex].X = 0;
			Players[newIndex].Y = 0;
			Players[newIndex].timeWarp = 0;
			Players[newIndex].WarpIndex = 0;

			Players[newIndex].MovedInPen = false;
			Players[newIndex].timeSpawn = 0;
			Players[newIndex].timeDead = 0;

			Players[newIndex].MissileCharges = 0;
			Players[newIndex].MissileRecharge = 0;
			Players[newIndex].GrenadeCharges = 0;
			Players[newIndex].GrenadeRecharge = 0;
			Players[newIndex].BouncyCharges = 0;
			Players[newIndex].BouncyRecharge = 0;

			Players[newIndex].timeCorrectPosition = 0;
			Players[newIndex].timeTeamSwitch = 0;
			Players[newIndex].timestampLastEpoch = 0;
			Players[newIndex].timestampLastTick = 0;

			return Players[newIndex];
		}
		else
		{
			return null;
		}
	}

	public final void delPerson(int Index)
	{

		if (Players[Index].State == State_Game)
		{
			if (Players[Index].hasFlag == true)
			{
				Flag flag = server.map.Flags.get(Players[Index].FlagIndex);
				flag.onGround = true;
				flag.onPlayer = false;
				flag.X = Players[Index].X + 16;
				flag.Y = Players[Index].Y + 16;
				flag.timeDrop = server.Tick + 5000;
				flag.timeReturn = server.Tick + 60000;
				flag.lastTeam = Players[Index].Team;
				Players[Index].hasFlag = false;
				sendGame(Index, "flag " + Players[Index].FlagIndex + " " + 5
						+ " " + (int) flag.X + " " + (int) flag.Y);
			}
		}

		if (Players[Index].Verified == true)
		{
			sendGame(Index, "remove " + Index);
		}

		Players[Index].inUse = false;
		Players[Index].State = 0;
		Players[Index].connection = null;
		Players[Index].Verified = false;
		Players[Index].Name = "";
		Players[Index].timestampLastEpoch = 0;
		Players[Index].timestampLastTick = 0;
	}

	public final void sendTo(int Index, String msg)
	{
		Players[Index].sendTCP(msg);

	}

	public final void sendUDP(int Index, String msg)
	{
		Players[Index].sendUDP(msg);
	}

	public final void sendUDPAck(int Index)
	{
		Players[Index].sendUDPAck();
	}

	public final void sendGame(int Index, String msg)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse && i != Index
					&& Players[i].State == State_Game)
				Players[i].sendTCP(msg);
		}
	}

	public final void sendToTeam(int Index, int Team, String msg)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse && i != Index
					&& Players[i].State == State_Game
					&& Players[i].Team == Team)
				Players[i].sendTCP(msg);
		}
	}

	public final void sendToSpectatorsUDP(int Index, String msg)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse && i != Index
					&& Players[i].State == State_Game && Players[i].Team == -1)
				Players[i].sendUDP(msg);
		}
	}

	public final void sendToNonSpectatorsUDP(int Index, String msg)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse && i != Index
					&& Players[i].State == State_Game && Players[i].Team > -1)
				Players[i].sendUDP(msg);
		}
	}

	public final void sendGameUDP(int Index, String msg)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse && i != Index
					&& Players[i].State == State_Game)
				Players[i].sendUDP(msg);
		}
	}

	private final int findIndex()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == false)
			{
				return i;
			}
		}
		return -1;
	}

	public final void sendPlayers(int Index)
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true && Players[i].Name != null
					&& Players[i].Verified == true)
			{
				Players[Index].sendTCP("create " + Players[i].Name + " "
						+ Players[i].Index + " " + Players[i].Team + " "
						+ Players[i].Score + " " + Players[i].Clan);
			}
		}
	}

	public final void ChangeMap()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				if (Players[i].Team >= server.map.GameMap.numTeams)
				{
					Players[i].Team = getTeam();
				}

				sendTo(i, "changeMap " + server.map.MapFile);

				Players[i].MissileRecharge = 0;
				Players[i].MissileCharges = 0;
				Players[i].GrenadeRecharge = 0;
				Players[i].GrenadeCharges = 0;
				Players[i].BouncyRecharge = 0;
				Players[i].BouncyCharges = 0;
				Players[i].Score = 0;

				for (int j = 0; j < server.map.Flags.size(); j++)
				{
					Flag flag = server.map.Flags.get(j);
					if (flag.onPole == true)
						Players[i].sendTCP("flag " + j + " " + 2 + " "
								+ flag.PoleIndex);
					else if (flag.onGround == true)
						Players[i].sendTCP("flag " + j + " " + 3 + " "
								+ (int) flag.X + " " + (int) flag.Y);
					else if (flag.onPlayer == true)
						Players[i].sendTCP("flag " + j + " " + 4 + " "
								+ flag.PlayerIndex);
				}
				for (int j = 0; j < server.map.Switches.size(); j++)
				{
					Switch Switch = server.map.Switches.get(j);
					if (Switch.Team > -1)
						Players[i].sendTCP("switch " + j + " " + 1 + " "
								+ Switch.Team);
				}
				server.players.sendTo(i, "recharge "
						+ Players[i].MissileRecharge + " "
						+ Players[i].MissileCharges + " "
						+ Players[i].GrenadeRecharge + " "
						+ Players[i].GrenadeCharges + " "
						+ Players[i].BouncyRecharge + " "
						+ Players[i].BouncyCharges);

				if (Players[i].Team != -1)
					holdPen(i);
			}
		}
	}

	public final void joinGame(int Index)
	{
		Players[Index].Team = getTeam();

		Players[Index].MissileRecharge = 0;
		Players[Index].MissileCharges = 0;
		Players[Index].GrenadeRecharge = 0;
		Players[Index].GrenadeCharges = 0;
		Players[Index].BouncyRecharge = 0;
		Players[Index].BouncyCharges = 0;
		Players[Index].Score = 0;

		Players[Index].State = State_Game;
		sendPlayers(Index);

		Players[Index].sendTCP("joinGame " + Index + " " + Players[Index].Team
				+ " " + server.TeamScores[0] + " " + server.TeamScores[1] + " "
				+ server.TeamScores[2] + " " + server.TeamScores[3]);

		for (int i = 0; i < server.map.Flags.size(); i++)
		{
			Flag flag = server.map.Flags.get(i);
			if (flag.onPole == true)
				Players[Index].sendTCP("flag " + i + " " + 2 + " "
						+ flag.PoleIndex);
			else if (flag.onGround == true)
				Players[Index].sendTCP("flag " + i + " " + 3 + " "
						+ (int) flag.X + " " + (int) flag.Y);
			else if (flag.onPlayer == true)
				Players[Index].sendTCP("flag " + i + " " + 4 + " "
						+ flag.PlayerIndex);
		}
		for (int i = 0; i < server.map.Switches.size(); i++)
		{
			Switch Switch = server.map.Switches.get(i);
			if (Switch.Team > -1)
				Players[Index].sendTCP("switch " + i + " " + 1 + " "
						+ Switch.Team);
		}
		server.players.sendTo(Index, "recharge "
				+ Players[Index].MissileRecharge + " "
				+ Players[Index].MissileCharges + " "
				+ Players[Index].GrenadeRecharge + " "
				+ Players[Index].GrenadeCharges + " "
				+ Players[Index].BouncyRecharge + " "
				+ Players[Index].BouncyCharges);

		sendGame(Index, "create " + Players[Index].Name + " " + Index + " "
				+ Players[Index].Team + " " + Players[Index].Score + " "
				+ Players[Index].Clan);
		sendGame(Index, "joinedGame " + Index + " " + Players[Index].Team);

		Players[Index].timeDead = 0;

		if (Players[Index].Team != -1)
			holdPen(Index);
		else
			Players[Index].timeSpawn = 1;
	}

	public final int getPlayerCount()
	{
		int PlayerCount = 0;
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true && Players[i].Team >= 0)
			{
				PlayerCount++;
			}
		}
		return PlayerCount;
	}

	public final int getPlayerTotalCount()
	{
		int PlayerCount = 0;
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				PlayerCount++;
			}
		}
		return PlayerCount;
	}

	public final int getTeam()
	{
		int TheTeam = 0;

		int LowestTeam[] = new int[4];

		if (getPlayerCount() < server.MaxPlayers)
		{
			for (int j = 0; j < server.MaxTotalPlayers; j++)
			{
				if (Players[j].State == State_Game)
				{
					if (Players[j].Team >= 0)
						LowestTeam[Players[j].Team]++;
				}
			}

			for (int j = 0; j < server.map.GameMap.numTeams; j++)
			{
				if (LowestTeam[j] < LowestTeam[TheTeam])
					TheTeam = j;
			}

			if (TheTeam == 0)
			{
				int rand = server.rand.nextInt(server.map.GameMap.numTeams);
				if (LowestTeam[rand] <= LowestTeam[0])
				{
					TheTeam = rand;
				}
			}
		}
		else
		{
			TheTeam = -1;
		}

		return TheTeam;
	}

	public final void setTeam(int Index, int Team)
	{
		int OldTeam = Players[Index].Team;
		Players[Index].Team = Team;
		sendGame(-1, "team " + Index + " " + Team);
		if (Team >= 0)
			Players[Index].timeDead = server.Tick + 2000;
		Players[Index].Score = 0;
		Players[Index].timeWarp = 0;
		Players[Index].timeSpawn = 0;

		Players[Index].MissileRecharge = 0;
		Players[Index].MissileCharges = 0;
		Players[Index].GrenadeRecharge = 0;
		Players[Index].GrenadeCharges = 0;
		Players[Index].BouncyRecharge = 0;
		Players[Index].BouncyCharges = 0;

		server.players.sendTo(Index, "recharge "
				+ Players[Index].MissileRecharge + " "
				+ Players[Index].MissileCharges + " "
				+ Players[Index].GrenadeRecharge + " "
				+ Players[Index].GrenadeCharges + " "
				+ Players[Index].BouncyRecharge + " "
				+ Players[Index].BouncyCharges);

		if (Players[Index].hasFlag == true)
		{
			Flag flag = server.map.Flags.get(Players[Index].FlagIndex);
			flag.onGround = true;
			flag.onPlayer = false;
			flag.onPole = false;
			flag.X = Players[Index].X + 16;
			flag.Y = Players[Index].Y + 16;
			flag.timeDrop = server.Tick + 5000;
			flag.timeReturn = server.Tick + 60000;
			flag.lastTeam = OldTeam;
			Players[Index].hasFlag = false;
			sendGame(-1, "flag " + Players[Index].FlagIndex + " " + 5 + " "
					+ (int) flag.X + " " + (int) flag.Y);
		}
	}

	public final void ClearFlags()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].hasFlag == true)
			{
				Players[i].hasFlag = false;
			}
		}
	}

	public final void ResetGame()
	{
		ClearFlags();
		sendGame(-1, "reset");
	}

	public final void Death(int Index, int Killer)
	{
		Players[Index].timeDead = server.Tick + 3000;
		Players[Index].timeWarp = 0;
		Players[Index].timeSpawn = 0;

		Players[Index].MissileRecharge = 0;
		Players[Index].MissileCharges = 0;
		Players[Index].GrenadeRecharge = 0;
		Players[Index].GrenadeCharges = 0;
		Players[Index].BouncyRecharge = 0;
		Players[Index].BouncyCharges = 0;

		Players[Index].Score -= (Players[Index].Score / 10);
		if (Players[Index].Score < 0)
			Players[Index].Score = 0;

		server.players.sendTo(Index, "recharge "
				+ Players[Index].MissileRecharge + " "
				+ Players[Index].MissileCharges + " "
				+ Players[Index].GrenadeRecharge + " "
				+ Players[Index].GrenadeCharges + " "
				+ Players[Index].BouncyRecharge + " "
				+ Players[Index].BouncyCharges);

		if (Index != Killer)
			Players[Killer].Score++;

		sendGame(Index, "death " + Index + " " + Killer);
		sendGame(-1, "score " + Index + " " + Players[Index].Score);
		if (Index != Killer)
			sendGame(-1, "score " + Killer + " " + Players[Killer].Score);

		if (Players[Index].hasFlag == true)
		{
			Flag flag = server.map.Flags.get(Players[Index].FlagIndex);
			flag.onGround = true;
			flag.onPlayer = false;
			flag.onPole = false;
			flag.X = Players[Index].X + 16;
			flag.Y = Players[Index].Y + 16;
			flag.timeDrop = server.Tick + 5000;
			flag.timeReturn = server.Tick + 60000;
			flag.lastTeam = Players[Index].Team;
			Players[Index].hasFlag = false;
			sendGame(-1, "flag " + Players[Index].FlagIndex + " " + 5 + " "
					+ (int) flag.X + " " + (int) flag.Y);
		}
	}

	public final void Won(int Team)
	{
		server.TeamScores[Team]++;

		server.players.sendGame(-1, "win " + Team + " " + server.TeamScores[0]
				+ " " + server.TeamScores[1] + " " + server.TeamScores[2] + " "
				+ server.TeamScores[3]);

		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true && Players[i].Team >= 0)
			{
				if (Players[i].Team == Team)
				{
					Players[i].Score = (Players[i].Score + 2);
				}
				else if (Players[i].Score > 0)
					Players[i].Score--;

				Players[i].MissileRecharge = 0;
				Players[i].MissileCharges = 0;
				Players[i].GrenadeRecharge = 0;
				Players[i].GrenadeCharges = 0;
				Players[i].BouncyRecharge = 0;
				Players[i].BouncyCharges = 0;

				server.players.sendTo(i, "recharge "
						+ Players[i].MissileRecharge + " "
						+ Players[i].MissileCharges + " "
						+ Players[i].GrenadeRecharge + " "
						+ Players[i].GrenadeCharges + " "
						+ Players[i].BouncyRecharge + " "
						+ Players[i].BouncyCharges);

				server.players.sendGame(-1, "score " + i + " "
						+ Players[i].Score);

				Players[i].timeDead = server.Tick + 2000;
			}
		}

		server.map = new Map(server, server.map.MapFile);

		for (int i = 0; i < server.map.Flags.size(); i++)
		{
			Flag flag = server.map.Flags.get(i);
			if (flag.onPole == true)
				sendGame(-1, "flag " + i + " " + 2 + " " + flag.PoleIndex);
		}
	}

	public final void holdPen(int Index)
	{
		if (Players[Index].inUse == true && Players[Index].State == State_Game)
		{
			switch (Players[Index].Team)
			{
				case 0:
				{
					final int randompen = server.rand
							.nextInt(server.map.holdpenGreen.size());
					final MapPen pen = server.map.holdpenGreen.get(randompen);
					Players[Index].X = pen.X * 16 - 8;
					Players[Index].Y = pen.Y * 16 - 8;
				}
					break;
				case 1:
				{
					final int randompen = server.rand
							.nextInt(server.map.holdpenRed.size());
					final MapPen pen = server.map.holdpenRed.get(randompen);
					Players[Index].X = pen.X * 16 - 8;
					Players[Index].Y = pen.Y * 16 - 8;
				}
					break;
				case 2:
				{
					final int randompen = server.rand
							.nextInt(server.map.holdpenBlue.size());
					final MapPen pen = server.map.holdpenBlue.get(randompen);
					Players[Index].X = pen.X * 16 - 8;
					Players[Index].Y = pen.Y * 16 - 8;
				}
					break;
				case 3:
				{
					final int randompen = server.rand
							.nextInt(server.map.holdpenYellow.size());
					final MapPen pen = server.map.holdpenYellow.get(randompen);
					Players[Index].X = pen.X * 16 - 8;
					Players[Index].Y = pen.Y * 16 - 8;
				}
					break;
			}
			Players[Index].timeDead = 0;
			Players[Index].timeSpawn = server.Tick
					+ server.map.GameMap.holdingTime * 1000;
			Players[Index].MovedInPen = false;
			server.players.sendGame(-1, "holdpen " + Index + " "
					+ (int) Players[Index].X + " " + (int) Players[Index].Y);

			if (server.engine.Locked == true)
			{
				server.players.sendTo(Index, "Locked");
			}
		}
	}

	public final void holdPenAll()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].inUse == true && Players[i].State == State_Game)
			{
				holdPen(i);
			}
		}
	}

	public final void DisconnectAllPlayers()
	{
		for (int i = 0; i < server.MaxTotalPlayers; i++)
		{
			if (Players[i].connection != null)
			{
				try
				{
					Players[i].connection.close();
				}
				catch (Exception e)
				{

				}
				Players[i].connection = null;
			}
		}
	}

	public final void CloseConnection(int Index)
	{
		if (Players[Index].connection != null)
		{
			try
			{
				Players[Index].connection.close();
			}
			catch (Exception e)
			{

			}
			Players[Index].connection = null;
		}
	}
}