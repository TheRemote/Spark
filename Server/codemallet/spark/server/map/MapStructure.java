package codemallet.spark.server.map;
public class MapStructure
{
	public int id;

	public int header;

	public char version;

	public int width;

	public int height;

	public char maxPlayers;

	public char holdingTime;

	public char numTeams;

	public char objective;

	public char laserDamage;

	public char specialDamage;

	public char rechargeRate;

	public char misslesEnabled;

	public char bombsEnabled;

	public char bounciesEnabled;

	public int powerupCount;

	public char maxSimulPowerups;

	public char switchCount;

	public char flagCount[] = new char[4];

	public char flagPoleCount[] = new char[4];

	public char flagPoleData[] = new char[4];

	public int nameLength;

	public String name;

	public int descriptionLength;

	public String description;

	public char neutralCount;
	
	public int[] MapData;
	
	public int MissileTime;

	public int GrenadeTime;

	public int BouncyTime;
}
