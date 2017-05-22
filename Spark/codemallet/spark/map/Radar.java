package codemallet.spark.map;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class Radar
{
	private final Spark game;

	public boolean Blink;

	long BlinkTime;

	public Radar(Spark game)
	{
		this.game = game;
	}

	public final void Render()
	{
		if (game.gameInterface.InterfaceMode != 0)
			return;

		if (game.gamePlayer.HoldingPen == true)
			return;

		if (game.Tick > BlinkTime)
		{
			if (Blink == true)
				Blink = false;
			else
				Blink = true;
			BlinkTime = game.Tick + 250;
		}

		game.gameEngine.setLineWidth(0.6f);
		if (game.gameOptions.displayDisableAntiAliasing == false)
			game.gameEngine.setAntiAlias(true);
		int StartX = 0;
		int StartY = 0;
		for (int i = 0; i < game.gameMap.FlagPoles.size(); i++)
		{
			FlagPole flagpole = game.gameMap.FlagPoles.get(i);
			StartX = (int) (flagpole.X * 16 - game.gamePlayer.X);
			StartY = (int) (flagpole.Y * 16 - game.gamePlayer.Y);

			int RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 2;
			int RadarY = (int) (66 + ((StartY * 5) / 100)) - 3;

			if (RadarX > game.ResolutionX - 140
					&& RadarX < game.ResolutionX - 35 && RadarY > 12
					&& RadarY < 112)
			{
				game.gameEngine.setColor(game.gameChat.getColor(flagpole.Team));
				game.gameEngine.drawOval(RadarX, RadarY, 4, 4);
			}
		}

		for (int i = 0; i < game.gameMap.Switches.size(); i++)
		{
			Switch Switch = game.gameMap.Switches.get(i);
			StartX = (int) (Switch.X * 16 - game.gamePlayer.X);
			StartY = (int) (Switch.Y * 16 - game.gamePlayer.Y);

			int RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 1;
			int RadarY = (int) (66 + ((StartY * 5) / 100)) - 4;

			if (RadarX > game.ResolutionX - 140
					&& RadarX < game.ResolutionX - 35 && RadarY > 12
					&& RadarY < 112)
			{
				if (Switch.Team > -1)
				{
					game.gameEngine.setColor(game.gameChat
							.getColor(Switch.Team));
					game.gameEngine.fillOval(RadarX + 1, RadarY + 1, 4, 4);
				}
				game.gameEngine.setColor(GameResources.colorWhite);
				game.gameEngine.drawOval(RadarX, RadarY, 6, 6);
			}
		}

		game.gamePlayers.drawRadar();

		if (game.gameOptions.displayDisableAntiAliasing == false)
			game.gameEngine.setAntiAlias(false);

		for (int i = 0; i < game.gameMap.FlagPoles.size(); i++)
		{
			FlagPole flagpole = game.gameMap.FlagPoles.get(i);
			StartX = (int) (flagpole.X * 16 - game.gamePlayer.X);
			StartY = (int) (flagpole.Y * 16 - game.gamePlayer.Y);

			int RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 2;
			int RadarY = (int) (66 + ((StartY * 5) / 100)) - 3;

			if (RadarX > game.ResolutionX - 140
					&& RadarX < game.ResolutionX - 35 && RadarY > 12
					&& RadarY < 112)
			{
				if (flagpole.hasFlag == true)
				{
					Flag flag = game.gameMap.Flags.get(flagpole.FlagIndex);
					StartX = (int) (flagpole.X * 16 - game.gamePlayer.X);
					StartY = (int) (flagpole.Y * 16 - game.gamePlayer.Y);

					RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 4;
					RadarY = (int) (66 + ((StartY * 5) / 100)) - 6;

					game.gameEngine.drawImage(GameResources.imgTuna, RadarX,
							RadarY, RadarX + 8, RadarY + 8, 288,
							311 + flag.Team * 32, 288 + 16,
							311 + flag.Team * 32 + 16);
				}
			}
		}

		for (int i = 0; i < game.gameMap.Flags.size(); i++)
		{
			Flag flag = game.gameMap.Flags.get(i);
			if (flag.onGround == true)
			{
				StartX = (int) (flag.X - game.gamePlayer.X);
				StartY = (int) (flag.Y - game.gamePlayer.Y);

				int RadarX = (int) (game.ResolutionX - 89 + ((StartX * 5) / 100)) + 3;
				int RadarY = (int) (66 + ((StartY * 5) / 100)) - 6;
				if (RadarX > game.ResolutionX - 140
						&& RadarX < game.ResolutionX - 35 && RadarY > 12
						&& RadarY < 112)
				{
					if (flag.timeTotalBlink > game.Tick && flag.Blink == 0)
					{

					}
					else
						game.gameEngine.drawImage(GameResources.imgTuna,
								RadarX, RadarY, RadarX + 8, RadarY + 8, 288,
								311 + flag.Team * 32, 288 + 16,
								311 + flag.Team * 32 + 16);
				}
			}
		}

		if (game.gameOptions.displayDisableRadarGuideBox == false)
		{
			int BeginX = game.ResolutionX - 86;
			int BeginY = 64;
			StartX = game.ResolutionX / 2;
			int LeftX = BeginX - ((StartX * 5) / 100);
			StartY = game.ResolutionY / 2;
			int TopY = BeginY - ((StartY * 5) / 100);
			int DiameterX = (BeginX - LeftX) * 2;
			int DiameterY = (BeginY - TopY) * 2;

			game.gameEngine.setColor(GameResources.colorMagenta);
			if (game.gameOptions.displayDisableAntiAliasing == false)
				game.gameEngine.setAntiAlias(true);
			game.gameEngine.drawLine(LeftX, TopY + DiameterY, LeftX + 4, TopY
					+ DiameterY);
			game.gameEngine.drawLine(LeftX + DiameterX - 4, TopY + DiameterY,
					LeftX + DiameterX, TopY + DiameterY);
			game.gameEngine.drawLine(LeftX, TopY, LeftX + 4, TopY);
			game.gameEngine.drawLine(LeftX + DiameterX - 4, TopY, LeftX
					+ DiameterX, TopY);
			game.gameEngine.drawLine(LeftX, TopY, LeftX, TopY + 4);
			game.gameEngine.drawLine(LeftX, TopY + DiameterY, LeftX, TopY
					+ DiameterY - 4);
			game.gameEngine.drawLine(LeftX + DiameterX, TopY,
					LeftX + DiameterX, TopY + 4);
			game.gameEngine.drawLine(LeftX + DiameterX, TopY + DiameterY - 4,
					LeftX + DiameterX, TopY + DiameterY);
			if (game.gameOptions.displayDisableAntiAliasing == false)
				game.gameEngine.setAntiAlias(false);
		}
	}
}
