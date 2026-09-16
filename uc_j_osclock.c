#include "uc_j_OSClock.h"
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
	#include <windows.h>
	typedef ULONGLONG (WINAPI *GetTickCount64_t)(void);
	static GetTickCount64_t GetTickCountU64;
	static uint64_t start_epoch_millis;
	static uint64_t start_tick_count;
	static double nanos_per_count;
#else
	#include <sys/time.h>
	#ifndef CLOCK_REALTIME_COARSE
		#define CLOCK_REALTIME_COARSE 5
	#endif
	#ifndef CLOCK_MONOTONIC_COARSE
		#define CLOCK_MONOTONIC_COARSE 6
	#endif
	#ifdef __APPLE__
		#include <mach/mach_time.h>
		extern uint64_t mach_approximate_time(void) __attribute__((weak_import));
		static double timebase_info_ratio;
	#endif
#endif

/**
 * Initialize JNI
 */
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
#if defined _WIN32
	HMODULE hmKernel32 = GetModuleHandleA("kernel32.dll");
	if(hmKernel32 != NULL) {
		GetTickCountU64 = (GetTickCount64_t)GetProcAddress(hmKernel32, "GetTickCount64");
		if(GetTickCountU64 != NULL) {
			FILETIME lpSystemTimeAsFileTime;
			GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
			uint64_t now_ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
			start_epoch_millis = now_ns / 10000ull - 11644473600000ull;
			start_tick_count = GetTickCountU64();
		}
	}
	LARGE_INTEGER lpFrequency;
	QueryPerformanceFrequency(&lpFrequency);
	nanos_per_count = 1.0e9 / (double)lpFrequency.QuadPart;
#elif defined __APPLE__
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	timebase_info_ratio = (double)info.numer / (double)info.denom;
#endif
#if defined __ANDROID__
	/*
	 * Android Runtime (ART) don't support JNI_VERSION_1_1.
	 * See: https://android.googlesource.com/platform/art/+/master/runtime/jni/java_vm_ext.cc#67
	 */
	return JNI_VERSION_1_2;
#else
	return JNI_VERSION_1_1;
#endif
}

/**
 * Returns milliseconds since epoch, like System.currentTimeMillis()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_currentTimeMillis(JNIEnv *env, jclass clazz) {
uint64_t ms;
#if defined _WIN32
	FILETIME lpSystemTimeAsFileTime;
	GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
	uint64_t now_ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
	ms = now_ns / 10000ull - 11644473600000ull;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ms = (uint64_t)tv.tv_sec * 1000ull + (uint64_t)tv.tv_usec / 1000ull;
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		ms = (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ms = (uint64_t)tv.tv_sec * 1000ull + (uint64_t)tv.tv_usec / 1000ull;
	}
#endif
	return (jlong)ms;
}

/**
 * Returns nanoseconds from some arbitrary point, like System.nanoTime()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_nanoTime(JNIEnv *env, jclass clazz) {
jlong ns;
#if defined _WIN32
	LARGE_INTEGER lpPerformanceCount;
	QueryPerformanceCounter(&lpPerformanceCount);
	ns = (jlong)((double)lpPerformanceCount.QuadPart * nanos_per_count);
#elif defined __APPLE__
	uint64_t now_tick = mach_absolute_time();
	ns = (jlong)(now_tick * timebase_info_ratio);
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		ns = (jlong)ts.tv_sec * 1000000000ll + (jlong)ts.tv_nsec;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ns = (jlong)tv.tv_sec * 1000000000ll + (jlong)tv.tv_usec * 1000ll;
	}
#endif
	return ns;
}

/**
 * Returns coarse monotonic milliseconds from some arbitrary point
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_monotonicMillis(JNIEnv *env, jclass clazz) {
uint64_t ms;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		ms = GetTickCountU64();
	}
	else {
		LARGE_INTEGER lpPerformanceCount;
		QueryPerformanceCounter(&lpPerformanceCount);
		ms = (uint64_t)((double)lpPerformanceCount.QuadPart * nanos_per_count / 1.0e6);
	}
#elif defined __APPLE__
	uint64_t now_tick;
	if(mach_approximate_time != NULL) {
		now_tick = mach_approximate_time();
	}
	else {
		now_tick = mach_absolute_time();
	}
	ms = (uint64_t)(now_tick * timebase_info_ratio / 1.0e6);
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC_COARSE, &ts) == 0 || clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		ms = (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ms = (uint64_t)tv.tv_sec * 1000ull + (uint64_t)tv.tv_usec / 1000ull;
	}
#endif
	return (jlong)ms;
}

/**
 * High performance, fast, save but less accurate to returns nanoseconds from some arbitrary point
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_monotonicNanos(JNIEnv *env, jclass clazz) {
uint64_t ns;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		ns = GetTickCountU64() * 1000000ull;
	}
	else {
		LARGE_INTEGER lpPerformanceCount;
		QueryPerformanceCounter(&lpPerformanceCount);
		ns = (uint64_t)((double)lpPerformanceCount.QuadPart * nanos_per_count);
	}
#elif defined __APPLE__
	uint64_t now_tick;
	if(mach_approximate_time != NULL) {
		now_tick = mach_approximate_time();
	}
	else {
		now_tick = mach_absolute_time();
	}
	ns = (uint64_t)(now_tick * timebase_info_ratio);
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC_COARSE, &ts) == 0 || clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		ns = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ns = (uint64_t)tv.tv_sec * 1000000000ull + (uint64_t)tv.tv_usec * 1000ull;
	}
#endif
	return (jlong)ns;
}

/**
 * Returns a fast, coarse-grained estimate of Unix epoch time, in seconds
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_epochSeconds(JNIEnv *env, jclass clazz) {
uint64_t secs;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		uint64_t now_tick = GetTickCountU64() - start_tick_count;
		secs = (start_epoch_millis + now_tick) / 1000ull;
	}
	else {
		FILETIME lpSystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
		uint64_t now_ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
		secs = now_ns / 10000000ull - 11644473600ull;
	}
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	secs = (uint64_t)tv.tv_sec;
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME_COARSE, &ts) == 0 || clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		secs = (uint64_t)ts.tv_sec;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		secs = (uint64_t)tv.tv_sec;
	}
#endif
	return (jlong)secs;
}

/**
 * Returns a fast, coarse-grained estimate of Unix epoch time, in milliseconds
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_epochMillis(JNIEnv *env, jclass clazz) {
uint64_t ms;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		uint64_t now_tick = GetTickCountU64() - start_tick_count;
		ms = start_epoch_millis + now_tick;
	}
	else {
		FILETIME lpSystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
		uint64_t now_ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
		ms = now_ns / 10000ull - 11644473600000ull;
	}
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ms = (uint64_t)tv.tv_sec * 1000ull + (uint64_t)tv.tv_usec / 1000ull;
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME_COARSE, &ts) == 0 || clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		ms = (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ms = (uint64_t)tv.tv_sec * 1000ull + (uint64_t)tv.tv_usec / 1000ull;
	}
#endif
	return (jlong)ms;
}