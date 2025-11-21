package uc.j;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;
public final class OSClock {
	static {
		NativeUtils.loadLibraryFromJar();
	}
	/**
	 * Returns the current time in milliseconds.
	 * @return The difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
	 * @since 1.0.0
	 * @see System#currentTimeMillis()
	 */
	public static final native long currentTimeMillis();
	/**
	 * This method can only be used to measure elapsed time and is not related to any other notion of system or wall-clock time.
	 * @return The current value of the running Java Virtual Machine's high-resolution time source, in nanoseconds
	 * @since 1.0.0
	 * @see System#nanoTime()
	 */
	public static final native long nanoTime();
	/**
	 * Fast, high performance but less accurate method to return the current time in seconds.
	 * @return The difference, measured in seconds, between the current time and midnight, January 1, 1970 UTC.
	 * @since 1.0.0
	 */
	public static final native long getEpochSecond();
	/**
	 * A fast, high performance but less accurate version of {@link System#currentTimeMillis()}.
	 * @return The difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
	 * @since 1.0.0
	 */
	public static final native long getEpochMillis();
	private static final class NativeUtils {
		private static final void loadLibraryFromJar() {
			String os = System.getProperty("os.name").toLowerCase(Locale.ENGLISH), arch = System.getProperty("os.arch").toLowerCase(Locale.ENGLISH), nativePathSuffix, libExtension;
			try {
				Class.forName("android.os.Build", false, OSClock.class.getClassLoader());
				libExtension = ".so";
				if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "android-arm64-v8a";
				else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "android-x86_64";
				else if(arch.indexOf("arm") >= 0) nativePathSuffix = "android-armeabi-v7a";
				else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "android-x86";
				else throw new RuntimeException("Unsupported Android architecture: " + arch);
			}
			catch(ClassNotFoundException e) {
				if(os.indexOf("mac") >= 0 || os.indexOf("darwin") >= 0) {
					libExtension = ".dylib";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "macos-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "macos-x64";
					else throw new RuntimeException("Unsupported macOS architecture: " + arch);
				}
				else if(os.indexOf("win") >= 0) {
					libExtension = ".dll";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "windows-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "windows-x64";
					else if(arch.indexOf("arm") >= 0) nativePathSuffix = "windows-arm";
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "windows-x86";
					else throw new RuntimeException("Unsupported Windows architecture: " + arch);
				}
				else if(os.indexOf("nux") >= 0) {
					libExtension = ".so";
					if(arch.equals("aarch64") || arch.equals("arm64")) nativePathSuffix = "linux-arm64";
					else if(arch.equals("amd64") || arch.equals("x86_64")) nativePathSuffix = "linux-x64";
					else if(arch.indexOf("arm") >= 0) nativePathSuffix = "linux-arm";
					else if(arch.equals("x86") || arch.equals("i386") || arch.equals("i686")) nativePathSuffix = "linux-x86";
					else throw new RuntimeException("Unsupported Linux architecture: " + arch);
				}
				else throw new RuntimeException("Unsupported OS: " + os);
			}
			String resourcePath = "/uc/j/native/" + nativePathSuffix + "/jOSClock" + libExtension;
			InputStream is = null;
			File temp = null;
			FileOutputStream fos = null;
			try {
				is = OSClock.class.getResourceAsStream(resourcePath);
				if(is == null) throw new UnsatisfiedLinkError("Native lib: " + resourcePath + " not found in JAR!");
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
				throw new RuntimeException("Failed to load native lib: " + resourcePath + ": " + e);
			}
			finally {
				if(fos != null) {
					try {
						fos.close();
					}
					catch(IOException ignored) {}
					finally {
						fos = null;
					}
				}
				if(is != null) {
					try {
						is.close();
					}
					catch(IOException ignored) {}
					finally {
						is = null;
					}
				}
			}
		}
	}
}