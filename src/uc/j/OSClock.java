package uc.j;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;
/**
 * This class cannot be instantiated.
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
	private static final class NativeLoader {
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
				Class.forName("android.os.Build", false, OSClock.class.getClassLoader());
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
		private static final void loadLibrary(final String osArchDir, final String libExt) {
			final String resourcePath = new StringBuffer("/uc/j/native/").append(osArchDir).append("/jOSClock").append(libExt).toString();
			InputStream is = null;
			File temp = null;
			FileOutputStream fos = null;
			try {
				is = OSClock.class.getResourceAsStream(resourcePath);
				if(is == null) {
					throw new UnsatisfiedLinkError(new StringBuffer("Native lib: ").append(resourcePath).append(" not found in JAR!").toString());
				}
				temp = File.createTempFile("jOSClock", libExt);
				temp.deleteOnExit();
				fos = new FileOutputStream(temp);
				final byte[] buf = new byte[4096];
				for(int count; (count = is.read(buf)) > 0;) {
					fos.write(buf, 0, count);
				}
				fos.close();
				fos = null;
				is.close();
				is = null;
				System.load(temp.getAbsolutePath());
			}
			catch(final IOException e) {
				if(temp != null) {
					temp.delete();
				}
				throw new UnsatisfiedLinkError(new StringBuffer("Failed to load native lib: ").append(resourcePath).append(System.getProperty("line.separator")).append(e.toString()).toString());
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