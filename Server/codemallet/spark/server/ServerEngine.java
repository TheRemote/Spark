package codemallet.spark.server;

import codemallet.spark.server.map.Flag;
import codemallet.spark.server.map.FlagPole;

public final class ServerEngine extends Thread
{
	private final SparkServer server;

	long timeShutdown;

	long timeCheckShutdown;

	long timeFlags;

	long timeHubPing;

	long timePlayers;

	public boolean Locked = false;

	ServerEngine(SparkServer server)
	{
		this.server = server;
	}

	public final void run()
	{
		while (server.running == true)
		{
			try
			{
				server.Tick = System.currentTimeMillis();

				server.netServer.ProcessQueue();
				
				server.Tick = System.currentTimeMillis();

				if (server.Tick > timePlayers)
				{
					timePlayers = server.Tick + 50;
					server.players.Cycle();
				}

				if (server.Tick > timeFlags)
				{
					timeFlags = timeFlags + 500;
					final int FlagSize = server.map.Flags.size();
					for (int i = 0; i < FlagSize; i++)
					{
						final Flag flag = server.map.Flags.get(i);
						if (flag.onGround == true)
						{
							if (flag.Team < 4)
							{
								if (server.Tick > flag.timeReturn)
								{
									for (int j = 0; j < server.map.FlagPoles
											.size(); j++)
									{
										final FlagPole pole = server.map.FlagPoles
												.get(j);
										if (pole.hasFlag == false
												&& pole.Team == flag.Team
												&& pole.FlagType == flag.Team)
										{
											pole.hasFlag = true;
											pole.FlagIndex = i;

											flag.onPole = true;
											flag.onGround = false;
											flag.onPlayer = false;
											flag.PoleIndex = j;

											server.players.sendGame(-1, "flag "
													+ pole.FlagIndex + " " + 8
													+ " " + j);
											break;
										}
									}
								}
							}
						}
					}
				}

				if (server.Tick > timeCheckShutdown)
				{
					timeCheckShutdown = server.Tick + 30000;
					int i = server.players.getPlayerTotalCount();
					if (i == 0)
					{
						if (timeShutdown == 0)
						{
							timeShutdown = server.Tick + 190000;
						}
					}
					else
					{
						timeShutdown = 0;
					}
				}

				if (timeShutdown > 0)
				{
					if (server.Tick > timeShutdown)
					{
						System.err
								.println("Server has been empty for too long.  Shutting down...");
						server.Close();
					}
				}

				if (server.Debug == false)
				{
					server.netLinkHub.readTCPData();

					if (server.Tick > timeHubPing)
					{
						timeHubPing = server.Tick + 30000;
						server.netLinkHub.SendTCP("ping");
					}
				}
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}

			try
			{
				Thread.sleep(1);
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}
	}
}
