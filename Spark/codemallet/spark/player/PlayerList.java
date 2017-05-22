package codemallet.spark.player;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class PlayerList
{
	private final Spark game;

	public final Player Players[];

	private final Player PlayersByScore[];

	public static final int MaxPlayers = 64;

	public PlayerList(Spark game)
	{
		this.game = game;

		Players = new Player[MaxPlayers];

		for (int i = 0; i < MaxPlayers; i++)
		{
			Players[i] = new Player(game);
		}

		PlayersByScore = new Player[MaxPlayers];
		for (int i = 0; i < MaxPlayers; i++)
		{
			PlayersByScore[i] = Players[i];
		}
	}

	public final int getPlayerCount()
	{
		int numPlayers = 0;
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
				numPlayers++;
		}

		return numPlayers;
	}

	public final void PrintWhoData()
	{
		String PlayerString = "";
		int PlayerCount = 0;
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				PlayerCount++;
				if (PlayerString.length() == 0)
					PlayerString += Players[i].Name;
				else
				{
					PlayerString += ", ";
					PlayerString += Players[i].Name;
				}

			}
		}

		String JoinString = "";
		JoinString = "There are ";
		JoinString += PlayerCount;
		JoinString += " players currently online:  ";
		JoinString += PlayerString;
		game.gameChat.AddMessage(JoinString, GameResources.colorYellow);
	}

	public final void Cycle(int delta)
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				Players[i].Cycle(delta);
			}
		}
	}

	public final void Cycle(int Index, int delta)
	{
		Players[Index].Cycle(delta);
	}

	public final void drawPlayers()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				if (i != game.gamePlayer.Index && Players[i].Team >= 0)
				{
					Players[i].drawPlayer();
				}
			}
		}
		if (Players[game.gamePlayer.Index].Team >= 0)
		{
			Players[game.gamePlayer.Index].drawPlayer();
		}
	}

	public final void drawPlayerNames()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				if (i != game.gamePlayer.Index && Players[i].Team >= 0)
					Players[i].drawPlayerName();
			}
		}
		if (Players[game.gamePlayer.Index].Team >= 0)
			Players[game.gamePlayer.Index].drawPlayerName();
	}

	public final void drawRadar()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				if (Players[i].Team >= 0)
				{
					Players[i].drawRadar();
				}
			}
		}
	}

	public final void addPlayer(int Index, String Name, int Team, int Score,
			int Clan)
	{
		Players[Index].Name = Name;
		Players[Index].Index = Index;
		Players[Index].Team = Team;
		Players[Index].Score = Score;
		Players[Index].Clan = Clan;
		Players[Index].inUse = true;

		Players[Index].X = 0;
		Players[Index].Y = 0;

		Players[Index].blendAlpha = 255;
		Players[Index].blendRed = 255;
		Players[Index].blendGreen = 255;
		Players[Index].blendBlue = 255;

		System.err.println("New player added: " + Name + "   Index: " + Index);
	}

	public final void removePlayer(int Index)
	{
		System.err.println("Player removed " + Players[Index].Name
				+ "   Index: " + Index);

		if (Players[Index].Name != null)
		{
			game.gameChat.AddMessage(Players[Index].Name + " left game . . .",
					GameResources.colorWhite);
		}

		Players[Index].Name = null;
		Players[Index].Index = Index;
		Players[Index].inUse = false;
	}

	public final void ResetGame()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			Players[i].hasFlag = false;
			Players[i].Score = 0;
			Players[i].Smoking = false;
		}
		game.gamePlayer.hasFlag = false;
	}

	public final void setX(int Index, float X)
	{
		Players[Index].X = X;
	}

	public final void setY(int Index, float Y)
	{
		Players[Index].Y = Y;
	}

	public final void setMoveX(int Index, int MoveX)
	{
		Players[Index].MoveXInput = MoveX;
	}

	public final void setMoveY(int Index, int MoveY)
	{
		Players[Index].MoveYInput = MoveY;
	}

	public final void setDead(int Index, boolean Dead)
	{
		Players[Index].Dead = Dead;
	}

	public final void setHoldPen(int Index, boolean HoldPen)
	{
		Players[Index].HoldPen = HoldPen;
	}

	public final void setHasFlag(int Index, boolean hasFlag, int FlagIndex)
	{
		Players[Index].hasFlag = hasFlag;
		Players[Index].FlagIndex = FlagIndex;
		if (Index == game.gamePlayer.Index)
		{
			game.gamePlayer.hasFlag = hasFlag;
			game.gamePlayer.FlagIndex = FlagIndex;
		}
	}

	public final void setTeam(int Index, int Team)
	{
		Players[Index].Team = Team;
	}

	public final void setScore(int Index, int Score)
	{
		Players[Index].Score = Score;
	}

	public final void setKills(int Index, int Kills)
	{
		Players[Index].Kills = Kills;
	}

	public final void setDeaths(int Index, int Deaths)
	{
		Players[Index].Deaths = Deaths;
	}

	public final void setSmoking(int Index, boolean Smoking)
	{
		Players[Index].Smoking = Smoking;
	}

	public final void setIgnore(int Index, boolean Ignore)
	{
		Players[Index].Ignore = Ignore;
	}

	public final void setPing(int Index, int Ping)
	{
		Players[Index].Ping = Ping;
	}

	public final void setHealth(int Index, int Health)
	{
		Players[Index].Health = Health;
		if (game.gamePlayer.Team == -1
				&& game.gamePlayer.SpectatingPlayer == Index)
		{
			game.gameWeapons.Health = Health;
		}
	}

	public final void setEnergy(int Index, int Energy)
	{
		Players[Index].Energy = Energy;
		if (game.gamePlayer.Team == -1
				&& game.gamePlayer.SpectatingPlayer == Index)
		{
			game.gameWeapons.Energy = Energy;
		}
	}

	public final void setWarpEffect(int Index, int WarpEffect)
	{
		Players[Index].WarpEffect = WarpEffect;
		Players[Index].WarpStartX = Players[Index].X;
		Players[Index].WarpStartY = Players[Index].Y;
	}

	public final void setLastUpdate(int Index, long lastUpdate)
	{
		Players[Index].lastUpdate = lastUpdate;
	}

	public final int getDirection(int Index)
	{
		return Players[Index].Direction;
	}

	public final int getHealth(int Index)
	{
		if (Index > -1)
			return Players[Index].Health;
		else
			return 0;
	}

	public final int getEnergy(int Index)
	{
		if (Index > -1)
			return Players[Index].Energy;
		else
			return 0;
	}

	public final float getX(int Index)
	{
		return Players[Index].X;
	}

	public final float getY(int Index)
	{
		return Players[Index].Y;
	}

	public final int getKills(int Index)
	{
		return Players[Index].Kills;
	}

	public final int getDeaths(int Index)
	{
		return Players[Index].Deaths;
	}

	public final String getName(int Index)
	{
		return Players[Index].Name;
	}

	public final int getTeam(int Index)
	{
		return Players[Index].Team;
	}

	public final int getPing(int Index)
	{
		return Players[Index].Ping;
	}

	public final boolean getIgnore(int Index)
	{
		return Players[Index].Ignore;
	}

	public final long getLastUpdate(int Index)
	{
		return Players[Index].lastUpdate;
	}

	public final void sortPlayersByScore()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (PlayersByScore[i].inUse == true)
			{
				for (int j = MaxPlayers - 1; j >= 0; j--)
				{
					if (PlayersByScore[j].inUse == true)
					{
						if (j < i
								&& PlayersByScore[i].Score > PlayersByScore[j].Score)
						{
							Player swapPlayer = PlayersByScore[j];
							PlayersByScore[j] = PlayersByScore[i];
							PlayersByScore[i] = swapPlayer;
						}
					}
				}
			}
		}
	}

	public final Player[] getPlayerScores()
	{
		return PlayersByScore;
	}

	public final int getNextPlayer(int CurrentPlayer)
	{
		boolean NobodySelected = false;
		int ListPosition = CurrentPlayer;

		if (CurrentPlayer == -1)
		{
			ListPosition = 0;
		}

		int NewPlayer = -1;
		for (int i = ListPosition; i < MaxPlayers; i++)
		{
			if (i != game.gamePlayer.Index && Players[i].inUse == true
					&& Players[i].Team >= 0 && CurrentPlayer != i)
			{
				NewPlayer = i;
				break;
			}
		}

		if (NewPlayer == -1)
		{
			for (int i = 0; i < ListPosition; i++)
			{
				if (i != game.gamePlayer.Index && Players[i].inUse == true
						&& Players[i].Team >= 0 && CurrentPlayer != i)
				{
					NewPlayer = i;
					break;
				}
			}
		}

		if (NewPlayer == -1 && NobodySelected == false)
			NewPlayer = CurrentPlayer;

		return NewPlayer;
	}

	public final int getPreviousPlayer(int CurrentPlayer)
	{
		boolean NobodySelected = false;
		int ListPosition = CurrentPlayer;

		if (CurrentPlayer == -1)
		{
			NobodySelected = true;
			CurrentPlayer = 0;
		}

		int NewPlayer = -1;
		for (int i = ListPosition; i >= 0; i--)
		{
			if (i != game.gamePlayer.Index && Players[i].inUse == true
					&& Players[i].Team >= 0 && CurrentPlayer != i)
			{
				NewPlayer = i;
				break;
			}
		}

		if (NewPlayer == -1)
		{
			for (int i = MaxPlayers - 1; i > ListPosition; i--)
			{
				if (i != game.gamePlayer.Index && Players[i].inUse == true
						&& Players[i].Team >= 0 && CurrentPlayer != i)
				{
					NewPlayer = i;
					break;
				}
			}
		}

		if (NewPlayer == -1 && NobodySelected == false)
			NewPlayer = CurrentPlayer;

		return NewPlayer;
	}

	public final void setColor(int Index, int Red, int Green, int Blue,
			int Alpha)
	{
		Players[Index].blendRed = Red;
		Players[Index].blendGreen = Green;
		Players[Index].blendBlue = Blue;
		Players[Index].blendAlpha = Alpha;
	}

	public final void updateIgnoreList()
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].Name != null)
			{
				String Name = Players[i].Name.toLowerCase();
				if (game.gameIgnoreList.IgnoreList.contains(Name))
				{
					Players[i].Ignore = true;
				}
				else
				{
					Players[i].Ignore = false;
				}
			}
		}
	}

	public final int findPlayerByName(String Name)
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].inUse == true && Players[i].Name != null
					&& Players[i].Name.compareTo(Name) == 0)
			{
				return i;
			}
		}
		return -1;
	}

	public final int findPlayerByName(int Index, String Name)
	{
		// Check for full match
		for (int i = 0; i < MaxPlayers; i++)
		{
			String CompareName = Players[i].Name;
			if (CompareName != null
					&& CompareName.compareToIgnoreCase(Name) == 0)
			{
				return i;
			}
		}

		// Check for partial match
		int Size = Name.length();
		for (int i = 0; i < MaxPlayers; i++)
		{
			String CompareName = Players[i].Name;
			if (CompareName != null && CompareName.length() >= Size)
			{
				CompareName = CompareName.substring(0, Size);
				if (CompareName.compareToIgnoreCase(Name) == 0)
				{
					return i;
				}
			}
		}
		return -1;
	}

	public final void removeAllFromClan(int Index)
	{
		for (int i = 0; i < MaxPlayers; i++)
		{
			if (Players[i].Clan == Index)
			{
				Players[i].Clan = -1;
			}
		}
	}
}
