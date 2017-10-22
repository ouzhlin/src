#include "manager/vo/JOLuaHandleVO.h"

NS_JOFW_BEGIN

JOLuaHandleVO::JOLuaHandleVO()
: m_handle(-1)
, m_handleType(0)
{

}

JOLuaHandleVO::~JOLuaHandleVO()
{

}

void JOLuaHandleVO::init(LUA_FUNCTION luaHandle, unsigned int handleType)
{
	m_handle = luaHandle;
	m_handleType = handleType;
}

NS_JOFW_END