#include "uc_j_OSClock.h"
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
	#include <windows.h>
	static double nanos_per_count;
#else
	#include <sys/time.h>
	#ifdef __APPLE__
		#include <mach/mach_time.h>
		extern uint64_t mach_approximate_time(void) __attribute__((weak_import));
		static double mach_timebase_ratio;
	#endif
#endif

/**
 * Initialize JNI
 */
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
#if defined _WIN32
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	nanos_per_count = 1.0E9 / freq.QuadPart;
#elif defined __APPLE__
	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);
	mach_timebase_ratio = (double)timebase_info.numer / (double)timebase_info.denom;
#endif
	return JNI_VERSION_1_2;
}

/**
 * Returns milliseconds since epoch, like System.currentTimeMillis()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_currentTimeMillis(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	ULARGE_INTEGER li;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64_t ms = li.QuadPart / 10000ULL - 11644473600000ULL;
	return (jlong)ms;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	uint64_t ms = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
	return (jlong)ms;
#endif
}

/**
 * Returns nanoseconds from some arbitrary point, like System.nanoTime()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_nanoTime(JNIEnv *env, jclass cls) {
#if defined _WIN32
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	jlong ns = (jlong)(counter.QuadPart * nanos_per_count);
	return ns;
#elif defined __APPLE__
	uint64_t tm;
	if (mach_approximate_time != NULL) {
		tm = mach_approximate_time();
	}
	else {
		tm = mach_absolute_time();
	}
	jlong ns = (jlong)(tm * mach_timebase_ratio);
	return ns;
#elif defined CLOCK_MONOTONIC
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	jlong ns = ts.tv_sec * 1000000000LL + ts.tv_nsec;
	return ns;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	jlong ns = tv.tv_sec * 1000000000LL + tv.tv_usec * 1000LL;
	return ns;
#endif
}

/**
 * High performance, fast, save but less accurate version of currentTimeMillis()
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_getEpochMillis(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	ULARGE_INTEGER li;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64_t ms = li.QuadPart / 10000ULL - 11644473600000ULL;
	return (jlong)ms;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME_COARSE
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME_COARSE, &ts);

	uint64_t ms = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	uint64_t ms = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
	return (jlong)ms;
#endif
}

/**
 * High performance, fast, save but less accurate to returns seconds since epoch
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_getEpochSecond(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	ULARGE_INTEGER li;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64_t sec = li.QuadPart / 10000000ULL - 11644473600ULL;
	return (jlong)sec;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (jlong)tv.tv_sec;
#elif defined CLOCK_REALTIME_COARSE
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME_COARSE, &ts);

	return (jlong)ts.tv_sec;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	return (jlong)ts.tv_sec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (jlong)tv.tv_sec;
#endif
}