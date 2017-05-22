package codemallet.spark.server.net;

import org.xsocket.connection.IConnectHandler;
import org.xsocket.connection.IConnectionTimeoutHandler;
import org.xsocket.connection.IDataHandler;
import org.xsocket.connection.IDisconnectHandler;
import org.xsocket.connection.IIdleTimeoutHandler;
import org.xsocket.connection.INonBlockingConnection;

import codemallet.spark.server.SparkServer;

public final class NetHandler implements IDataHandler, IConnectHandler,
		IDisconnectHandler, IIdleTimeoutHandler, IConnectionTimeoutHandler
{
	private final SparkServer server;

	public NetHandler(SparkServer server)
	{
		this.server = server;
	}

	public final boolean onData(INonBlockingConnection con)
	{
		final NetEvent event = new NetEvent(con);
		server.serverPacketQueue.add(event);

		return true;
	}

	public final boolean onConnectionTimeout(INonBlockingConnection con)
	{
		final NetEvent event = new NetEvent(con, 1);
		server.serverPacketQueue.add(event);

		return true;
	}
	public final boolean onIdleTimeout(INonBlockingConnection con)
	{

		final NetEvent event = new NetEvent(con, 2);
		server.serverPacketQueue.add(event);

		return true;
	}

	public final boolean onDisconnect(INonBlockingConnection con)
	{
		final NetEvent event = new NetEvent(con, 3);
		server.serverPacketQueue.add(event);

		return true;
	}

	public final boolean onConnect(INonBlockingConnection con)
	{
		final NetEvent event = new NetEvent(con, 4);
		server.serverPacketQueue.add(event);

		return true;
	}
}
