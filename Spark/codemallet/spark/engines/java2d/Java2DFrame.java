package codemallet.spark.engines.java2d;

import java.awt.Canvas;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.image.MemoryImageSource;

import javax.swing.ImageIcon;
import javax.swing.JFrame;

import codemallet.spark.Spark;
import codemallet.spark.engines.KeyboardEvent;
import codemallet.spark.engines.MousePressEvent;
import codemallet.spark.engines.SparkEvent;
import codemallet.spark.engines.SparkEvents;
import codemallet.spark.input.KeyConversion;

public final class Java2DFrame extends JFrame implements KeyListener, MouseListener,
		MouseWheelListener, WindowListener
{
	private static final long serialVersionUID = 1L;

	private final Spark game;

	private final GraphicsEnvironment ge;

	protected final GraphicsConfiguration gc;

	private final GraphicsDevice gd;

	protected final Canvas drawCanvas;

	protected boolean Iconified;

	protected boolean Destroying;

	public Java2DFrame(Spark game)
	{
		super(GraphicsEnvironment.getLocalGraphicsEnvironment()
				.getDefaultScreenDevice().getDefaultConfiguration());

		this.game = game;

		ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		gd = ge.getDefaultScreenDevice();
		gc = gd.getDefaultConfiguration();

		setTitle("Spark");

		setFocusable(false);
		setBackground(Color.black);
		setFocusTraversalKeysEnabled(false);
		setResizable(false);
		setIgnoreRepaint(true);

		if (game.gameOptions.displayWindowed == false)
			setUndecorated(true);

		setIconImage(new ImageIcon("spark/images/imgIcon.gif").getImage());

		// Invisible cursor
		int[] pixels = new int[16 * 16];
		Image image = Toolkit.getDefaultToolkit().createImage(
				new MemoryImageSource(16, 16, pixels, 0, 16));
		Cursor transparentCursor = Toolkit.getDefaultToolkit()
				.createCustomCursor(image, new Point(0, 0), "invisibleCursor");

		setCursor(transparentCursor);

		drawCanvas = new Canvas(gc);
		drawCanvas.setPreferredSize(new Dimension(game.ResolutionX,
				game.ResolutionY));
		drawCanvas.setBackground(Color.black);
		drawCanvas.setIgnoreRepaint(true);
		drawCanvas.setFocusTraversalKeysEnabled(false);
		drawCanvas.addKeyListener(this);
		drawCanvas.addMouseListener(this);
		drawCanvas.addMouseWheelListener(this);
		drawCanvas.setCursor(transparentCursor);
		add(drawCanvas);

		addWindowListener(this);

		pack();
		setLocationRelativeTo(null);

		if (game.gameOptions.displayWindowed == false)
		{
			setFullScreen();
		}

		setVisible(true);

		drawCanvas.requestFocus();
	}

	public final void keyPressed(KeyEvent e)
	{
		game.gameQueue.add(new KeyboardEvent(SparkEvents.E_KEYDOWN,
				KeyConversion.convertToLWJGL(e.getKeyCode()), e.getKeyChar()));
	}

	public final void keyReleased(KeyEvent e)
	{
		game.gameQueue.add(new KeyboardEvent(SparkEvents.E_KEYUP, KeyConversion
				.convertToLWJGL(e.getKeyCode()), e.getKeyChar()));
	}

	public final void keyTyped(KeyEvent e)
	{

	}

	public final void mousePressed(MouseEvent e)
	{
		int internalButton = 0;
		switch (e.getButton())
		{
			case MouseEvent.BUTTON1:
				internalButton = 0;
				break;
			case MouseEvent.BUTTON3:
				internalButton = 1;
				break;
			case MouseEvent.BUTTON2:
				internalButton = 2;
				break;
		}

		game.gameQueue.add(new MousePressEvent(SparkEvents.E_MOUSEDOWN,
				internalButton, e.getX(), e.getY()));
	}

	public final void mouseReleased(MouseEvent e)
	{
		int internalButton = 0;
		switch (e.getButton())
		{
			case MouseEvent.BUTTON1:
				internalButton = 0;
				break;
			case MouseEvent.BUTTON3:
				internalButton = 1;
				break;
			case MouseEvent.BUTTON2:
				internalButton = 2;
				break;
		}
		
		game.gameQueue.add(new MousePressEvent(SparkEvents.E_MOUSEUP,
				internalButton, e.getX(), e.getY()));
	}

	public final void mouseEntered(MouseEvent e)
	{

	}

	public final void mouseExited(MouseEvent e)
	{

	}

	public final void mouseClicked(MouseEvent e)
	{

	}

	public final void mouseWheelMoved(MouseWheelEvent e)
	{
		int notches = e.getWheelRotation();
		int button = 0;
		if (notches < 0)
		{
			button = 4;
		}
		else
		{
			button = 5;
		}

		game.gameQueue.add(new MousePressEvent(SparkEvents.E_MOUSEDOWN,
				button, e.getX(), e.getY()));
	}

	public final void windowClosing(WindowEvent e)
	{
		if (Destroying == true)
			return;

		System.err.println("Java2D Frame Closing.");

		dispose();
	}

	public final void windowClosed(WindowEvent e)
	{
		if (Destroying == true)
			return;

		System.err.println("Java2D Frame Closed.");

		game.gameQueue.add(new SparkEvent(SparkEvents.E_DESTROY));
	}

	public final void windowDeactivated(WindowEvent e)
	{

	}

	public final void windowActivated(WindowEvent e)
	{

	}

	public final void windowOpened(WindowEvent e)
	{

	}

	public final void windowIconified(WindowEvent e)
	{
		Iconified = true;
	}

	public final void windowDeiconified(WindowEvent e)
	{
		Iconified = false;
	}

	public final void setFullScreen()
	{
		gd.setFullScreenWindow(this);
		if (gd.isDisplayChangeSupported())
		{
			try
			{
				gd
						.setDisplayMode(new DisplayMode(game.ResolutionX,
								game.ResolutionY, 32,
								DisplayMode.REFRESH_RATE_UNKNOWN));
			}
			catch (IllegalArgumentException ex)
			{
				ex.printStackTrace();
			}
		}
	}
}
