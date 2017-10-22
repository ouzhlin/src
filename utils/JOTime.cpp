#include "utils/JOTime.h"

#ifdef WIN32
#	include <Windows.h>
#else
#	include <sys/time.h>
#endif

NS_JOFW_BEGIN

JODuration::JODuration() : start_time_(std::chrono::system_clock::now())
{}

void JODuration::reset()
{
	start_time_ = std::chrono::system_clock::now();
}

std::chrono::seconds::rep JODuration::seconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time_).count();
}

std::chrono::milliseconds::rep JODuration::milli_seconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_).count();
}

std::chrono::microseconds::rep JODuration::micro_seconds()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_).count();
}

std::chrono::nanoseconds::rep JODuration::nano_seconds()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start_time_).count();
}


clock_t JOTime::getTimeofday()
{
	struct timeval tp;
	getTimeofday(&tp);
	return tp.tv_sec + tp.tv_usec/1000000;
}

void JOTime::getTimeofday(struct timeval* tp)
{
#ifdef WIN32
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);

	struct tm tmNow;
	tmNow.tm_year = wtm.wYear - 1900;
	tmNow.tm_mon = wtm.wMonth - 1;
	tmNow.tm_mday = wtm.wDay;
	tmNow.tm_hour = wtm.wHour;
	tmNow.tm_min = wtm.wMinute;
	tmNow.tm_sec = wtm.wSecond;
	tmNow.tm_isdst = -1;
	time_t clock = mktime(&tmNow);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
#else
	gettimeofday(tp, NULL);
#endif
}

NS_JOFW_END