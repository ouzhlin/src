#include "module/loader/JOAsynchSoundLoader.h"
#include "module/loader/vo/JOAsynchSoundVO.h"
#include "manager/JOCachePoolMgr.h"

#include "audio/include/SimpleAudioEngine.h"
#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

JOAsynchSoundLoader::JOAsynchSoundLoader() :asyncRefCount(0)
{
	m_pthread = JOThread::createThread(this, true);
}


JOAsynchSoundLoader::~JOAsynchSoundLoader()
{
	Director::getInstance()->getScheduler()->unscheduleUpdate(this);
	mLock.lock();
	while (!soundQueue.empty())
	{
		JOAsynchSoundVO *vo = soundQueue.front();
		soundQueue.pop();
		POOL_RECOVER(vo, JOAsynchSoundVO, "JOAsynchSoundLoader");
	}
	mLock.unlock();
	JOThread::destroThread(m_pthread);
}


void JOAsynchSoundLoader::addSoundAsync(const std::string &filepath, bool isMusic, const std::function<void(void)>& callback)
{
	JOAsynchSoundVO *vo = POOL_GET(JOAsynchSoundVO, "JOAsynchSoundLoader");
	vo->filepath = filepath;
	vo->isMusic = isMusic;
	vo->callback = callback;
	
	if (m_pthread->isSuspend())
	{
		m_pthread->work();
	}
	if (asyncRefCount==0)
		Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
	++asyncRefCount;

	JOLockGuard tempLock(mLock);
	soundQueue.push(vo);	
}

void JOAsynchSoundLoader::onThreadProcess(JOThread *pThread)
{
	mLock.lock();
	if (soundQueue.empty())
	{
		pThread->suspend();
		return;
	}
	JOAsynchSoundVO *vo = soundQueue.front();	
	mLock.unlock();
	
	if (vo->isMusic)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(vo->filepath.c_str());
	}
	else
	{
		CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(vo->filepath.c_str());
	}
}

void JOAsynchSoundLoader::update(float dt)
{
	mLock.lock();
	if (soundQueue.empty())
	{
		Director::getInstance()->getScheduler()->unscheduleUpdate(this);
		mLock.unlock();
		return;
	}
	JOAsynchSoundVO *vo = soundQueue.front();
	soundQueue.pop();
	mLock.unlock();

	vo->callback();

	POOL_RECOVER(vo, JOAsynchSoundVO, "JOAsynchSoundLoader");
}

NS_JOFW_END