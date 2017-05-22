package codemallet.spark.server.net;

import org.xsocket.connection.INonBlockingConnection;

import codemallet.spark.server.player.Player;

public class NetEvent
{
	public int Type;

	public INonBlockingConnection con;
	
	public String line;
	public Player localPlayer;

	public NetEvent(INonBlockingConnection con)
	{
		this.con = con;
		this.Type = 0;
	}

	public NetEvent(INonBlockingConnection con, int Type)
	{
		this.con = con;
		this.Type = Type;
	}
	
	public NetEvent(Player localPlayer, String line)
	{
		this.localPlayer = localPlayer;
		this.line = line;
		this.Type = 5;
	}
}
