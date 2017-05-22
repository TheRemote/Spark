package codemallet.spark.map;

public final class MapStructure
{
	int id;

	int header;

	char version;

	public int width;

	public int height;

	char maxPlayers;

	public char holdingTime;

	public char numTeams;

	char objective;

	public char laserDamage;

	public char specialDamage;

	char rechargeRate;

	char misslesEnabled;

	char bombsEnabled;

	char bounciesEnabled;

	int powerupCount;

	char maxSimulPowerups;

	char switchCount;

	final char flagCount[] = new char[4];

	final char flagPoleCount[] = new char[4];

	final char flagPoleData[] = new char[4];

	int nameLength;

	public String name;

	int descriptionLength;

	String description;

	char neutralCount;

	public int[] MapData;
}
