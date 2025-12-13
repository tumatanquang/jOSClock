# About `jOSClock`:
Simple JNI implementation that replaces `System.currentTimeMillis()`, backports `System.nanoTime()` and a few other high-performance time methods.

## Requirements:
- Java: **1.2+**.
- Android: See [Compatibility](#compatibility).

## Compatibility:

|         **OS / Arch**         | **Windows** | **Linux** | **macOS** |               **Android**              |
|:-----------------------------:|:-----------:|:---------:|:---------:|:--------------------------------------:|
|       `x86` (**32-bit**)      |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **2.3+** (API level **9+**) |
| `x64` (`x86-64` / **64-bit**) |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |
|  `ARM` (`ARM32` / `AArch32`)  |      ✔️      |     ✔️     |     ✖️     |  ✔️ Android **1.5+** (API level **3+**) |
|      `ARM64` (`AArch64`)      |      ✔️      |     ✔️     |     ✔️     | ✔️ Android **5.0+** (API level **21+**) |