package codemallet.spark.net;

import java.net.InetAddress;
import java.nio.BufferUnderflowException;
import java.nio.channels.ClosedChannelException;

import org.xsocket.connection.INonBlockingConnection;
import org.xsocket.connection.NonBlockingConnection;

import codemallet.spark.Spark;

public final class NetLinkLobby
{
	private final Spark game;

	private INonBlockingConnection connection;

	private InetAddress serveraddress;

	private final NetLinkLobbyProcessor netLinkLobbyProcessor;

	public NetLinkLobby(Spark game)
	{
		this.game = game;
		netLinkLobbyProcessor = new NetLinkLobbyProcessor(game);
	}

	public final void Initialize()
	{
		makeContact();
	}

	public final void makeContact()
	{
		if (game.Debug == false)
		{
			try
			{
				serveraddress = InetAddress.getByName("127.0.0.1");
				connection = new NonBlockingConnection(serveraddress, 5041);
				connection.setIdleTimeoutMillis(120000);
				connection.setOption(NonBlockingConnection.TCP_NODELAY, true);
				connection.setOption(NonBlockingConnection.SO_SNDBUF, 65536);
				connection.setOption(NonBlockingConnection.SO_RCVBUF, 65536);

				if (game.gamePlayer.Clan > -1)
				{
					SendTCP("getClan " + game.gamePlayer.Clan);
				}
			}
			catch (Exception e)
			{
				e.printStackTrace();
				System.err.println("Unable to connect to the lobby!");
				game.CloseMessage = "Unable to connect to the lobby!";
				game.destroy();
			}
		}
	}

	public final void closeLink()
	{
		try
		{
			if (connection != null)
				connection.close();
		}
		catch (Exception e)
		{

		}
	}

	public final void updateStats()
	{
		SendTCP("updatestats "
				+ game.gamePlayers.getTeam(game.gamePlayer.Index) + " "
				+ game.gamePlayers.Players[game.gamePlayer.Index].Score + " "
				+ game.gamePlayers.getKills(game.gamePlayer.Index) + " "
				+ game.gamePlayers.getDeaths(game.gamePlayer.Index));

	}

	public final void SendTCP(String Message)
	{
		try
		{
			if (connection != null)
			{
				connection.write(Message + "\r\n" + NetCode.Delim);
			}
		}
		catch (ClosedChannelException e)
		{
			System.err.println("Connection to the lobby was closed.");
			game.CloseMessage = "Connection to the lobby was closed.";
			game.destroy();
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public final void readTCPData()
	{
		try
		{
			if (connection != null)
			{
				String line = null;

				line = connection.readStringByDelimiter("\r\n" + NetCode.Delim);

				if (line != null && line.length() > 0)
				{
					netLinkLobbyProcessor.doCommand(line);
				}
			}
		}
		catch (BufferUnderflowException e)
		{

		}
		catch (ClosedChannelException e)
		{
			System.err.println("Connection to the lobby was closed.");
			game.CloseMessage = "Connection to the lobby was closed.";
			game.destroy();
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}
}
