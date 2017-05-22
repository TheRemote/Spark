package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;


public final class MissileList
{
	private final ArrayList<Missile> MissileList = new ArrayList<Missile>();

	private final Spark game;

	public MissileList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		final ListIterator<Missile> iterator = MissileList.listIterator();

		while (iterator.hasNext())
		{
			final Missile Missile = (Missile) iterator.next();
			Missile.Cycle(delta);

			if (Missile.Life < 0)
			{
				iterator.remove();
			}
		}
	}

	public final Missile addMissile(double X, double Y, double Angle,
			int Owner)
	{
		final Missile missile = new Missile(game);
		missile.X = X;
		missile.Y = Y;
		missile.StartX = X;
		missile.StartY = Y;
		missile.Angle = Angle;
		missile.Owner = Owner;
		missile.Team = game.gamePlayers.getTeam(Owner);

		missile.Life = 480;
		missile.MaxLife = missile.Life;

		MissileList.add(missile);
		return missile;
	}

	public final void drawMissiles()
	{
		final ListIterator<Missile> iterator = MissileList.listIterator();

		while (iterator.hasNext())
		{
			Missile Missile = (Missile) iterator.next();
			Missile.Render();
		}
	}
}
