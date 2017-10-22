#include "module/event/JOEventDispatcher.h"
#include <iostream>
#include "core/datautils/JODataPool.h"
#include "core/datautils/JODataCoder.h"
#include "manager/JOSocketMgr.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"
#include "module/event/JOFunctionBinder.h"
#include "module/event/JOEventDef.h"

NS_JOFW_BEGIN

JOEventDispatcher::JOEventDispatcher()
:_pABinder(new JOFunctionBinder())
, _pEBinder(new JOFunctionBinder())
{
	
}

JOEventDispatcher::~JOEventDispatcher()
{
	//JODataCoderMgr::Destroy();
	JO_SAFE_DELETE(_pABinder);
	JO_SAFE_DELETE(_pEBinder);
}

void JOEventDispatcher::tick()
{    
	_pABinder->tick();
	_pEBinder->tick();
	//JODataCoderMgr::Instance()->updateCoderVec();	    
}

void JOEventDispatcher::regAction(unsigned int sn, unsigned int actId, const std::function<void(unsigned int, void*, short)> &onEvent)
{
	_pABinder->bind(sn, actId, onEvent);
}

void JOEventDispatcher::regEvent(unsigned int sn, unsigned int eveId, const std::function<void(unsigned int, void*, short)> &onEvent)
{
	_pEBinder->bind(sn, eveId, onEvent);
}
/*
void JOEventDispatcher::regAction(unsigned int sn, unsigned int actId, LUA_FUNCTION luaEvent)
{
	_pABinder->bind(sn, actId, luaEvent);
}
void JOEventDispatcher::regEvent(unsigned int sn, unsigned int eveId, LUA_FUNCTION luaEvent)
{
	_pEBinder->bind(sn, eveId, luaEvent);
}
*/
void JOEventDispatcher::unRegAction(unsigned int sn)
{
	_pABinder->unbind(sn);
}

void JOEventDispatcher::unRegAction(unsigned int sn, unsigned int actId)
{
	_pABinder->unbind(sn, actId);
}

void JOEventDispatcher::unRegEvent(unsigned int sn)
{
	_pEBinder->unbind(sn);
}

void JOEventDispatcher::unRegEvent(unsigned int sn, unsigned int eveId)
{
	_pEBinder->unbind(sn, eveId);
}

void JOEventDispatcher::unRegAll(unsigned int sn)
{
	_pABinder->unbind(sn);
	_pEBinder->unbind(sn);
}

void JOEventDispatcher::clearAll()
{
	_pABinder->removeAllFunctions();
	_pEBinder->removeAllFunctions();
}

//void JOEventDispatcher::dispatchAction(JODataCoder *dataCoder, short socketId /*= -1*/)
//{
//	dataCoder->seek(0);
//	unsigned short actionId = dataCoder->readUShort();
//
//	if (actionId % 2 == 1)
//	{
//		dataCoder->seek(0);
//		JOSocketMgr::Instance()->sendMessage(dataCoder);
//	}
//	else
//	{
//		_pABinder->exec(actionId, dataCoder, socketId);
//		JODataPool::Instance()->recover(dataCoder);
//	}
//}

void JOEventDispatcher::g2cAction(JODataCoder* dataCoder, int socketId /*= -1*/)
{
	_pABinder->exec(dataCoder->getOp(), dataCoder, socketId);
	JODataPool::Instance()->recover(dataCoder);
}

void JOEventDispatcher::c2gAction(JODataCoder* dataCoder, int socketId /*= -1*/)
{
	JOSocketMgr::Instance()->sendMessage(dataCoder, socketId);
}
/*
void JOEventDispatcher::dispatchEvent(JODataBundle *dataBundle)
{
	unsigned int eventId = dataBundle->getNumber(JOEVENT_KEY);
	_pEBinder->exec(eventId, dataBundle);
	JODataPool::Instance()->recover(dataBundle);
}
*/
void JOEventDispatcher::dispatchEvent(unsigned int eveId, JODataCoder *dataCoder/*=nullptr*/)
{
	_pEBinder->exec(eveId, dataCoder);
	JODataPool::Instance()->recover(dataCoder);
}



NS_JOFW_END