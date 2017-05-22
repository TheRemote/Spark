package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class TrailList
{
	private final ArrayList<Trail> TrailList = new ArrayList<Trail>();

	private final Spark game;

	public TrailList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle()
	{
		final ListIterator<Trail> iterator = TrailList.listIterator();

		while (iterator.hasNext())
		{
			Trail Trail = iterator.next();
			Trail.Cycle();

			if (Trail.Animation > 8)
				iterator.remove();
		}
	}

	public final void addTrail(int X, int Y, int Owner, int Team, int Type)
	{
		final Trail Trail = new Trail(game);
		Trail.X = X;
		Trail.Y = Y;
		Trail.Owner = Owner;
		Trail.Team = Team;
		Trail.Type = Type;
		switch (Type)
		{
			case 0:
				Trail.AnimationTimeLength = 25;
				break;
			case 1:
				Trail.AnimationTimeLength = 75;
				break;
		}
		Trail.timeAnimation = game.Tick + Trail.AnimationTimeLength;

		TrailList.add(Trail);
	}

	public final void drawTrails()
	{
		ListIterator<Trail> iterator = TrailList.listIterator();

		while (iterator.hasNext())
		{
			Trail Trail = iterator.next();
			Trail.Render();
		}
	}
}
