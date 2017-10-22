#include "utils/JOMemery.h"
#include <assert.h>
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOMemery::JOMemery() :m_bDump(false)
{
}

JOMemery::~JOMemery()
{
	JOLockGuard tempLock(m_lock);

	ALLOC_MAP::iterator itr = m_mapAllocs.begin();
	while (itr != m_mapAllocs.begin())
	{
		free(itr->first);
		++itr;
	}
	m_mapAllocs.clear();
}

void* JOMemery::allocBytes(size_t sCount, const char* lpszFile /*= NULL*/, unsigned int nLine /*= 0*/)
{	
	// > 10M
	if (sCount > 10000000)
	{
		assert(false);
	}
	void *p = malloc(sCount);
	if (m_bDump){
		recordAlloc(p, sCount, lpszFile, nLine);
	}
	return p;
}

void JOMemery::deallocBytes(void* lpPtr)
{
	if (!lpPtr) return;
	if (m_bDump)
	{
		recordDeallooc(lpPtr);
	}
	free(lpPtr);
}

void JOMemery::enableDumpMemeryLeak(bool bDump)
{

}

void JOMemery::reportLeaks()
{
	if (!m_bDump) return;
	JOLockGuard tempLock(m_lock);

	if (m_mapAllocs.empty()) return;

	LOG_DEBUG("JOMemery", "-------------------BEGIN	MEMERY CHECK--------------------------");
	ALLOC_MAP::iterator itr = m_mapAllocs.begin();
	while (itr != m_mapAllocs.end())
	{
		ALLOC_INFO& rInfo = itr->second;

		LOG_DEBUG("JOMemery", "Memery Leak: Normal block at 0x%x, %d bytes at line:%d in file %s.", itr->first, rInfo.sBytes, rInfo.nLine, rInfo.strFile.c_str());
		++itr;
	}
	LOG_DEBUG("JOMemery", "-------------------END MEMERY CHECK--------------------------");

}

void JOMemery::recordAlloc(void* lpPtr, size_t sCount, const char* lpszFile, unsigned int nLine)
{
	if (!lpPtr) return;
	
	JOLockGuard tempLock(m_lock);

	ALLOC_INFO allocInfo(sCount, lpszFile, nLine);
	m_mapAllocs[lpPtr] = allocInfo;
}

void JOMemery::recordDeallooc(void* lpPtr)
{
	if (!lpPtr) return;
	
	JOLockGuard tempLock(m_lock);
	ALLOC_MAP::iterator itr = m_mapAllocs.find(lpPtr);
	if (itr != m_mapAllocs.end())
	{
		m_mapAllocs.erase(lpPtr);
	}
}

NS_JOFW_END