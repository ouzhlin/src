/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOTickMgr.cpp
AUTHOR:  James Ou 
*********************************************************************/

#include "manager/JOTickMgr.h"
#include "utils/JOLuaUtils.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"
#include "manager/JOCachePoolMgr.h"

NS_JOFW_BEGIN

JOTickVO::JOTickVO() :
c_funct(nullptr)
, lua_funct(-1)
, beRemove(false)
{}

void JOTickVO::init()
{
	c_funct = nullptr;
	lua_funct = -1;
	beRemove = false;
}

void JOTickVO::run()
{
	if (c_funct)
	{
		c_funct();
	}
	else if (lua_funct != -1)
	{
		JOLuaUtils::executeFunction(lua_funct, nullptr, JOLuaArgBaseType::eJOLUA_ARGS_TYPE_NULL);
	}
}

//////////////////////////////////////////////////////////////////////////

JOTickMgr::JOTickMgr():
m_deltaTime(0.0f)
, m_tickCount(0)
{

}

JOTickMgr::~JOTickMgr()
{
	clear();
}

void JOTickMgr::registerTick(unsigned int sn, std::function<void(void)> cFunct)
{
	TICK_MAP::iterator itr = tickMap.find(sn);
	if (itr != tickMap.end())
	{
		LOG_ERROR("JOTickMgr", "already register sn[%d]", sn);
		return;
	}
	JOTickVO *vo = POOL_GET(JOTickVO, "JOTickMgr");
	vo->init();
	vo->c_funct = cFunct;
	tickMap[sn] = vo;
}

void JOTickMgr::registerTick(unsigned int sn, LUA_FUNCTION luaFunct)
{
	TICK_MAP::iterator itr = tickMap.find(sn);
	if (itr != tickMap.end())
	{
		LOG_ERROR("JOTickMgr", "already register sn[%d]", sn);
		return;
	}
	JOTickVO *vo = POOL_GET(JOTickVO, "JOTickMgr");
	vo->init();
	vo->lua_funct = luaFunct;
	tickMap[sn] = vo;
}


void JOTickMgr::unRegisterTick(unsigned int sn)
{
	TICK_MAP::iterator itr = tickMap.find(sn);
	if (itr != tickMap.end())
	{
		itr->second->beRemove = true;		
	}
}

void JOTickMgr::tick( float dt )
{
	++m_tickCount;
	m_deltaTime = dt;

	JOTickVO *vo = nullptr;
	TICK_MAP::iterator itr = tickMap.begin();
	while (itr != tickMap.end())
	{
		vo = itr->second;
		if (vo->beRemove == true)
		{
			POOL_RECOVER(vo, "JOTickVO", "JOTickMgr");
			tickMap.erase(itr++);
		}
		else
		{
			vo->run();
			++itr;
		}
	}
	
    //特意不在这里加lock
	if (!_functionsToPerform.empty())
	{
        _performMutex.lock();
		const auto funct = _functionsToPerform.front();
		_functionsToPerform.pop_front();
		_performMutex.unlock();

		funct();
	}
	
	/*
	if (!_functionsToPerform.empty())

		_performMutex.lock();
		// fixed #4123: Save the callback functions, they must be invoked after '_performMutex.unlock()', otherwise if new functions are added in callback, it will cause thread deadlock.
		auto temp = _functionsToPerform;
		_functionsToPerform.clear();
		_performMutex.unlock();
		for (const auto &function : temp) {
			function();
		}
	}
	*/
}

void JOTickMgr::clear()
{
	TICK_MAP::iterator itr = tickMap.begin();
	while (itr != tickMap.end())
	{
		POOL_RECOVER(itr->second, "JOTickVO", "JOTickMgr");
		++itr;
	}
	tickMap.clear();
}

void JOTickMgr::runInMainThread(const std::function<void()> &function)
{
	JOLockGuard tempLock(_performMutex);
	_functionsToPerform.push_back(function);	
}



NS_JOFW_END