package codemallet.spark.engines;

public final class MousePressEvent extends SparkEvent
{
	public int button;

	public int x;

	public int y;

	public MousePressEvent(int Type, int button, int x, int y)
	{
		super(Type);

		this.button = button;
		this.x = x;
		this.y = y;
	}
}
