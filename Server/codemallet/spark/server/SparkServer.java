package codemallet.spark.server;

import java.util.Random;
import java.util.concurrent.ConcurrentLinkedQueue;

import codemallet.spark.server.map.Map;
import codemallet.spark.server.net.NetCode;
import codemallet.spark.server.net.NetEvent;
import codemallet.spark.server.net.NetLinkHub;
import codemallet.spark.server.net.NetPingPort;
import codemallet.spark.server.net.NetProcessor;
import codemallet.spark.server.net.NetUDP;
import codemallet.spark.server.player.PlayerList;

public class SparkServer
{
	public static final String Version = "1.41";

	public int Port;

	public int MaxTotalPlayers;

	public int MaxPlayers;

	public int MaxSpectators;

	public int PingPort;

	public int LaunchID;

	public String Password;

	public final PlayerList players = new PlayerList(this);

	public final ServerEngine engine = new ServerEngine(this);

	public final NetLinkHub netLinkHub = new NetLinkHub(this);

	public final NetProcessor netProcessor = new NetProcessor(this);

	public final NetUDP netudp = new NetUDP(this);

	public NetPingPort netPingPort;

	public final NetCode netServer = new NetCode(this);

	public final DateFormatting dateFormatting = new DateFormatting();

	public final Random rand = new Random();

	public Map map;

	public boolean running;

	public long Tick;

	public int TeamScores[] = new int[4];

	public String IPAddress;

	public boolean Debug;

	public final ConcurrentLinkedQueue<NetEvent> serverPacketQueue = new ConcurrentLinkedQueue<NetEvent>();

	public SparkServer(int Port, int PingPort, int Resolution, String Map,
			int MaxPlayers, int MaxSpectators, String Password, int LaunchID,
			String IPAddress, boolean Debug)
	{
		this.Port = Port;
		this.PingPort = PingPort;
		this.MaxPlayers = MaxPlayers;
		this.MaxSpectators = MaxSpectators;
		this.MaxTotalPlayers = MaxPlayers + MaxSpectators;
		this.Password = Password;
		this.LaunchID = LaunchID;
		this.IPAddress = IPAddress;
		this.Debug = Debug;
		running = true;

		map = new Map(this, Map);

		if (running == false)
		{
			Close();
			return;
		}

		try
		{
			netLinkHub.Initialize();
			netLinkHub.SendTCP("version " + SparkServer.Version);
			netLinkHub.SendTCP("" + LaunchID);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			Close();
			return;
		}

		if (running == false)
		{
			Close();
			return;
		}

		try
		{
			netServer.Initialize();
		}
		catch (Exception e)
		{
			e.printStackTrace();
			Close();
		}

		if (running == false)
		{
			Close();
			return;
		}

		players.Initialize();

		engine.start();
	}

	public static void main(String args[])
	{
		try
		{
			int Port = Integer.parseInt(args[0]);
			int PingPort = Integer.parseInt(args[1]);
			int Resolution = Integer.parseInt(args[2]);
			String Map = args[3];
			int MaxPlayers = Integer.parseInt(args[4]);
			int Spectators = Integer.parseInt(args[5]);
			String Password = args[6];
			int LaunchID = Integer.parseInt(args[7]);
			String IPAddress = args[8];
			boolean Debug = false;
			if (args.length > 9)
				Debug = true;

			new SparkServer(Port, PingPort, Resolution, Map, MaxPlayers,
					Spectators, Password, LaunchID, IPAddress, Debug);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return;
		}
	}

	public void Close()
	{
		running = false;

		players.DisconnectAllPlayers();

		if (netServer != null)
			netServer.Close();
		if (netLinkHub != null)
			netLinkHub.Close();

		try
		{
			netPingPort.Close();
		}
		catch (Exception e)
		{

		}
		
		System.exit(0);
	}
}
