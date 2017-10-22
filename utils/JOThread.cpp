#include "utils/JOThread.h"
#include "utils/JOMemery.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOThread* JOThread::createThread(IJOThreadDelegate* delegate, bool bSuspend /*= false*/, unsigned int nWorkInterval /*= 500*/)
{
	if (!delegate) return nullptr;

	return JOMEM_NEW(JOThread);
}

void JOThread::destroThread(JOThread* pThread)
{
	if (!pThread) return;

	pThread->close();
	JOMEM_DELETE(pThread);
}

JOThread::JOThread() :m_pDelegate(nullptr), m_ucStatus(STATUS_CLOSE), m_nWorkInterval(500)
{
#ifndef JOUSE_STD_THREAD
	pthread_cond_init(&m_TCond, NULL);
	pthread_cond_init(&m_TWorkCond, NULL);
#endif // !JOUSE_STD_THREAD

}

JOThread::~JOThread()
{
#ifndef JOUSE_STD_THREAD
	pthread_cond_destroy(&m_TCond);
	pthread_cond_destroy(&m_TWorkCond);
#endif
}


bool JOThread::init(IJOThreadDelegate* pDelegate, bool bSuspend /*= false*/, unsigned int nWorkInterval /*= 500*/)
{
	if (!pDelegate) return false;

	m_pDelegate = pDelegate;
	m_ucStatus = bSuspend ? STATUS_SUSPEND : STATUS_RUNING;
	m_nWorkInterval = nWorkInterval;

#ifdef UNIX
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);

	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)
	if (rc != 0)
	{
		LOG_ERROR("JOThread", "pthread_sigmask Error! how:%d, Error Code:%d", SIG_BLOCK, rc);
	}
#endif //  UNIX

#ifdef JOUSE_STD_THREAD
	m_thread = std::thread(JOThread::_runThread, this);
#else
	int nRet = pthread_create(&m_thread, NULL, &this->_runThread, (void*)this);
	if (nRet != 0)
	{
		LOG_ERROR("JOThread", "create thread Error! Error Code:%d", nRet);
		return false;
	}
#endif // JOUSE_STD_THREAD

	m_pDelegate->onThreadCreate(this);
	return true;
}


void JOThread::resume()
{
#ifdef JOUSE_STD_THREAD
	if (STATUS_SUSPEND == m_ucStatus)
	{
		m_ucStatus = STATUS_RUNING;
		m_TCond.notify_one();
	}
#else
	JOLockGuard tempLock(m_lock);
	if (STATUS_SUSPEND == m_ucStatus)
	{
		pthread_cond_signal(&m_TCond);
	}
#endif // JOUSE_STD_THREAD

}

void JOThread::suspend()
{
	JOLockGuard tempLock(m_lock);
	if (STATUS_RUNING == m_ucStatus)
	{
		m_ucStatus = STATUS_SUSPEND;
	}
}

void JOThread::work()
{
#ifdef JOUSE_STD_THREAD
	m_ucStatus = STATUS_RUNING;
	m_TWorkCond.notify_one();
#else
	JOLockGuard tempLock(m_lock);
	m_ucStatus = STATUS_RUNING;
	pthread_cond_signal(&m_TWorkCond);
#endif // JOUSE_STD_THREAD
}

bool JOThread::isSuspend()
{
	JOLockGuard tempLock(m_lock);
	return (STATUS_SUSPEND == m_ucStatus);
}

void JOThread::close()
{
#ifdef JOUSE_STD_THREAD
	m_ucStatus = STATUS_CLOSE;
	m_TCond.notify_one();
	m_TWorkCond.notify_one();
	m_thread.join();
#else
	m_lock.lock();
	m_ucStatus = STATUS_CLOSE;
	pthread_cond_signal(&m_TCond);
	pthread_cond_signal(&m_TWorkCond);
	m_lock.unlock();
	pthread_join(m_thread, NULL);
#endif // JOUSE_STD_THREAD
	m_pDelegate->onThreadDestroy(this);
}
#ifdef JOUSE_STD_THREAD
void JOThread::_runThread(JOThread* pthread)
{
	//JOThread* pthread = reinterpret_cast<JOThread*>(n);
	if (!pthread) return;

	while (pthread->m_ucStatus != STATUS_CLOSE)
	{
		while (pthread->m_ucStatus == STATUS_SUSPEND)
		{
			std::unique_lock <std::mutex> lck(pthread->m_lock.getMutex());
			pthread->m_TCond.wait(lck);
		}
		if (pthread->m_nWorkInterval > 0)
		{
			std::unique_lock <std::mutex> lck(pthread->m_lock.getMutex());
			pthread->m_TWorkCond.wait_for(lck, std::chrono::milliseconds(pthread->m_nWorkInterval));
		}
		if (pthread->m_pDelegate)
		{
			pthread->m_pDelegate->onThreadProcess(pthread);
		}
	}
}
#else
void JOThread::_runThread(void* pThreadParam)
{
	JOThread *pthread = (JOThread*)pThreadParam;

	if (!pthread) return;
	pthread_cleanup_push(pthread->_clean, (void*)pthread);

	while (pthread->m_ucStatus != STATUS_CLOSE)
	{
		pthread->m_lock.lock();
		while (pthread->m_ucStatus == STATUS_SUSPEND)
		{
			pthread_cond_wait(&pthread->m_TCond, &pthread->m_lock.getMutex());
		}
		if (pthread->m_nWorkInterval > 0)
		{
			struct timeval curTime;			
			JOTime::getTimeofday(&curTime);
			const unsigned short MSEC = 1000;

			long lTargetMsec = curTime.tv_usec/MSEC + pthread->m_nWorkInterval; //tv_usec

			timespec objTime;
			objTime.tv_sec = curTime.tv_sec + lTargetMsec/MSEC;
			objTime.tv_nsec = (lTargetMsec%MSEC)*1000000;
			pthread_cond_timewait(&pthread->m_TWorkCond, &pthread->m_lock.getMutex(), &objTime);
		}
		pthread->m_lock.unlock();

		if (pthread->m_pDelegate)
		{
			pthread->m_pDelegate->onThreadProcess(pthread);
		}
	}
	pthread_cleanup_pop(1);	
}
#endif

void JOThread::_clean(void* pThreadParam)
{
	JOThread* pthread = (JOThread*)pThreadParam;
	if (!pthread) return;
#ifndef JOUSE_STD_THREAD
	pthread_mutex_unlock( &(pthread->m_lock.getMutex()) );
#endif // !JOUSE_STD_THREAD
}
NS_JOFW_END