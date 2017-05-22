package codemallet.spark.weapon;

import java.util.ArrayList;
import java.util.ListIterator;

import codemallet.spark.Spark;

public final class ExplosionList
{
	private final ArrayList<Explosion> ExplosionList = new ArrayList<Explosion>();

	private final Spark game;

	public ExplosionList(Spark game)
	{
		this.game = game;
	}

	public final void Cycle()
	{
		final ListIterator<Explosion> iterator = ExplosionList.listIterator();

		while (iterator.hasNext())
		{
			final Explosion Explosion = iterator.next();
			Explosion.Cycle();

			if (Explosion.Finished)
				iterator.remove();
		}
	}

	public final void addExplosion(int X, int Y, int Type)
	{
		final Explosion Explosion = new Explosion(game);
		Explosion.X = X;
		Explosion.Y = Y;
		Explosion.Type = Type;

		switch (Type)
		{
			case 0:
				Explosion.AnimationTimeLength = 35;
				break;
			case 1:
				Explosion.AnimationTimeLength = 35;
				break;
			case 2:
				Explosion.AnimationTimeLength = 35;

				/*
				ConfigurableEmitter explosionEmitter = ((ConfigurableEmitter) game.explosionSystem
						.getEmitter(0)).duplicate();
				explosionEmitter.setEnabled(true);
				game.explosionSystem.addEmitter(explosionEmitter);
				Explosion.explosionEmitter = explosionEmitter;
				*/

				break;
			case 3:
				/*
				explosionEmitter = ((ConfigurableEmitter) game.laserHitSystem
						.getEmitter(0)).duplicate();
				explosionEmitter.setEnabled(true);
				game.laserHitSystem.addEmitter(explosionEmitter);
				Explosion.explosionEmitter = explosionEmitter;
				*/

				break;
		}
		Explosion.timeAnimation = game.Tick + Explosion.AnimationTimeLength;

		ExplosionList.add(Explosion);
	}

	public final void drawExplosions()
	{
		final ListIterator<Explosion> iterator = ExplosionList.listIterator();

		while (iterator.hasNext())
		{
			Explosion Explosion = (Explosion) iterator.next();
			Explosion.Render();
		}
	}
}
