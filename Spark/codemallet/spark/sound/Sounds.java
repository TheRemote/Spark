package codemallet.spark.sound;

import static codemallet.spark.engines.GameResources.colorWhite;

import java.io.FileInputStream;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.HashMap;

import org.lwjgl.BufferUtils;
import org.lwjgl.openal.AL;
import org.lwjgl.openal.AL10;
import org.lwjgl.openal.OpenALException;

import codemallet.spark.Spark;
import codemallet.spark.engines.GameResources;

public final class Sounds
{
	private final Spark game;

	public float MaxDistance = 0;

	public static final float MinVolume = 0.7f;

	public static final float Gain = 15.0f;

	// OpenAL
	public final int[] sounds;

	public final int[] soundChannels;

	/** The buffer used to set the position of a source */
	private final FloatBuffer sourcePos = BufferUtils.createFloatBuffer(3);

	/** The OpenGL AL sound sources in use */
	private IntBuffer sources;

	/** The number of sound sources enabled - default 8 */
	private int sourceCount;

	/** The maximum number of sources */
	private static final int maxSources = 64;

	/** The map of references to IDs of previously loaded sounds */
	private final HashMap<String, Integer> loaded = new HashMap<String, Integer>();

	/** Working IntBuffer optimization */
	private final IntBuffer intBuffer = BufferUtils.createIntBuffer(1);

	public Sounds(Spark game)
	{
		this.game = game;

		sounds = new int[GameResources.sndCount];
		soundChannels = new int[GameResources.sndCount];
	}

	public final void Initialize()
	{
		System.err.println("Initialising sounds..");

		AccessController.doPrivileged(new PrivilegedAction<Object>()
		{
			public Object run()
			{
				try
				{
					AL.create();
					System.err.println("OpenAL Sound is available");
				}
				catch (Exception e)
				{
					System.err.println("OpenAL Sound initialization failure.");
					e.printStackTrace();
					game.gameOptions.soundEnable = false;
				}

				return null;
			}
		});

		if (game.gameOptions.soundEnable == true)
		{
			sourceCount = 0;
			sources = BufferUtils.createIntBuffer(maxSources);
			while (AL10.alGetError() == AL10.AL_NO_ERROR)
			{
				IntBuffer temp = BufferUtils.createIntBuffer(1);

				try
				{
					AL10.alGenSources(temp);

					if (AL10.alGetError() == AL10.AL_NO_ERROR)
					{
						sourceCount++;
						sources.put(temp.get(0));
						if (sourceCount > maxSources - 1)
						{
							break;
						}
					}
				}
				catch (OpenALException e)
				{
					// expected at the end
					break;
				}
			}
			System.err.println(sourceCount + " OpenAL sources available");

			if (AL10.alGetError() != AL10.AL_NO_ERROR)
			{
				game.gameOptions.soundEnable = false;
				System.err.println("OpenAL init failed");
			}
			else
			{
				FloatBuffer listenerOri = BufferUtils.createFloatBuffer(6).put(
						new float[]
						{ 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f });
				FloatBuffer listenerPos = BufferUtils.createFloatBuffer(3).put(
						new float[]
						{ 0.0f, 0.0f, 0.0f });
				listenerPos.flip();
				listenerOri.flip();
				AL10.alListener(AL10.AL_POSITION, listenerPos);
				AL10.alListener(AL10.AL_ORIENTATION, listenerOri);

				System.err.println("OpenAL initialization complete");
			}
		}

		LoadSounds();
	}

	public final void LoadSounds()
	{
		if (game.gameOptions.soundEnable == false)
			return;

		System.err.println("Loading Sounds...");

		try
		{
			sounds[GameResources.sndLaser] = LoadOGG("spark/sounds/sndLaser.ogg");
			sounds[GameResources.sndSpark] = LoadOGG("spark/sounds/sndSpark.ogg");
			sounds[GameResources.sndMissile] = LoadOGG("spark/sounds/sndMissile.ogg");
			sounds[GameResources.sndGrenade] = LoadOGG("spark/sounds/sndGrenade.ogg");
			sounds[GameResources.sndBouncy] = LoadOGG("spark/sounds/sndBouncy.ogg");
			sounds[GameResources.sndBounce] = LoadOGG("spark/sounds/sndBounce.ogg");
			sounds[GameResources.sndExplosion] = LoadOGG("spark/sounds/sndExplosion.ogg");
			sounds[GameResources.sndHitLaser] = LoadOGG("spark/sounds/sndHitLaser.ogg");
			sounds[GameResources.sndHitMissile] = LoadOGG("spark/sounds/sndHitMissile.ogg");
			sounds[GameResources.sndHitGrenade] = LoadOGG("spark/sounds/sndHitGrenade.ogg");
			sounds[GameResources.sndHitBouncy] = LoadOGG("spark/sounds/sndHitBouncy.ogg");

			sounds[GameResources.sndWelcome] = LoadOGG("spark/sounds/sndWelcome.ogg");

			sounds[GameResources.sndOpenPanel] = LoadOGG("spark/sounds/sndOpenPanel.ogg");
			sounds[GameResources.sndClosePanel] = LoadOGG("spark/sounds/sndClosePanel.ogg");

			sounds[GameResources.sndArmorCritical] = LoadOGG("spark/sounds/sndArmorCritical.ogg");
			sounds[GameResources.sndSpawn] = LoadOGG("spark/sounds/sndSpawn.ogg");
			sounds[GameResources.sndDropFlag] = LoadOGG("spark/sounds/sndDropFlag.ogg");
			sounds[GameResources.sndWarp] = LoadOGG("spark/sounds/sndWarp.ogg");

			sounds[GameResources.sndSelectMissile] = LoadOGG("spark/sounds/sndSelectMissile.ogg");
			sounds[GameResources.sndSelectGrenade] = LoadOGG("spark/sounds/sndSelectGrenade.ogg");
			sounds[GameResources.sndSelectBouncy] = LoadOGG("spark/sounds/sndSelectBouncy.ogg");

			sounds[GameResources.sndWin] = LoadOGG("spark/sounds/sndWin.ogg");
			sounds[GameResources.sndLose] = LoadOGG("spark/sounds/sndLose.ogg");
			sounds[GameResources.sndTeam] = LoadOGG("spark/sounds/sndTeam.ogg");
			sounds[GameResources.sndFlagCaptured] = LoadOGG("spark/sounds/sndFlagCaptured.ogg");
			sounds[GameResources.sndFlagReturned] = LoadOGG("spark/sounds/sndFlagReturned.ogg");
			sounds[GameResources.sndBase] = LoadOGG("spark/sounds/sndBase.ogg");
			sounds[GameResources.sndTeamWins] = LoadOGG("spark/sounds/sndTeamWins.ogg");
			sounds[GameResources.sndSwitchFlipped] = LoadOGG("spark/sounds/sndSwitchFlipped.ogg");

			sounds[GameResources.sndTeamRed] = LoadOGG("spark/sounds/sndTeamRed.ogg");
			sounds[GameResources.sndTeamGreen] = LoadOGG("spark/sounds/sndTeamGreen.ogg");
			sounds[GameResources.sndTeamBlue] = LoadOGG("spark/sounds/sndTeamBlue.ogg");
			sounds[GameResources.sndTeamYellow] = LoadOGG("spark/sounds/sndTeamYellow.ogg");
			sounds[GameResources.sndTeamNeutral] = LoadOGG("spark/sounds/sndTeamNeutral.ogg");
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public final int LoadWAV(String File)
	{
		if (game.gameOptions.soundEnable == false)
		{
			return -1;
		}

		int buffer = -1;

		if (loaded.get(File) != null)
		{
			buffer = ((Integer) loaded.get(File)).intValue();
		}
		else
		{
			FileInputStream file = null;
			try
			{
				IntBuffer buf = BufferUtils.createIntBuffer(1);

				file = new FileInputStream(File);
				WaveData data = WaveData.create(file);
				AL10.alGenBuffers(buf);
				AL10.alBufferData(buf.get(0), data.format, data.data,
						data.samplerate);

				loaded.put(File, new Integer(buf.get(0)));
				buffer = buf.get(0);
			}
			catch (Exception e)
			{
				game.throwException(e);
			}
			finally
			{
				if (file != null)
				{
					try
					{
						file.close();
					}
					catch (Exception e)
					{

					}
				}
			}
		}

		if (buffer == -1)
		{
			System.err.println("Unable to load: " + File);
		}

		return buffer;
	}

	public final int LoadOGG(String File)
	{
		if (game.gameOptions.soundEnable == false)
		{
			return -1;
		}

		int buffer = -1;

		if (loaded.get(File) != null)
		{
			buffer = ((Integer) loaded.get(File)).intValue();
		}
		else
		{
			FileInputStream file = null;
			try
			{
				IntBuffer buf = BufferUtils.createIntBuffer(1);

				file = new FileInputStream(File);

				OggDecoder decoder = new OggDecoder();
				OggData ogg = decoder.getData(file);

				AL10.alGenBuffers(buf);
				AL10.alBufferData(buf.get(0),
						ogg.channels > 1 ? AL10.AL_FORMAT_STEREO16
								: AL10.AL_FORMAT_MONO16, ogg.data, ogg.rate);

				loaded.put(File, new Integer(buf.get(0)));
				buffer = buf.get(0);
			}
			catch (Exception e)
			{
				game.throwException(e);
			}
			finally
			{
				if (file != null)
				{
					try
					{
						file.close();
					}
					catch (Exception e)
					{

					}
				}
			}
		}

		if (buffer == -1)
		{
			System.err.println("Unable to load: " + File);
		}

		return buffer;
	}

	public float getVolume(int X, int Y)
	{
		int XDistance = (int) (game.gamePlayer.X - X + 16);
		int YDistance = (int) (game.gamePlayer.Y - Y + 16);
		int Distance = (int) Math.sqrt(Math.pow(XDistance, 2)
				+ Math.pow(YDistance, 2));
		if (Math.abs(XDistance) > MaxDistance
				|| Math.abs(YDistance) > MaxDistance)
			return 0;

		float Volume = 0.9f - (MinVolume * (Distance / MaxDistance));
		return Volume;
	}

	public float getPan(int X, int Y, float MaxDistance)
	{
		int XDistance = (int) (game.gamePlayer.X - X + 16);
		int YDistance = (int) (game.gamePlayer.Y - Y + 16);
		if (Math.abs(XDistance) > MaxDistance
				|| Math.abs(YDistance) > MaxDistance)
			return 0;

		float Pan = -(1.0f * (XDistance / MaxDistance));
		return Pan;
	}

	public void addFlagCaptured(int FlagTeam, int CaptureTeam)
	{
		if (game.gameOptions.soundEnable == false)
			return;

		switch (FlagTeam)
		{
			case 0:
				QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				QueueSound(GameResources.sndTeamYellow);
				break;
			case 4:
				QueueSound(GameResources.sndTeamNeutral);
				break;
		}

		QueueSound(GameResources.sndFlagCaptured);

		switch (CaptureTeam)
		{
			case 0:
				QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				QueueSound(GameResources.sndTeamYellow);
				break;
			case 4:
				QueueSound(GameResources.sndTeamNeutral);
				break;
		}

		QueueSound(GameResources.sndTeam);
	}

	public void addFlagReturned(int FlagTeam, int ReturnTeam)
	{
		if (game.gameOptions.soundEnable == false)
			return;

		switch (FlagTeam)
		{
			case 0:
				QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				QueueSound(GameResources.sndTeamYellow);
				break;
			case 4:
				QueueSound(GameResources.sndTeamNeutral);
				break;
		}

		QueueSound(GameResources.sndFlagReturned);

		switch (ReturnTeam)
		{
			case 0:
				QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				QueueSound(GameResources.sndTeamYellow);
				break;
			case 4:
				QueueSound(GameResources.sndTeamNeutral);
				break;
		}

		QueueSound(GameResources.sndBase);
	}

	public void addWin(int Team)
	{
		switch (Team)
		{
			case 0:
				game.gameChat.AddMessage("Green Team Wins!",
						GameResources.colorGreen);
				break;
			case 1:
				game.gameChat.AddMessage("Red Team Wins!",
						GameResources.colorRed);
				break;
			case 2:
				game.gameChat.AddMessage("Blue Team Wins!",
						GameResources.colorBlue);
				break;
			case 3:
				game.gameChat.AddMessage("Yellow Team Wins!",
						GameResources.colorYellow);
				break;
		}

		if (game.gameOptions.soundEnable == false)
			return;

		if (game.gamePlayer.Team > -1)
		{
			if (game.gamePlayer.Team == Team)
			{
				playSound(GameResources.sndWin);
			}
			else
			{
				playSound(GameResources.sndLose);
			}
		}

		switch (Team)
		{
			case 0:
				game.gameSound.QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				game.gameSound.QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				game.gameSound.QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				game.gameSound.QueueSound(GameResources.sndTeamYellow);
				break;
		}

		game.gameSound.QueueSound(GameResources.sndTeamWins);
	}

	public void addSwitchFlipped(int Index)
	{
		if (game.gameOptions.soundEnable == false)
			return;

		game.gameSound.QueueSound(GameResources.sndSwitchFlipped);

		switch (game.gamePlayers.getTeam(Index))
		{
			case 0:
				game.gameSound.QueueSound(GameResources.sndTeamGreen);
				break;
			case 1:
				game.gameSound.QueueSound(GameResources.sndTeamRed);
				break;
			case 2:
				game.gameSound.QueueSound(GameResources.sndTeamBlue);
				break;
			case 3:
				game.gameSound.QueueSound(GameResources.sndTeamYellow);
				break;
		}

		game.gameSound.QueueSound(GameResources.sndTeam);
		game.gameChat.Add2PartMessage(game.gamePlayers.getName(Index),
				" flipped a mega power switch", game.gameChat
						.getColor(game.gamePlayers.getTeam(Index)), colorWhite);
	}

	public void Cycle()
	{
		CheckBuffers();
	}

	private int playAsSoundAt(int buffer, float pitch, float gain, float x,
			float y)
	{
		// gain *= soundVolume;
		if (gain == 0)
		{
			gain = 0.001f;
		}
		if (game.gameOptions.soundEnable == true)
		{
			int nextSource = findFreeSource();
			if (nextSource == -1)
			{
				return -1;
			}

			AL10.alSourceStop(sources.get(nextSource));

			AL10.alSourcei(sources.get(nextSource), AL10.AL_BUFFER, buffer);
			AL10.alSourcef(sources.get(nextSource), AL10.AL_PITCH, pitch);
			AL10.alSourcef(sources.get(nextSource), AL10.AL_GAIN, gain);

			sourcePos.clear();
			sourcePos.put(new float[]
			{ x, y, 0.0f });
			sourcePos.flip();
			AL10.alSource(sources.get(nextSource), AL10.AL_POSITION, sourcePos);

			AL10.alSourcePlay(sources.get(nextSource));

			return nextSource;
		}

		return -1;
	}

	/**
	 * Find a free sound source
	 * 
	 * @return The index of the free sound source
	 */
	private int findFreeSource()
	{
		for (int i = 1; i < sourceCount - 1; i++)
		{
			int state = AL10.alGetSourcei(sources.get(i), AL10.AL_SOURCE_STATE);

			if (state != AL10.AL_PLAYING)
			{
				return i;
			}
		}

		return -1;
	}

	public void playSound(int Sound)
	{
		if (game.LoadingResources == true || game.LoadingGame == true
				|| game.gameOptions.soundEnable == false)
			return;

		if (game.gameEngine == null || game.gameEngine.hasFocus() == false)
			return;

		try
		{
			soundChannels[Sound] = playAsSoundAt(sounds[Sound], 1.0f, 1.0f,
					0.0f, 0.0f);
		}
		catch (Exception e)
		{
			game.throwException(e);
		}
	}

	public void play3DSound(int Sound, float X, float Y)
	{
		if (game.LoadingResources == true || game.LoadingGame == true
				|| game.gameOptions.soundEnable == false)
			return;

		if (game.gameEngine == null || game.gameEngine.hasFocus() == false)
			return;

		int XDistance = (int) (game.gamePlayer.X - X + 16);
		int YDistance = (int) (game.gamePlayer.Y - Y + 16);

		if (XDistance < game.gameSound.MaxDistance
				&& YDistance < game.gameSound.MaxDistance)
		{
			float PanX = -(Gain * (XDistance / game.gameSound.MaxDistance));
			float PanY = -(Gain * (YDistance / game.gameSound.MaxDistance));

			try
			{
				soundChannels[Sound] = playAsSoundAt(sounds[Sound], 1.0f, 1.0f,
						PanX, PanY);
			}
			catch (Exception e)
			{
				game.throwException(e);
			}
		}
	}

	private void CheckBuffers()
	{
		if (game.gameOptions.soundEnable == false)
			return;

		int TheChannel = sources.get(0);
		int Count = -1;
		if (TheChannel >= 0)
		{
			Count = AL10.alGetSourcei(TheChannel, AL10.AL_BUFFERS_PROCESSED);
			while (true)
			{
				if (Count > 0)
				{
					Count--;
					intBuffer.clear();
					intBuffer.put(0);
					intBuffer.flip();
					AL10.alSourceUnqueueBuffers(sources.get(0), intBuffer);
				}
				else
				{
					break;
				}
			}
		}
	}

	private void QueueSound(int sound)
	{
		if (game.gameOptions.soundEnable == false)
			return;

		intBuffer.clear();
		intBuffer.put(sounds[sound]);
		intBuffer.flip();
		AL10.alSourceQueueBuffers(sources.get(0), intBuffer);

		int state = AL10.alGetSourcei(sources.get(0), AL10.AL_SOURCE_STATE);

		if (state != AL10.AL_PLAYING)
		{
			sourcePos.clear();
			sourcePos.put(0).put(0).put(0);
			sourcePos.flip();

			int Source = sources.get(0);

			AL10.alSourceStop(Source);

			AL10.alSourcef(Source, AL10.AL_PITCH, 1.0f);
			AL10.alSourcef(Source, AL10.AL_GAIN, 1.0f);

			AL10.alSource(Source, AL10.AL_POSITION, sourcePos);
			AL10.alSourcePlay(Source);
		}
	}
}
