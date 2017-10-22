#include "module/event/JOFunctionBinder.h"
#include <algorithm>
#include "module/event/vo/JOEventBindVO.h"
#include "core/datautils/JODataCoder.h"
#include "utils/JOLog.h"
#include "utils/JOLuaUtils.h"
#include "manager/JOCachePoolMgr.h"

NS_JOFW_BEGIN

JOFunctionBinder::~JOFunctionBinder()
{
	removeAllFunctions();
	tick();
}

void JOFunctionBinder::bind(unsigned int sn, unsigned int eventId, const std::function<void(unsigned int, void*, short)> &cFunc)
{
	JOEventBindVO* vo = POOL_GET(JOEventBindVO, "JOFunctionBinder");
	vo->init(sn, eventId, cFunc);
	_bind(sn, eventId, vo);
}

void JOFunctionBinder::bind(unsigned int sn, unsigned int eventId, LUA_FUNCTION luaFunc)
{
	JOEventBindVO* vo = POOL_GET(JOEventBindVO, "JOFunctionBinder");
	vo->init(sn, eventId, luaFunc);
	_bind(sn, eventId, vo);
}

void JOFunctionBinder::_bind(unsigned int sn, unsigned int eventId, JOEventBindVO* vo)
{
	SN_VO_MAP::iterator itr = sn_vo_map.find(sn);
	if (itr != sn_vo_map.end())
	{
		VO_MAP::iterator voItr = itr->second.find(eventId);
		if (voItr != itr->second.end())
		{
			LOG_WARN("JOFunctionBinder", "sn[%d] already have eventId[%d]!!!!!!\nnow relpace new data!!!!!", sn, eventId);
			//unbind(sn, eventId);
            JOEventBindVO* removeVo = itr->second[eventId];
            itr->second.erase(eventId);
            _removeListVO(sn, eventId);
            removeList.push_back(removeVo);
		}
		itr->second[eventId] = vo;
	}
	else
	{
		VO_MAP vo_map;
		vo_map[eventId] = vo;
		sn_vo_map[sn] = vo_map;
	}
	EVENTID_VOs_MAP::iterator eventItr = eventId_vos_map.find(eventId);
	if (eventItr != eventId_vos_map.end())
	{
		eventItr->second.push_back(vo);
	}
	else
	{
		VO_LIST vo_list;
		vo_list.push_back(vo);
		eventId_vos_map[eventId] = vo_list;
	}
}

void JOFunctionBinder::unbind(unsigned int sn, unsigned int eventId)
{
	SN_VO_MAP::iterator itr = sn_vo_map.find(sn);
	if (itr != sn_vo_map.end())
	{
		JOEventBindVO* vo = itr->second[eventId];
		itr->second.erase(eventId);
		if (itr->second.empty())
		{
			sn_vo_map.erase(itr);
		}
		_removeListVO(sn, eventId);
		removeList.push_back(vo);		
	}
}

void JOFunctionBinder::unbind(unsigned int sn)
{
	SN_VO_MAP::iterator snItr = sn_vo_map.find(sn);
	if (snItr != sn_vo_map.end())
	{
		VO_MAP::iterator voItr = snItr->second.begin();
		while (voItr != snItr->second.end())
		{
			_removeListVO(sn, voItr->first);
			removeList.push_back(voItr->second);
			++voItr;
		}
		sn_vo_map.erase(snItr);
	}
}

void JOFunctionBinder::_removeListVO(unsigned int sn, unsigned int eventId)
{
	EVENTID_VOs_MAP::iterator eventItr = eventId_vos_map.find(eventId);
	if (eventItr != eventId_vos_map.end())
	{
		VO_LIST* tmpList = &eventItr->second;
		vo_list_itr = tmpList->begin();
		
		while (vo_list_itr != tmpList->end())
		{
			if ((*vo_list_itr)->isSn(sn))
			{
				vo_list_itr = tmpList->erase(vo_list_itr);
				vo_remove_flag = true;
			}
			else
				++vo_list_itr;
		}
	}
}

void JOFunctionBinder::removeAllFunctions()
{
	EVENTID_VOs_MAP::iterator eventItr = eventId_vos_map.begin();
	while (eventItr != eventId_vos_map.end())
	{
		VO_LIST* tmpList = &eventItr->second;
		vo_list_itr = tmpList->begin();
		while (vo_list_itr != tmpList->end())
		{
			removeList.push_back((*vo_list_itr));
			++vo_list_itr;
			vo_remove_flag = true;
		}
		++eventItr;
	}
	eventId_vos_map.clear();
	sn_vo_map.clear();
}

void JOFunctionBinder::tick()
{
	std::list<JOEventBindVO*>::iterator itr = removeList.begin();
	while (itr != removeList.end())
	{
		(*itr)->clear();
		POOL_RECOVER((*itr), JOEventBindVO, "JOFunctionBinder");
		++itr;
	}
	removeList.clear();
}

void JOFunctionBinder::exec(unsigned int eventId, JODataCoder* dataCoder, int socketId)
{
	EVENTID_VOs_MAP::iterator eventItr = eventId_vos_map.find(eventId);
	if (eventItr != eventId_vos_map.end())
	{
		VO_LIST* tmpList = &eventItr->second;
		vo_list_itr = tmpList->begin();
		while (vo_list_itr != tmpList->end())
		{
			vo_remove_flag = false;
			(*vo_list_itr)->exec(eventId, dataCoder, socketId);
			if (vo_remove_flag==false)
				++vo_list_itr;
		}
	}
}
/*
void JOFunctionBinder::exec(unsigned int eventId, JODataBundle *dataBundle)
{
	EVENTID_VOs_MAP::iterator eventItr = eventId_vos_map.find(eventId);
	if (eventItr != eventId_vos_map.end())
	{
		VO_LIST* tmpList = &eventItr->second;
		vo_list_itr = tmpList->begin();
		while (vo_list_itr != tmpList->end())
		{
			vo_remove_flag = false;
			(*vo_list_itr)->exec(dataBundle);
			if (vo_remove_flag == false)
				++vo_list_itr;
		}
	}
}
*/


NS_JOFW_END
