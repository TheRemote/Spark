package codemallet.spark.util;

import java.io.FileInputStream;
import java.util.zip.CRC32;
import java.util.zip.CheckedInputStream;

public final class SparkUtil
{
	public static final int BUFFER_SIZE = 8192;

	public static final byte[] THROWAWAY_BUFFER = new byte[BUFFER_SIZE];

	public static final double roundDouble(double d, int places)
	{
		return Math.round(d * Math.pow(10, (double) places))
				/ Math.pow(10, (double) places);
	}

	public static long getCRC32(String File)
	{
		try
		{
			FileInputStream fis = new FileInputStream(File);
			CRC32 crc = new CRC32();
			CheckedInputStream cis = new CheckedInputStream(fis, crc);
			while (cis.read(THROWAWAY_BUFFER, 0, THROWAWAY_BUFFER.length) != -1)
				;

			cis.close();
			return crc.getValue();
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return -1;
		}
	}
}
