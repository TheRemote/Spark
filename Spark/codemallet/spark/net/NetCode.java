package codemallet.spark.net;

import java.io.FileInputStream;
import java.net.InetAddress;
import java.nio.BufferUnderflowException;
import java.nio.channels.ClosedChannelException;
import java.nio.charset.MalformedInputException;
import java.security.KeyStore;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.xsocket.connection.INonBlockingConnection;
import org.xsocket.connection.NonBlockingConnection;
import org.xsocket.datagram.Endpoint;
import org.xsocket.datagram.IEndpoint;
import org.xsocket.datagram.UserDatagram;

import codemallet.spark.Spark;

public final class NetCode
{
	private final Spark game;

	private INonBlockingConnection connectionTCP;

	private IEndpoint connectionUDP;

	private InetAddress serveraddress;

	public String IPAddress;

	public int PORT = 5760;

	private final NetProcessor netProcessor;

	// UDP
	public long ReceiveCounter = -1;

	public long GoodPackets;

	public long SendCounter;

	public float PacketLoss = 0;

	public int TCPPing = -1;

	public int UDPPing = -1;

	public int StoredPingStatus;

	public int AveragePing;

	long timePingUpdate;

	int UDPStrikes;

	long TCPSent;

	long UDPSent;

	public long TCPSpikeTimer;

	public static final float CriticalPacketLoss = 10.0f;

	public static final char Delim = 0;

	public int LagCompensation = 30;

	public final ArrayList<Integer> Pings = new ArrayList<Integer>();

	public NetCode(Spark game)
	{
		this.game = game;

		netProcessor = new NetProcessor(game);
	}

	public final void Initialize(String IPAddress, int Port)
	{
		this.IPAddress = IPAddress;
		this.PORT = Port;

		makeContact();
	}

	private final void makeContact()
	{
		try
		{
			try
			{
				serveraddress = InetAddress.getByName(IPAddress);
			}
			catch (Exception e)
			{
				game.throwException(e);
			}

			connectionTCP = new NonBlockingConnection(serveraddress, PORT,
					getSSLContext(), true);
			connectionTCP.setIdleTimeoutMillis(30000);
			connectionTCP.setOption(NonBlockingConnection.TCP_NODELAY, true);
			connectionTCP.setOption(NonBlockingConnection.SO_SNDBUF, 65536);
			connectionTCP.setOption(NonBlockingConnection.SO_RCVBUF, 65536);

			connectionUDP = new Endpoint(256);

			SendTCP("version " + Spark.Version + " " + game.gamePlayer.Name
					+ " " + game.Password + " " + game.gamePlayer.Clan);
		}
		catch (Exception e)
		{
			System.err.println("Unable to connect to the server - "
					+ e.getMessage());
			game.CloseMessage = "Unable to connect to the server!";
			game.destroy();
		}
	}

	public final void closeLink()
	{
		try
		{
			if (connectionUDP != null)
				connectionUDP.close();
		}
		catch (Exception e)
		{

		}
		connectionUDP = null;

		try
		{
			if (connectionTCP != null)
				connectionTCP.close();
		}
		catch (Exception e)
		{

		}
		connectionTCP = null;
	}

	public final void SendTCP(String Message)
	{
		try
		{
			connectionTCP.write(Message + "\r\n" + Delim);
		}
		catch (ClosedChannelException e)
		{
			System.err
					.println("TCP Send - Connection to the server was closed.");
			if (game.CloseMessage == null)
				game.CloseMessage = "Connection to the server was closed.";
			game.destroy();
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public final void SendUDP(final String line)
	{
		if (PacketLoss > CriticalPacketLoss)
		{
			SendTCP(line);
			return;
		}

		final String sendline = game.gamePlayer.Index + " " + SendCounter + " "
				+ line;
		SendCounter++;

		try
		{
			final UserDatagram packet = new UserDatagram(serveraddress, PORT,
					sendline.length());
			packet.write(sendline);
			connectionUDP.send(packet);
		}
		catch (Exception e)
		{
			if (connectionUDP != null)
			{
				try
				{
					connectionUDP.close();
				}
				catch (Exception ee)
				{

				}
			}
			game.gameNetCode.GoodPackets = 0;
			game.gameNetCode.PacketLoss = 100;
			game.gameNetCode.ReceiveCounter = 10000000;
			if (game.gameNetCode != null)
				game.gameNetCode.SendTCP("disableUDP");
			e.printStackTrace();
			System.err.println("Client-Side UDP Disabled - UDP Error: "
					+ e.toString());
		}
	}

	public final void SendTCPPing()
	{
		if (game.gameNetLinkLobby != null)
			game.gameNetLinkLobby.SendTCP("ping");
		SendTCP("tcpping");
		TCPSent = game.Tick;
	}

	public final void SendUDPPing()
	{
		if (PacketLoss >= CriticalPacketLoss)
		{
			return;
		}

		SendUDP("ping");
		UDPSent = game.Tick;
	}

	public final void ReceiveTCPPing()
	{
		TCPPing = (int) (game.Tick - TCPSent);
		game.gamePlayers.setPing(game.gamePlayer.Index, TCPPing);

		if (TCPPing > 1000)
		{
			if (game.Tick > TCPSpikeTimer)
			{
				TCPSpikeTimer = game.Tick + 30000;
			}
			else
			{
				Pings.add(TCPPing);
			}
		}
		else
		{
			Pings.add(TCPPing);
		}

		AveragePing = 0;

		final Iterator<Integer> iter = Pings.iterator();
		while (iter.hasNext())
		{
			int IterPing = iter.next();
			AveragePing = AveragePing + IterPing;
		}
		final int Size = Pings.size();

		if (Size > 0)
		{
			AveragePing = AveragePing / Pings.size();
		}
		else
		{
			AveragePing = 0;
		}

		if (Size > 15)
		{
			if (Size > 100)
			{
				Pings.remove(0);
			}

			if (AveragePing > 700)
			{
				System.err.println("Average ping is too high: " + AveragePing
						+ " ms!");
				game.CloseMessage = "Your ping is too high: " + AveragePing
						+ " ms!";
				game.destroy();
			}
		}

		int PingStatus = getPingStatus(AveragePing);
		if (game.Tick > timePingUpdate && PingStatus != StoredPingStatus)
		{
			timePingUpdate = game.Tick + 15000;
			StoredPingStatus = PingStatus;
			if (game.gameNetLinkLobby != null)
				game.gameNetLinkLobby.SendTCP("updateping " + AveragePing);
		}
	}

	public final void ReceiveUDPPing()
	{
		long Total = (game.Tick - UDPSent);
		UDPPing = (int) Total;
		if (UDPPing > TCPPing)
		{
			int Difference = UDPPing - TCPPing;

			if (Difference > 50)
			{
				UDPStrikes++;
				if (UDPStrikes > 10)
				{
					if (game.gameNetCode != null)
					{
						game.gameNetCode.GoodPackets = 0;
						game.gameNetCode.PacketLoss = 100;
						game.gameNetCode.UDPPing = -1;
						System.err
								.println("Client-Side UDP Disabled - Pings Too High");
						game.gameNetCode.SendTCP("disableUDP");
					}
				}
			}
			else
				UDPStrikes = 0;
		}
		else
			UDPStrikes = 0;
	}

	public final int getPingStatus(long Ping)
	{
		if (Ping < 0)
		{
			return 1;
		}
		else if (Ping < 100)
		{
			return 2;
		}
		else if (Ping < 200)
		{
			return 3;
		}
		else if (Ping < 300)
		{
			return 4;
		}

		return 5;
	}

	public final int getPingIcon(long Ping)
	{
		if (Ping < 100)
		{
			return 0;
		}
		else if (Ping < 200)
		{
			return 1;
		}
		else if (Ping < 300)
		{
			return 2;
		}

		return 3;
	}

	public final void readTCPData()
	{
		String line = null;
		try
		{
			if (connectionTCP != null)
			{
				while (true)
				{
					line = connectionTCP.readStringByDelimiter("\r\n" + Delim);

					if (line != null && line.length() > 0)
					{
						netProcessor.doCommand(line);
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
			System.err
					.println("TCP Receive - Connection to the server was closed.");
			game.CloseMessage = "Connection to the server was closed.";
			game.destroy();
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public final void readUDPData()
	{
		try
		{
			if (connectionUDP != null)
			{
				while (true)
				{
					final UserDatagram packet = connectionUDP.receive();
					if (packet != null)
					{
						String line = packet.readString().trim();

						final StringTokenizer st = new StringTokenizer(line);
						final String Token = st.nextToken();
						int ReceivedCount = Integer.parseInt(Token);

						line = line.substring(Token.length() + 1);
						if (ReceiveCounter + 1 == ReceivedCount)
						{
							GoodPackets++;
							ReceiveCounter = ReceivedCount;
						}
						else
						{
							ReceiveCounter = ReceivedCount;
							PacketLoss = 100.0f - (((float) GoodPackets / (float) ReceivedCount) * 100.0f);
							if (ReceiveCounter > 100)
							{
								if (PacketLoss > CriticalPacketLoss)
								{
									game.gameNetCode.GoodPackets = 0;
									game.gameNetCode.PacketLoss = 100;
									game.gameNetCode.UDPPing = -1;
									System.err
											.println("Client-Side UDP disabled - packet loss too high - "
													+ PacketLoss
													+ " - Good Packets: "
													+ GoodPackets
													+ " - Total Packets: "
													+ ReceiveCounter);
									game.gameNetCode.SendTCP("disableUDP");
								}
							}
						}

						if (line != null && line.length() > 0)
						{
							netProcessor.doCommand(line);
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
		catch (MalformedInputException e)
		{
			System.err.println("Warning: a malformed UDP packet was dropped.");
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public static SSLContext getSSLContext()
	{
		try
		{
			String filename = "Keystore";
			String PASSWORD = "secret";

			char[] passphrase = PASSWORD.toCharArray();
			KeyStore ks = KeyStore.getInstance("JKS");
			ks.load(new FileInputStream(filename), passphrase);

			KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
			kmf.init(ks, passphrase);

			TrustManagerFactory tmf = TrustManagerFactory
					.getInstance("SunX509");
			tmf.init(ks);

			SSLContext sslContext = SSLContext.getInstance("TLS");
			sslContext.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);

			return sslContext;

		}
		catch (Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
}
