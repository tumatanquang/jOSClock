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
	#ifdef __APPLE__
		#include <mach/mach_time.h>
		extern uint64_t mach_approximate_time(void) __attribute__((weak_import));
		static double timebase_info_ratio;
	#endif
	#ifndef CLOCK_REALTIME_COARSE
		#define CLOCK_REALTIME_COARSE 5
	#endif
	#ifndef CLOCK_MONOTONIC_COARSE
		#define CLOCK_MONOTONIC_COARSE 6
	#endif
#endif

/**
 * Initialize JNI
 */
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
#if defined _WIN32
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if(hKernel32 != NULL) {
		GetTickCountU64 = (GetTickCount64_t)GetProcAddress(hKernel32, "GetTickCount64");
	}
	if(GetTickCountU64 != NULL) {
		FILETIME lpSystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
		uint64_t ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
		start_epoch_millis = ns / 10000ull - 11644473600000ull;
		start_tick_count = GetTickCountU64();
	}
	LARGE_INTEGER lpFrequency;
	QueryPerformanceFrequency(&lpFrequency);
	nanos_per_count = 1.0e9 / (double)lpFrequency.QuadPart;
#elif defined __APPLE__
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	timebase_info_ratio = (double)info.numer / (double)info.denom;
#endif
	return JNI_VERSION_1_2;
}

/**
 * Returns milliseconds since epoch, like System.currentTimeMillis()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_currentTimeMillis(JNIEnv *env, jclass cls) {
uint64_t ms;
#if defined _WIN32
	FILETIME lpSystemTimeAsFileTime;
	GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
	uint64_t ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
	ms = ns / 10000ull - 11644473600000ull;
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
Java_uc_j_OSClock_nanoTime(JNIEnv *env, jclass cls) {
jlong ns;
#if defined _WIN32
	LARGE_INTEGER lpPerformanceCount;
	QueryPerformanceCounter(&lpPerformanceCount);
	ns = (jlong)(lpPerformanceCount.QuadPart * nanos_per_count);
#elif defined __APPLE__
	uint64_t tm;
	if(mach_approximate_time != NULL) {
		tm = mach_approximate_time();
	}
	else {
		tm = mach_absolute_time();
	}
	ns = (jlong)(tm * timebase_info_ratio);
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		ns = (jlong)ts.tv_sec * 1000000000ll + ts.tv_nsec;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ns = tv.tv_sec * 1000000000ll + tv.tv_usec * 1000ll;
	}
#endif
	return (jlong)ns;
}

/**
 * High performance, fast, save but less accurate to returns nanoseconds from some arbitrary point
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_monotonicNanos(JNIEnv *env, jclass cls) {
uint64_t ns;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		ns = GetTickCountU64() * 1000000ull;
	}
	else {
		LARGE_INTEGER lpPerformanceCount;
		QueryPerformanceCounter(&lpPerformanceCount);
		ns = (jlong)(lpPerformanceCount.QuadPart * nanos_per_count);
	}
#elif defined __APPLE__
	uint64_t tm;
	if(mach_approximate_time != NULL) {
		tm = mach_approximate_time();
	}
	else {
		tm = mach_absolute_time();
	}
	ns = (uint64_t)(tm * timebase_info_ratio);
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
 * High performance, fast, save but less accurate to returns milliseconds since epoch
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_epochMillis(JNIEnv *env, jclass cls) {
uint64_t ms;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		uint64_t now_tick = GetTickCountU64() - start_tick_count;
		ms = start_epoch_millis + now_tick;
	}
	else {
		FILETIME lpSystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
		uint64_t ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
		ms = ns / 10000ull - 11644473600000ull;
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

/**
 * High performance, fast, save but less accurate to returns seconds since epoch
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_epochSeconds(JNIEnv *env, jclass cls) {
uint64_t sec;
#if defined _WIN32
	if(GetTickCountU64 != NULL) {
		uint64_t now_tick = GetTickCountU64() - start_tick_count;
		sec = (start_epoch_millis + now_tick) / 1000ull;
	}
	else {
		FILETIME lpSystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
		uint64_t ns = (uint64_t)lpSystemTimeAsFileTime.dwHighDateTime << 32ull | (uint64_t)lpSystemTimeAsFileTime.dwLowDateTime;
		sec = ns / 10000000ull - 11644473600ull;
	}
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sec = (uint64_t)tv.tv_sec;
#else
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME_COARSE, &ts) == 0 || clock_gettime(CLOCK_REALTIME, &ts) == 0) {
		sec = (uint64_t)ts.tv_sec;
	}
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		sec = (uint64_t)tv.tv_sec;
	}
#endif
	return (jlong)sec;
}