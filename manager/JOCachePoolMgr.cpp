#include "manager/JOCachePoolMgr.h"
#include "manager/JOTickMgr.h"
#include "utils/JOMemery.h"
#include "utils/JOLog.h"
#include "utils/JOTime.h"



NS_JOFW_BEGIN

JOCachePoolMgr::JOCachePoolMgr() :checkInterval(60 * 3), isDebug(false)
{
}

JOCachePoolMgr::~JOCachePoolMgr()
{
	POOL_MAP::iterator itr = mPool.begin();
	while (itr != mPool.end())
	{
		JOPoolCacheInfo *info = itr->second;
		std::vector<void*>::iterator subItr = info->pool.begin();
		while (subItr != info->pool.end())
		{
			CLS_DELETE_MAP::iterator deleteItr = clsDeleteMap.find(itr->first);
			if (deleteItr != clsDeleteMap.end()){
				deleteItr->second(*subItr);
			}
			else{
				delete *subItr;
			}
			++subItr;
		}
		info->pool.clear();
		++itr;
	}
	mPool.clear();
}


void* JOCachePoolMgr::getObj(const std::string& clsName, const std::string& refPath)
{
	JOPoolCacheInfo *info = nullptr;
	POOL_MAP::iterator itr = mPool.find(clsName);
	if (itr != mPool.end())
	{
		info = itr->second;
	}
	else
	{
		info = new JOPoolCacheInfo;
		mPool[clsName] = info;
	}
	++info->totalQuoteCount;
	if (isDebug)
	{
		int quoteCount = info->quoteRecord[refPath];
		info->quoteRecord[refPath] = quoteCount ? quoteCount + 1 : 1;
	}

	void* srcObj = nullptr;
	std::vector<void*> *tmpPool = &info->pool;
	if (tmpPool->empty())
	{
        CLS_CREAT_MAP::iterator itr = clsCreateMap.find(clsName);
        if (itr != clsCreateMap.end())
        {
            srcObj = itr->second();
        }
        else
        {
            LOG_ERROR("create [%s] fail !!!", clsName.c_str());
        }
	}
	else
	{
        srcObj = tmpPool->back();
        tmpPool->pop_back();
	}
	return srcObj;
}

void JOCachePoolMgr::recover(void* obj, const std::string& clsName, const std::string& refPath)
{
	if (!obj || refPath.length() < 1)
	{
		LOG_WARN("JOCachePool", "clsName[%s] refPath nil", clsName.c_str());
		return;
	}

	POOL_MAP::iterator itr = mPool.find(clsName);
	if (itr == mPool.end())
	{
		return;
	}
	JOPoolCacheInfo* info = itr->second;
	if (isDebug)
	{
		JOPoolCacheInfo::QUOTE_MAP::iterator quoteItr = info->quoteRecord.find(refPath);
		if (quoteItr == info->quoteRecord.end())
		{
			/*
			传入的引用路径，没有引用记录
			*/
			LOG_WARN("JOCachePool", "clsName[%s] [%s] not ref record recover fail", clsName.c_str(), refPath.c_str());
		}
		else if (quoteItr->second > 0) // ref instace -1
		{
			--quoteItr->second;
		}
	}
	
	if (info->totalQuoteCount > 0)
	{
		--info->totalQuoteCount;
	}
	
	if (info->totalQuoteCount <= 0)
	{
		info->totalQuoteCount = 0;
		info->noQuoteTime = JOTime::getTimeofday();
	}
	/*
	else
	{
		info->noQuoteTime = 0;
	}
	*/
	info->pool.push_back(obj);
}

void JOCachePoolMgr::tick()
{
	static float totalInterval = 0;
	totalInterval += JOTickMgr::Instance()->deltaTime();
	if (totalInterval > checkInterval)
	{
		_checkQuoteClass();
		_recycleCollect();
		totalInterval = 0;
	}
}

void JOCachePoolMgr::_checkQuoteClass()
{
	clock_t curSec = JOTime::getTimeofday();
	std::string debugInfo;
	POOL_MAP::iterator itr = mPool.begin();
	while (itr != mPool.end())
	{
		JOPoolCacheInfo* tmpPoolInfo = itr->second;
		if (isDebug && tmpPoolInfo->totalQuoteCount > 60)
		{
			debugInfo = JOString::formatString("%s cls[%s] refPath: \n", debugInfo.c_str(), itr->first.c_str());
			JOPoolCacheInfo::QUOTE_MAP::iterator quoteItr = tmpPoolInfo->quoteRecord.begin();
			unsigned int idx = 1;
			while (quoteItr != tmpPoolInfo->quoteRecord.end())
			{
				debugInfo = JOString::formatString("----%s[%d]: %s -> (%d) \n", debugInfo.c_str(), idx, quoteItr->first.c_str(), quoteItr->second);
				++idx;
				++quoteItr;
			}
		}
		if (tmpPoolInfo->totalQuoteCount == 0)
		{
			if (curSec - tmpPoolInfo->noQuoteTime > 180)
			{
				std::vector<void*>::iterator poolItr = tmpPoolInfo->pool.begin();
				while (poolItr != tmpPoolInfo->pool.end())
				{
					CLS_DELETE_MAP::iterator deleteItr = clsDeleteMap.find(itr->first);
					if (deleteItr != clsDeleteMap.end()){
						deleteItr->second(*poolItr);
					}
					else{
						delete *poolItr;
					}
					++poolItr;
				}
				
				itr = mPool.erase(itr);
				delete tmpPoolInfo;

				continue;
			}
		}
		else
		{			
			unsigned int poolCount = tmpPoolInfo->pool.size();
			if (poolCount > 60)
			{
				std::vector<void*>::reverse_iterator poolItr = tmpPoolInfo->pool.rbegin();
				while (poolCount > 60)// && poolItr != itr->second->pool.rend())
				{
					CLS_DELETE_MAP::iterator deleteItr = clsDeleteMap.find(itr->first);
					if (deleteItr != clsDeleteMap.end()){
						deleteItr->second(*poolItr);
					}
					else{
						delete *poolItr;
					}
					poolItr = std::vector<void*>::reverse_iterator(tmpPoolInfo->pool.erase((++poolItr).base()));
					--poolCount;
				}
			}
		}
		++itr;
	}
	if (!debugInfo.empty())
	{
		LOG_DEBUG("JOCachePool", debugInfo.c_str());
	}
}

void JOCachePoolMgr::registerCls(const std::string& clsname, const std::function<void*(void)>& call, const std::function<void(void*)>& deletelCall/* = nullptr*/)
{
	clsCreateMap[clsname] = call;
	if (deletelCall){
		clsDeleteMap[clsname] = deletelCall;
	}
	
}
//////////////////////////////////////////////////////////////////////////
void JOCachePoolMgr::setCollectCondition(unsigned int size)
{	
	_limitSize = size;
}

void JOCachePoolMgr::_recycleCollect()
{
	if (_collectCallback && _curSize > _limitSize) {
		unsigned int collectSize = _collectCallback();
		if (_curSize >= collectSize) {
			_curSize -= collectSize;
		}
		else{
			_curSize = 0;
		}
	}	
}

void JOCachePoolMgr::onAddCacheCallback(unsigned int addSize)
{	
	_curSize = _curSize + addSize;
}

void JOCachePoolMgr::registerAddCollectCallback(const std::function<unsigned int()> &collectCallback)
{
	_collectCallback = collectCallback;
}


NS_JOFW_END
