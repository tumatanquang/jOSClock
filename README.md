# About `jOSClock`:

Simple JNI implementation that replaces `System.currentTimeMillis()`, backports `System.nanoTime()` and a few other high-performance time methods.

## Performance Tuning (Java **1.6+**):

The JVM does not inline JNI. To ensure optimal performance, you can add the following `java` command line option:

```console
-XX:CompileCommand="inline uc/j/OSClock currentTimeMillis" -XX:CompileCommand="inline uc/j/OSClock nanoTime" -XX:CompileCommand="inline uc/j/OSClock monotonicNanos" -XX:CompileCommand="inline uc/j/OSClock epochMillis" -XX:CompileCommand="inline uc/j/OSClock epochSeconds"
```

Alternatively, you can download the `.hotspot_compiler` file, then move to your application's folder, and add the following `java` command line option:

```console
-XX:CompileCommandFile=.hotspot_compiler
```

Otherwise, you will need to specify the full path to the `.hotspot_compiler` file:

```console
-XX:CompileCommandFile=/path/to/.hotspot_compiler
```

## Requirements:

- Android: See [Compatibility](#compatibility).

## Compatibility:

|         **OS / Arch**         | **Windows** | **Linux** | **macOS** |               **Android**              |
|:-----------------------------:|:-----------:|:---------:|:---------:|:--------------------------------------:|
|       `x86` (**32-bit**)      |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **2.3+** (API level **9+**) |
| `x64` (`x86-64` / **64-bit**) |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |
|  `ARM` (`ARM32` / `AArch32`)  |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **1.5+** (API level **3+**) |
|      `ARM64` (`AArch64`)      |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |