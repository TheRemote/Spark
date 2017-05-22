package codemallet.spark.input;

import org.lwjgl.input.Keyboard;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;
import codemallet.spark.util.SparkUtil;

public final class Input
{
	private final Spark game;

	public int MouseX;

	public int MouseY;

	public boolean KeyLeft;

	public boolean KeyRight;

	public boolean KeyUp;

	public boolean KeyDown;

	public boolean KeyCruiseLeft;

	public boolean KeyCruiseRight;

	public boolean KeyCruiseUp;

	public boolean KeyCruiseDown;

	public boolean MouseUp;

	public boolean MouseDown;

	public boolean Cruise;

	public boolean ChatToggle;

	public boolean SpecialsToggled;

	public Input(Spark game)
	{
		this.game = game;
	}

	public final void processKey(int keyCode, char keyChar, boolean Event)
	{
		if (game.gamePlayer.Team > -1)
			game.timeAFKKick = game.Tick + 180000;
		else
			game.timeAFKKick = game.Tick + 600000;

		keyCode = KeyConversion.fixKeysHack(keyCode);

		if (game.gameDialog.DialogOpen == true)
		{
			game.gameDialog.processDialogKey(keyCode, keyChar, Event);
			return;
		}

		if ((game.gameOptions.chatToggling == true && ChatToggle == false)
				|| game.gameOptions.chatToggling == false)
		{
			// Directional arrows
			if (keyCode == game.gameOptions.keyLeft)
			{
				Cruise = false;
				if (Event == true)
					KeyLeft = true;
				else
					KeyLeft = false;

				return;
			}
			else if (keyCode == game.gameOptions.keyRight)
			{
				Cruise = false;
				if (Event == true)
					KeyRight = true;
				else
					KeyRight = false;

				return;
			}
			else if (keyCode == game.gameOptions.keyUp)
			{
				Cruise = false;
				if (Event == true)
					KeyUp = true;
				else
					KeyUp = false;

				return;
			}
			else if (keyCode == game.gameOptions.keyDown)
			{
				Cruise = false;
				if (Event == true)
					KeyDown = true;
				else
					KeyDown = false;

				return;
			}
			else if (keyCode == game.gameOptions.keyCruiseUp)
			{
				if (Event == true)
				{
					Cruise = true;
					KeyCruiseUp = true;
					checkCruiseControl();
				}
				else
					KeyCruiseUp = false;
				return;
			}
			else if (keyCode == game.gameOptions.keyCruiseDown)
			{
				if (Event == true)
				{
					Cruise = true;
					KeyCruiseDown = true;
					checkCruiseControl();
				}
				else
					KeyCruiseDown = false;
				return;
			}
			else if (keyCode == game.gameOptions.keyCruiseLeft)
			{
				if (Event == true)
				{
					Cruise = true;
					KeyCruiseLeft = true;
					checkCruiseControl();
				}
				else
					KeyCruiseLeft = false;
				return;
			}
			else if (keyCode == game.gameOptions.keyCruiseRight)
			{
				if (Event == true)
				{
					Cruise = true;
					KeyCruiseRight = true;
					checkCruiseControl();
				}
				else
					KeyCruiseRight = false;
				return;
			}
		}

		if (Event == true)
		{
			if (keyCode == game.gameOptions.keySendChat)
			{
				if ((game.gameOptions.chatToggling == true && ChatToggle == true)
						|| game.gameOptions.chatToggling == false)
				{
					game.gameChat.enter();
					ChatToggle = false;
					return;
				}
			}
			if (keyCode == game.gameOptions.keyToggleChat)
			{
				if (game.gameOptions.chatToggling == true)
				{
					if (ChatToggle == false)
					{
						ChatToggle = true;
						return;
					}
				}
			}
			if (keyCode == game.gameOptions.keyToggleSpecial)
			{
				if ((game.gameOptions.chatToggling == true && ChatToggle == false)
						|| game.gameOptions.chatToggling == false)
				{
					game.gameInterface.ToggleSpecials();
					return;
				}
			}
			if (keyCode == game.gameOptions.keyDropFlag)
			{
				if ((game.gameOptions.chatToggling == true && ChatToggle == false)
						|| game.gameOptions.chatToggling == false)
				{
					if (game.gamePlayer.hasFlag == true)
					{
						if (game.gameNetCode != null)
							game.gameNetCode.SendTCP("drop");
					}
					return;
				}
			}
			if (keyCode == game.gameOptions.keyDisplayPlayers)
			{
				if ((game.gameOptions.chatToggling == true && ChatToggle == false)
						|| game.gameOptions.chatToggling == false)
				{
					if (game.gameChat.DrawPlayerList == false)
						game.gameChat.DrawPlayerList = true;
					else
						game.gameChat.DrawPlayerList = false;
					return;
				}
			}
			if (keyCode == game.gameOptions.keySwitchTeams)
			{
				if ((game.gameOptions.chatToggling == true && ChatToggle == false)
						|| game.gameOptions.chatToggling == false)
				{
					game.gameDialog.showTeamSwitchDialog();
					return;
				}
			}
			switch (keyCode)
			{
				case Keyboard.KEY_ESCAPE:
					game.gameDialog.showLeaveGameDialog();
					break;
				case Keyboard.KEY_BACK:
				case Keyboard.KEY_DELETE:
					if ((game.gameOptions.chatToggling == true && ChatToggle == true)
							|| game.gameOptions.chatToggling == false)
						game.gameChat.backSpace();
					break;
				case Keyboard.KEY_GRAVE:
					game.gameChat.printDebugData();
					break;
				case Keyboard.KEY_SPACE:
					if ((game.gameOptions.chatToggling == true && ChatToggle == true)
							|| game.gameOptions.chatToggling == false)
						game.gameChat.AppendChatString(" ");
					break;
				default:
				{
					if ((game.gameOptions.chatToggling == true && ChatToggle == true)
							|| game.gameOptions.chatToggling == false)
					{
						char Key = keyChar;
						if (Character.isDefined(Key)
								&& Character.isIdentifierIgnorable(Key) == false)
						{
							String newString = "";
							newString += keyChar;
							game.gameChat.AppendChatString(newString);
						}
					}

				}
					break;
			}
		}

	}

	public final void processClick(int button)
	{
		if (game.gamePlayer.Team > -1)
			game.timeAFKKick = game.Tick + 180000;
		else
			game.timeAFKKick = game.Tick + 600000;

		if (game.gameDialog.DialogOpen == true)
		{
			game.gameDialog.processDialogClick(MouseX, MouseY, button);
			return;
		}

		if (button == 0 && game.gameOptions.displayDisablePanel == false
				&& game.gameInterface.MouseMap(MouseX, MouseY) == true)
			return;

		if (game.gamePlayer.Team < 0)
		{
			if (button == 0)
			{
				game.gamePlayer.SpectatingPlayer = game.gamePlayers
						.getNextPlayer(game.gamePlayer.SpectatingPlayer);
				if (game.gamePlayer.SpectatingPlayer != -1)
				{
					game.gameWeapons.Health = game.gamePlayers
							.getHealth(game.gamePlayer.SpectatingPlayer);
					game.gameWeapons.Energy = game.gamePlayers
							.getEnergy(game.gamePlayer.SpectatingPlayer);
				}
			}
			else if (button == 1)
			{
				game.gamePlayer.SpectatingPlayer = game.gamePlayers
						.getPreviousPlayer(game.gamePlayer.SpectatingPlayer);
				if (game.gamePlayer.SpectatingPlayer != -1)
				{
					game.gameWeapons.Health = game.gamePlayers
							.getHealth(game.gamePlayer.SpectatingPlayer);
					game.gameWeapons.Energy = game.gamePlayers
							.getEnergy(game.gamePlayer.SpectatingPlayer);
				}
			}
			return;
		}

		double shootX = game.gamePlayer.X + 16;
		double shootY = game.gamePlayer.Y + 16;
		double Angle = Math.atan2(
				(double) (MouseY - game.gamePlayer.OffsetY - 16),
				(double) (MouseX - game.gamePlayer.OffsetX - 16));

		shootX = SparkUtil.roundDouble(shootX, 2);
		shootY = SparkUtil.roundDouble(shootY, 2);
		Angle = SparkUtil.roundDouble(Angle, 4);

		if (button == 0) // Left Click
		{
			if (game.gameOptions.mouseClassicMode == false)
			{
				game.gameWeapons.shootLaser(shootX, shootY, Angle);
			}
			else
			{
				if (SpecialsToggled == false)
				{
					game.gameWeapons.shootLaser(shootX, shootY, Angle);
				}
				else
				{
					switch (game.gameInterface.SelectedWeapon)
					{
						case 0: // Missile
							game.gameWeapons
									.shootMissile(shootX, shootY, Angle);
							break;
						case 1: // Grenade
							game.gameWeapons.shootGrenade(shootX, shootY,
									MouseX, MouseY, Angle);
							break;
						case 2: // Bouncy
							game.gameWeapons.shootBouncy(shootX, shootY, Angle);
							break;
					}
				}
			}
		}
		else if (button == 1) // Right Click
		{
			if (game.gameOptions.mouseClassicMode == false)
			{
				switch (game.gameInterface.SelectedWeapon)
				{
					case 0: // Missile
						game.gameWeapons.shootMissile(shootX, shootY, Angle);
						break;
					case 1: // Grenade
						game.gameWeapons.shootGrenade(shootX, shootY, MouseX,
								MouseY, Angle);
						break;
					case 2: // Bouncy
						game.gameWeapons.shootBouncy(shootX, shootY, Angle);
						break;
				}
			}
			else
			{
				if (SpecialsToggled == false)
					SpecialsToggled = true;
				else
					SpecialsToggled = false;
			}
		}
		else if (button == 2) // Middle Click
		{
			if (game.gameInterface.SelectedWeapon != 1)
			{
				game.gameInterface.SelectedWeapon = 1;

				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectGrenade);
			}
		}
		else if (button == 4) // Scroll Up
		{
			if (game.gameInterface.SelectedWeapon != 0)
			{
				game.gameInterface.SelectedWeapon = 0;
				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectMissile);
			}
		}
		else if (button == 5) // Scroll Down
		{

			if (game.gameInterface.SelectedWeapon != 2)
			{
				game.gameInterface.SelectedWeapon = 2;
				if (game.gameOptions.soundSelectedVoiceNotification == true)
					game.gameSound.playSound(GameResources.sndSelectBouncy);
			}
		}
	}

	private final void checkCruiseControl()
	{
		game.gamePlayer.CruiseX = 0;
		game.gamePlayer.CruiseY = 0;
		if (KeyCruiseDown)
		{
			game.gamePlayer.CruiseY = 1;
		}

		if (KeyCruiseUp)
		{
			game.gamePlayer.CruiseY -= 1;
		}

		if (KeyCruiseRight)
		{
			game.gamePlayer.CruiseX = 1;
		}

		if (KeyCruiseLeft)
		{
			game.gamePlayer.CruiseX -= 1;
		}
	}
}
