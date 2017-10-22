#include "module/event/vo/JOEventBindVO.h"
#include <algorithm>
#include "core/datautils/JODataCoder.h"
#include "utils/JOLog.h"
#include "utils/JOLuaUtils.h"

NS_JOFW_BEGIN

JOEventBindVO::JOEventBindVO()
:m_sn(0)
, m_eventId(0)
, m_cFunc(nullptr)
, m_luaFunc(-1)
{

}

JOEventBindVO::~JOEventBindVO()
{

}

void JOEventBindVO::init(unsigned int sn, unsigned int eventId, const std::function<void(unsigned int, void*, short)> cFunc)
{
	m_sn = sn;
	m_eventId = eventId;
	m_cFunc = cFunc;
	m_luaFunc = -1;
}

void JOEventBindVO::init(unsigned int sn, unsigned int eventId, LUA_FUNCTION luaFunc)
{
	m_sn = sn;
	m_eventId = eventId;
	m_luaFunc = luaFunc;
	m_cFunc = nullptr;	
}

bool JOEventBindVO::exec(unsigned int id, JODataCoder* dataCoder, short socketId)
{
	if (m_cFunc){
		if (dataCoder){
			dataCoder->seek(0);
		}
		m_cFunc(id, dataCoder, socketId);
		return true;
	}
	/*
	else if (m_luaFunc > 0)
	{
		std::list<JOLuaArgs> tempList;
		JOLuaArgs luaArg1;
		luaArg1._argObj = (void*)dataCoder;
		luaArg1._argObjType = "JODataCoder";
		luaArg1._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_OBJ;
		tempList.push_back(luaArg1);

		JOLuaArgs luaArg2;
		luaArg2._argBase = socketId;
		luaArg2._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_INT;
		tempList.push_back(luaArg2);

		JOLuaUtils::executeFunction(m_luaFunc, tempList);

		//JOLuaUtils::executeFunction(m_luaFunc, dataCoder, "JODataCoder");
		return true;
	}
	*/
	return false;
}


void JOEventBindVO::clear()
{
	m_sn = 0;
	m_eventId = 0;
	m_cFunc = nullptr;
	m_luaFunc = -1;
}

NS_JOFW_END