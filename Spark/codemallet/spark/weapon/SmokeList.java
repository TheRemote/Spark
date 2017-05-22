package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class SmokeList
{
	private final ArrayList<Smoke> SmokeList = new ArrayList<Smoke>();

	private final Spark game;

	public SmokeList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle()
	{
		final ListIterator<Smoke> iterator = SmokeList.listIterator();

		while (iterator.hasNext())
		{
			final Smoke Smoke = iterator.next();
			if (Smoke.Active == true)
			{
				Smoke.Cycle();

				if (Smoke.Animation > 8)
				{
					Smoke.Active = false;
				}
			}
		}
	}

	public final void addSmoke(int X, int Y, int Owner, int Type)
	{
		final Smoke Smoke = findSmoke();
		Smoke.Active = true;
		Smoke.X = X;
		Smoke.Y = Y;
		Smoke.Owner = Owner;
		Smoke.Team = game.gamePlayers.getTeam(Owner);
		Smoke.Type = Type;
		Smoke.Animation = 0;
		switch (Type)
		{
			case 0:
				Smoke.AnimationTimeLength = 80;
				break;
		}
		Smoke.timeAnimation = game.Tick + Smoke.AnimationTimeLength;
	}

	public final void drawSmoke()
	{
		final ListIterator<Smoke> iterator = SmokeList.listIterator();

		while (iterator.hasNext())
		{
			final Smoke Smoke = iterator.next();

			if (Smoke.Active == true)
			{
				Smoke.Render();
			}
		}
	}

	public final Smoke findSmoke()
	{
		final ListIterator<Smoke> iterator = SmokeList.listIterator();

		while (iterator.hasNext())
		{
			final Smoke Smoke = (Smoke) iterator.next();
			if (Smoke.Active == false)
			{
				return Smoke;
			}
		}

		final Smoke Smoke = new Smoke(game);
		SmokeList.add(Smoke);
		return Smoke;
	}
}
