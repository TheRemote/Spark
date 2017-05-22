package codemallet.spark.player;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.LinkedList;

public final class IgnoreList
{
	public LinkedList<String> IgnoreList;

	public IgnoreList()
	{
		LoadIgnoreList();
	}

	@SuppressWarnings("unchecked")
	public void LoadIgnoreList()
	{
		try
		{
			ObjectInputStream s = new ObjectInputStream(new FileInputStream(
					"portal/IgnoreList.dat"));

			IgnoreList = (LinkedList<String>) s.readObject();
			s.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
			IgnoreList = new LinkedList<String>();
			SaveIgnoreList();
		}
	}

	public void SaveIgnoreList()
	{
		new File("portal//IgnoreList.dat").delete();

		try
		{
			ObjectOutputStream s = new ObjectOutputStream(new FileOutputStream(
					"portal/IgnoreList.dat"));

			s.writeObject(IgnoreList);
			s.flush();
			s.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
