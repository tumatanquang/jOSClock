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
		extern uint64_t clock_gettime_nsec_np(clockid_t) __attribute__((weak_import));
		extern uint64_t mach_approximate_time(void) __attribute__((weak_import));
		static bool clock_gettime_nsec_np_exist;
		static bool mach_approximate_time_exist;
		static double timebase_info_ratio;
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
	nanos_per_count = 1.0E9 / (uint64_t)freq.QuadPart;
#elif defined __APPLE__
	clock_gettime_nsec_np_exist = (void*)clock_gettime_nsec_np != NULL;
	mach_approximate_time_exist = (void*)mach_approximate_time != NULL;
	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);
	timebase_info_ratio = (double)timebase_info.numer / (double)timebase_info.denom;
#endif
	return JNI_VERSION_1_2;
}

/**
 * Returns milliseconds since epoch, like System.currentTimeMillis()
 * The return value is always positive
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_currentTimeMillis(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t ns = (uint64_t)ft.dwHighDateTime << 32ULL | (uint64_t)ft.dwLowDateTime;
	uint64_t ms = ns / 10000ULL - 11644473600000ULL;
	return (jlong)ms;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t ms = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
	return (jlong)ms;
#endif
}

/**
 * Returns nanoseconds from some arbitrary point, like System.nanoTime()
 * The return value is always positive
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_nanoTime(JNIEnv *env, jclass cls) {
#if defined _WIN32
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	uint64_t ns = (uint64_t)((uint64_t)counter.QuadPart * nanos_per_count);
	return (jlong)ns;
#elif defined __APPLE__
	uint64_t tm;
	if(clock_gettime_nsec_np_exist) {
		tm = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
	}
	else if(mach_approximate_time_exist) {
		tm = mach_approximate_time();
	}
	else {
		tm = mach_absolute_time();
	}
	uint64_t ns = (uint64_t)(tm * timebase_info_ratio);
	return (jlong)ns;
#elif defined CLOCK_MONOTONIC_COARSE
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
	uint64_t ns = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
	return (jlong)ns;
#elif defined CLOCK_MONOTONIC
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t ns = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
	return (jlong)ns;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ns = (uint64_t)tv.tv_sec * 1000000000ULL + (uint64_t)tv.tv_usec * 1000ULL;
	return (jlong)ns;
#endif
}

/**
 * High performance, fast, save but less accurate version of currentTimeMillis()
 * The return value is always positive
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_getEpochMillis(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t ns = (uint64_t)ft.dwHighDateTime << 32ULL | (uint64_t)ft.dwLowDateTime;
	uint64_t ms = ns / 10000ULL - 11644473600000ULL;
	return (jlong)ms;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME_COARSE
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME_COARSE, &ts);
	uint64_t ms = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t ms = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
	return (jlong)ms;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
	return (jlong)ms;
#endif
}

/**
 * High performance, fast, save but less accurate to returns seconds since epoch
 * The return value is always positive
 */
JNIEXPORT jlong JNICALL
Java_uc_j_OSClock_getEpochSecond(JNIEnv *env, jclass cls) {
#if defined _WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t ns = (uint64_t)ft.dwHighDateTime << 32ULL | (uint64_t)ft.dwLowDateTime;
	uint64_t sec = ns / 10000000ULL - 11644473600ULL;
	return (jlong)sec;
#elif defined __APPLE__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t sec = (uint64_t)tv.tv_sec;
	return (jlong)sec;
#elif defined CLOCK_REALTIME_COARSE
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME_COARSE, &ts);
	uint64_t sec = (uint64_t)ts.tv_sec;
	return (jlong)sec;
#elif defined CLOCK_REALTIME
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t sec = (uint64_t)ts.tv_sec;
	return (jlong)sec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t sec = (uint64_t)tv.tv_sec;
	return (jlong)sec;
#endif
}