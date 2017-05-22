package codemallet.spark.gameinterface;

// Interface.java
// Copyright James Chambers, May 2006, 05jchambers@gmail.com

import static codemallet.spark.engines.GameResources.colorEnergy;
import static codemallet.spark.engines.GameResources.colorYellow;
import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;
import codemallet.spark.player.Player;
import codemallet.spark.player.PlayerList;

public final class Interface
{
	private final Spark game;

	// Panel
	public int InterfaceMode;

	public boolean ClosingPanel;

	public boolean OpeningPanel;

	public int NextMode;

	public int PanelProgress;

	public long PanelTick;

	public final int TeamScores[] = new int[4];

	// Player List
	public int SelectedPlayer;

	public int SortingMode;

	public long UpdatePlayerListTick;

	// Mouse Cursor
	public long ArrowTick;

	public int Arrow;

	public int SelectedWeapon;

	// Optimization

	public int InterfaceBlocks;

	public Interface(Spark game)
	{
		this.game = game;
		PanelProgress = 100;
	}

	public final void Cycle()
	{
		if (game.Tick > ArrowTick)
		{
			ArrowTick = game.Tick + 80;
			Arrow++;
			if (Arrow > 3)
				Arrow = 0;
		}

		if (game.Tick > UpdatePlayerListTick)
		{
			UpdatePlayerListTick = game.Tick + 500;
			game.gamePlayers.sortPlayersByScore();
		}

		if ((ClosingPanel == true || OpeningPanel == true)
				&& game.Tick > PanelTick)
		{
			if (ClosingPanel == true)
			{
				if (PanelProgress == 100)
				{
					game.gameSound.playSound(GameResources.sndOpenPanel);
				}
				PanelProgress -= 5;
				if (PanelProgress < 0)
				{
					PanelProgress = 0;
					ClosingPanel = false;
					OpeningPanel = true;
					InterfaceMode = NextMode;
					PanelTick = game.Tick + 300;
					return;
				}
			}

			if (OpeningPanel == true)
			{
				if (PanelProgress == 0)
				{
					game.gameSound.playSound(GameResources.sndClosePanel);
				}
				PanelProgress += 5;
				if (PanelProgress > 100)
				{
					PanelProgress = 100;
					ClosingPanel = false;
					OpeningPanel = false;
					return;
				}
			}

			PanelTick = game.Tick + 25;
		}
	}

	public final void drawInterfaceGroundwork()
	{
		switch (InterfaceMode)
		{
			case 0: // Radar
				game.gameEngine.setColor(GameResources.colorBackground);
				game.gameEngine.fillRect(game.ResolutionX - 135, 18, 100, 94);
				game.gameEngine.setLineWidth(1.1f);
				game.gameRadar.Render();
				break;
			case 2: // Player List
				drawPlayerList();
				break;
		}

		game.gameEngine.setColor(GameResources.colorHealth);
		game.gameEngine.fillRect(game.ResolutionX - 106
				+ game.gameWeapons.MaxHealth / 4 - game.gameWeapons.Health / 4,
				164, game.gameWeapons.Health / 4, 14);
		game.gameEngine.setColor(colorEnergy);
		game.gameEngine.fillRect(game.ResolutionX - 106
				+ (game.gameWeapons.MaxEnergy - game.gameWeapons.Energy), 182,
				game.gameWeapons.Energy, 14);

		int DrawCharges = 0;

		game.gameEngine.setColor(GameResources.colorInterfaceShade);
		int DrawX = game.ResolutionX - 53;
		int DrawY = 0;

		if (SelectedWeapon == 0)
			DrawX -= 10;
		game.gameEngine.fillRect(DrawX, 250, 21, 3);
		DrawX = game.ResolutionX - 53;

		if (SelectedWeapon == 1)
			DrawX -= 10;
		game.gameEngine.fillRect(DrawX, 285, 21, 3);
		DrawX = game.ResolutionX - 53;

		if (SelectedWeapon == 2)
			DrawX -= 10;
		game.gameEngine.fillRect(DrawX, 320, 21, 3);
		DrawX = game.ResolutionX - 53;

		DrawX = game.ResolutionX - 36;
		DrawY = 0;

		if (SelectedWeapon == 0)
			DrawX -= 10;
		DrawCharges = 3;
		DrawY = 245;
		while (DrawCharges > 0)
		{
			game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
			DrawY -= 4;
			DrawCharges--;
		}
		DrawX = game.ResolutionX - 36;

		if (SelectedWeapon == 1)
			DrawX -= 10;
		DrawCharges = 2;
		DrawY = 280;
		while (DrawCharges > 0)
		{
			game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
			DrawY -= 4;
			DrawCharges--;
		}
		DrawX = game.ResolutionX - 36;

		if (SelectedWeapon == 2)
			DrawX -= 10;
		DrawCharges = 3;
		DrawY = 315;
		while (DrawCharges > 0)
		{
			game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
			DrawY -= 4;
			DrawCharges--;
		}
		DrawX = game.ResolutionX - 36;

		if (game.gamePlayer.Dead == false
				&& game.gamePlayer.HoldingPen == false)
		{
			game.gameEngine.setColor(GameResources.colorGreen);
			DrawX = game.ResolutionX - 53;
			DrawY = 0;
			if (game.gameWeapons.MissileRecharge > 0)
			{
				if (SelectedWeapon == 0)
					DrawX -= 10;
				game.gameEngine.fillRect(DrawX, 250,
						game.gameWeapons.MissileRecharge, 3);
				DrawX = game.ResolutionX - 53;
			}

			if (game.gameWeapons.GrenadeRecharge > 0)
			{
				if (SelectedWeapon == 1)
					DrawX -= 10;
				game.gameEngine.fillRect(DrawX, 285,
						game.gameWeapons.GrenadeRecharge, 3);
				DrawX = game.ResolutionX - 53;
			}

			if (game.gameWeapons.BouncyRecharge > 0)
			{
				if (SelectedWeapon == 2)
					DrawX -= 10;
				game.gameEngine.fillRect(DrawX, 320,
						game.gameWeapons.BouncyRecharge, 3);
				DrawX = game.ResolutionX - 53;
			}

			DrawX = game.ResolutionX - 36;
			DrawY = 0;
			if (game.gameWeapons.MissileCharges > 0)
			{
				if (SelectedWeapon == 0)
					DrawX -= 10;
				DrawCharges = game.gameWeapons.MissileCharges;
				DrawY = 245;
				while (DrawCharges > 0)
				{
					game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
					DrawY -= 4;
					DrawCharges--;
				}
				DrawX = game.ResolutionX - 36;
			}

			if (game.gameWeapons.GrenadeCharges > 0)
			{
				if (SelectedWeapon == 1)
					DrawX -= 10;
				DrawCharges = game.gameWeapons.GrenadeCharges;
				DrawY = 280;
				while (DrawCharges > 0)
				{
					game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
					DrawY -= 4;
					DrawCharges--;
				}
				DrawX = game.ResolutionX - 36;
			}

			if (game.gameWeapons.BouncyCharges > 0)
			{
				if (SelectedWeapon == 2)
					DrawX -= 10;
				DrawCharges = game.gameWeapons.BouncyCharges;
				DrawY = 315;
				while (DrawCharges > 0)
				{
					game.gameEngine.fillRect(DrawX, DrawY, 4, 3);
					DrawY -= 4;
					DrawCharges--;
				}
				DrawX = game.ResolutionX - 36;
			}
		}
		else
		{
			game.gameWeapons.MissileCharges = 0;
			game.gameWeapons.GrenadeCharges = 0;
			game.gameWeapons.BouncyCharges = 0;
			game.gameWeapons.MissileRecharge = 0;
			game.gameWeapons.GrenadeRecharge = 0;
			game.gameWeapons.BouncyRecharge = 0;
		}
	}

	public final void drawInterface()
	{
		if (game.gameOptions.displayDisablePanel == false)
		{
			switch (InterfaceMode)
			{
				case 0: // Radar
					break;
				case 1: // Sound
					game.gameEngine.drawInUse(game.ResolutionX - 135, 18,
							game.ResolutionX - 135 + 102, 18 + 96, 539, 1006,
							539 + 102, 1006 + 96);
					break;
				case 2: // Player List
					break;
				case 3: // Configuration
					game.gameEngine.drawInUse(game.ResolutionX - 135, 18,
							game.ResolutionX - 135 + 102, 18 + 96, 429, 1006,
							429 + 102, 1006 + 96);
					break;
			}

			if (OpeningPanel == true || ClosingPanel == true)
			{
				int DrawX = game.ResolutionX - 135;
				int DrawY = 18;

				game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 102, DrawY + 96
						- PanelProgress, 360, 452 + PanelProgress, 360 + 102,
						452 + 96);

				DrawY = 18 + PanelProgress;
				game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 102, DrawY + 96
						- PanelProgress, 360, 548, 360 + 102,
						548 + 96 - PanelProgress);

				DrawX = game.ResolutionX - 135;
				DrawY = 18;
				game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 102
						- PanelProgress, DrawY + 96, 360 + PanelProgress, 260,
						360 + 102, 260 + 96);

				DrawX = game.ResolutionX - 135 + PanelProgress;
				game.gameEngine.drawInUse(DrawX, DrawY, DrawX + 102
						- PanelProgress, DrawY + 96, 360, 355,
						360 + 102 - PanelProgress, 355 + 96);
			}

			switch (SelectedWeapon)
			{
				case 0: // Missile
					game.gameEngine.drawInUse(game.ResolutionX - 146, 0,
							game.ResolutionX - 146 + 146, 0 + 231, 494, 260,
							494 + 146, 260 + 231);
					game.gameEngine.drawInUse(game.ResolutionX - 72, 230,
							game.ResolutionX - 72 + 72, 230 + 30, 536, 740,
							536 + 72, 740 + 30);
					game.gameEngine.drawInUse(game.ResolutionX - 146, 260,
							game.ResolutionX - 146 + 146, 260 + 220, 494, 520,
							494 + 146, 520 + 220);
					break;
				case 1: // Grenade
					game.gameEngine.drawInUse(game.ResolutionX - 146, 0,
							game.ResolutionX - 146 + 146, 0 + 266, 494, 260,
							494 + 146, 260 + 266);
					game.gameEngine.drawInUse(game.ResolutionX - 72, 266,
							game.ResolutionX - 72 + 72, 266 + 30, 536, 776,
							536 + 72, 776 + 30);
					game.gameEngine.drawInUse(game.ResolutionX - 146, 296,
							game.ResolutionX - 146 + 146, 296 + 184, 494, 556,
							494 + 146, 556 + 184);
					break;
				case 2: // Bouncy
					game.gameEngine.drawInUse(game.ResolutionX - 146, 0,
							game.ResolutionX - 146 + 146, 0 + 301, 494, 260,
							494 + 146, 260 + 301);
					game.gameEngine.drawInUse(game.ResolutionX - 72, 301,
							game.ResolutionX - 72 + 72, 301 + 30, 536, 811,
							536 + 72, 811 + 30);
					game.gameEngine.drawInUse(game.ResolutionX - 146, 329,
							game.ResolutionX - 146 + 146, 329 + 151, 494, 589,
							494 + 146, 589 + 151);
					break;
			}

			game.gameEngine.drawInUse(game.ResolutionX - 30, 480,
					game.ResolutionX - 30 + 30, 480 + 16, 610, 892, 610 + 30,
					892 + 16);

			if (InterfaceBlocks > 0)
			{
				for (int i = 0; i < InterfaceBlocks; i++)
				{
					game.gameEngine.drawInUse(game.ResolutionX - 25,
							496 + (i * 16), game.ResolutionX - 25 + 25,
							496 + (i * 16) + 16, 615, 909, 615 + 25, 909 + 16);
				}
			}

			if (game.gamePlayer.hasFlag)
			{
				game.gameEngine.drawInUse(game.ResolutionX - 27, 165,
						game.ResolutionX - 27 + 24, 165 + 30, 613, 861,
						613 + 24, 861 + 30);
			}

			switch (NextMode)
			{
				case 0: // Radar
					game.gameEngine.drawInUse(game.ResolutionX - 27, 5,
							game.ResolutionX - 27 + 24, 5 + 30, 613, 741,
							613 + 24, 741 + 30);
					break;
				case 1: // Sound
					game.gameEngine.drawInUse(game.ResolutionX - 27, 35,
							game.ResolutionX - 27 + 24, 35 + 30, 613, 771,
							613 + 24, 771 + 30);
					break;
				case 2: // Player List
					game.gameEngine.drawInUse(game.ResolutionX - 27, 65,
							game.ResolutionX - 27 + 24, 65 + 30, 613, 801,
							613 + 24, 801 + 30);
					break;
				case 3: // Options
					game.gameEngine.drawInUse(game.ResolutionX - 27, 95,
							game.ResolutionX - 27 + 24, 95 + 30, 613, 831,
							613 + 24, 831 + 30);
					break;
			}
		}

		// Draw Team Scores
		for (int i = 0; i < 4; i++)
		{
			int FirstNumber = TeamScores[i] / 10;
			int SecondNumber = TeamScores[i] % 10;

			game.gameEngine.drawInUse(game.ResolutionX - 33, 392 + i * 20,
					game.ResolutionX - 33 + 7, 392 + i * 20 + 12, 120
							+ FirstNumber * 7 + i * 70, 1189, 120 + FirstNumber
							* 7 + i * 70 + 7, 1189 + 12);

			game.gameEngine.drawInUse(game.ResolutionX - 24, 392 + i * 20,
					game.ResolutionX - 24 + 7, 392 + i * 20 + 12, 120
							+ SecondNumber * 7 + i * 70, 1189, 120
							+ SecondNumber * 7 + i * 70 + 7, 1189 + 12);
		}
	}

	public final void ToggleSpecials()
	{
		if (game.gameOptions.specialsPrevious == false)
		{
			SelectedWeapon++;
			if (SelectedWeapon > 2)
				SelectedWeapon = 0;
		}
		else
		{
			SelectedWeapon--;
			if (SelectedWeapon < 0)
				SelectedWeapon = 2;
		}

		if (game.gameOptions.soundSelectedVoiceNotification == true)
		{
			switch (SelectedWeapon)
			{
				case 0:
					// Missile
					game.gameSound.playSound(GameResources.sndSelectMissile);
					break;
				case 1:
					game.gameSound.playSound(GameResources.sndSelectGrenade);
					break;
				case 2:
					game.gameSound.playSound(GameResources.sndSelectBouncy);
					break;
			}
		}
	}

	public final boolean MouseMap(int x, int y)
	// Handle the mouse map in the interface
	{
		if (game.gameInput.MouseX > game.ResolutionX - 136
				&& game.gameInput.MouseX < game.ResolutionX - 36
				&& game.gameInput.MouseY > 18 && game.gameInput.MouseY < 112)
		{
			switch (NextMode)
			{
				case 0: // Radar;
					break;
				case 1: // Sound
					break;
				case 2: // Player List
					int ClickY = game.gameInput.MouseY - 30;
					if (ClickY > 0)
					{
						if (ClickY > 70) // Buttons along the bottom
						{
							// Ignore Button
							if (game.gameInput.MouseX > game.ResolutionX - 130
									&& game.gameInput.MouseX < game.ResolutionX - 88)
							{
								if (SelectedPlayer > -1
										&& game.gamePlayers.Players[SelectedPlayer].inUse == true)
								{
									String Name = game.gamePlayers.getName(
											SelectedPlayer).toLowerCase();
									if (game.gameIgnoreList.IgnoreList
											.contains(Name))
									{
										game.gamePlayers.setIgnore(
												SelectedPlayer, false);
										game.gameIgnoreList.IgnoreList
												.remove(Name);
										game.gameIgnoreList.SaveIgnoreList();
										if (game.gameNetLinkLobby != null)
											game.gameNetLinkLobby
													.SendTCP("updateIgnoreList");
									}
									else
									{
										game.gamePlayers.setIgnore(
												SelectedPlayer, true);
										game.gameIgnoreList.IgnoreList
												.add(Name);
										game.gameIgnoreList.SaveIgnoreList();
										if (game.gameNetLinkLobby != null)
											game.gameNetLinkLobby
													.SendTCP("updateIgnoreList");
									}
								}
							}
						}
						else
						{
							int TempSelectedPlayer = ClickY / 10;

							int DrawnPlayers = 0;
							Player[] Players = game.gamePlayers
									.getPlayerScores();
							for (int i = 0; i < PlayerList
									.MaxPlayers; i++)
							{
								if (Players[i].inUse == true)
								{
									if (DrawnPlayers == TempSelectedPlayer)
									{
										TempSelectedPlayer = -1;
										SelectedPlayer = i;
										break;
									}
									DrawnPlayers++;
									if (DrawnPlayers > 10)
										break;
								}
							}

							if (TempSelectedPlayer != -1)
								SelectedPlayer = 0;
						}
					}
					return true;
				case 3: // Options
					if (game.gameInput.MouseX > game.ResolutionX - 130
							&& game.gameInput.MouseX < game.ResolutionX - 41)
					{
						if (game.gameInput.MouseY > 39
								&& game.gameInput.MouseY < 51) // Help
						{

							return true;
						}

						if (game.gameInput.MouseY > 53
								&& game.gameInput.MouseY < 65) // New Team
						{
							game.gameDialog.showTeamSwitchDialog();
							return true;
						}

						if (game.gameInput.MouseY > 67
								&& game.gameInput.MouseY < 79) // Configure
						{
							game.gameChat
									.AddMessage(
											"Configuration is currently handled from the lobby.  This will change later.",
											GameResources.colorWhite);
							return true;
						}

						if (game.gameInput.MouseY > 81
								&& game.gameInput.MouseY < 93) // Exit
						{
							game.gameDialog.showLeaveGameDialog();
							return true;
						}
					}
					break;
			}
		}

		// Interface selection
		if (x > game.ResolutionX - 35)
		{
			if (y >= 230 && y < 263)
			{
				// Missile
				SelectedWeapon = 0;
				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectMissile);
				return true;
			}

			if (y >= 263 && y < 298)
			{
				// Grenade
				SelectedWeapon = 1;
				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectGrenade);
				return true;
			}

			if (y >= 298 && y < 330)
			{
				// Bouncy
				SelectedWeapon = 2;
				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectBouncy);
				return true;
			}

			if (x > game.ResolutionX - 32)
			{
				if (y > 165 && y < 195)
				{
					if (game.gamePlayer.hasFlag == true)
					{
						if (game.gameNetCode != null)
							game.gameNetCode.SendTCP("drop");
					}
					return true;
				}

				if (y > 5 && y < 30) // Radar
				{
					if (ClosingPanel == false && OpeningPanel == false
							&& NextMode != 0)
					{
						NextMode = 0;
						ClosingPanel = true;
					}
					return true;
				}

				if (y > 30 && y < 60) // Sound
				{
					if (ClosingPanel == false && OpeningPanel == false
							&& NextMode != 1)
					{
						NextMode = 1;
						ClosingPanel = true;
					}
					return true;
				}

				if (y > 60 && y < 90) // Player List
				{
					if (ClosingPanel == false && OpeningPanel == false
							&& NextMode != 2)
					{
						NextMode = 2;
						ClosingPanel = true;
					}
					return true;
				}

				if (y > 90 && y < 120) // Configuration
				{
					if (ClosingPanel == false && OpeningPanel == false
							&& InterfaceMode != 3)
					{
						NextMode = 3;
						ClosingPanel = true;
					}
					return true;
				}
			}
		}
		return false;
	}

	public final void drawCursor()
	{
		boolean DrawArrow = false;
		if (game.gameOptions.displayDisablePanel == false)
		{
			if (game.gameInput.MouseX > game.ResolutionX - 136
					&& game.gameInput.MouseX < game.ResolutionX - 36
					&& game.gameInput.MouseY > 18
					&& game.gameInput.MouseY < 112)
			{
				switch (NextMode)
				{
					case 0: // Radar;
						break;
					case 1: // Sound
						break;
					case 2: // Player List
						game.gameEngine.drawInUse(game.gameInput.MouseX,
								game.gameInput.MouseY,
								game.gameInput.MouseX + 23,
								game.gameInput.MouseY + 23, 288,
								164 + game.gameInterface.Arrow * 32, 288 + 23,
								164 + game.gameInterface.Arrow * 32 + 23);
						return;
					case 3: // Options
						if (game.gameInput.MouseX > game.ResolutionX - 130
								&& game.gameInput.MouseX < game.ResolutionX - 41)
						{
							if (game.gameInput.MouseY > 39
									&& game.gameInput.MouseY < 51) // Help
							{
								game.gameEngine
										.drawInUse(
												game.gameInput.MouseX,
												game.gameInput.MouseY,
												game.gameInput.MouseX + 23,
												game.gameInput.MouseY + 23,
												288,
												164 + game.gameInterface.Arrow * 32,
												288 + 23,
												164 + game.gameInterface.Arrow * 32 + 23);
								return;
							}

							if (game.gameInput.MouseY > 53
									&& game.gameInput.MouseY < 65) // New Team
							{
								game.gameEngine
										.drawInUse(
												game.gameInput.MouseX,
												game.gameInput.MouseY,
												game.gameInput.MouseX + 23,
												game.gameInput.MouseY + 23,
												288,
												164 + game.gameInterface.Arrow * 32,
												288 + 23,
												164 + game.gameInterface.Arrow * 32 + 23);
								return;
							}

							if (game.gameInput.MouseY > 67
									&& game.gameInput.MouseY < 79) // Configure
							{
								game.gameEngine
										.drawInUse(
												game.gameInput.MouseX,
												game.gameInput.MouseY,
												game.gameInput.MouseX + 23,
												game.gameInput.MouseY + 23,
												288,
												164 + game.gameInterface.Arrow * 32,
												288 + 23,
												164 + game.gameInterface.Arrow * 32 + 23);
								return;
							}

							if (game.gameInput.MouseY > 81
									&& game.gameInput.MouseY < 93) // Exit
							{
								game.gameEngine
										.drawInUse(
												game.gameInput.MouseX,
												game.gameInput.MouseY,
												game.gameInput.MouseX + 23,
												game.gameInput.MouseY + 23,
												288,
												164 + game.gameInterface.Arrow * 32,
												288 + 23,
												164 + game.gameInterface.Arrow * 32 + 23);
								return;
							}
						}
						break;
				}
			}

			if (game.gameInput.MouseX > game.ResolutionX - 35)
			{
				if (game.gameInput.MouseY >= 230 && game.gameInput.MouseY < 330)
				{
					DrawArrow = true;
				}
				else
				{
					if (game.gameInput.MouseX > game.ResolutionX - 25)
						DrawArrow = true;
				}
			}
		}

		if (DrawArrow == true)
		{
			game.gameEngine.drawInUse(game.gameInput.MouseX - 23,
					game.gameInput.MouseY, game.gameInput.MouseX,
					game.gameInput.MouseY + 23, 260,
					(int) (164 + game.gameInterface.Arrow * 32), 260 + 23,
					(int) (164 + game.gameInterface.Arrow * 32 + 23));
		}
		else
		{
			boolean LaserCursor = false;
			if (game.gameOptions.mouseClassicMode == true
					&& game.gameInput.SpecialsToggled == false)
			{
				LaserCursor = true;
			}
			if (LaserCursor == false)
			{
				int DrawCharges = 0;
				int WeaponAvailable = 0;
				switch (SelectedWeapon)
				{
					case 0:
						DrawCharges = game.gameWeapons.MissileCharges;
						if (DrawCharges > 0
								&& game.gameWeapons
										.hasEnergy(game.gameWeapons.EnergyMissile) == true)
							WeaponAvailable = 1;
						break;
					case 1:
						DrawCharges = game.gameWeapons.GrenadeCharges;
						if (DrawCharges > 0
								&& game.gameWeapons
										.hasEnergy(game.gameWeapons.EnergyGrenade) == true)
							WeaponAvailable = 1;
						break;
					case 2:
						DrawCharges = game.gameWeapons.BouncyCharges;
						if (DrawCharges > 0
								&& game.gameWeapons
										.hasEnergy(game.gameWeapons.EnergyBouncy) == true)
							WeaponAvailable = 1;
						break;
				}
				int SrcX = 64 + (SelectedWeapon * 2) * 32 + WeaponAvailable
						* 32;
				int SrcY = 260 - DrawCharges * 32;
				game.gameEngine.drawInUse(game.gameInput.MouseX - 16,
						game.gameInput.MouseY - 16, game.gameInput.MouseX + 16,
						game.gameInput.MouseY + 16, SrcX, SrcY, SrcX + 32,
						SrcY + 32);
			}
			else
			{
				int SrcX = 0;
				int SrcY = 260;
				if (game.gameWeapons.hasEnergy(game.gameWeapons.EnergyLaser) == true)
				{
					SrcX = SrcX + 32;
					game.gameEngine.drawInUse(game.gameInput.MouseX - 16,
							game.gameInput.MouseY - 16,
							game.gameInput.MouseX + 16,
							game.gameInput.MouseY + 16, SrcX, SrcY, SrcX + 32,
							SrcY + 32);
				}
				else
				{
					game.gameEngine.drawInUse(game.gameInput.MouseX - 16,
							game.gameInput.MouseY - 16,
							game.gameInput.MouseX + 16,
							game.gameInput.MouseY + 16, SrcX, SrcY, SrcX + 32,
							SrcY + 32);
				}
			}
		}
	}

	public final void drawPlayerList()
	{
		game.gameEngine.drawImage(GameResources.imgTuna,
				game.ResolutionX - 135, 18, game.ResolutionX - 135 + 102,
				18 + 96, 360, 644, 360 + 102, 644 + 96);

		int DrawnPlayers = 0;
		Player[] Players = game.gamePlayers.getPlayerScores();
		for (int i = 0; i < PlayerList.MaxPlayers; i++)
		{
			if (Players[i].inUse == true)
			{
				if (SelectedPlayer == i) // This is our selected player
				{
					game.gameEngine.setColor(GameResources.colorDarkGray);
					game.gameEngine.fillRect(game.ResolutionX - 134,
							30 + DrawnPlayers * 10 - 1, 92, 11);
					game.gameEngine.setColor(colorYellow);
					game.gameEngine.drawRect(game.ResolutionX - 134,
							30 + DrawnPlayers * 10 - 1, 92, 11);
				}

				String Score = Players[i].Score + "";

				game.gameEngine.setColor(game.gameChat
						.getColor(Players[i].Team));
				game.gameEngine.drawString(Players[i].Name,
						game.ResolutionX - 132, 32 + DrawnPlayers * 10);
				game.gameEngine.drawString(Score, game.ResolutionX - 44
						- game.gameEngine.getStringWidth(Score),
						32 + DrawnPlayers * 10);

				if (game.gamePlayers.getIgnore(i) == true)
				{
					game.gameEngine.setColor(GameResources.colorYellow);
					game.gameEngine.drawLine(game.ResolutionX - 132,
							34 + DrawnPlayers * 10, game.ResolutionX
									- 132
									+ game.gameEngine
											.getStringWidth(Players[i].Name),
							34 + DrawnPlayers * 10);
				}

				DrawnPlayers++;
				if (DrawnPlayers > 10)
					break;
			}
		}
	}
}
