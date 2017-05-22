package codemallet.spark.security;

import java.security.MessageDigest;

import codemallet.spark.util.SparkUtil;

public final class Security
{
	public static boolean CheckHash(String Original, String Hash)
	{
		MessageDigest md = null;
		MessageDigest md2 = null;
		StringBuffer FinalHash = null;
		try
		{
			md = MessageDigest.getInstance("SHA");
			md.reset();
			md.update(Original.getBytes());
			byte hash[] = md.digest();
			md2 = MessageDigest.getInstance("MD5");
			md2.reset();
			md2.update(hash);
			hash = md2.digest();
			FinalHash = new StringBuffer();
			for (int i = 0; i < hash.length; i++)
			{
				FinalHash.append(Integer.toHexString(0xFF & hash[i]));
			}
		}
		catch (Exception e)
		{
			System.err.println("Hash Exception");
			e.printStackTrace();
			return false;
		}

		if (FinalHash.toString().compareTo(Hash) != 0)
			return false;

		return true;
	}

	public static final boolean CheckCriticalFilesModified()
	{
		if (SparkUtil.getCRC32("spark/images/imgShipCollision.png") != 2121338462L)
			return true;
		if (SparkUtil.getCRC32("spark/images/imgCollision.png") != 3134881142L)
			return true;

		return false;
	}
}
