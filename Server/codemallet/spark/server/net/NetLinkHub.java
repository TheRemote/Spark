package codemallet.spark.server.net;

import java.io.FileInputStream;
import java.io.InputStream;
import java.net.InetAddress;
import java.nio.BufferUnderflowException;
import java.nio.channels.ClosedChannelException;
import java.util.StringTokenizer;

import org.xsocket.connection.INonBlockingConnection;
import org.xsocket.connection.NonBlockingConnection;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.map.Map;
import codemallet.spark.server.player.Player;

public final class NetLinkHub
{
	private final SparkServer server;

	private INonBlockingConnection connection;

	private InetAddress serveraddress;

	public NetLinkHub(SparkServer server)
	{
		this.server = server;
	}

	public final void Initialize()
	{
		makeContact();
	}

	private final void makeContact()
	{
		if (server.Debug == false)
		{
			try
			{
				if (server.IPAddress != null && server.IPAddress.length() > 0)
					serveraddress = InetAddress.getByName(server.IPAddress);
				else
					serveraddress = InetAddress.getByName("localhost");
				connection = new NonBlockingConnection(serveraddress, 5758);
				connection.setIdleTimeoutMillis(120000);
			}
			catch (Exception e)
			{
				e.printStackTrace();
				System.err.println("IP Address: " + server.IPAddress);
				System.err.println("Unable to connect to the hub server!");
				server.Close();
			}
		}
	}

	public final void Close()
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
			System.err
					.println("NetLinkHub is closed (SendTCP).  Shutting down server...");
			server.Close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public final void readTCPData()
	{
		try
		{
			if (connection != null)
			{
				while (true)
				{
					final String line = connection.readStringByDelimiter("\r\n"
							+ NetCode.Delim);
					if (line != null && line.length() > 0)
					{
						try
						{
							doCommand(line);
						}
						catch (Exception e)
						{
							e.printStackTrace();
						}
					}
					else
					{
						break;
					}
				}
			}
		}
		catch (BufferUnderflowException e)
		{

		}
		catch (ClosedChannelException e)
		{
			server.Close();
			System.err
					.println("NetLinkHub is closed (Receive).  Shutting down server...");
			return;
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public final void doCommand(String line)
	{
		if (line.equals("outdated"))
		{
			processOutdated();
		}
		else if (line.equals("ping"))
		{
			SendTCP("pong");
		}
		else if (line.equals("pong"))
		{
			processPong();
		}
		else if (line.startsWith("sparkServerLock "))
		{
			processSparkServerLock(line);
		}
		else if (line.startsWith("sparkServerUnlock "))
		{
			processSparkServerUnlock(line);
		}
		else if (line.startsWith("sparkServerStart "))
		{
			processSparkServerStart(line);
		}
		else if (line.startsWith("sparkServerRestart "))
		{
			processSparkServerRestart(line);
		}
		else if (line.startsWith("sparkServerKick "))
		{
			processSparkServerKick(line);
		}
		else if (line.startsWith("sparkServerChangeMap "))
		{
			processSparkServerChangeMap(line);
		}
		else
		{
			System.err.println("Unrecognized packet: " + line);
		}
	}

	private final void processOutdated()
	{
		System.err.println("Spark server is outdated!");
		server.Close();
	}

	private final void processSparkServerLock(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		server.engine.Locked = true;
		server.players.holdPenAll();

		String Player = st.nextToken();
		System.err.println("(ADMIN) Server has been locked by " + Player);
	}

	private final void processSparkServerUnlock(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		server.engine.Locked = false;
		server.players.sendGame(-1, "Unlocked");

		String Player = st.nextToken();
		System.err.println("(ADMIN) Server has been unlocked by " + Player);
	}

	private final void processSparkServerStart(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		server.map = new Map(server, server.map.MapFile);
		server.players.ChangeMap();
		server.players.holdPenAll();

		String Player = st.nextToken();
		System.err.println("(ADMIN) Server has been started by " + Player);
	}

	private final void processSparkServerRestart(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		server.TeamScores[0] = 0;
		server.TeamScores[1] = 0;
		server.TeamScores[2] = 0;
		server.TeamScores[3] = 0;

		server.players.sendGame(-1, "TeamScores " + server.TeamScores[0] + " "
				+ server.TeamScores[1] + " " + server.TeamScores[2] + " "
				+ server.TeamScores[3]);

		server.players.ResetGame();
		server.map = new Map(server, server.map.MapFile);
		server.players.ChangeMap();

		String Player = st.nextToken();
		System.err.println("(ADMIN) Server has been restarted by " + Player);
	}

	private final void processSparkServerKick(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		int PlayerIndex = Integer.parseInt(st.nextToken());
		Player player = server.players.getPlayer(PlayerIndex);
		String Admin = st.nextToken();
		String Reason = st.nextToken();
		while (st.hasMoreTokens())
		{
			Reason = Reason + " " + st.nextToken();
		}
		if (player.inUse == true)
		{
			server.players.sendTo(PlayerIndex, "kick " + Admin + " " + Reason);
			System.err
					.println("(ADMIN) " + player.Name + " has been kicked by "
							+ Admin + " for reason: " + Reason);
			server.players.CloseConnection(PlayerIndex);
		}
	}

	private final void processPong()
	{

	}

	private final void processSparkServerChangeMap(String line)
	{
		StringTokenizer st = new StringTokenizer(line);
		st.nextToken();

		try
		{
			final String Map = st.nextToken();
			final int PlayerIndex = Integer.parseInt(st.nextToken());
			final Player player = server.players.getPlayer(PlayerIndex);

			try
			{
				InputStream in = new FileInputStream("spark/maps/" + Map);
				in.close();

				server.players.ResetGame();
				server.map = new Map(server, Map);
				server.players.ChangeMap();

				server.netLinkHub.SendTCP("changeMap " + Map);

				System.err.println("(ADMIN) " + player.Name
						+ " has changed the map to " + Map);
			}
			catch (Exception e)
			{
				if (player.inUse == true)
				{
					server.players.sendTo(PlayerIndex, "message Map " + Map
							+ " was not found.");
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
