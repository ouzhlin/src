
#include "utils/JOLock.h"

NS_JOFW_BEGIN

JOLock::JOLock() :m_nRef(0)
{
#ifdef JOUSE_STD_THREAD

#else
	pthread_mutex_init(&m_Mutex, NULL);
#endif
}

JOLock::~JOLock()
{
	while (m_nRef > 0)
	{
		unlock();
	}
#ifdef JOUSE_STD_THREAD

#else
	pthread_mutex_destroy(&m_Mutex);
#endif
}

void JOLock::lock()
{
#ifdef JOUSE_STD_THREAD
	m_Mutex.lock();
#else
	pthread_mutex_lock(&m_Mutex);
#endif
	++m_nRef;
}

void JOLock::unlock()
{
	--m_nRef;
#ifdef JOUSE_STD_THREAD
	m_Mutex.unlock();
#else
	pthread_mutex_unlock(&m_Mutex);
#endif
}


JOLockGuard::JOLockGuard(JOLock& rlock)
	: m_lock(rlock)
{
	m_lock.lock();
}


JOLockGuard::~JOLockGuard()
{
	m_lock.unlock();
}


JOLockJudgeGuard::JOLockJudgeGuard(JOLock& rlock, bool beIgnore /*= false*/)
	: m_lock(rlock)
	, m_ignoreLock(beIgnore)
{
	if (m_ignoreLock)
	{
		return;
	}
	m_lock.lock();
}

JOLockJudgeGuard::~JOLockJudgeGuard()
{
	if (m_ignoreLock)
	{
		return;
	}
	m_lock.unlock();
}

NS_JOFW_END