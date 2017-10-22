#include "core/datautils/JODataPool.h"
#include "core/datautils/JOData.h"
#include "core/datautils/JODataUtils.h"
#include "core/datautils/JODataCoder.h"
//#include "core/datautils/JODataBundle.h"
#include "manager/JOTickMgr.h"

NS_JOFW_BEGIN

const unsigned short c_opOccupy = 2;
const unsigned short c_allOccupy = 2;

JODataPool::JODataPool()
	: elapsedTime(0.0f)
	, checkInterval(180.0f)
{
}

JODataPool::~JODataPool()
{
	clearCoder();
	//clearBundle();
}

void JODataPool::clearCoder()
{
	std::list<JODataCoder*>::iterator itr = m_coderList.begin();
	while (itr != m_coderList.end())
	{
		JO_SAFE_DELETE(*itr);
		++itr;
	}
	m_coderList.clear();

	JOLockGuard tempLock(m_coderLock);
	std::list<JODataCoder*>::iterator itrAsyn = m_coderAsynList.begin();
	while (itrAsyn != m_coderAsynList.end())
	{
		JO_SAFE_DELETE(*itrAsyn);
		++itrAsyn;
	}
	m_coderAsynList.clear();
}
/*
void JODataPool::clearBundle()
{
	std::list<JODataBundle*>::iterator itr = m_bundleList.begin();
	while (itr != m_bundleList.end())
	{
		JO_SAFE_DELETE(*itr);
		++itr;
	}
	m_bundleList.clear();

	JOLockGuard tempLock(m_bundleLock);
	std::list<JODataBundle*>::iterator itrAsyn = m_bundleAsynList.begin();
	while (itrAsyn != m_bundleAsynList.end())
	{
		JO_SAFE_DELETE(*itrAsyn);
		++itrAsyn;
	}
	m_bundleAsynList.clear();
}
*/
JODataCoder* JODataPool::getDataCoder(bool beOccupy/*=false*/, bool bigEndian /*= false*/, bool isAsyn /*= false*/)
{
	if (!isAsyn)
	{
		JODataCoder* coder = nullptr;
		if (m_coderList.empty()){
			if (beOccupy)
				coder = new JODataCoder(c_opOccupy, c_allOccupy);
			else
				coder = new JODataCoder();
		}
		else{
			coder = m_coderList.front();
			m_coderList.pop_front();
			if (beOccupy)
				coder->init(c_opOccupy, c_allOccupy);
			else
				coder->init();
		}		
		coder->setBigEndian(bigEndian);
		return coder;
	}
	else
	{
		m_coderLock.lock();
		JODataCoder* coder = nullptr;
		if (m_coderAsynList.empty()){
			if (beOccupy)
				coder = new JODataCoder(c_opOccupy, c_allOccupy);
			else
				coder = new JODataCoder();
		}
		else{
			coder = m_coderAsynList.front();
			m_coderAsynList.pop_front();
			if (beOccupy)
				coder->init(c_opOccupy, c_allOccupy);
			else
				coder->init();
		}		
		coder->setBigEndian(bigEndian);
		m_coderLock.unlock();
		return coder;
	}
}
/*
JODataBundle* JODataPool::getDataBundle(bool isAsyn )
{
	if (!isAsyn)
	{
		if (m_bundleList.empty())
		{
			return new JODataBundle();
		}
		JODataBundle* coder = m_bundleList.front();
		m_bundleList.pop_front();
		coder->init();
		return coder;
	}
	else
	{
		m_bundleLock.lock();
		JODataBundle* coder = nullptr;		
		if (m_bundleAsynList.empty())
		{
			coder = new JODataBundle();
		}
		coder = m_bundleAsynList.front();
		m_bundleAsynList.pop_front();
		coder->init();
		m_bundleLock.unlock();
		return coder;
	}
}
*/
void JODataPool::recover(JODataCoder* coder, bool isAsyn/*=false*/)
{
	if (coder == nullptr) return;
	
	if (!isAsyn)
	{
		m_coderList.push_back(coder);
	}
	else
	{
		JOLockGuard tempLock(m_coderLock);
		m_coderAsynList.push_back(coder);
	}	
}
/*
void JODataPool::recover(JODataBundle* bundle, bool isAsyn )
{
	if (!isAsyn)
	{
		m_bundleList.push_back(bundle);
	}
	else
	{
		JOLockGuard tempLock(m_bundleLock);
		m_bundleAsynList.push_back(bundle);
	}
}
*/
void JODataPool::tick()
{
	elapsedTime += JOTickMgr::Instance()->deltaTime();
	if (elapsedTime>checkInterval)
	{
		elapsedTime = 0;
		_checkCoderList();
		//_checkBundleList();
	}	
}

void JODataPool::_checkCoderList()
{	
	unsigned int count = m_coderList.size();
	while (count > 50)
	{
		JODataCoder* coder = m_coderList.front();
		m_coderList.pop_front();
		delete coder;
		--count;
	}

	JOLockGuard tempLock(m_coderLock);
	count = m_coderAsynList.size();
	while (count > 50)
	{
		JODataCoder* coder = m_coderAsynList.front();
		m_coderAsynList.pop_front();
		delete coder;
		--count;
	}
}
/*
void JODataPool::_checkBundleList()
{
	unsigned int count = m_bundleList.size();
	while (count > 50)
	{
		JODataBundle* coder = m_bundleList.front();
		m_bundleList.pop_front();
		delete coder;
		--count;
	}

	JOLockGuard tempLock(m_bundleLock);
	count = m_bundleAsynList.size();
	while (count > 50)
	{
		JODataBundle* coder = m_bundleAsynList.front();
		m_bundleAsynList.pop_front();
		delete coder;
		--count;
	}
}
*/

NS_JOFW_END