package codemallet.spark.net;

import java.util.StringTokenizer;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class NetLinkLobbyProcessor
{
	private final Spark game;

	NetLinkLobbyProcessor(Spark game)
	{
		this.game = game;
	}

	public final void doCommand(final String line)
	{
		if (line.startsWith("pm "))
		{
			game.gameChat.AddMessage(line.substring(3),
					GameResources.colorPrivateMessage);
		}
		else if (line.startsWith("message "))
		{
			game.gameChat.AddMessage(line.substring(8),
					GameResources.colorWhite);
		}
		else if (line.startsWith("banned"))
		{
			System.err.println("Player has been banned");
			game.destroy();
		}
		else if (line.startsWith("updateIgnoreList"))
		{
			game.gameIgnoreList.LoadIgnoreList();
			game.gamePlayers.updateIgnoreList();
		}
		else if (line.startsWith("playerStatus "))
		{
			processPlayerStatus(line);
		}
		else if (line.startsWith("clan "))
		{
			processClan(line);
		}
		else if (line.startsWith("destroyClan "))
		{
			processDestroyClan(line);
		}
	}

	private final void processPlayerStatus(final String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		String Name = st.nextToken();
		int Ping = Integer.parseInt(st.nextToken());

		int Index = game.gamePlayers.findPlayerByName(Name);
		if (Index != -1)
		{
			game.gamePlayers.setPing(Index, Ping);
		}
	}

	private final void processClan(final String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		final int Index = Integer.parseInt(st.nextToken());
		final String Tag = st.nextToken();
		final boolean TagAfterName = Boolean.parseBoolean(st.nextToken());
		final boolean TagSpaceBetween = Boolean.parseBoolean(st.nextToken());
		String Name = st.nextToken();
		while (st.hasMoreTokens())
		{
			Name = Name + " " + st.nextToken();
		}

		game.gameClans.LoadClan(Index, Name, Tag, TagAfterName,
				TagSpaceBetween);
	}
	
	private final void processDestroyClan(final String line)
	{
		final StringTokenizer st = new StringTokenizer(line);
		st.nextToken();
		
		int Index = Integer.parseInt(st.nextToken());
		game.gameClans.DestroyClan(Index);
	}
}
