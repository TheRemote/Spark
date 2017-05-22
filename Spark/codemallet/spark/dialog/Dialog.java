package codemallet.spark.dialog;

import org.lwjgl.input.Keyboard;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class Dialog
{
	private final Spark game;

	public static final int dialogSwitchTeam = 1;

	public static final int dialogLeaveGame = 2;

	public boolean DialogOpen;

	public int DialogID;

	// Private Variables
	private int Width;

	private int Height;

	private int StartX;

	private int StartY;

	private int BlocksWidth;

	private int BlocksHeight;

	private int TotalBlocksWidth;

	private int TotalBlocksHeight;

	// Strings
	private static final String LeaveString = "Are you sure you want to leave Spark?";

	private static final String ChangeTeamString = "Click the ship of the team you want to change to:";

	public Dialog(Spark game)
	{
		this.game = game;
	}

	public void processDialogKey(int keyCode, char keyChar, boolean Event)
	{
		if (Event == true && keyCode == Keyboard.KEY_ESCAPE)
			DialogOpen = false;

		switch (DialogID)
		{
			case dialogSwitchTeam:
				break;
			case dialogLeaveGame:
				if (Event == true)
				{
					if (keyCode == Keyboard.KEY_RETURN
							|| keyCode == Keyboard.KEY_Y)
					{
						System.err.println("Player has chosen to quit.");
						game.destroy();
						return;
					}
					else if (keyCode == Keyboard.KEY_N)
					{
						DialogOpen = false;
						return;
					}
				}
				break;
		}
	}

	public void processDialogClick(int x, int y, int button)
	{
		int DialogX = x - StartX;
		int DialogY = y - StartY;
		if (DialogX < 0 || DialogY < 0)
			return;

		switch (DialogID)
		{
			case dialogSwitchTeam:
				if (DialogY > 35 && DialogY < 80) // Team Switching
				{
					if (DialogX > 15 && DialogX < 75) // Green Team
					{
						if (game.gamePlayer.Team != 0
								&& game.gameMap.GameMap.numTeams > 0)
						{
							game.gamePlayer.TeamSwitchTimer = game.Tick + 7000;
							DialogOpen = false;
							if (game.gameNetCode != null)
								game.gameNetCode.SendTCP("changeTeam " + 0);
						}
					}
					else if (DialogX > 85 && DialogX < 145) // Red Team
					{
						if (game.gamePlayer.Team != 1
								&& game.gameMap.GameMap.numTeams > 1)
						{
							game.gamePlayer.TeamSwitchTimer = game.Tick + 7000;
							DialogOpen = false;
							if (game.gameNetCode != null)
								game.gameNetCode.SendTCP("changeTeam " + 1);
						}
					}
					else if (DialogX > 155 && DialogX < 215) // Blue Team
					{
						if (game.gamePlayer.Team != 2
								&& game.gameMap.GameMap.numTeams > 2)
						{
							game.gamePlayer.TeamSwitchTimer = game.Tick + 7000;
							DialogOpen = false;
							if (game.gameNetCode != null)
								game.gameNetCode.SendTCP("changeTeam " + 2);
						}
					}
					else if (DialogX > 225 && DialogX < 285) // Yellow Team
					{
						if (game.gamePlayer.Team != 3
								&& game.gameMap.GameMap.numTeams > 3)
						{
							game.gamePlayer.TeamSwitchTimer = game.Tick + 7000;
							DialogOpen = false;
							if (game.gameNetCode != null)
								game.gameNetCode.SendTCP("changeTeam " + 3);
						}
					}
				}
				else if (DialogX > 120 && DialogX < 180 && DialogY > 90
						&& DialogY < 110)
				{
					DialogOpen = false;
					return;
				}
				break;
			case dialogLeaveGame:
				if (DialogX > 50 && DialogX < 110 && DialogY > 40
						&& DialogY < 60)
				{
					System.err.println("Player has chosen to quit.");
					game.destroy();
					DialogOpen = false;
					return;
				}
				else if (DialogX > 125 && DialogX < 185 && DialogY > 40
						&& DialogY < 60)
				{
					DialogOpen = false;
					return;
				}
				break;
		}
	}

	public void drawDialog()
	{
		int drawX = 0;

		game.gameEngine.beginUse(GameResources.imgTuna);
		for (int i = 0; i < BlocksWidth; i++)
		{
			for (int j = 0; j < BlocksHeight; j++)
			{
				if (i == 0)
				{
					if (j == 0)
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 0, 170, 16, 170 + 16);
					}
					else if (j == TotalBlocksHeight)
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 0, 170 + 32, 16, 170 + 48);
					}
					else
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 0, 170 + 16, 16, 170 + 32);
					}
				}
				else if (j == 0)
				{
					if (i == TotalBlocksWidth)
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 32, 170, 48, 170 + 16);
					}
					else
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 16, 170, 32, 170 + 16);
					}
				}
				else if (i == TotalBlocksWidth)
				{
					if (j == TotalBlocksHeight)
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 32, 170 + 32, 48, 170 + 48);
					}
					else
					{
						game.gameEngine.drawInUse(StartX + i * 16, StartY + j
								* 16, StartX + i * 16 + 16, StartY + j * 16
								+ 16, 32, 170 + 16, 48, 170 + 32);
					}
				}
				else if (j == TotalBlocksHeight)
				{
					game.gameEngine.drawInUse(StartX + i * 16, StartY + j * 16,
							StartX + i * 16 + 16, StartY + j * 16 + 16, 16,
							170 + 32, 32, 170 + 48);
				}
				else
				{
					game.gameEngine.drawInUse(StartX + i * 16, StartY + j * 16,
							StartX + i * 16 + 16, StartY + j * 16 + 16, 16,
							170 + 16, 32, 170 + 32);
				}
			}
		}

		game.gameEngine.endUse();

		switch (DialogID)
		{
			case dialogSwitchTeam:
				drawX = StartX
						+ (Width / 2)
						- (game.gameEngine.getStringWidth(ChangeTeamString) / 2);
				if (game.gameOptions.fontChatShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString(ChangeTeamString, drawX + 1,
							StartY + game.gameEngine.getFontHeight() + 1);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawString(ChangeTeamString, drawX, StartY
						+ game.gameEngine.getFontHeight());

				int DrawTeams = game.gameMap.GameMap.numTeams;

				if (DrawTeams > 0)
					game.gameEngine.drawImage(GameResources.imgTuna,
							StartX + 15, StartY + 35, StartX + 15 + 60,
							StartY + 35 + 45, 64, 89, 64 + 60, 89 + 45);
				DrawTeams--;
				if (DrawTeams > 0)
					game.gameEngine.drawImage(GameResources.imgTuna,
							StartX + 85, StartY + 35, StartX + 85 + 60,
							StartY + 35 + 45, 124, 89, 124 + 60, 89 + 45);
				DrawTeams--;
				if (DrawTeams > 0)
					game.gameEngine.drawImage(GameResources.imgTuna,
							StartX + 155, StartY + 35, StartX + 155 + 60,
							StartY + 35 + 45, 184, 89, 184 + 60, 89 + 45);
				DrawTeams--;
				if (DrawTeams > 0)
					game.gameEngine.drawImage(GameResources.imgTuna,
							StartX + 225, StartY + 35, StartX + 225 + 60,
							StartY + 35 + 45, 244, 89, 244 + 60, 89 + 45);

				// Cancel Button
				game.gameEngine.drawImage(GameResources.imgTuna, StartX + 120,
						StartY + 90, StartX + 120 + 59, StartY + 90 + 20, 0,
						109, 59, 109 + 20);
				break;
			case dialogLeaveGame:
				drawX = StartX + (Width / 2)
						- (game.gameEngine.getStringWidth(LeaveString) / 2);
				if (game.gameOptions.fontChatShadow == true)
				{
					game.gameEngine.setColor(GameResources.colorBlack);
					game.gameEngine.drawString(LeaveString, drawX + 1, StartY
							+ game.gameEngine.getFontHeight() + 1);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawString(LeaveString, drawX, StartY
						+ game.gameEngine.getFontHeight());

				// Yes Button
				game.gameEngine.drawImage(GameResources.imgTuna, StartX + 50,
						StartY + 40, StartX + 50 + 59, StartY + 40 + 20, 0,
						129, 59, 129 + 20);

				// No Button
				game.gameEngine.drawImage(GameResources.imgTuna, StartX + 125,
						StartY + 40, StartX + 125 + 59, StartY + 40 + 20, 0,
						149, 59, 149 + 20);
				break;
		}
	}

	public void showTeamSwitchDialog()
	{
		if (game.Tick > game.gamePlayer.TeamSwitchTimer)
		{
			DialogOpen = true;
			DialogID = Dialog.dialogSwitchTeam;

			game.gameInput.KeyLeft = false;
			game.gameInput.KeyRight = false;
			game.gameInput.KeyUp = false;
			game.gameInput.KeyDown = false;

			Width = 304;
			Height = 128;
			BlocksWidth = Width / 16;
			BlocksHeight = Height / 16;
			TotalBlocksWidth = BlocksWidth - 1;
			TotalBlocksHeight = BlocksHeight - 1;
			StartX = game.ResolutionX / 2 - Width / 2;
			StartY = game.ResolutionY / 2 - Height / 2;
		}
		else
		{
			game.gameChat.AddMessage("It is too soon to change teams again!",
					GameResources.colorWhite);
		}
	}

	public void showLeaveGameDialog()
	{
		DialogOpen = true;
		DialogID = Dialog.dialogLeaveGame;

		game.gameInput.KeyLeft = false;
		game.gameInput.KeyRight = false;
		game.gameInput.KeyUp = false;
		game.gameInput.KeyDown = false;

		Width = 240;
		Height = 90;
		BlocksWidth = Width / 16;
		BlocksHeight = Height / 16;
		TotalBlocksWidth = BlocksWidth - 1;
		TotalBlocksHeight = BlocksHeight - 1;
		StartX = game.ResolutionX / 2 - Width / 2;
		StartY = game.ResolutionY / 2 - Height / 2;
	}

	public void drawCursor()
	{
		if (game.gameInput.MouseX > StartX && game.gameInput.MouseY > StartY
				&& game.gameInput.MouseX < StartX + Width
				&& game.gameInput.MouseY < game.gameInput.MouseY + Height)
		{
			game.gameEngine.drawInUse(game.gameInput.MouseX,
					game.gameInput.MouseY, game.gameInput.MouseX + 23,
					game.gameInput.MouseY + 23, 288,
					(int) (164 + game.gameInterface.Arrow * 32), 288 + 23,
					(int) (164 + game.gameInterface.Arrow * 32 + 23));
		}
		else
		{
			game.gameInterface.drawCursor();
		}
	}
}
