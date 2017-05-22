package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class GrenadeList
{
	private final ArrayList<Grenade> GrenadeList = new ArrayList<Grenade>();

	private final Spark game;

	public float NadeSpeed = 8.5f;

	public GrenadeList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle(int delta)
	{
		final ListIterator<Grenade> iterator = GrenadeList.listIterator();

		while (iterator.hasNext())
		{
			Grenade Grenade = (Grenade) iterator.next();
			Grenade.Cycle(delta);

			if (Grenade.Life < 0)
			{
				iterator.remove();
			}
		}
	}

	public final Grenade addGrenade(double X, double Y, double TargetX, double TargetY,
			double Angle, int Owner)
	{
		final Grenade grenade = new Grenade(game);
		grenade.X = X;
		grenade.Y = Y;
		grenade.StartX = X;
		grenade.StartY = Y;
		grenade.TargetX = TargetX;
		grenade.TargetY = TargetY;
		grenade.Owner = Owner;
		grenade.Team = game.gamePlayers.getTeam(Owner);

		int XDistance = (int) (X - TargetX);
		int YDistance = (int) (Y - TargetY);
		int Distance = (int) Math.sqrt(Math.pow(XDistance, 2)
				+ Math.pow(YDistance, 2));

		if (Angle > Math.PI) Angle = Math.PI;
		if (Angle < -Math.PI) Angle = -Math.PI;
		
		grenade.Angle = Angle;
		grenade.TotalDistance = Distance;
		
		// Old distance formula - changed to always use 5 frames
		//if (Distance > 320.0f)
			grenade.TotalFrames = 5;
		//else
		//	grenade.TotalFrames = (int) (((float) Distance / 320.0f * 5.0f)) + 1;

		grenade.Magnitude = ((float) Math.sqrt(Distance) * NadeSpeed) / 1000.0f;
		GrenadeList.add(grenade);
		return grenade;
	}

	public final void drawGrenades()
	{
		final ListIterator<Grenade> iterator = GrenadeList.listIterator();

		while (iterator.hasNext())
		{
			final Grenade Grenade = (Grenade) iterator.next();
			Grenade.Render();
		}
	}
}
