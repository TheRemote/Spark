package codemallet.spark.weapon;

import codemallet.spark.Spark;

public final class Trail
{
	private final Spark game;

	int Animation;

	int AnimationTimeLength;

	long timeAnimation;

	int X;

	int Y;

	int Team;

	int Owner;

	int Type;

	Trail(Spark game)
	{
		this.game = game;
	}

	public final void Cycle()
	{
		if (game.Tick > timeAnimation)
		{
			Animation++;
			timeAnimation = game.Tick + AnimationTimeLength;
		}
	}

	public final void Render()
	{
		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);

		switch (Type)
		{
			case 0:
				DrawX -= 5;
				DrawY -= 5;
				break;
			case 1:
				DrawX -= 8;
				DrawY -= 8;
				break;
			case 2:
				DrawX -= 8;
				DrawY -= 8;
				break;
			default:
				break;
		}

		int SrcX = 0;
		int SrcY = 0;
		int AddX = 0;
		int AddY = 0;
		int Size = 0;

		switch (Animation)
		{
			case 0:
				Size = 4;
				SrcX = 12;
				SrcY = 693;
				AddX = 3;
				AddY = 3;
				break;
			case 1:
				Size = 5;
				SrcX = 17;
				SrcY = 693;
				AddX = 3;
				AddY = 2;
				break;
			case 2:
				Size = 8;
				SrcX = 22;
				SrcY = 693;
				AddX = 1;
				AddY = 2;
				break;
			case 3:
				Size = 8;
				SrcX = 32;
				SrcY = 693;
				AddX = 1;
				AddY = 1;
				break;
			case 4:
				Size = 9;
				SrcX = 42;
				SrcY = 692;
				AddX = 1;
				AddY = 0;
				break;
			case 5:
				Size = 10;
				SrcX = 53;
				SrcY = 693;
				AddX = 0;
				AddY = 0;
				break;
			case 6:
				Size = 10;
				SrcX = 65;
				SrcY = 693;
				AddX = 0;
				AddY = 0;
				break;
			case 7:
				Size = 9;
				SrcX = 76;
				SrcY = 692;
				AddX = 1;
				AddY = 0;
				break;
			case 8:
				Size = 9;
				SrcX = 88;
				SrcY = 692;
				AddX = 1;
				AddY = 0;
				break;
		}

		if (Type == 1)
		{
			Size *= 2;
			Size -= 3;
			SrcX -= 12;
			SrcX *= 2;
			SrcX = SrcX + 12;
			SrcY = 620 + (SrcY - 692);
			AddX *= 2;
			AddY *= 2;
			AddX += 1;
			AddY += 1;
			if (Animation == 4)
				AddY += 1;

			SrcY = SrcY + Team * 17;
		}
		else if (Type == 0)
		{
			switch (Team)
			{
				case 0:
					break;
				case 1:
					SrcX += 90;
					break;
				case 2:
					SrcY += 11;
					break;
				case 3:
					SrcY += 11;
					SrcX += 90;
					break;
				case 4:
					break;
			}
		}
		else if (Type == 2)
		{
			Size *= 2;
			Size -= 3;
			SrcX -= 12;
			SrcX *= 2;
			SrcX = SrcX + 12;
			SrcY = 620 + (SrcY - 692) - 17;
			AddX *= 2;
			AddY *= 2;
			AddX += 1;
			AddY += 1;
			if (Animation == 4)
				AddY += 1;
		}

		DrawX += AddX;
		DrawY += AddY;

		game.gameEngine.drawInUse(DrawX, DrawY, DrawX + Size, DrawY + Size,
				SrcX, SrcY, SrcX + Size, SrcY + Size);
	}
}
