package codemallet.spark.server.net;

import java.util.StringTokenizer;

import org.xsocket.datagram.IDatagramHandler;
import org.xsocket.datagram.IEndpoint;
import org.xsocket.datagram.UserDatagram;

import codemallet.spark.server.SparkServer;
import codemallet.spark.server.player.Player;

public final class NetUDP implements IDatagramHandler
{
	private final SparkServer server;

	public NetUDP(SparkServer server)
	{
		this.server = server;
	}

	public final boolean onDatagram(IEndpoint endpoint)
	{
		try
		{
			final UserDatagram packet = endpoint.receive();
			if (packet != null)
			{
				final String line = packet.readString().trim();
				if (line != null)
				{
					final StringTokenizer st = new StringTokenizer(line);
					final String player = st.nextToken();

					final int playerIndex = Integer.parseInt(player);
					final Player localPlayer = server.players
							.getPlayer(playerIndex);

					if (localPlayer.packet == null)
					{
						localPlayer.packet = new UserDatagram(packet
								.getRemoteAddress(), packet.getRemotePort(),
								256);
						localPlayer.sendEstablishUDP();
					}

					final String counter = st.nextToken();
					final long receivedcount = Long.parseLong(counter);
					if (receivedcount == localPlayer.ReceiveCounter + 1)
						localPlayer.GoodPackets++;

					localPlayer.ReceiveCounter = receivedcount;

					final String ProcessLine = line.substring(player.length() + counter.length()
							+ 2);

					final NetEvent event = new NetEvent(localPlayer, ProcessLine);
					server.serverPacketQueue.add(event);
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		return true;
	}
}
