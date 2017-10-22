#include "manager/commandqueue/JOCommandVO.h"
#include "utils/JOLuaUtils.h"
#include "utils/JOString.h"

NS_JOFW_BEGIN

JOCommandVO::JOCommandVO() 
:c_runFunct(nullptr)
, lua_runFunct(-1)
, m_bCancel(false)
, m_id(0)
{

}

void JOCommandVO::init(unsigned int id, std::function<bool(void)> cRunFun)
{
	lua_runFunct = -1;

	m_id = id;
	c_runFunct = cRunFun;
	m_bCancel = false;
}

void JOCommandVO::init(unsigned int id, LUA_FUNCTION luaRunFun)
{
	c_runFunct = nullptr;

	m_id = id;
	lua_runFunct = luaRunFun;
	m_bCancel = false;	
}

bool JOCommandVO::run()
{
	if (m_bCancel)
		return true;
	
	if (c_runFunct != nullptr)
	{
		return c_runFunct();
	}
	else if (lua_runFunct != -1)
	{
		std::list<JOLuaArgs> argList;
		std::list<JOLuaResult> resultList;
		JOLuaUtils::executeFunctionWithResult(lua_runFunct, argList, 1, resultList);
		if (resultList.size() > 0) {
			JOLuaResult res = *resultList.begin();
			if (res._resultType == JOLuaArgBaseType::eJOLUA_ARGS_TYPE_BOOL) {
				return JOString::toBool(res._resultBase.c_str());
			}
		}
	}
	return true;
}



NS_JOFW_END