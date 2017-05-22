package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import org.newdawn.slick.Color;

import codemallet.spark.Spark;

public final class BounceList
{
	private final ArrayList<Bounce> BounceList = new ArrayList<Bounce>();

	private final ArrayList<Bounce> BounceListQueue = new ArrayList<Bounce>();

	private final Spark game;

	public BounceList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		ListIterator<Bounce> iterator = BounceListQueue.listIterator();

		while (iterator.hasNext())
		{
			final Bounce bounce = iterator.next();

			BounceList.add(bounce);
			iterator.remove();
		}

		iterator = BounceList.listIterator();

		while (iterator.hasNext())
		{
			final Bounce bounce = (Bounce) iterator.next();
			bounce.Cycle(delta);

			if (bounce.TrailLength < 0)
			{
				iterator.remove();
			}
		}
	}

	public final Bounce addBounce(double X, double Y, double Angle, int Owner)
	{
		final Bounce bounce = new Bounce(game);
		bounce.X = X;
		bounce.Y = Y;
		bounce.StartX = X;
		bounce.StartY = Y;
		bounce.Angle = Angle;
		bounce.Owner = Owner;
		bounce.Team = game.gamePlayers.getTeam(Owner);

		Color color;
		switch (bounce.Team)
		{
			case 0:
				color = new Color(game.gameOptions.colorGreenWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 1:
				color = new Color(game.gameOptions.colorRedWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 2:
				color = new Color(game.gameOptions.colorBlueWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 3:
				color = new Color(game.gameOptions.colorYellowWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
		}

		bounce.Life = 970;
		bounce.MaxLife = bounce.Life;
		bounce.TrailLength = bounce.MaxTrailLength;

		if (bounce.testCollision() == false)
		{
			BounceList.add(bounce);
		}

		return bounce;
	}

	public final void addBounce(double X, double Y, double Angle, int Owner,
			int BounceLife)
	{
		final Bounce bounce = new Bounce(game);
		bounce.X = X;
		bounce.Y = Y;
		bounce.StartX = X;
		bounce.StartY = Y;
		bounce.Angle = Angle;
		bounce.Owner = Owner;
		bounce.Team = game.gamePlayers.getTeam(Owner);

		Color color;
		switch (bounce.Team)
		{
			case 0:
				color = new Color(game.gameOptions.colorGreenWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 1:
				color = new Color(game.gameOptions.colorRedWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 2:
				color = new Color(game.gameOptions.colorBlueWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
			case 3:
				color = new Color(game.gameOptions.colorYellowWeapons);
				bounce.Red = color.r;
				bounce.Green = color.g;
				bounce.Blue = color.b;
				break;
		}
		bounce.Life = BounceLife;
		bounce.MaxLife = bounce.Life;
		bounce.TrailLength = bounce.MaxTrailLength;

		if (bounce.testCollision() == false)
		{
			BounceListQueue.add(bounce);
		}
	}

	public final void drawBouncies()
	{
		final ListIterator<Bounce> iterator = BounceList.listIterator();

		while (iterator.hasNext())
		{
			final Bounce bounce = (Bounce) iterator.next();
			bounce.Render();
		}
	}
}
