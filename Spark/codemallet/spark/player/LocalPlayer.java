package codemallet.spark.player;

import codemallet.spark.Spark;
import codemallet.spark.util.SparkUtil;

public final class LocalPlayer
{
	private final Spark game;

	public String Name;

	public int Index;

	public float X;

	public float Y;

	public int CruiseX;

	public int CruiseY;

	public int OldCruiseX;

	public int OldCruiseY;

	public int OffsetX;

	public int OffsetY;

	float LastMovingX;

	float LastMovingY;

	public int Team;

	public boolean Dead;

	public boolean HoldingPen;

	public boolean Smoking;

	public boolean hasFlag;

	public int FlagIndex;

	public boolean Frozen;

	public long timeHoldingPen;

	private long MovementTick;

	public long TeamSwitchTimer;

	public int SpectatingPlayer = -1;
	
	public int Clan;

	public LocalPlayer(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		if (Dead == true && Team >= 0)
		{
			game.gamePlayers.setMoveX(Index, 0);
			game.gamePlayers.setMoveY(Index, 0);
			return;
		}

		int MovingX = 0;
		int MovingY = 0;
		if (Frozen == false)
		{
			if (game.gameInput.KeyDown)
			{
				MovingY = 1;
			}

			if (game.gameInput.KeyUp)
			{
				MovingY -= 1;
			}

			if (game.gameInput.KeyRight)
			{
				MovingX = 1;
			}

			if (game.gameInput.KeyLeft)
			{
				MovingX -= 1;
			}

			if (MovingX == 0 && MovingY == 0 && game.gameInput.Cruise == true)
			{
				MovingX = CruiseX;
				MovingY = CruiseY;
			}
		}

		if (game.Tick > MovementTick || MovingX != LastMovingX
				|| MovingY != LastMovingY)
		{
			if (game.gameNetCode != null && Team >= 0)
			{
				game.gameNetCode.SendUDP("m "
						+ (float) SparkUtil.roundDouble(X, 2) + " "
						+ (float) SparkUtil.roundDouble(Y, 2) + " "
						+ (int) MovingX + " " + (int) MovingY + " "
						+ (int) game.gameWeapons.Health + " "
						+ (int) game.gameWeapons.Energy);
			}
			MovementTick = game.Tick + 500;
		}

		LastMovingX = MovingX;
		LastMovingY = MovingY;

		game.gamePlayers.setMoveX(Index, MovingX);
		game.gamePlayers.setMoveY(Index, MovingY);
	}
}
