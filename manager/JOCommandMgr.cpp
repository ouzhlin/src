/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOCommandMgr.cpp
AUTHOR:  James Ou 
*********************************************************************/
#include "manager/JOCommandMgr.h"
#include "manager/commandqueue/JOCommandQueue.h"
#include "manager/commandqueue/JOCommandVO.h"
#include "manager/JOCachePoolMgr.h"

NS_JOFW_BEGIN

JOCommandMgr::JOCommandMgr()
{	
}

JOCommandMgr::~JOCommandMgr()
{
	clear();
}

void JOCommandMgr::tick()
{
	m_commandQueue.update();
	m_asynCommandQueue.update();
}

void JOCommandMgr::appendCommandTail(unsigned int id, std::function<bool(void)> cRunFun, bool beAsyn)
{
	JOCommandVO* vo = POOL_GET(JOCommandVO, "JOCommandMgr");
	vo->init(id, cRunFun);
	if (beAsyn)
	{
		m_asynCommandQueue.pushBack(vo);
	}
	else
	{
		m_commandQueue.pushBack(vo);
	}
}

void JOCommandMgr::insertCommandFront(unsigned int id, std::function<bool(void)> cRunFun, bool beAsyn)
{
	JOCommandVO* vo = POOL_GET(JOCommandVO, "JOCommandMgr");
	vo->init(id, cRunFun);
	if (beAsyn)
	{
		m_asynCommandQueue.pushFront(vo);
	}
	else
	{
		m_commandQueue.pushFront(vo);
	}
}

void JOCommandMgr::appendLuaCommandTail(unsigned int id, LUA_FUNCTION luaRunFun, bool beAsyn)
{
	JOCommandVO* vo = POOL_GET(JOCommandVO, "JOCommandMgr");
	vo->init(id, luaRunFun);
	if (beAsyn)
	{
		m_asynCommandQueue.pushBack(vo);
	}
	else
	{
		m_commandQueue.pushBack(vo);
	}
}

void JOCommandMgr::insertLuaCommandFront(unsigned int id, LUA_FUNCTION luaRunFun, bool beAsyn)
{
	JOCommandVO* vo = POOL_GET(JOCommandVO, "JOCommandMgr");
	vo->init(id, luaRunFun);
	if (beAsyn)
	{
		m_asynCommandQueue.pushFront(vo);
	}
	else
	{
		m_commandQueue.pushFront(vo);
	}	
}

void JOCommandMgr::removeCommand(unsigned int id)
{
	if (!m_commandQueue.removeCommand(id))
		m_asynCommandQueue.removeCommand(id);
}

void JOCommandMgr::clear()
{
    m_commandQueue.clearAll();
	m_asynCommandQueue.clearAll();
}

NS_JOFW_END