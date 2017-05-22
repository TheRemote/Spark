package codemallet.spark.engines;

public interface GameEngine
{
	public static int TargetFPS = 150;
	
	public static int MaxDelta = 30;
	
	public void Initialize();

	public void Destroy();

	public boolean hasFocus();

	public void beginUse(int Resource);

	public void endUse();

	public void drawImage(int Image, int DrawX, int DrawY, int DrawX2,
			int DrawY2, int SrcX, int SrcY, int SrcX2, int SrcY2);

	public void drawInUse(int DrawX, int DrawY, int DrawX2, int DrawY2,
			int SrcX, int SrcY, int SrcX2, int SrcY2);

	public void drawRect(float X, float Y, float Width, float Height);

	public void fillRect(float X, float Y, float Width, float Height);

	public void drawOval(float X, float Y, float Width, float Height);

	public void fillOval(float X, float Y, float Width, float Height);

	public void drawLine(float X1, float Y1, float X2, float Y2);

	public void drawGradientLine(float X1, float Y1, float Red1, float Green1,
			float Blue1, float Alpha1, float X2, float Y2, float Red2,
			float Green2, float Blue2, float Alpha2);

	public void setFont(int Font);

	public void drawString(String DrawString, int X, int Y);

	public int getStringWidth(String WidthString);

	public int getFontHeight();

	// Drawing
	public void setAntiAlias(boolean Enabled);

	public void setLineWidth(float Width);

	public void setColor(int Color);

	public void setColorRGBA(int Red, int Green, int Blue, int Alpha);

	public void setColorBlending(boolean Blending);
}
