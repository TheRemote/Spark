package codemallet.spark.server.net;

import java.io.FileInputStream;
import java.net.InetAddress;
import java.nio.BufferUnderflowException;
import java.nio.channels.ClosedChannelException;
import java.security.KeyStore;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.xsocket.connection.ConnectionUtils;
import org.xsocket.connection.IServer;
import org.xsocket.connection.NonBlockingConnection;
import org.xsocket.connection.Server;
import org.xsocket.datagram.Endpoint;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.player.Player;

public final class NetCode
{
	private final SparkServer server;

	private IServer xSocketServer;

	public Endpoint connectionUDP;

	private InetAddress address;

	public static char Delim = 0;

	public NetCode(SparkServer server)
	{
		this.server = server;
	}

	public final void Initialize()
	{
		try
		{
			if (server.IPAddress != null && server.IPAddress.length() > 0)
			{
				try
				{
					address = InetAddress.getByName(server.IPAddress);
					xSocketServer = new Server(address, server.Port,
							new NetHandler(server), getSSLContext(), true);
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
			}
			else
			{
				xSocketServer = new Server(server.Port, new NetHandler(server));
			}
			xSocketServer.setIdleTimeoutMillis(30000);
			ConnectionUtils.start(xSocketServer);

			connectionUDP = new Endpoint(256, server.netudp, address,
					server.Port);

			server.netPingPort = new NetPingPort(address, server.PingPort);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public final void Close()
	{
		try
		{
			xSocketServer.close();
		}
		catch (Exception e)
		{

		}
		try
		{
			connectionUDP.close();
		}
		catch (Exception e)
		{

		}
	}

	public final void ProcessQueue()
	{
		while (true)
		{
			final NetEvent event = server.serverPacketQueue.poll();
			if (event != null)
			{
				switch (event.Type)
				{
					case 0:
					{
						try
						{
							final Player player = (Player) event.con
									.getAttachment();
							if (player != null)
							{
								while (true)
								{
									final String line = event.con
											.readStringByDelimiter("\r\n"
													+ NetCode.Delim);

									if (line != null)
									{
										server.netProcessor.doRequest(line,
												player);
									}
									else
									{
										break;
									}
								}
							}
							else
							{
								System.err
										.println("Connection's player is null - closing connection");
								try
								{
									event.con.close();
								}
								catch (Exception e)
								{

								}
							}
						}
						catch (BufferUnderflowException e)
						{

						}
						catch (ClosedChannelException e)
						{
							try
							{
								event.con.close();
							}
							catch (Exception ee)
							{

							}
						}
						catch (Exception e)
						{
							try
							{
								event.con.close();
							}
							catch (Exception ee)
							{

							}
							e.printStackTrace();
						}
					}
						break;
					case 1:
					{
						final Player player = (Player) event.con
								.getAttachment();

						if (player != null)
						{
							if (player.Name != null)
							{
								System.err.println("ConnectionTimeout: "
										+ player.Name + " - "
										+ player.IPAddress);
							}
							else
							{
								System.err.println("ConnectionTimeout: "
										+ player.IPAddress);
							}
							server.players.delPerson(player.Index);

							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
						else
						{
							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
					}
						break;
					case 2:
					{
						final Player player = (Player) event.con
								.getAttachment();

						if (player != null)
						{
							if (player.Name != null)
							{
								System.err.println("IdleTimeout: "
										+ player.Name + " - "
										+ player.IPAddress);
							}
							else
							{
								System.err.println("IdleTimeout: "
										+ player.IPAddress);
							}
							server.players.delPerson(player.Index);

							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
						else
						{
							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
					}
						break;
					case 3:
					{
						final Player player = (Player) event.con
								.getAttachment();

						if (player != null)
						{
							if (player.Name != null)
							{
								System.err.println("Disconnected: "
										+ player.Name + " - "
										+ player.IPAddress);
							}
							else
							{
								System.err.println("Disconnected: "
										+ player.IPAddress);
							}
							server.players.delPerson(player.Index);

							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
						else
						{
							try
							{
								event.con.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
					}
						break;
					case 4:
					{
						try
						{
							event.con.setIdleTimeoutMillis(20000);
							event.con.setOption(
									NonBlockingConnection.TCP_NODELAY, true);
							event.con.setOption(
									NonBlockingConnection.SO_SNDBUF, 65536);
							event.con.setOption(
									NonBlockingConnection.SO_RCVBUF, 65536);
							Player player = server.players.addPerson(event.con);
							if (player != null)
							{
								InetAddress address = event.con
										.getRemoteAddress();
								player.IPAddress = address.getHostAddress();
								event.con.setAttachment(player);
								System.err.println("Connection from "
										+ player.IPAddress);
							}
							else
							{
								System.err
										.println("Could not accept new connection, server is full.");
								event.con.close();
							}
						}
						catch (Exception e)
						{
							e.printStackTrace();
						}
					}
						break;
					case 5:
					{
						server.netProcessor.doRequest(event.line,
								event.localPlayer);
					}
						break;
				}
			}
			else
			{
				break;
			}
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
