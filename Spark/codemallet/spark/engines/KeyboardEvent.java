package codemallet.spark.engines;

public final class KeyboardEvent extends SparkEvent
{
	public int Key;

	public char c;

	public KeyboardEvent(int Type, int Key, char c)
	{
		super(Type);

		this.Key = Key;
		this.c = c;
	}
}
