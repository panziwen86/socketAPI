#include<stdio.h>
#include<time.h>

inline long long GetTick(void)
{
#ifdef _WIN32
	//return GetTickCount64();
	return GetTickCount();
#elif defined MAC_OS
    struct timeval us;
    gettimeofday(&us, NULL);
    return (us.tv_sec*1000 + us.tv_usec/1000);
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((long long)ts.tv_sec * 1000LL + ts.tv_nsec/(1000 * 1000));
#endif
}