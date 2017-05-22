package codemallet.spark.server;

import java.text.SimpleDateFormat;

public class DateFormatting
{
	public SimpleDateFormat dateFormatDay;

	public SimpleDateFormat dateFormatMinute;

	public DateFormatting()
	{
		try
		{
			dateFormatDay = new SimpleDateFormat("dd MMM yyyy");

			dateFormatMinute = new SimpleDateFormat("dd MMM yyyy H:M a");
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
