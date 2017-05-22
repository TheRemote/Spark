package codemallet.spark.chat;

import static codemallet.spark.engines.GameResources.colorWhite;

import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.StringTokenizer;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;
import codemallet.spark.player.Player;
import codemallet.spark.player.PlayerList;

public final class Chat
{
	private final Spark game;

	private String ChatString = "";

	private final ArrayList<ChatMessage> Messages = new ArrayList<ChatMessage>();

	public boolean DrawPlayerList;

	private static final String Help = "Press F2 for Help and Game Information";

	public Chat(Spark game)
	{
		this.game = game;
	}

	public void drawChat()
	{
		int ChatY = 0;
		int MessageCount = 0;

		if (Messages.size() > 8)
		{
			ChatY = 7 * game.gameEngine.getFontHeight() + 2;
		}
		else
		{
			ChatY = (Messages.size() - 1) * game.gameEngine.getFontHeight() + 2;
		}

		ListIterator<ChatMessage> iterator = Messages.listIterator();
		while (iterator.hasNext())
		{
			ChatMessage msg = (ChatMessage) iterator.next();

			if (game.Tick < msg.timeExpiration)
			{
				if (msg.Parts == 0)
				{
					if (game.gameOptions.fontChatShadow == true)
					{
						game.gameEngine.setColor(GameResources.colorBlack);
						game.gameEngine.drawString(msg.Message, 3 + 1,
								ChatY + 1);
					}
					game.gameEngine.setColor(msg.color);
					game.gameEngine.drawString(msg.Message, 3, ChatY);
				}
				else
				{
					int xpos = 3;
					if (game.gameOptions.fontChatShadow == true)
					{
						game.gameEngine.setColor(GameResources.colorBlack);
						game.gameEngine.drawString(msg.Part1, xpos + 1,
								ChatY + 1);
					}
					game.gameEngine.setColor(msg.color1);
					game.gameEngine.drawString(msg.Part1, xpos, ChatY);

					xpos += msg.Length1;
					if (game.gameOptions.fontChatShadow == true)
					{
						game.gameEngine.setColor(GameResources.colorBlack);
						game.gameEngine.drawString(msg.Part2, xpos + 1,
								ChatY + 1);
					}
					game.gameEngine.setColor(msg.color2);
					game.gameEngine.drawString(msg.Part2, xpos, ChatY);

					if (msg.Parts > 2)
					{
						xpos += msg.Length2;
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(msg.Part3, xpos + 1,
									ChatY + 1);
						}
						game.gameEngine.setColor(msg.color3);
						game.gameEngine.drawString(msg.Part3, xpos, ChatY);
					}
					if (msg.Parts > 3)
					{
						xpos += msg.Length3;
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(msg.Part4, xpos + 1,
									ChatY + 1);
						}
						game.gameEngine.setColor(msg.color4);
						game.gameEngine.drawString(msg.Part4, xpos, ChatY);
					}
				}
			}
			else
			{
				iterator.remove();
			}
			ChatY -= game.gameEngine.getFontHeight();
			MessageCount++;

			if (MessageCount > 7)
			{
				break;
			}
		}

		int xpos = 3;
		if (ChatString.length() > 0)
		{
			if (game.gameInput.ChatToggle == true)
			{
				if (game.gameOptions.fontChatShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString("> " + ChatString, xpos + 1,
							game.ResolutionY - game.gameEngine.getFontHeight()
									- 2);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawString("> " + ChatString, xpos,
						game.ResolutionY - game.gameEngine.getFontHeight() - 3);
			}
			else
			{
				if (game.gameOptions.fontChatShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString(ChatString, xpos + 1,
							game.ResolutionY - game.gameEngine.getFontHeight()
									- 2);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawString(ChatString, xpos, game.ResolutionY
						- game.gameEngine.getFontHeight() - 3);
			}
		}
		else
		{
			if (game.gameInput.ChatToggle == true)
			{
				if (game.gameOptions.fontChatShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString("> ", xpos + 1, game.ResolutionY
							- game.gameEngine.getFontHeight() - 2);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawString("> ", xpos, game.ResolutionY
						- game.gameEngine.getFontHeight() - 3);
			}
		}

		if (game.gamePlayer.HoldingPen == true)
		{
			String Time = "";
			if (game.Locked == false)
			{
				int HoldingTime = (int) ((game.gamePlayer.timeHoldingPen - game.Tick) / 1000);
				if (HoldingTime >= 0)
				{
					Time = "Holding Time Left:  " + HoldingTime;
				}
				else
				{
					Time = "Move to Exit the Holding Pen!";
				}
			}
			else
			{
				Time = "The game has been locked by an admin!";
			}

			xpos = game.ResolutionX / 2
					- (game.gameEngine.getStringWidth(Help) / 2);
			if (game.gameOptions.fontChatShadow == true)
			{
				game.gameEngine.setColor(GameResources.colorBlack);
				game.gameEngine.drawString(Help, xpos + 1,
						game.ResolutionY / 2 - 100 + 1);
			}
			game.gameEngine.setColor(GameResources.colorWhite);
			game.gameEngine.drawString(Help, xpos, game.ResolutionY / 2 - 100);
			xpos = game.ResolutionX / 2
					- (game.gameEngine.getStringWidth(Time) / 2);
			if (game.gameOptions.fontChatShadow == true)
			{
				game.gameEngine.setColor(GameResources.colorBlack);
				game.gameEngine.drawString(Time, xpos + 1,
						game.ResolutionY / 2 - 70 + 1);
			}
			game.gameEngine.setColor(GameResources.colorWhite);
			game.gameEngine.drawString(Time, xpos, game.ResolutionY / 2 - 70);
		}

		if (DrawPlayerList == true)
		{
			drawPlayerList();
		}
	}

	public void drawPlayerList()
	{
		boolean BlueTeam = false;
		boolean YellowTeam = false;
		boolean Spectator = false;
		if (game.gameMap.GameMap.numTeams >= 4)
		{
			BlueTeam = true;
			YellowTeam = true;
		}
		else if (game.gameMap.GameMap.numTeams == 3)
		{
			BlueTeam = true;
		}

		if (game.gameOptions.fontChatShadow == true)
		{
			game.gameEngine.setColor(GameResources.colorBlack);
			game.gameEngine.drawString("Green Team", 15 + 1, 125 + 1);
		}
		game.gameEngine.setColor(GameResources.colorGreen);
		game.gameEngine.drawString("Green Team", 15, 125);
		if (game.gameOptions.fontChatShadow == true)
		{
			game.gameEngine.setColor(GameResources.colorBlack);
			game.gameEngine.drawString("Red Team", 150 + 1, 125 + 1);
		}
		game.gameEngine.setColor(GameResources.colorRed);
		game.gameEngine.drawString("Red Team", 150, 125);
		if (BlueTeam)
		{
			if (game.gameOptions.fontChatShadow == true)
			{
				game.gameEngine.setColor(GameResources.colorBlack);
				game.gameEngine.drawString("Blue Team", 285 + 1, 125 + 1);
			}
			game.gameEngine.setColor(GameResources.colorBlue);
			game.gameEngine.drawString("Blue Team", 285, 125);
		}
		if (YellowTeam)
		{
			if (game.gameOptions.fontChatShadow == true)
			{
				game.gameEngine.setColor(GameResources.colorBlack);
				game.gameEngine.drawString("Yellow Team", 420 + 1, 125 + 1);
			}
			game.gameEngine.setColor(GameResources.colorYellow);
			game.gameEngine.drawString("Yellow Team", 420, 125);
		}

		int Team[] = new int[5];

		Player[] Players = game.gamePlayers.getPlayerScores();
		for (int i = 0; i < PlayerList.MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				String Score = Players[i].Score + "";

				switch (Players[i].Team)
				{
					case -1: // Spectator
						if (Spectator == false)
						{
							Spectator = true;
							if (game.gameOptions.fontChatShadow == true)
							{
								game.gameEngine
										.setColor(GameResources.colorBlack);
								game.gameEngine.drawString("Spectators",
										555 + 1, 125 + 1);
							}
							game.gameEngine.setColor(GameResources.colorWhite);
							game.gameEngine.drawString("Spectators", 555, 125);
						}
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(Players[i].Name,
									555 + 1, 147 + Team[4] * 16 + 1);
						}
						game.gameEngine.setColor(GameResources.colorWhite);
						game.gameEngine.drawString(Players[i].Name, 555,
								147 + Team[4] * 16);
						Team[4]++;
						break;
					case 0:
						if (game.gameOptions.displayDisableMrBandwidth == false)
						{
							game.gameEngine
									.drawImage(
											GameResources.imgMrBandwidth,
											3,
											144 + Team[Players[i].Team] * 16,
											3 + 16,
											144 + Team[Players[i].Team] * 16 + 16,
											0,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16,
											16,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16 + 16);
						}
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(Players[i].Name, 37 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
							game.gameEngine.drawString(Score, 20 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
						}
						if (Players[i].HoldPen == false)
						{
							game.gameEngine.setColor(GameResources.colorWhite);
							game.gameEngine.drawString(Players[i].Name, 37,
									147 + Team[Players[i].Team] * 16);
						}
						else
						{
							game.gameEngine.setColor(GameResources.colorGray);
							game.gameEngine.drawString(Players[i].Name, 37,
									147 + Team[Players[i].Team] * 16);
						}
						game.gameEngine.setColor(GameResources.colorYellow);
						game.gameEngine.drawString(Score, 20,
								147 + Team[Players[i].Team] * 16);
						break;
					case 1:
						if (game.gameOptions.displayDisableMrBandwidth == false)
						{
							game.gameEngine
									.drawImage(
											GameResources.imgMrBandwidth,
											138,
											144 + Team[Players[i].Team] * 16,
											138 + 16,
											144 + Team[Players[i].Team] * 16 + 16,
											0,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16,
											16,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16 + 16);
						}
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(Players[i].Name,
									172 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
							game.gameEngine.drawString(Score, 155 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
						}
						if (Players[i].HoldPen == false)
						{
							game.gameEngine.setColor(GameResources.colorWhite);
							game.gameEngine.drawString(Players[i].Name, 172,
									147 + Team[Players[i].Team] * 16);
						}
						else
						{
							game.gameEngine.setColor(GameResources.colorGray);
							game.gameEngine.drawString(Players[i].Name, 172,
									147 + Team[Players[i].Team] * 16);
						}
						game.gameEngine.setColor(GameResources.colorYellow);
						game.gameEngine.drawString(Score, 155,
								147 + Team[Players[i].Team] * 16);
						break;
					case 2:
						if (game.gameOptions.displayDisableMrBandwidth == false)
						{
							game.gameEngine
									.drawImage(
											GameResources.imgMrBandwidth,
											272,
											144 + Team[Players[i].Team] * 16,
											272 + 16,
											144 + Team[Players[i].Team] * 16 + 16,
											0,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16,
											16,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16 + 16);
						}
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(Players[i].Name,
									307 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
							game.gameEngine.drawString(Score, 290 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
						}
						if (Players[i].HoldPen == false)
						{
							game.gameEngine.setColor(GameResources.colorWhite);
							game.gameEngine.drawString(Players[i].Name, 307,
									147 + Team[Players[i].Team] * 16);
						}
						else
						{
							game.gameEngine.setColor(GameResources.colorGray);
							game.gameEngine.drawString(Players[i].Name, 307,
									147 + Team[Players[i].Team] * 16);
						}
						game.gameEngine.setColor(GameResources.colorYellow);
						game.gameEngine.drawString(Score, 290,
								147 + Team[Players[i].Team] * 16);
						break;
					case 3:
						if (game.gameOptions.displayDisableMrBandwidth == false)
						{
							game.gameEngine
									.drawImage(
											GameResources.imgMrBandwidth,
											408,
											144 + Team[Players[i].Team] * 16,
											408 + 16,
											144 + Team[Players[i].Team] * 16 + 16,
											0,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16,
											16,
											game.gameNetCode
													.getPingIcon(Players[i].Ping) * 16 + 16);
						}
						if (game.gameOptions.fontChatShadow == true)
						{
							game.gameEngine.setColor(GameResources.colorBlack);
							game.gameEngine.drawString(Players[i].Name,
									442 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
							game.gameEngine.drawString(Score, 425 + 1,
									147 + Team[Players[i].Team] * 16 + 1);
						}
						if (Players[i].HoldPen == false)
						{
							game.gameEngine.setColor(GameResources.colorWhite);
							game.gameEngine.drawString(Players[i].Name, 442,
									147 + Team[Players[i].Team] * 16);
						}
						else
						{
							game.gameEngine.setColor(GameResources.colorGray);
							game.gameEngine.drawString(Players[i].Name, 442,
									147 + Team[Players[i].Team] * 16);
						}
						game.gameEngine.setColor(GameResources.colorYellow);
						game.gameEngine.drawString(Score, 425,
								147 + Team[Players[i].Team] * 16);
						break;
				}
				if (Players[i].Team >= 0)
					Team[Players[i].Team]++;
			}
		}
	}

	public void AddMessage(String Message, int color)
	{
		ChatMessage NewMessage = new ChatMessage();
		NewMessage.Message = Message;
		NewMessage.color = color;
		NewMessage.timeExpiration = game.Tick
				+ game.gameOptions.chatMessageLifetime * 1000;

		Messages.add(0, NewMessage);

		if (Messages.size() > 50)
			Messages.remove(Messages.size() - 1);
	}

	public void Add2PartMessage(String Part1, String Part2, int color1,
			int color2)
	{
		try
		{
			ChatMessage NewMessage = new ChatMessage();
			NewMessage.Parts = 2;
			NewMessage.Part1 = Part1;
			if (Part1 != null)
				NewMessage.Length1 = game.gameEngine.getStringWidth(Part1);
			NewMessage.Part2 = Part2;
			NewMessage.color1 = color1;
			NewMessage.color2 = color2;
			NewMessage.timeExpiration = game.Tick
					+ game.gameOptions.chatMessageLifetime * 1000;
			Messages.add(0, NewMessage);

			if (Messages.size() > 50)
				Messages.remove(Messages.size() - 1);

		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public void Add3PartMessage(String Part1, String Part2, String Part3,
			int color1, int color2, int color3)
	{
		try
		{
			ChatMessage NewMessage = new ChatMessage();
			NewMessage.Parts = 3;
			NewMessage.Part1 = Part1;
			if (Part1 != null)
				NewMessage.Length1 = game.gameEngine.getStringWidth(Part1);
			NewMessage.Part2 = Part2;
			if (Part2 != null)
				NewMessage.Length2 = game.gameEngine.getStringWidth(Part2);
			NewMessage.Part3 = Part3;
			NewMessage.color1 = color1;
			NewMessage.color2 = color2;
			NewMessage.color3 = color3;
			NewMessage.timeExpiration = game.Tick
					+ game.gameOptions.chatMessageLifetime * 1000;
			Messages.add(0, NewMessage);

			if (Messages.size() > 50)
			{
				Messages.remove(Messages.size() - 1);
			}
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public void Add4PartMessage(String Part1, String Part2, String Part3,
			String Part4, int color1, int color2, int color3, int color4)
	{
		try
		{
			ChatMessage NewMessage = new ChatMessage();
			NewMessage.Parts = 4;
			NewMessage.Part1 = Part1;
			if (Part1 != null)
				NewMessage.Length1 = game.gameEngine.getStringWidth(Part1);
			NewMessage.Part2 = Part2;
			if (Part2 != null)
				NewMessage.Length2 = game.gameEngine.getStringWidth(Part2);
			NewMessage.Part3 = Part3;
			if (Part3 != null)
				NewMessage.Length3 = game.gameEngine.getStringWidth(Part3);
			NewMessage.Part4 = Part4;
			NewMessage.color1 = color1;
			NewMessage.color2 = color2;
			NewMessage.color3 = color3;
			NewMessage.color4 = color4;
			NewMessage.timeExpiration = game.Tick
					+ game.gameOptions.chatMessageLifetime * 1000;
			Messages.add(0, NewMessage);

			if (Messages.size() > 50)
				Messages.remove(Messages.size() - 1);

		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public void AppendChatString(String append)
	{
		if (game.gameEngine.getStringWidth(ChatString) < 525)
			ChatString += append;
	}

	public void backSpace()
	{
		if (ChatString.length() < 1)
		{
			ChatString = "";
			if (game.gameOptions.chatToggling == true
					&& game.gameInput.ChatToggle == true)
			{
				game.gameInput.ChatToggle = false;
			}
		}
		else
			ChatString = ChatString.substring(0, ChatString.length() - 1);
	}

	public void enter()
	{
		if (ChatString.length() > 0)
		{
			if (ChatString.charAt(0) == KeyEvent.VK_SLASH) // Command
			{
				try
				{
					processCommand();
				}
				catch (Exception e)
				{
					System.err.println("Bad command - " + ChatString);
				}
			}
			else if (ChatString.charAt(0) == KeyEvent.VK_SEMICOLON) // Team Chat
			{
				if (ChatString.length() > 1)
				{
					ChatString = ChatString.substring(1);
					AddMessage(game.gamePlayer.Name + " (TO TEAM) > "
							+ ChatString, getColor(game.gamePlayer.Team));
					if (game.gameNetCode != null)
						game.gameNetCode.SendTCP("teamchat " + ChatString);
				}
				ChatString = "";
			}
			else
			{
				AddMessage(game.gamePlayer.Name + " > " + ChatString,
						getColor(game.gamePlayer.Team));
				if (game.gameNetCode != null)
					game.gameNetCode.SendTCP("broadcast " + ChatString);
				ChatString = "";
			}
		}
	}

	public int getColor(int team)
	{
		switch (team)
		{
			case -1:
				return GameResources.colorWhite;
			case 0:
				return GameResources.colorGreen;
			case 1:
				return GameResources.colorRed;
			case 2:
				return GameResources.colorBlue;
			case 3:
				return GameResources.colorYellow;
			case 4:
				return GameResources.colorWhite;
		}

		return GameResources.colorBlack;
	}

	public void processCommand()
	{
		StringTokenizer st = new StringTokenizer(ChatString);
		String command = st.nextToken();

		if (command.equals("/admin"))
		{
			command = st.nextToken();

			if (command.equals("map"))
			{
				String Map = st.nextToken();
				game.gameNetLinkLobby.SendTCP("changeMap " + Map + " "
						+ game.gamePlayer.Index);
				ChatString = "";
				return;
			}

			if (command.equals("shutdown"))
			{
				game.gameNetLinkLobby.SendTCP("shutdown");
				ChatString = "";
				return;
			}

			if (command.equals("lock"))
			{
				game.gameNetLinkLobby.SendTCP("lock");
				ChatString = "";
				return;
			}

			if (command.equals("unlock"))
			{
				game.gameNetLinkLobby.SendTCP("unlock");
				ChatString = "";
				return;
			}

			if (command.equals("restart"))
			{
				game.gameNetLinkLobby.SendTCP("restart");
				ChatString = "";
				return;
			}

			if (command.equals("start"))
			{
				game.gameNetLinkLobby.SendTCP("start");
				ChatString = "";
				return;
			}

			if (command.equals("kick"))
			{
				if (st.hasMoreTokens())
				{
					String Player = st.nextToken();
					int Index = game.gamePlayers.findPlayerByName(
							game.gamePlayer.Index, Player);
					if (Index > -1)
					{
						if (st.hasMoreTokens())
						{
							int StartPosition = command.length()
									+ Player.length() + 9;
							String Reason = ChatString.substring(StartPosition);
							String PlayerName = game.gamePlayers.getName(Index);
							if (game.gameNetLinkLobby != null)
							{
								game.gameNetLinkLobby.SendTCP("kick " + Index
										+ " " + PlayerName + " " + Reason);
							}
						}
						else
						{
							AddMessage("Kick:  You must give a reason!",
									GameResources.colorWhite);
						}
					}
					else
					{
						AddMessage("Kick:  Player not found!",
								GameResources.colorWhite);
					}
				}
				else
				{
					AddMessage("Kick:  You must enter a player.",
							GameResources.colorWhite);
				}
				ChatString = "";
				return;
			}
			else if (command.equals("rgba"))
			{
				try
				{
					int Red = Integer.parseInt(st.nextToken());
					int Green = Integer.parseInt(st.nextToken());
					int Blue = Integer.parseInt(st.nextToken());
					int Alpha = Integer.parseInt(st.nextToken());
					if (game.gameNetCode != null)
						game.gameNetCode.SendTCP("rgba " + Red + " " + Green
								+ " " + Blue + " " + Alpha);
				}
				catch (Exception e)
				{

				}
				ChatString = "";
				return;
			}
			else if (command.equals("nadespeed"))
			{
				try
				{
					float Distance = Float.parseFloat(st.nextToken());
					if (game.gameNetCode != null)
						game.gameNetCode.SendTCP("nadespeed " + Distance);
				}
				catch (Exception e)
				{

				}
				ChatString = "";
				return;
			}
			else if (command.equals("crazyweapons"))
			{
				if (game.gameNetCode != null)
					game.gameNetCode.SendTCP("crazyweapons");
				ChatString = "";
				return;
			}
			else if (command.equals("lagcompensation"))
			{
				try
				{
					if (game.gameNetCode != null)
						game.gameNetCode.SendTCP("LagCompensation "
								+ st.nextToken());
				}
				catch (Exception e)
				{

				}
				ChatString = "";
				return;
			}

			AddMessage("Admin commands: kick, map, shutdown, lock, unlock, start, restart",
					GameResources.colorWhite);
		}

		if (command.equals("/pm"))
		{
			try
			{
				st.nextToken();

				try
				{
					st.nextToken();
					if (game.gameNetLinkLobby != null)
						game.gameNetLinkLobby.SendTCP("pm "
								+ ChatString.substring(3));
				}
				catch (Exception e)
				{
					AddMessage("PM:  Please specify a message to send.",
							GameResources.colorWhite);
				}
			}
			catch (Exception e)
			{
				AddMessage("PM:  Please specify a player to private message.",
						GameResources.colorWhite);
			}
			ChatString = "";
			return;
		}

		if (command.equals("/find"))
		{
			try
			{
				if (game.gameNetLinkLobby != null)
					game.gameNetLinkLobby.SendTCP("find " + st.nextToken());
			}
			catch (Exception e)
			{
				AddMessage("Find:  Please specify a player to find.",
						GameResources.colorWhite);
			}
			ChatString = "";
			return;
		}

		if (command.equals("/clear"))
		{
			Messages.clear();
			game.gameNetCode.ReceiveCounter = 0;
			ChatString = "";
			return;
		}

		if (command.equals("/UDPPingxxx"))
		{
			AddMessage("UDP Ping:  " + game.gameNetCode.UDPPing,
					GameResources.colorWhite);
			ChatString = "";
			return;
		}

		if (command.equals("/disableUDPxxx"))
		{
			AddMessage("Disabling UDP", GameResources.colorWhite);
			game.gameNetCode.GoodPackets = 0;
			game.gameNetCode.PacketLoss = 100;
			game.gameNetCode.ReceiveCounter = 10000000;
			if (game.gameNetCode != null)
				game.gameNetCode.SendTCP("disableUDP");
			ChatString = "";
			return;
		}

		if (command.equals("/spectate"))
		{
			if (game.gamePlayer.Team >= 0)
			{
				game.gamePlayer.SpectatingPlayer = -1;
				game.gamePlayer.Team = -1;
				if (game.gameNetCode != null)
					game.gameNetCode.SendTCP("changeTeam " + -1);
			}
			else
			{
				AddMessage("You are already spectating!",
						GameResources.colorWhite);
			}
			ChatString = "";
			return;
		}

		if (command.equals("/?"))
		{
			AddMessage("Available commands: find, pm, clear, spectate",
					GameResources.colorWhite);
			ChatString = "";
			return;
		}

		AddMessage("No such command: " + command
				+ ".  Type /? for available commands.",
				GameResources.colorWhite);

		ChatString = "";
	}

	public void printDebugData()
	{
		String Print = "";

		Print = "FPS: " + game.FPS + "    Average Ping: "
				+ game.gameNetCode.AveragePing + "    Last Ping: "
				+ game.gameNetCode.TCPPing;

		AddMessage(Print, colorWhite);
	}
}
