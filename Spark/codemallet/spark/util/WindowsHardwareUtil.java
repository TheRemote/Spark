package codemallet.spark.util;

import static java.text.DateFormat.SHORT;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.text.DateFormat;
import java.util.Date;
import java.util.StringTokenizer;

public final class WindowsHardwareUtil
{
	private String VideoHardwareKey;

	public String VideoDisplayDevice;

	public String VideoDisplayService;

	public String VideoDriverDate;

	public String OperatingSystem;

	public String SystemPath;

	public String CPUName;

	public String CPUSpeed;

	public String CPUVendor;

	private static final String REGQUERY_UTIL = "reg query ";

	private static final String REGSTR_TOKEN = "REG_SZ";

	private static final String REGMSTR_TOKEN = "REG_MULTI_SZ";

	private static final String REGDWORD_TOKEN = "REG_DWORD";

	private static final String CPU_SPEED_CMD = REGQUERY_UTIL
			+ "\"HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\""
			+ " /v ~MHz";

	private static final String CPU_NAME_CMD = REGQUERY_UTIL
			+ "\"HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\""
			+ " /v ProcessorNameString";

	private static final String CPU_VENDOR_CMD = REGQUERY_UTIL
			+ "\"HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\""
			+ " /v VendorIdentifier";

	public WindowsHardwareUtil()
	{
		Initialize();
	}

	public void Initialize()
	{
		OperatingSystem = System.getProperty("os.name");
		SystemPath = System.getenv("windir");

		VideoHardwareKey = getVideoHardwareKey();
		VideoDisplayDevice = getVideoDisplayDevice();
		VideoDisplayService = getVideoDisplayService();
		VideoDriverDate = getVideoDriverDate();

		CPUName = getCPUName();
		CPUSpeed = getCPUSpeed();
		CPUVendor = getCPUVendor();
	}

	private String getVideoHardwareKey()
	{
		try
		{
			if (OperatingSystem.toLowerCase().contains("vista") == true)
			{
				Process process = Runtime
						.getRuntime()
						.exec(
								REGQUERY_UTIL
										+ "\"HKLM\\System\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Control\\VIDEO\"");
				StreamReader reader = new StreamReader(process.getInputStream());

				reader.start();
				process.waitFor();
				reader.join();

				String result = reader.getResult();

				StringTokenizer st = new StringTokenizer(result, "\\");
				String HardwareID = null;
				while (st.hasMoreTokens() == true)
				{
					HardwareID = st.nextToken().trim();
				}

				String FinalString = "\"HKLM\\System\\CurrentControlSet\\Control\\VIDEO\\";
				FinalString = FinalString + HardwareID + "\\0000\"";

				return FinalString;
			}
			else
			{
				Process process = Runtime
						.getRuntime()
						.exec(
								REGQUERY_UTIL
										+ "\"HKLM\\Hardware\\DeviceMap\\VIDEO\" /v \\Device\\Video0");
				StreamReader reader = new StreamReader(process.getInputStream());

				reader.start();
				process.waitFor();
				reader.join();

				String result = reader.getResult();
				int p = result.indexOf(REGSTR_TOKEN);

				if (p == -1)
					return null;

				result = result.substring(p + REGSTR_TOKEN.length()).trim()
						.toLowerCase();
				result = result.replace("\\registry\\machine", "HKLM");

				return result;
			}
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private static String getCPUSpeed()
	{
		try
		{
			Process process = Runtime.getRuntime().exec(CPU_SPEED_CMD);
			StreamReader reader = new StreamReader(process.getInputStream());

			reader.start();
			process.waitFor();
			reader.join();

			String result = reader.getResult();
			int p = result.indexOf(REGDWORD_TOKEN);

			if (p == -1)
				return null;

			// CPU speed in Mhz (minus 1) in HEX notation, convert it to DEC
			String temp = result.substring(p + REGDWORD_TOKEN.length()).trim();
			return Integer.toString((Integer.parseInt(temp.substring("0x"
					.length()), 16) + 1));
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private static String getCPUName()
	{
		try
		{
			Process process = Runtime.getRuntime().exec(CPU_NAME_CMD);
			StreamReader reader = new StreamReader(process.getInputStream());

			reader.start();
			process.waitFor();
			reader.join();

			String result = reader.getResult();
			int p = result.indexOf(REGSTR_TOKEN);

			if (p == -1)
				return null;

			return result.substring(p + REGSTR_TOKEN.length()).trim();
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private static String getCPUVendor()
	{
		try
		{
			Process process = Runtime.getRuntime().exec(CPU_VENDOR_CMD);
			StreamReader reader = new StreamReader(process.getInputStream());

			reader.start();
			process.waitFor();
			reader.join();

			String result = reader.getResult();
			int p = result.indexOf(REGSTR_TOKEN);

			if (p == -1)
				return null;

			return result.substring(p + REGSTR_TOKEN.length()).trim();
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private String getVideoDisplayDevice()
	{
		try
		{
			String line = REGQUERY_UTIL + VideoHardwareKey
					+ " /v \"Device Description\"";
			Process process = Runtime.getRuntime().exec(line);
			StreamReader reader = new StreamReader(process.getInputStream());

			reader.start();
			process.waitFor();
			reader.join();

			String result = reader.getResult();
			int p = result.indexOf(REGSTR_TOKEN);

			if (p == -1)
				return null;

			return result.substring(p + REGSTR_TOKEN.length()).trim();
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private String getVideoDisplayService()
	{
		try
		{
			String line = REGQUERY_UTIL + VideoHardwareKey
					+ " /v \"InstalledDisplayDrivers\"";
			Process process = Runtime.getRuntime().exec(line);
			StreamReader reader = new StreamReader(process.getInputStream());

			reader.start();
			process.waitFor();
			reader.join();

			String result = reader.getResult();
			int p = result.indexOf(REGMSTR_TOKEN);

			if (p == -1)
				return null;

			result = result.substring(p + REGMSTR_TOKEN.length()).trim();
			result = result.replace("\\0", "");

			return result;
		}
		catch (Exception e)
		{
			return null;
		}
	}

	private String getVideoDriverDate()
	{
		try
		{
			File file = new File(SystemPath + "\\system32\\"
					+ VideoDisplayService + ".dll");
			if (file.exists() == true)
			{
				long FileDate = file.lastModified();
				Date date = new Date(FileDate);
				DateFormat dateFormatter = DateFormat.getDateInstance(SHORT);
				String DateString = dateFormatter.format(date);
				return DateString;
			}
			else
			{
				return null;
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}

	static class StreamReader extends Thread
	{
		private InputStream is;

		private StringWriter sw;

		StreamReader(InputStream is)
		{
			this.is = is;
			sw = new StringWriter();
		}

		public void run()
		{
			try
			{
				int c;
				while ((c = is.read()) != -1)
					sw.write(c);
			}
			catch (IOException e)
			{
				;
			}
		}

		String getResult()
		{
			return sw.toString();
		}
	}
}
