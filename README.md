# About `jOSClock`:

Simple JNI implementation that replaces `System.currentTimeMillis()`, backports `System.nanoTime()` and a few other high-performance time methods.

## Requirements:

- Java: **1.1+** (because Java 1.0 does not support JNI).
- Android: See [Compatibility](#compatibility).

## Compatibility:

|         **OS / Arch**         | **Windows** | **Linux** | **macOS** |               **Android**              |
|:-----------------------------:|:-----------:|:---------:|:---------:|:--------------------------------------:|
|       `x86` (**32-bit**)      |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **2.3+** (API level **9+**) |
| `x64` (`x86-64` / **64-bit**) |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |
|  `ARM` (`ARM32` / `AArch32`)  |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **1.5+** (API level **3+**) |
|      `ARM64` (`AArch64`)      |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |

## Native library cache

The loader extracts the selected native library to `.jOSClock`, preferring `user.dir`, then `user.home`, then `java.io.tmpdir`.

When a cached library exists, the loader tries to read the resource's uncompressed size and CRC-32 from JAR entry metadata:

- A size mismatch skips checksum reads and replaces the cached library.
- When the size matches (or is unknown) and the entry CRC is available, only the cached file is read to calculate CRC-32.
- If JAR metadata is unavailable, the loader calculates CRC-32 for both the resource stream and cached file.

On Windows, the loader tries Java 7 NIO to set the directory's hidden attribute, falling back to `attrib +h` when unavailable. These APIs are optional: the legacy loading path remains available on older JVMs. Cache validation still reads the cached file when comparing CRC-32; it is not a zero-I/O path.

## Java compilation

Use JDK 8 with `-source 1.3 -target 1.1`. Put the JDK 1.1 stub first on the bootclasspath and the JDK 8 runtime second, so optional helper classes can reference JAR/NIO APIs without raising the bytecode target. No Java 5+ language features are used.

Manual Java compilation and JNI header generation in PowerShell (`JAVA_HOME` must point to JDK 8):

```powershell
javac -g:none -h build -d build -encoding utf8 -extdirs "" -source 1.3 -target 1.1 -bootclasspath "lib/rt-1_1_8_16.jar;$env:JAVA_HOME/jre/lib/rt.jar" src/uc/j/*.java
```

On Linux/macOS, use `:` instead of `;` in the bootclasspath and `${JAVA_HOME}` instead of `$env:JAVA_HOME`. The CI workflow uses this configuration.