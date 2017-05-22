package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class SparksList
{
	private final ArrayList<Sparks> SparkList = new ArrayList<Sparks>();

	private final Spark game;

	public SparksList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		final ListIterator<Sparks> iterator = SparkList.listIterator();

		while (iterator.hasNext())
		{
			final Sparks spark = iterator.next();
			if (spark.Active == true)
			{
				spark.Cycle(delta);

				if (game.Tick > spark.Time)
				{
					spark.Active = false;
				}
			}
		}
	}

	public final void addSpark(double X, double Y, double Angle)
	{
		final Sparks spark = findSpark();

		spark.Active = true;
		spark.X = X;
		spark.Y = Y;

		double tmpX = Math.cos(Angle);
		double tmpY = Math.sin(Angle);
		spark.Angle = Math.atan2(-tmpY, -tmpX);
		spark.Angle += (1.50f / game.gameRandom.nextFloat());
		spark.Angle -= (1.50f / game.gameRandom.nextFloat());

		int Reducer = game.gameRandom.nextInt(100);
		int Red = 255 - Reducer;
		int Green = 255 - Reducer;
		int Blue = game.gameRandom.nextInt(255);

		spark.Red = Red;
		spark.Green = Green;
		spark.Blue = Blue;

		spark.Time = game.Tick + 70 + game.gameRandom.nextInt(150);
		spark.Magnitude = game.gameRandom.nextFloat() * 0.050f;
	}

	public final void drawSparks()
	{
		final ListIterator<Sparks> iterator = SparkList.listIterator();

		while (iterator.hasNext())
		{
			final Sparks spark = iterator.next();

			if (spark.Active == true)
			{
				spark.Render();
			}
		}
	}

	public final Sparks findSpark()
	{
		final ListIterator<Sparks> iterator = SparkList.listIterator();

		while (iterator.hasNext())
		{
			final Sparks Spark = (Sparks) iterator.next();
			if (Spark.Active == false)
			{
				return Spark;
			}
		}

		final Sparks Spark = new Sparks(game);
		SparkList.add(Spark);
		return Spark;
	}
}
