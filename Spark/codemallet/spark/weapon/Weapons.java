package codemallet.spark.weapon;

import static codemallet.spark.engines.GameResources.colorWhite;
import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;
import codemallet.spark.util.SparkUtil;

public final class Weapons
{
	private final Spark game;

	// Stats
	long timeEnergy;

	long timeSmoke;

	int SmokeAnimation;

	public int Energy;

	public int MaxEnergy = 56;

	public int Health = 224;

	public int MaxHealth = 224;

	public int MissileRecharge;

	public int MissileCharges;

	public int GrenadeRecharge;

	public int GrenadeCharges;

	public int BouncyRecharge;

	public int BouncyCharges;

	public int MaxMissileRecharge = 21;

	public int MaxGrenadeRecharge = 21;

	public int MaxBouncyRecharge = 21;

	// Weapon costs
	public int HealthBonus = 60;

	public int HealthSmoke = 80;

	public int EnergyLaser = 12;

	public int EnergyMissile = 37;

	public int EnergyGrenade = 19;

	public int EnergyBouncy = 12;

	public int HealthLaser = 9;

	public int HealthMissile = 34;

	public int HealthGrenade = 7;

	public int HealthBouncy = 16;

	public int HealthMissileShrapnel = 9;

	public int TotalHealthLaser;

	public int TotalHealthMissile;

	public int TotalHealthGrenade;

	public int TotalHealthBouncy;

	public int TotalHealthMissileShrapnel;

	public int EnergyRechargeTime = 60;

	long timeClick;

	static final int Tolerance = 2;

	// Weapon statics

	public static final int weaponLaser = 1;

	public static final int weaponMissile = 2;

	public static final int weaponGrenade = 3;

	public static final int weaponBouncy = 4;

	public Weapons(Spark game)
	{
		this.game = game;
	}

	public final void CalculateDamage()
	{
		TotalHealthLaser = HealthLaser * (game.gameMap.GameMap.laserDamage + 1);
		TotalHealthMissile = HealthMissile
				* (game.gameMap.GameMap.specialDamage + 1);
		TotalHealthGrenade = HealthGrenade
				* (game.gameMap.GameMap.specialDamage + 1);
		TotalHealthBouncy = HealthBouncy
				* (game.gameMap.GameMap.specialDamage + 1);
		TotalHealthMissileShrapnel = HealthMissileShrapnel
				* game.gameMap.GameMap.specialDamage;
	}

	public final void Cycle(int delta)
	{
		game.gameLasers.Cycle(delta);
		game.gameMissiles.Cycle(delta);
		game.gameGrenades.Cycle(delta);
		game.gameBouncies.Cycle(delta);
		game.gameSparks.Cycle(delta);
		game.gameShrapnel.Cycle(delta);
		game.gameTrails.Cycle();
		game.gameExplosions.Cycle();
		game.gameSmoke.Cycle();

		if (game.gamePlayer.Dead == false
				|| game.gamePlayer.SpectatingPlayer > -1)
		{
			if (game.Tick > timeEnergy)
			{
				if (Energy < MaxEnergy)
					Energy++;
				timeEnergy = game.Tick + EnergyRechargeTime;
			}
		}

		if (game.gamePlayer.Dead == false && game.gamePlayer.Smoking == true
				&& game.Tick > timeSmoke)
		{
			timeSmoke = game.Tick + 400;
			game.gameSmoke.addSmoke((int) (game.gamePlayer.X + game.gameRandom
					.nextInt(25)), (int) (game.gamePlayer.Y + game.gameRandom
					.nextInt(25)), 0, 0);

			SmokeAnimation = 1 - SmokeAnimation;
		}
	}

	public final void takeEnergy(int Energy)
	{
		this.Energy -= Energy;
		if (this.Energy < 0)
			this.Energy = 0;
	}

	public final boolean hasEnergy(int Energy)
	{
		if (Energy > this.Energy)
		{
			return false;
		}
		return true;
	}

	public final void takeHealth(int Health)
	{
		this.Health -= Health;
		if (this.Health < HealthSmoke && game.gamePlayer.Smoking == false)
		{
			game.gamePlayer.Smoking = true;
			game.gameNetCode.SendUDP("smoking 1");
			game.gameSound.playSound(GameResources.sndArmorCritical);
		}
		if (this.Health < 0)
			this.Health = 0;
	}

	public final boolean hasHealth(int Health)
	{
		if (Health > this.Health)
		{
			return false;
		}
		return true;
	}

	public final void laserHit(int Owner, int Team, int X, int Y, float Angle)
	{
		game.gameSound.play3DSound(GameResources.sndHitLaser, (float) X,
				(float) Y);
		for (int i = 0; i < 30; i++)
		{
			game.gameSparks.addSpark(X, Y, Angle);
		}
		game.gameNetCode.SendUDP("hit " + Owner + " " + weaponLaser + " "
				+ TotalHealthLaser + " " + Health + " " + X + " " + Y + " "
				+ Angle);
		if (game.gamePlayer.Team != Team)
		{
			if (hasHealth(TotalHealthLaser) == false)
			{
				game.gameNetCode.SendTCP("death " + Owner);
				game.gamePlayer.Dead = true;
				game.gameExplosions.addExplosion(
						(int) (game.gamePlayer.X - 16),
						(int) (game.gamePlayer.Y - 16), 2);
				game.gameSound.play3DSound(GameResources.sndExplosion,
						game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
				Health = 0;
				Energy = 0;
				game.gamePlayers.setDeaths(game.gamePlayer.Index,
						game.gamePlayers.getDeaths(game.gamePlayer.Index) + 1);
				game.gamePlayers.setDead(game.gamePlayer.Index, true);
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Owner),
						" destroyed ", game.gamePlayer.Name, game.gameChat
								.getColor(game.gamePlayers.getTeam(Owner)),
						GameResources.colorWhite, game.gameChat
								.getColor(game.gamePlayer.Team));
			}
			else
				takeHealth(TotalHealthLaser);
		}
	}

	public final void missileHit(int Owner, int Team, int X, int Y)
	{
		game.gameSound.play3DSound(GameResources.sndExplosion, (float) X,
				(float) Y);
		game.gameNetCode.SendUDP("hit " + Owner + " " + weaponMissile + " "
				+ TotalHealthLaser + " " + Health + " " + X + " " + Y + " "
				+ "0");
		game.gameExplosions.addExplosion((int) (X - 10), (int) (Y - 10), 0);
		if (game.gamePlayer.Team != Team)
		{
			if (hasHealth(TotalHealthMissile) == false)
			{
				game.gameNetCode.SendTCP("death " + Owner);
				game.gamePlayer.Dead = true;
				game.gameExplosions.addExplosion(
						(int) (game.gamePlayer.X - 16),
						(int) (game.gamePlayer.Y - 16), 2);
				game.gameSound.play3DSound(GameResources.sndExplosion,
						game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
				Health = 0;
				Energy = 0;
				game.gamePlayers.setDeaths(game.gamePlayer.Index,
						game.gamePlayers.getDeaths(game.gamePlayer.Index) + 1);
				game.gamePlayers.setDead(game.gamePlayer.Index, true);
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Owner),
						" destroyed ", game.gamePlayer.Name, game.gameChat
								.getColor(game.gamePlayers.getTeam(Owner)),
						GameResources.colorWhite, game.gameChat
								.getColor(game.gamePlayer.Team));
			}
			else
				takeHealth(TotalHealthMissile);
		}
	}

	public final void grenadeHit(int Owner, int Team)
	{
		if (game.gamePlayer.Team != Team && game.gamePlayer.HoldingPen == false)
		{
			if (hasHealth(TotalHealthGrenade) == false)
			{
				game.gameNetCode.SendTCP("death " + Owner);
				game.gamePlayer.Dead = true;
				game.gameExplosions.addExplosion(
						(int) (game.gamePlayer.X - 16),
						(int) (game.gamePlayer.Y - 16), 2);
				if (game.gameSound != null)
					game.gameSound.play3DSound(GameResources.sndExplosion,
							game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
				Health = 0;
				Energy = 0;
				game.gamePlayers.setDeaths(game.gamePlayer.Index,
						game.gamePlayers.getDeaths(game.gamePlayer.Index) + 1);
				game.gamePlayers.setDead(game.gamePlayer.Index, true);
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Owner),
						" destroyed ", game.gamePlayer.Name, game.gameChat
								.getColor(game.gamePlayers.getTeam(Owner)),
						GameResources.colorWhite, game.gameChat
								.getColor(game.gamePlayer.Team));
			}
			else
				takeHealth(TotalHealthGrenade);
		}
	}

	public final void bounceHit(int Owner, int Team, int X, int Y, float Angle)
	{
		game.gameSound.play3DSound(GameResources.sndHitBouncy, (float) X,
				(float) Y);
		for (int i = 0; i < 30; i++)
		{
			game.gameSparks.addSpark(X, Y, Angle);
		}
		game.gameNetCode.SendUDP("hit " + Owner + " " + weaponBouncy + " "
				+ TotalHealthLaser + " " + Health + " " + X + " " + Y + " "
				+ Angle);
		if (game.gamePlayer.Team != Team)
		{
			if (hasHealth(TotalHealthBouncy) == false)
			{
				game.gameNetCode.SendTCP("death " + Owner);
				game.gamePlayer.Dead = true;
				game.gameExplosions.addExplosion(
						(int) (game.gamePlayer.X - 16),
						(int) (game.gamePlayer.Y - 16), 2);
				game.gameSound.play3DSound(GameResources.sndExplosion,
						game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
				Health = 0;
				Energy = 0;
				game.gamePlayers.setDeaths(game.gamePlayer.Index,
						game.gamePlayers.getDeaths(game.gamePlayer.Index) + 1);
				game.gamePlayers.setDead(game.gamePlayer.Index, true);
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Owner),
						" destroyed ", game.gamePlayer.Name, game.gameChat
								.getColor(game.gamePlayers.getTeam(Owner)),
						GameResources.colorWhite, game.gameChat
								.getColor(game.gamePlayer.Team));
			}
			else
				takeHealth(TotalHealthBouncy);
		}
	}

	public final void missileShrapnelHit(int Owner, int Team)
	{
		if (game.gamePlayer.Team != Team)
		{
			if (hasHealth(TotalHealthMissileShrapnel) == false)
			{
				game.gameNetCode.SendTCP("death " + Owner);
				game.gamePlayer.Dead = true;
				game.gameExplosions.addExplosion(
						(int) (game.gamePlayer.X - 16),
						(int) (game.gamePlayer.Y - 16), 2);
				game.gameSound.play3DSound(GameResources.sndExplosion,
						game.gamePlayer.X - 16, game.gamePlayer.Y - 16);
				Health = 0;
				Energy = 0;
				game.gamePlayers.setDeaths(game.gamePlayer.Index,
						game.gamePlayers.getDeaths(game.gamePlayer.Index) + 1);
				game.gamePlayers.setDead(game.gamePlayer.Index, true);
				game.gameChat.Add3PartMessage(game.gamePlayers.getName(Owner),
						" destroyed ", game.gamePlayer.Name, game.gameChat
								.getColor(game.gamePlayers.getTeam(Owner)),
						colorWhite, game.gameChat
								.getColor(game.gamePlayer.Team));
			}
			else
				takeHealth(TotalHealthMissileShrapnel);
		}
	}

	public final void shootLaser(double shootX, double shootY, double Angle)
	{
		if (game.gamePlayer.Dead == false && game.Tick > timeClick)
		{
			timeClick = game.Tick + 92;
			if (hasEnergy(EnergyLaser) == true)
			{
				game.gameLasers.addLaser(shootX, shootY, Angle,
						game.gamePlayer.Index);
				if (game.gameNetCode != null)
					game.gameNetCode.SendUDP("laser "
							+ (float) SparkUtil.roundDouble(shootX, 2) + " "
							+ (float) SparkUtil.roundDouble(shootY, 2) + " "
							+ SparkUtil.roundDouble(Angle, 4));
				game.gameSound.play3DSound(GameResources.sndLaser,
						(int) shootX, (int) shootY);
				takeEnergy(EnergyLaser);
			}
		}
	}

	public final void shootMissile(double shootX, double shootY, double Angle)
	{
		if (game.gamePlayer.Dead == false)
		{
			if (MissileCharges > 0 && hasEnergy(EnergyMissile) == true)
			{
				MissileCharges--;
				game.gameMissiles.addMissile(shootX, shootY, Angle,
						game.gamePlayer.Index);
				game.gameNetCode.SendUDP("missile "
						+ (float) SparkUtil.roundDouble(shootX, 2) + " "
						+ (float) SparkUtil.roundDouble(shootY, 2) + " "
						+ SparkUtil.roundDouble(Angle, 4));
				game.gameSound.play3DSound(GameResources.sndMissile,
						(int) shootX, (int) shootY);
				takeEnergy(EnergyMissile);
			}
		}
	}

	public final void shootGrenade(double shootX, double shootY, int x, int y,
			double Angle)
	{
		if (game.gamePlayer.Dead == false)
		{
			if (GrenadeCharges > 0 && hasEnergy(EnergyGrenade) == true)
			{
				double DestX = shootX + x - game.gamePlayer.OffsetX - 16;
				double DestY = shootY + y - game.gamePlayer.OffsetY - 16;
				DestX = SparkUtil.roundDouble(DestX, 2);
				DestY = SparkUtil.roundDouble(DestY, 2);
				GrenadeCharges--;
				game.gameGrenades.addGrenade(shootX, shootY, DestX, DestY,
						Angle, game.gamePlayer.Index);
				game.gameNetCode.SendUDP("grenade "
						+ (float) SparkUtil.roundDouble(shootX, 2) + " "
						+ (float) SparkUtil.roundDouble(shootY, 2) + " "
						+ SparkUtil.roundDouble(Angle, 4) + " "
						+ (float) SparkUtil.roundDouble(DestX, 2) + " "
						+ (float) SparkUtil.roundDouble(DestY, 2));
				game.gameSound.play3DSound(GameResources.sndGrenade,
						(int) shootX, (int) shootY);
				takeEnergy(EnergyGrenade);
			}
		}
	}

	public final void shootBouncy(double shootX, double shootY, double Angle)
	{
		if (game.gamePlayer.Dead == false)
		{
			if (BouncyCharges > 0 && hasEnergy(EnergyBouncy) == true)
			{
				BouncyCharges--;
				game.gameBouncies.addBounce(shootX, shootY, Angle,
						game.gamePlayer.Index);
				game.gameNetCode.SendUDP("bouncy "
						+ (float) SparkUtil.roundDouble(shootX, 2) + " "
						+ (float) SparkUtil.roundDouble(shootY, 2) + " "
						+ SparkUtil.roundDouble(Angle, 4));
				game.gameSound.play3DSound(GameResources.sndBouncy,
						(int) shootX, (int) shootY);
				takeEnergy(EnergyBouncy);
			}
		}
	}

	public final void addHealthBonus()
	{
		Health += HealthBonus;
		if (Health > HealthSmoke && game.gamePlayer.Smoking == true)
		{
			game.gamePlayer.Smoking = false;
			game.gameNetCode.SendUDP("smoking 0");
		}
		if (Health > MaxHealth)
			Health = MaxHealth;
	}
}
