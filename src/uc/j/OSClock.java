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
	 * @return The coarse value from some arbitrary point in time source, in nanoseconds
	 * @since 1.0.9
	 * @see System#nanoTime()
	 */
	public static final native long monotonicNanos();
	/**
	 * High performance, fast, save but less accurate to returns milliseconds since epoch
	 * @return The coarse difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC
	 * @since 1.0.9
	 */
	public static final native long epochMillis();
	/**
	 * High performance, fast, save but less accurate to returns seconds since epoch
	 * @return The coarse difference, measured in seconds, between the current time and midnight, January 1, 1970 UTC
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
			String os = System.getProperty("os.name").toLowerCase(Locale.ENGLISH), arch = System.getProperty("os.arch").toLowerCase(Locale.ENGLISH), nativePathSuffix, libExtension;
			try {
				Class.forName("android.os.Build", false, OSClock.class.getClassLoader());
				libExtension = ".so";
				if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "android-arm64-v8a";
				else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "android-x86_64";
				else if(arch.indexOf("arm") >= 0) nativePathSuffix = "android-armeabi-v7a";
				else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "android-x86";
				else throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Android architecture: ").append(arch).toString());
			}
			catch(ClassNotFoundException e) {
				if(os.indexOf("mac") >= 0 || os.indexOf("darwin") >= 0) {
					libExtension = ".dylib";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "macos-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "macos-x64";
					else throw new UnsatisfiedLinkError(new StringBuffer("Unsupported macOS architecture: ").append(arch).toString());
				}
				else if(os.indexOf("win") >= 0) {
					libExtension = ".dll";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "windows-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "windows-x64";
					else if(arch.indexOf("arm") >= 0) nativePathSuffix = "windows-arm";
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "windows-x86";
					else throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Windows architecture: ").append(arch).toString());
				}
				else if(os.indexOf("nux") >= 0) {
					libExtension = ".so";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "linux-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "linux-x64";
					else if(arch.indexOf("arm") >= 0) nativePathSuffix = "linux-arm";
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "linux-x86";
					else throw new UnsatisfiedLinkError(new StringBuffer("Unsupported Linux architecture: ").append(arch).toString());
				}
				else throw new UnsatisfiedLinkError(new StringBuffer("Unsupported OS: ").append(os).toString());
			}
			String resourcePath = new StringBuffer("/uc/j/native/").append(nativePathSuffix).append("/jOSClock").append(libExtension).toString();
			InputStream is = null;
			File temp = null;
			FileOutputStream fos = null;
			try {
				is = OSClock.class.getResourceAsStream(resourcePath);
				if(is == null) throw new UnsatisfiedLinkError(new StringBuffer("Native lib: ").append(resourcePath).append(" not found in JAR!").toString());
				temp = File.createTempFile("jOSClock", libExtension);
				temp.deleteOnExit();
				fos = new FileOutputStream(temp);
				byte[] buf = new byte[4096];
				for(int count; (count = is.read(buf)) > 0;) {
					fos.write(buf, 0, count);
				}
				fos.close();
				fos = null;
				is.close();
				is = null;
				System.load(temp.getAbsolutePath());
			}
			catch(Exception e) {
				if(temp != null) temp.delete();
				throw new UnsatisfiedLinkError(new StringBuffer("Failed to load native lib: ").append(resourcePath).append(System.getProperty("line.separator")).append(e.toString()).toString());
			}
			finally {
				if(fos != null) {
					try {
						fos.close();
					}
					catch(IOException e) {}
					finally {
						fos = null;
					}
				}
				if(is != null) {
					try {
						is.close();
					}
					catch(IOException e) {}
					finally {
						is = null;
					}
				}
			}
		}
	}
}