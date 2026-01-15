package uc.j;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;
import java.util.zip.CRC32;
/**
 * This class cannot be instantiated
 * @since 1.0.0
 */
public final class OSClock {
	static {
		NativeLoader.init();
	}
	/**
	 * Don't let anyone instantiate this class
	 */
	private OSClock() {
		throw new ExceptionInInitializerError("Do not instantiate");
	}
	/**
	 * Returns milliseconds since epoch, like {@link System#currentTimeMillis()}
	 * @return The difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC
	 * @since 1.0.0
	 * @see System#currentTimeMillis()
	 */
	public static final native long currentTimeMillis();
	/**
	 * Returns nanoseconds from some arbitrary point, like {@link System#nanoTime()}
	 * @return The current value of the running Java Virtual Machine's high-resolution time source, in nanoseconds
	 * @since 1.0.0
	 * @see System#nanoTime()
	 */
	public static final native long nanoTime();
	/**
	 * High performance, fast, save but less accurate to returns nanoseconds from some arbitrary point
	 * @return The coarse and monotonic value from some arbitrary point in time source, in nanoseconds
	 * @since 1.0.9
	 * @see System#nanoTime()
	 */
	public static final native long monotonicNanos();
	/**
	 * High performance, fast, save but less accurate and monotonic to returns milliseconds since epoch
	 * @return The coarse and monotonic difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC
	 * @since 1.0.9
	 */
	public static final native long epochMillis();
	/**
	 * High performance, fast, save but less accurate and monotonic to returns seconds since epoch
	 * @return The coarse and monotonic difference, measured in seconds, between the current time and midnight, January 1, 1970 UTC
	 * @since 1.0.9
	 */
	public static final native long epochSeconds();
	/**
	 * This class cannot be instantiated
	 * @since 1.0.0
	 */
	private static final class NativeLoader {
		/**
		 * User's home directory
		 */
		private static final String USER_HOME = "user.home";
		/**
		 * User's current working directory
		 */
		private static final String USER_DIR = "user.dir";
		/**
		 * Location of a temporary directory
		 * The location varies by application
		 * @see File#createTempFile(String, String)
		 */
		private static final String TEMP_DIR = "java.io.tmpdir";
		/**
		 * The name of the hidden folder where the target library file is extracted
		 */
		private static final String LIB_DIR = ".jOSClock";
		/**
		 * Don't let anyone instantiate this class
		 */
		private NativeLoader() {
			throw new ExceptionInInitializerError("Do not instantiate");
		}
		private static final void init() {
			final String os = System.getProperty("os.name").toLowerCase(Locale.ENGLISH);
			final String arch = System.getProperty("os.arch").toLowerCase(Locale.ENGLISH);
			try {
				Class.forName("android.os.Build");
				if(arch.equals("aarch64") || arch.equals("arm64")) {
					loadLibrary("android-arm64-v8a", ".so");
				}
				else if(arch.equals("amd64") || arch.equals("x86_64")) {
					loadLibrary("android-x86_64", ".so");
				}
				else if(arch.indexOf("arm") >= 0) {
					try {
						loadLibrary("android-armeabi-v7a", ".so");
					}
					catch(final UnsatisfiedLinkError e) {
						loadLibrary("android-armeabi", ".so");
					}
				}
				else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) {
					loadLibrary("android-x86", ".so");
				}
				else {
					throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Android architecture: ").append(arch).toString());
				}
			}
			catch(final ClassNotFoundException e) {
				if(os.indexOf("mac") >= 0 || os.indexOf("darwin") >= 0) {
					if(arch.equals("aarch64") || arch.equals("arm64")) {
						loadLibrary("macos-arm64", ".dylib");
					}
					else if(arch.equals("amd64") || arch.equals("x86_64")) {
						loadLibrary("macos-x64", ".dylib");
					}
					else {
						throw new UnsatisfiedLinkError(new StringBuffer("Unsupported macOS architecture: ").append(arch).toString());
					}
				}
				else if(os.indexOf("win") >= 0) {
					if(arch.equals("aarch64") || arch.equals("arm64")) {
						loadLibrary("windows-arm64", ".dll");
					}
					else if(arch.equals("amd64") || arch.equals("x86_64")) {
						loadLibrary("windows-x64", ".dll");
					}
					else if(arch.indexOf("arm") >= 0) {
						loadLibrary("windows-arm", ".dll");
					}
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) {
						loadLibrary("windows-x86", ".dll");
					}
					else {
						throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Windows architecture: ").append(arch).toString());
					}
				}
				else if(os.indexOf("nux") >= 0) {
					if(arch.equals("aarch64") || arch.equals("arm64")) {
						loadLibrary("linux-arm64", ".so");
					}
					else if(arch.equals("amd64") || arch.equals("x86_64")) {
						loadLibrary("linux-x64", ".so");
					}
					else if(arch.indexOf("arm") >= 0) {
						loadLibrary("linux-arm", ".so");
					}
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) {
						loadLibrary("linux-x86", ".so");
					}
					else {
						throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Linux architecture: ").append(arch).toString());
					}
				}
				else {
					throw new UnsatisfiedLinkError(new StringBuffer("Unsupported OS: ").append(os).toString());
				}
			}
		}
		private static final File initTargetFile(final String dirKey, final String fileName) {
			File targetFile = null;
			try {
				final String targetDir = System.getProperty(dirKey);
				if(targetDir != null && targetDir.length() != 0) {
					final File homeDir = new File(targetDir);
					final File libDir = new File(homeDir, LIB_DIR);
					if(!libDir.exists()) {
						libDir.mkdir();
					}
					if(libDir.exists() && libDir.isDirectory() && libDir.canWrite()) {
						targetFile = new File(libDir, fileName);
					}
					else if(homeDir.isDirectory() && homeDir.canWrite()) {
						targetFile = new File(homeDir, fileName);
					}
				}
			}
			catch(final Exception e) {}
			return targetFile;
		}
		private static final File initLibraryFile(final String fileName) {
			File libraryFile = initTargetFile(USER_HOME, fileName);
			if(libraryFile == null) {
				libraryFile = initTargetFile(USER_DIR, fileName);
			}
			if(libraryFile == null) {
				libraryFile = initTargetFile(TEMP_DIR, fileName);
			}
			if(libraryFile == null) {
				throw new ExceptionInInitializerError("Could not determine storage path for native library.");
			}
			return libraryFile;
		}
		private static final long getStreamChecksum(final InputStream in) throws IOException {
			final CRC32 crc32 = new CRC32();
			final byte[] buf = new byte[4096];
			int read;
			while((read = in.read(buf)) >= 0) {
				crc32.update(buf, 0, read);
			}
			return crc32.getValue();
		}
		private static final void loadLibrary(final String osArchDir, final String libExt) {
			final String fileName = new StringBuffer("jOSClock").append(libExt).toString();
			final String resourcePath = new StringBuffer("/uc/j/native/").append(osArchDir).append('/').append(fileName).toString();
			File f = null;
			InputStream is = null;
			FileOutputStream fos = null;
			try {
				f = initLibraryFile(fileName);
				if(f.exists() && f.canRead() && f.length() != 0) {
					is = OSClock.class.getResourceAsStream(resourcePath);
					if(is == null) {
						throw new UnsatisfiedLinkError(new StringBuffer("Native library file: ").append(resourcePath).append(" not found in JAR.").toString());
					}
					final long jarChecksum = getStreamChecksum(is);
					is.close();
					is = null;
					is = new FileInputStream(f);
					final long fileChecksum = getStreamChecksum(is);
					is.close();
					is = null;
					if(jarChecksum == fileChecksum) {
						try {
							System.load(f.getAbsolutePath());
							return;
						}
						catch(final UnsatisfiedLinkError e) {
							f.delete();
						}
					}
					else {
						f.delete();
					}
				}
				is = OSClock.class.getResourceAsStream(resourcePath);
				if(is == null) {
					throw new UnsatisfiedLinkError(new StringBuffer("Native library file: ").append(resourcePath).append(" not found in JAR.").toString());
				}
				fos = new FileOutputStream(f);
				final byte[] buf = new byte[4096];
				int read;
				while((read = is.read(buf)) >= 0) {
					fos.write(buf, 0, read);
				}
				fos.close();
				fos = null;
				is.close();
				is = null;
				try {
					System.load(f.getAbsolutePath());
				}
				catch(final UnsatisfiedLinkError e) {
					f.delete();
					throw e;
				}
			}
			catch(final Exception e) {
				if(f != null && f.exists()) {
					f.delete();
				}
				throw new UnsatisfiedLinkError(new StringBuffer("Failed to load native library file: ").append(resourcePath).append(System.getProperty("line.separator")).append(e.toString()).toString());
			}
			finally {
				if(fos != null) {
					try {
						fos.close();
					}
					catch(final IOException e) {}
					finally {
						fos = null;
					}
				}
				if(is != null) {
					try {
						is.close();
					}
					catch(final IOException e) {}
					finally {
						is = null;
					}
				}
			}
		}
	}
}