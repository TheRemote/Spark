package codemallet.spark.clan;

import java.util.HashMap;

import codemallet.spark.Spark;

public final class ClanList
{
	private final Spark game;

	public final HashMap<Integer, Clan> Clans = new HashMap<Integer, Clan>();

	public ClanList(Spark game)
	{
		this.game = game;
	}

	public Clan getClan(int Index)
	{
		return Clans.get(Index);
	}

	public void LoadClan(int Index, String Name, String Tag,
			boolean TagAfterName, boolean TagSpaceBetween)
	{
		Clan theClan = new Clan();

		theClan.inUse = true;
		theClan.Index = Index;
		theClan.Name = Name;
		theClan.Tag = Tag;
		theClan.TagAfterName = TagAfterName;
		theClan.TagSpaceBetween = TagSpaceBetween;
		Clans.put(Index, theClan);

		System.err.println("New clan:  " + theClan.Name);
	}

	public final void DestroyClan(int Index)
	{
		Clans.put(Index, null);

		game.gamePlayers.removeAllFromClan(Index);
	}

	public final boolean inUse(int Index)
	{
		Clan theClan = Clans.get(Index);
		if (theClan != null)
		{
			return theClan.inUse;
		}

		return false;
	}
}
