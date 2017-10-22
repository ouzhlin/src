#include "manager/JOLuaScriptMgr.h"

extern "C" {
#include "tolua++.h"
}

#include "utils/JOLog.h"
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "cocos2d.h"
#include "scripting/lua-bindings/manual/CCLuaStack.h"
#include "scripting/lua-bindings/manual/CCLuaValue.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"

USING_NS_CC;

NS_JOFW_BEGIN

JOLuaScriptMgr::JOLuaScriptMgr() 
{
	
}

JOLuaScriptMgr::~JOLuaScriptMgr()
{
	clear();
}

void JOLuaScriptMgr::addHandler(void* cObj, cocos2d::LUA_FUNCTION handle, unsigned short handleType)
{
	if (cObj==nullptr){
		return;
	}
	if (removeHandler(cObj, handleType)){
		LOG_WARN("JOLuaScriptMgr", "type[%d] already have add handle in same c-obj, now replace!!!!");
	}
	LUA_C_OBJ_MAP::iterator oItr = m_cObjMap.find(cObj);
	if (oItr != m_cObjMap.end()){
		oItr->second[handleType] = handle;
	}
	else{
		LUA_HANDLE_MAP handleMap;
		handleMap[handleType] = handle;
		m_cObjMap[cObj] = handleMap;
	}
}

bool JOLuaScriptMgr::removeHandler(void* cObj, unsigned short handleType)
{
	LUA_C_OBJ_MAP::iterator oItr = m_cObjMap.find(cObj);
	if (oItr != m_cObjMap.end()){
		LUA_HANDLE_MAP::iterator hItr = oItr->second.find(handleType);
		if (hItr != oItr->second.end())	{
			oItr->second.erase(hItr);
			//LuaEngine::getInstance()->removeScriptHandler(hItr->second);
			if (oItr->second.empty()){
				m_cObjMap.erase(oItr);
			}
			return true;
		}
	}
	return false;
}

void JOLuaScriptMgr::removeAll(void* cObj)
{
	LUA_C_OBJ_MAP::iterator oItr = m_cObjMap.find(cObj);
	if (oItr != m_cObjMap.end()){
		/*
		LUA_HANDLE_MAP::iterator hItr = oItr->second.begin();
		while (hItr != oItr->second.end()){			
			LuaEngine::getInstance()->removeScriptHandler(hItr->second);			
			hItr++;
		}
		*/
		m_cObjMap.erase(oItr);
	}
}

void JOLuaScriptMgr::clear()
{
	m_cObjMap.clear();
}

cocos2d::LUA_FUNCTION JOLuaScriptMgr::getHandle(void* cObj, unsigned short handleType)
{
	LUA_C_OBJ_MAP::iterator oItr = m_cObjMap.find(cObj);
	if (oItr != m_cObjMap.end()){
		LUA_HANDLE_MAP::iterator hItr = oItr->second.find(handleType);
		if (hItr != oItr->second.end())	{
			return hItr->second;
		}
	}
	return -1;
}

void JOLuaScriptMgr::_handle(void* cObj, cocos2d::LUA_FUNCTION handle, unsigned short handleType)
{
	bool beHandle = false;
	switch (handleType)
	{
	case 1:
	{
		JOLuaScriptMgr* o = static_cast<JOLuaScriptMgr*>(cObj);
		if (o){
			beHandle = true;
			break;
		}
		break;
	}
	default:
		break;
	}
	if (beHandle==false){
		removeHandler(cObj, handleType);
	}
}

NS_JOFW_END
//////////////////////////////////////////////////////////////////////////
USING_NS_JOFW;

static void tolua_reg_ScriptMgr_type(lua_State* tolua_S)
{
	tolua_usertype(tolua_S, "JOLuaScriptMgr");
}


#ifndef tolua_ScriptMgr_Instance00
static int tolua_ScriptMgr_Instance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
	tolua_Error tolua_err;
	if (!tolua_isusertable(tolua_S, 1, "JOLuaScriptMgr", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err))
		goto tolua_lerror;
	else
#endif
	{
		JOLuaScriptMgr* tolua_ret = (JOLuaScriptMgr*)JOLuaScriptMgr::Instance();
		tolua_pushusertype(tolua_S, (void*)tolua_ret, "JOLuaScriptMgr");
	}
	return 1;
#ifndef TOLUA_RELEASE
tolua_lerror :
	tolua_error(tolua_S, "#ferror in function 'Instance'.", &tolua_err);
	return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

static int tolua_ScriptMgr_addHandler00(lua_State* tolua_S)
{
	JOLuaScriptMgr* mgr = (JOLuaScriptMgr*)tolua_tousertype(tolua_S, 1, 0);
	if (mgr==nullptr){
		LOG_ERROR("JOLuaScriptMgr", "JOLuaScriptMgr is null error!!!! ");
		return 0;
	}
	
	short argc = lua_gettop(tolua_S) - 1;
	if (argc != 3){
		LOG_ERROR("JOLuaScriptMgr", "addHandler argc [%d] must be 3 error!!!! ", argc);
		return 0;
	}
	void* cobj = tolua_tousertype(tolua_S, 2, 0);
	cocos2d::LUA_FUNCTION handler = toluafix_ref_function(tolua_S, 3, 0);
	unsigned short handlerType = (unsigned short)tolua_tonumber(tolua_S, 4, 0);
	mgr->addHandler(cobj, handler, handlerType);
		
	return 1;
}

/* method: unregisterScriptHandler of class  ScriptHandlerMgr */
static int tolua_ScriptMgr_removeHandler00(lua_State* tolua_S)
{
	JOLuaScriptMgr* mgr = (JOLuaScriptMgr*)tolua_tousertype(tolua_S, 1, 0);
	if (mgr == nullptr){
		LOG_ERROR("JOLuaScriptMgr", "JOLuaScriptMgr is null error!!!! ");
		return 0;
	}

	short argc = lua_gettop(tolua_S) - 1;
	if (argc != 2){
		LOG_ERROR("JOLuaScriptMgr", "removeHandler argc [%d] must be 2 error!!!! ", argc);
		return 0;
	}
	void* cobj = tolua_tousertype(tolua_S, 2, 0);	
	unsigned short handlerType = (unsigned short)tolua_tonumber(tolua_S, 3, 0);
	mgr->removeHandler(cobj, handlerType);
	
	return 1;
}

static int tolua_ScriptMgr_removeAll00(lua_State* tolua_S)
{
	JOLuaScriptMgr* mgr = (JOLuaScriptMgr*)tolua_tousertype(tolua_S, 1, 0);
	if (mgr == nullptr){
		LOG_ERROR("JOLuaScriptMgr", "JOLuaScriptMgr is null error!!!! ");
		return 0;
	}

	short argc = lua_gettop(tolua_S) - 1;
	if (argc != 1){
		LOG_ERROR("JOLuaScriptMgr", "removeAll argc [%d] must be 2 error!!!! ", argc);
		return 0;
	}
	void* cobj = tolua_tousertype(tolua_S, 2, 0);	
	mgr->removeAll(cobj);
	return 1;
}

static int tolua_ScriptMgr_clear00(lua_State* tolua_S)
{
	JOLuaScriptMgr* mgr = (JOLuaScriptMgr*)tolua_tousertype(tolua_S, 1, 0);
	if (mgr == nullptr){
		LOG_ERROR("JOLuaScriptMgr", "JOLuaScriptMgr is null error!!!! ");
		return 0;
	}
	mgr->clear();
	return 1;
}

TOLUA_API int tolua_JOLuaScriptMgr_open(lua_State* tolua_S)
{
	tolua_open(tolua_S);
	tolua_reg_ScriptMgr_type(tolua_S);
	tolua_module(tolua_S, NULL, 0);
	tolua_beginmodule(tolua_S, NULL);
	tolua_cclass(tolua_S, "JOLuaScriptMgr", "JOLuaScriptMgr", "", NULL);
	tolua_beginmodule(tolua_S, "JOLuaScriptMgr");
	tolua_function(tolua_S, "Instance", tolua_ScriptMgr_Instance00);
	tolua_function(tolua_S, "addHandler", tolua_ScriptMgr_addHandler00);
	tolua_function(tolua_S, "removeHandler", tolua_ScriptMgr_removeHandler00);
	tolua_function(tolua_S, "removeAll", tolua_ScriptMgr_removeAll00);
	tolua_function(tolua_S, "clear", tolua_ScriptMgr_clear00);
	tolua_endmodule(tolua_S);
	tolua_endmodule(tolua_S);
	return 1;
}


//
//void db_Transform_to_luaval(lua_State* L, const dragonBones::Transform& trans)
//{
//	if (NULL == L)
//		return;
//	lua_newtable(L);                                    /* L: table */
//	lua_pushstring(L, "x");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.x);               /* L: table key value*/
//	lua_rawset(L, -3);                                  /* table[key] = value, L: table */
//	lua_pushstring(L, "y");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.y);               /* L: table key value*/
//	lua_rawset(L, -3);
//	lua_pushstring(L, "skewX");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.skewX);               /* L: table key value*/
//	lua_rawset(L, -3);
//	lua_pushstring(L, "skewY");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.skewY);               /* L: table key value*/
//	lua_rawset(L, -3);
//	lua_pushstring(L, "scaleX");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.scaleX);               /* L: table key value*/
//	lua_rawset(L, -3);
//	lua_pushstring(L, "scaleY");                             /* L: table key */
//	lua_pushnumber(L, (lua_Number)trans.scaleY);               /* L: table key value*/
//	lua_rawset(L, -3);
//}
