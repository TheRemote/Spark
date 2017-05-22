package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class ShrapnelList
{
	private final ArrayList<Shrapnel> ShrapnelList = new ArrayList<Shrapnel>();

	private final Spark game;

	public ShrapnelList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		final ListIterator<Shrapnel> iterator = ShrapnelList.listIterator();

		while (iterator.hasNext())
		{
			final Shrapnel Shrapnel = (Shrapnel) iterator.next();

			if (Shrapnel.Active == true)
			{
				Shrapnel.Cycle(delta);

				if (Shrapnel.Life < 0)
				{
					Shrapnel.Active = false;
				}
			}
		}
	}

	public final void addShrapnel(int X, int Y, int Owner, int Team,
			double Angle, int Type)
	{
		final Shrapnel Shrapnel = findShrapnel();
		Shrapnel.Active = true;
		Shrapnel.X = X;
		Shrapnel.Y = Y;
		Shrapnel.Owner = Owner;
		Shrapnel.Team = Team;
		Shrapnel.Type = Type;
		Shrapnel.Angle = Angle;
		if (Shrapnel.Angle > (Math.PI))
			Shrapnel.Angle -= (2 * Math.PI);

		switch (Type)
		{
			case 0:
				Shrapnel.Life = 30;
				Shrapnel.Magnitude = 0.11f;
				break;
			case 1:
				Shrapnel.Life = 128;
				Shrapnel.Magnitude = 0.19f;
				break;
		}
	}

	public final void drawShrapnel()
	{
		final ListIterator<Shrapnel> iterator = ShrapnelList.listIterator();

		while (iterator.hasNext())
		{
			Shrapnel Shrapnel = (Shrapnel) iterator.next();

			if (Shrapnel.Active == true)
			{
				Shrapnel.Render();
			}
		}
	}

	public final Shrapnel findShrapnel()
	{
		final ListIterator<Shrapnel> iterator = ShrapnelList.listIterator();

		while (iterator.hasNext())
		{
			Shrapnel Shrapnel = (Shrapnel) iterator.next();
			if (Shrapnel.Active == false)
			{
				return Shrapnel;
			}
		}

		Shrapnel Shrapnel = new Shrapnel(game);
		ShrapnelList.add(Shrapnel);
		return Shrapnel;
	}
}
