package codemallet.spark.weapon;

import codemallet.spark.Spark;

public final class Explosion
{
	private final Spark game;

	boolean Finished;

	int X;

	int Y;

	int Type;

	int Animation;

	int AnimationTimeLength;

	long timeAnimation;

	Explosion(Spark game)
	{
		this.game = game;
	}

	public final void Cycle()
	{
		switch (Type)
		{
			case 0:
				if (game.Tick > timeAnimation)
				{
					timeAnimation = game.Tick + AnimationTimeLength;
					Animation++;
				}
				if (Animation > 9)
					Finished = true;
				break;
			case 1:
				if (game.Tick > timeAnimation)
				{
					timeAnimation = game.Tick + AnimationTimeLength;
					Animation++;
				}
				if (Animation > 9)
					Finished = true;
				break;
			case 2:
				if (game.Tick > timeAnimation)
				{
					timeAnimation = game.Tick + AnimationTimeLength;
					Animation++;
				}
				if (Animation > 19)
					Finished = true;
				break;
		}
	}

	public final void Render()
	{
		int SrcX = 0;
		int SrcY = 0;
		int AddX = 0;
		int AddY = 0;
		int Size = 0;

		switch (Type)
		{
			case 0: // Small
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

				Size *= 2;
				Size -= 3;
				SrcX -= 12;
				SrcX *= 2;
				SrcX = SrcX + 12;
				SrcY = 720 + (SrcY - 692);
				AddX *= 2;
				AddY *= 2;
				AddX += 1;
				AddY += 1;
				if (Animation == 4)
					AddY += 1;

				break;
			case 1: // Medium
				SrcX = (Animation % 5) * 64;
				SrcY = 786 + ((Animation / 5) * 44);
				SrcX += (Animation / 5) * 2;
				Size = 44;
				break;
			case 2: // Large
				SrcX = (Animation % 5) * 64;
				SrcY = 886 + ((Animation / 5) * 65);
				Size = 64;
				break;
		}

		int DrawX = (int) X
				- (int) (game.gamePlayer.X - game.gamePlayer.OffsetX);
		int DrawY = (int) Y
				- (int) (game.gamePlayer.Y - game.gamePlayer.OffsetY);

		DrawX += AddX;
		DrawY += AddY;
		game.gameEngine.drawInUse(DrawX, DrawY, DrawX + Size, DrawY + Size, SrcX,
				SrcY, SrcX + Size, SrcY + Size);
	}
}
