package codemallet.spark.input;

// JFC
import java.awt.event.KeyEvent;
import java.lang.reflect.Field;

import org.lwjgl.input.Keyboard;

/**
 * Converts LWJGL to AWT keys, and vice versa
 */
public final class KeyConversion
{
	/**
	 * AWT to LWJGL key constants conversion.
	 */
	protected static final int[] LWJGL_KEY_CONVERSION = new int[Keyboard.KEYBOARD_SIZE];

	/**
	 * LWJGL to AWT key constants conversion.
	 */
	protected static final int[] AWT_KEY_CONVERSION = new int[550];

	/**
	 * Conversion LWJGL <-> AWT keycode.
	 */
	static
	{
		// AWT -> LWJGL conversion
		// used for keypressed and keyreleased
		// mapping Keyboard.KEY_ -> KeyEvent.VK_

		// loops through all of the registered keys in KeyEvent
		Field[] keys = KeyEvent.class.getFields();
		for (int i = 0; i < keys.length; i++)
		{

			try
			{
				// Converts the KeyEvent constant name to the LWJGL constant
				// name
				String field = "KEY_" + keys[i].getName().substring(3);
				Field lwjglKey = Keyboard.class.getField(field);

				// Sets LWJGL index to be the KeyCode value
				LWJGL_KEY_CONVERSION[lwjglKey.getInt(null)] = keys[i]
						.getInt(null);

			}
			catch (Exception e)
			{
			}
		}

		try
		{
			LWJGL_KEY_CONVERSION[Keyboard.KEY_BACK] = KeyEvent.VK_BACK_SPACE;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_LBRACKET] = KeyEvent.VK_BRACELEFT;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_RBRACKET] = KeyEvent.VK_BRACERIGHT;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_APOSTROPHE] = KeyEvent.VK_QUOTE;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_GRAVE] = KeyEvent.VK_BACK_QUOTE;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_BACKSLASH] = KeyEvent.VK_BACK_SLASH;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_CAPITAL] = KeyEvent.VK_CAPS_LOCK;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_NUMLOCK] = KeyEvent.VK_NUM_LOCK;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_SCROLL] = KeyEvent.VK_SCROLL_LOCK;

			// two to one buttons mapping
			LWJGL_KEY_CONVERSION[Keyboard.KEY_RETURN] = KeyEvent.VK_ENTER;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_NUMPADENTER] = KeyEvent.VK_ENTER;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_LCONTROL] = KeyEvent.VK_CONTROL;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_RCONTROL] = KeyEvent.VK_CONTROL;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_LSHIFT] = KeyEvent.VK_SHIFT;
			LWJGL_KEY_CONVERSION[Keyboard.KEY_RSHIFT] = KeyEvent.VK_SHIFT;
		}
		catch (Exception e)
		{
		}

		// LWJGL -> AWT conversion
		// used for keydown
		// mapping KeyEvent.VK_ -> Keyboard.KEY_
		try
		{
			AWT_KEY_CONVERSION[KeyEvent.VK_BACK_SPACE] = Keyboard.KEY_BACK;
			AWT_KEY_CONVERSION[KeyEvent.VK_BRACELEFT] = Keyboard.KEY_LBRACKET;
			AWT_KEY_CONVERSION[KeyEvent.VK_BRACERIGHT] = Keyboard.KEY_RBRACKET;
			AWT_KEY_CONVERSION[KeyEvent.VK_ENTER] = Keyboard.KEY_RETURN;
			AWT_KEY_CONVERSION[KeyEvent.VK_QUOTE] = Keyboard.KEY_APOSTROPHE;
			AWT_KEY_CONVERSION[KeyEvent.VK_BACK_QUOTE] = Keyboard.KEY_GRAVE;
			AWT_KEY_CONVERSION[KeyEvent.VK_BACK_SLASH] = Keyboard.KEY_BACKSLASH;
			AWT_KEY_CONVERSION[KeyEvent.VK_CAPS_LOCK] = Keyboard.KEY_CAPITAL;
			AWT_KEY_CONVERSION[KeyEvent.VK_NUM_LOCK] = Keyboard.KEY_NUMLOCK;
			AWT_KEY_CONVERSION[KeyEvent.VK_SCROLL_LOCK] = Keyboard.KEY_SCROLL;
			AWT_KEY_CONVERSION[KeyEvent.VK_PAGE_UP] = Keyboard.KEY_PRIOR;
			AWT_KEY_CONVERSION[KeyEvent.VK_PAGE_DOWN] = Keyboard.KEY_NEXT;
			AWT_KEY_CONVERSION[KeyEvent.VK_SHIFT] = Keyboard.KEY_RSHIFT;
			AWT_KEY_CONVERSION[KeyEvent.VK_CONTROL] = Keyboard.KEY_RCONTROL;
		}
		catch (Exception e)
		{
		}

		// loops through all of the registered keys in Keyboard
		keys = Keyboard.class.getFields();
		for (int i = 0; i < keys.length; i++)
		{

			try
			{
				// LWJGL constant -> AWT constant
				String field = "VK_" + keys[i].getName().substring(4);
				Field awtKey = KeyEvent.class.getField(field);

				// Sets AWT index to be the LWJGL value
				AWT_KEY_CONVERSION[awtKey.getInt(null)] = keys[i].getInt(null);

			}
			catch (Exception e)
			{

			}
		}

	}

	/** ************************************************************************* */
	/** ****************** LWJGL <-> AWT SPECIFIC FUNCTION ********************** */
	/** ************************************************************************* */

	/**
	 * Conversion LWJGL key code to AWT key code.
	 * 
	 * @param lwjglKeyCode
	 *            the LWJGL key code to convert
	 * @return The AWT key code for the given LWJGL key code.
	 */
	public static int convertToAWT(int lwjglKeyCode)
	{
		try
		{
			return LWJGL_KEY_CONVERSION[lwjglKeyCode];
		}
		catch (ArrayIndexOutOfBoundsException e)
		{
			System.err.println("ERROR: Invalid LWJGL KeyCode " + lwjglKeyCode);
			return -1;
		}
	}

	/**
	 * Conversion AWT key code to LWJGL key code.
	 * 
	 * @param awtKeyCode
	 *            the AWT key code to convert
	 * @return The LWJGL key code for the given AWT key code.
	 */
	public static int convertToLWJGL(int awtKeyCode)
	{
		try
		{
			return AWT_KEY_CONVERSION[awtKeyCode];
		}
		catch (ArrayIndexOutOfBoundsException e)
		{
			System.err.println("ERROR: Invalid AWT KeyCode " + awtKeyCode);
			return -1;
		}
	}

	/**
	 * A hack to fix the fact that AWT left and right control and shift keys
	 * have no distinction
	 * 
	 * @param awtKeyCode
	 *            the key code to check if it needs a conversion
	 * @return The hacked key code
	 */
	public static int fixKeysHack(int keyCode)
	{
		if (keyCode == Keyboard.KEY_LSHIFT)
			return Keyboard.KEY_RSHIFT;
		if (keyCode == Keyboard.KEY_LCONTROL)
			return Keyboard.KEY_RCONTROL;

		return keyCode;
	}

}