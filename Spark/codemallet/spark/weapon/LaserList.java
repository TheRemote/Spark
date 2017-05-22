package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import org.newdawn.slick.Color;

import codemallet.spark.Spark;

public final class LaserList
{
	private final ArrayList<Laser> LaserList = new ArrayList<Laser>();

	private final Spark game;

	public LaserList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		final ListIterator<Laser> iterator = LaserList.listIterator();

		while (iterator.hasNext())
		{
			final Laser laser = iterator.next();

			if (laser.Active == true)
			{
				laser.Cycle(delta);

				if (laser.TrailLength < 0)
				{
					iterator.remove();
				}
			}
		}
	}

	public final Laser addLaser(double X, double Y, double Angle, int Owner)
	{
		final Laser laser = findLaser();
		laser.Active = true;
		laser.X = X;
		laser.Y = Y;
		laser.StartX = X;
		laser.StartY = Y;
		laser.Angle = Angle;
		laser.Owner = Owner;
		laser.Team = game.gamePlayers.getTeam(Owner);

		Color color;
		switch (laser.Team)
		{
			case 0:
				color = new Color(game.gameOptions.colorGreenWeapons);
				laser.Red = color.r;
				laser.Green = color.g;
				laser.Blue = color.b;
				break;
			case 1:
				color = new Color(game.gameOptions.colorRedWeapons);
				laser.Red = color.r;
				laser.Green = color.g;
				laser.Blue = color.b;
				break;
			case 2:
				color = new Color(game.gameOptions.colorBlueWeapons);
				laser.Red = color.r;
				laser.Green = color.g;
				laser.Blue = color.b;
				break;
			case 3:
				color = new Color(game.gameOptions.colorYellowWeapons);
				laser.Red = color.r;
				laser.Green = color.g;
				laser.Blue = color.b;
				break;
		}

		laser.Life = 480;
		laser.MaxLife = laser.Life;

		return laser;
	}

	public final void drawLasers()
	{
		final ListIterator<Laser> iterator = LaserList.listIterator();

		while (iterator.hasNext())
		{
			Laser laser = iterator.next();

			if (laser.Active == true)
			{
				laser.Render();
			}
		}
	}

	public final Laser findLaser()
	{
		final ListIterator<Laser> iterator = LaserList.listIterator();

		while (iterator.hasNext())
		{
			Laser Laser = (Laser) iterator.next();
			if (Laser.Active == false)
			{
				return Laser;
			}
		}

		Laser Laser = new Laser(game);
		LaserList.add(Laser);
		return Laser;
	}
}
