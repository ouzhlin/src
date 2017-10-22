
#include "utils/JOClsMemoryPool.h"
#include "utils/JOString.h"
#include "utils/JOTime.h"
#include "manager/JOTickMgr.h"
#include "manager/JOCachePoolMgr.h"

NS_JOFW_BEGIN

JOClsMemoryVO::JOClsMemoryVO()
: m_clsname(JO_EMTYP_STRING)
, m_bOpen(true)
//, m_limitCount(0)
//, m_maxCount(100)
{
}

JOClsMemoryVO::~JOClsMemoryVO()
{
	
}


JOClsMemoryPool::JOClsMemoryPool()
:m_checkInterval(120)
, m_bDebug(false)
{
}

JOClsMemoryPool::~JOClsMemoryPool()
{
	clearAllPool();
}

bool JOClsMemoryPool::setPoolSwitch(const std::string& clsname, bool beOn)
{
	CLS_VO_MAP* tmpMap = nullptr;
	CLS_MAP::iterator clsItr = m_clsMap.begin();
	while (clsItr != m_clsMap.end()){
		tmpMap = &clsItr->second;
		CLS_VO_MAP::iterator tmpItr = tmpMap->find(clsname);
		if (tmpItr != tmpMap->end()){
			tmpItr->second->m_bOpen = beOn;
			return true;
		}		
		++clsItr;
	}
	return false;
}
/*
bool JOClsMemoryPool::setPoolLimit(const std::string& clsname, unsigned int count)
{
	CLS_VO_MAP* tmpMap = nullptr;
	CLS_MAP::iterator clsItr = m_clsMap.begin();
	while (clsItr != m_clsMap.end()){
		tmpMap = &clsItr->second;
		CLS_VO_MAP::iterator tmpItr = tmpMap->find(clsname);
		if (tmpItr != tmpMap->end()){
			tmpItr->second->m_limitCount = count;
			return true;
		}
		++clsItr;
	}
	return false;
}
*/
void JOClsMemoryPool::tick()
{
	static float tatolInterval = 0;
	tatolInterval += JOTickMgr::Instance()->deltaTime();
	if (tatolInterval < m_checkInterval) return;

	tatolInterval = 0;
	clock_t curSec = JOTime::getTimeofday();
	
	__MemoryPoolCacheInfo* info = nullptr;
	POOL_MAP::iterator itr = m_poolMap.begin();
	while (itr != m_poolMap.end())
	{
		info = itr->second;
		if ((info->m_tatolQuoteCount == 0) && (curSec - info->m_noQuoteTime > m_checkInterval))
		{
			std::list<void*>::iterator poolItr = info->m_pool.begin();
			while (poolItr != info->m_pool.end())
			{
				::free(*(poolItr));
				++poolItr;
			}
			info->m_pool.clear();
			
			itr = m_poolMap.erase(itr);
			delete info;

			continue;
		}
		else
		{
			if (info->m_tatolQuoteCount > 0)
			{
				if (info->m_tatolQuoteCount > info->m_maxCount)
					info->m_maxCount = info->m_tatolQuoteCount;
				if (m_bDebug)
				{
					CLS_MAP::iterator clsItr = m_clsMap.find(itr->first);
					if (clsItr != m_clsMap.end()){
						CLS_VO_MAP* tmpMap = &clsItr->second;
						std::string dest = JO_EMTYP_STRING;
						CLS_VO_MAP::iterator tmpItr = tmpMap->begin();
						while (tmpItr != tmpMap->end()){
							dest = tmpItr->first + ",";
							++tmpItr;
						}
						LOG_INFO("JOClsMemoryPool", "all cls: %s [%ld] Quote Count tatol is %d", dest.c_str(), itr->first, info->m_tatolQuoteCount);
					}
					else{
						LOG_INFO("JOClsMemoryPool", "ptr---size(%ld) Quote Count over 0. tatol is %d", itr->first, itr->second->m_tatolQuoteCount);
					}
				}
				
			}
			unsigned int curPoolCount = info->m_pool.size();
			if (curPoolCount>30)
			{
				unsigned int maxPoolCount = info->m_maxCount*0.8f+1;
				for (unsigned int i = curPoolCount; i > maxPoolCount; i--)
				{
					void* ptr = info->m_pool.front();
					::free(ptr);
					info->m_pool.pop_front();
				}
			}
			
		}
		++itr;
	}
}

void* JOClsMemoryPool::getPtr(size_t cacheSize, const std::string& clsname)
{
	if (m_bDebug)
	{
		JOClsMemoryVO* vo = nullptr;
		CLS_MAP::iterator clsItr = m_clsMap.find(cacheSize);
		if (clsItr != m_clsMap.end()){
			CLS_VO_MAP* tmpMap = &clsItr->second;
			CLS_VO_MAP::iterator tmpItr = tmpMap->find(clsname);
			if (tmpItr == tmpMap->end()){
				vo = POOL_GET(JOClsMemoryVO, "JOClsMemoryPool");
				vo->m_clsname = clsname;
				tmpMap->insert(make_pair(clsname, vo));
			}
			else{
				vo = tmpItr->second;
			}
		}
		else{
			CLS_VO_MAP tmpMap;
			vo = POOL_GET(JOClsMemoryVO, "JOClsMemoryPool");
			vo->m_clsname = clsname;
			tmpMap[clsname] = vo;
			m_clsMap[cacheSize] = tmpMap;
		}
		if (!vo->m_bOpen) return nullptr;
	}	
	
	void* ptr = nullptr;
	__MemoryPoolCacheInfo* info = nullptr;
	POOL_MAP::iterator itr = m_poolMap.find(cacheSize);
	if (itr != m_poolMap.end()){
		info = itr->second;
	}
	else{
		info = new __MemoryPoolCacheInfo();
		m_poolMap[cacheSize] = info;
	}
	info->m_tatolQuoteCount += 1;
	if (info->m_pool.empty()){
		ptr = ::malloc(cacheSize);
	}
	else{
		ptr = info->m_pool.front();
		info->m_pool.pop_front();
	}
	
	return ptr;
}

void JOClsMemoryPool::recover(void* ptr, size_t cacheSize)
{	
	POOL_MAP::iterator itr = m_poolMap.find(cacheSize);
	if (itr != m_poolMap.end()){
		__MemoryPoolCacheInfo* info = itr->second;
		info->m_tatolQuoteCount -= 1;		
		if (info->m_tatolQuoteCount <= 0)	{
			time_t now_time;
			now_time = time(NULL);
			info->m_noQuoteTime = now_time;
		}
		else{
			info->m_noQuoteTime = 0;
		}
		info->m_pool.push_back(ptr);
	}
	else{
		::free(ptr);
	}
}

void JOClsMemoryPool::clearPool(const std::string& clsname)
{
	if (m_bDebug)
	{
		CLS_VO_MAP* tmpMap = nullptr;
		CLS_MAP::iterator clsItr = m_clsMap.begin();
		while (clsItr != m_clsMap.end()){
			tmpMap = &clsItr->second;
			CLS_VO_MAP::iterator tmpItr = tmpMap->find(clsname);
			if (tmpItr != tmpMap->end()){
				POOL_MAP::iterator itr = m_poolMap.find(clsItr->first);
				if (itr != m_poolMap.end()){
					__MemoryPoolCacheInfo* delInfo = itr->second;
					std::list<void*>::iterator poolItr = delInfo->m_pool.begin();
					while (poolItr != delInfo->m_pool.end())
					{
						::free(*(poolItr));
						++poolItr;
					}
					delInfo->m_pool.clear();
					delete delInfo;
				}
				POOL_RECOVER(tmpItr->second, JOClsMemoryVO, "JOClsMemoryPool");
				return;
			}
			++clsItr;
		}
		m_clsMap.clear();
	}
}

void JOClsMemoryPool::clearAllPool()
{
	if (m_bDebug)
	{
		CLS_VO_MAP* tmpMap = nullptr;
		CLS_MAP::iterator clsItr = m_clsMap.begin();
		while (clsItr != m_clsMap.end()){
			tmpMap = &clsItr->second;
			CLS_VO_MAP::iterator tmpItr = tmpMap->begin();
			while (tmpItr != tmpMap->end()){
				POOL_RECOVER(tmpItr->second, JOClsMemoryVO, "JOClsMemoryPool");
				++tmpItr;
			}
			++clsItr;
		}
		m_clsMap.clear();
	}

	__MemoryPoolCacheInfo* delInfo = nullptr;
	POOL_MAP::iterator itr = m_poolMap.begin();
	while (itr != m_poolMap.end())
	{
		delInfo = itr->second;
		std::list<void*>::iterator poolItr = delInfo->m_pool.begin();
		while (poolItr != delInfo->m_pool.end())
		{
			::free(*(poolItr));
			++poolItr;
		}
		delInfo->m_pool.clear();
		 
		delete delInfo;
		++itr;
	}
	m_poolMap.clear();
}

NS_JOFW_END
