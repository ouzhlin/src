
#include "utils/JOLuaUtils.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

std::function<void(LUA_FUNCTION, std::list<JOLuaArgs>, short, std::list<JOLuaResult>&, bool)> JOLuaUtils::_executeFunCall = nullptr;
std::function<void(void)> JOLuaUtils::_dumpCall = nullptr;
std::function<const void*(lua_Object)> JOLuaUtils::_toPointerCall = nullptr;
std::function<const void*(LUA_FUNCTION)> JOLuaUtils::_toFunctionCall = nullptr;

std::function<const void*(void)> JOLuaUtils::_luaNewThreadStateCall = nullptr;
std::function<void(LUA_FUNCTION, const void*)> JOLuaUtils::_executeInThreadCall = nullptr;

const void* JOLuaUtils::lua2Pointer(lua_Object pointerIdx)
{
    if (_toPointerCall) {
        return _toPointerCall(pointerIdx);
    }
    return nullptr;
}
const void* JOLuaUtils::lua2Function(LUA_FUNCTION luaFunction)
{
    if (_toFunctionCall) {
        return _toFunctionCall(luaFunction);
    }
    return nullptr;
}

const void* JOLuaUtils::luaNewThreadState()
{
    if (_luaNewThreadStateCall){
        return _luaNewThreadStateCall();
    }
    return nullptr;
}

void JOLuaUtils::executeFunctionInThread(LUA_FUNCTION luaFunction, const void* luaThreadState)
{
    if (_executeInThreadCall) {
        _executeInThreadCall(luaFunction, luaThreadState);
    }
}

void JOLuaUtils::executeFunction(LUA_FUNCTION luaFunction, std::string argBase, JOLuaArgBaseType argBaseType)
{
    if (_executeFunCall != nullptr) {
        std::list<JOLuaArgs> tempList;
        JOLuaArgs luaArg;
        luaArg._argBase = argBase;
        luaArg._argBaseType = argBaseType;
        tempList.push_back(luaArg);        
        executeFunction(luaFunction, tempList);
    }
}


void JOLuaUtils::executeFunction( LUA_FUNCTION luaFunction, void* argObj, std::string argObjType)
{
    if (_executeFunCall != nullptr) {
        std::list<JOLuaArgs> tempList;
        JOLuaArgs luaArg;
        luaArg._argObj = argObj;
        luaArg._argObjType = argObjType;
        luaArg._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_OBJ;
        tempList.push_back(luaArg);
        
        executeFunction(luaFunction, tempList);
    }
    /*
#ifdef JO_LUA
    
    //LuaEngine *engine = LuaEngine::getInstance();
    //LuaStack *stack = engine->getLuaStack();
    tolua_pushusertype(_lua_State, argument, argType);
    // 	pStack->pushObject(argument, "DataBundle")
    // 	lua_pushlightuserdata( pStack->getLuaState(), argument );
    //stack->executeFunctionByHandler(luaFunction, 1);
    //stack->clean();
    
    toluafix_get_function_by_refid(_lua_State, luaFunction);                  // L: ... func
    if (!lua_isfunction(_lua_State, -1))
    {
        LOG_ERROR("JOLuaUtils", "[LUA ERROR] function refid '%d' does not reference a Lua function", luaFunction);
        lua_pop(_lua_State, 1);
        return ;
    }
    lua_insert(_lua_State, -(1 + 1));
    /////////////////////////////////////////////
    int numArgs = 1;
    int functionIndex = -(1 + 1);
    if (!lua_isfunction(_lua_State, functionIndex))
    {
        LOG_ERROR("JOLuaUtils", "value at stack [%d] is not function", functionIndex);
        lua_pop(_lua_State, numArgs + 1); // remove function and arguments
        return ;
    }
    
    int traceback = 0;
    lua_getglobal(_lua_State, "__G__TRACKBACK__");                         // L: ... func arg1 arg2 ... G
    if (!lua_isfunction(_lua_State, -1))
    {
        lua_pop(_lua_State, 1);                                            // L: ... func arg1 arg2 ...
    }
    else
    {
        lua_insert(_lua_State, functionIndex - 1);                         // L: ... G func arg1 arg2 ...
        traceback = functionIndex - 1;
    }
    
    int error = 0;

    error = lua_pcall(_lua_State, numArgs, 1, traceback);                  // L: ... [G] ret
    if (error)
    {
        if (traceback == 0)
        {
            LOG_ERROR("JOLuaUtils", "[LUA ERROR] %s", lua_tostring(_lua_State, - 1));        // L: ... error
            lua_pop(_lua_State, 1); // remove error message from stack
        }
        else                                                            // L: ... G error
        {
            lua_pop(_lua_State, 2); // remove __G__TRACKBACK__ and error message from stack
        }
        return ;
    }
    
    // get return value
    if (lua_isnumber(_lua_State, -1))
    {
        lua_tointeger(_lua_State, -1);
    }
    else if (lua_isboolean(_lua_State, -1))
    {
        lua_toboolean(_lua_State, -1);
    }
    // remove return value from stack
    lua_pop(_lua_State, 1);                                                // L: ... [G]
    
    if (traceback)
    {
        lua_pop(_lua_State, 1); // remove __G__TRACKBACK__ from stack      // L: ...
    }
    
    lua_settop(_lua_State, 0);
    
#else
    LOG_WARN("JOLuaUtils", "not define JO_LUA");
#endif
     */
}

void JOLuaUtils::executeFunction(LUA_FUNCTION luaFunction, std::list<JOLuaArgs> argList, bool bNewThread)
{
    if (_executeFunCall != nullptr) {
        std::list<JOLuaResult> relsultList;
        _executeFunCall(luaFunction, argList, 0, relsultList, bNewThread);
    }
}

void JOLuaUtils::executeFunctionWithResult(LUA_FUNCTION luaFunction, std::list<JOLuaArgs> argList, short resultNum, std::list<JOLuaResult> &resultList)
{
    if (_executeFunCall != nullptr) {
        _executeFunCall(luaFunction, argList, resultNum, resultList, false);
    }
}

void JOLuaUtils::setExecuteFunctionCall(const std::function<void(LUA_FUNCTION, std::list<JOLuaArgs>, short, std::list<JOLuaResult>&, bool)> &exeFunCall)
{
    _executeFunCall = exeFunCall;
}
void JOLuaUtils::setStackDumpCall(const std::function<void(void)> &dumpCall)
{
    _dumpCall = dumpCall;
}

void JOLuaUtils::setLua2Pointer(const std::function<const void*(lua_Object)> &toPointer)
{
    _toPointerCall = toPointer;
}
void JOLuaUtils::setLua2Function(const std::function<const void*(LUA_FUNCTION)> &toFunction)
{
    _toFunctionCall = toFunction;
}

void JOLuaUtils::setExecuteFunctionInThreadCall(const std::function<void(LUA_FUNCTION, const void*)> &exeFunCall)
{
    _executeInThreadCall = exeFunCall;
}
void JOLuaUtils::setLuaNewThreadState(const std::function<const void*(void)> &luaNewThreadState)
{
    _luaNewThreadStateCall = luaNewThreadState;
}
void JOLuaUtils::stackDump( )
{
    if (_dumpCall != nullptr) {
        _dumpCall();
    }
    /*
#ifdef JO_LUA
    //LuaEngine *engine = LuaEngine::getInstance();
    //LuaStack *_lua_State = engine->getLuaStack();
	LOG_INFO("JOLuaUtils", "-----------stack.dump----------\r\n");
	int i;
	int top = lua_gettop(_lua_State);
	for (i = 1; i <= top; i++) {  // repeat for each level
		int t = lua_type(_lua_State, i);
		switch (t) {
		case LUA_TSTRING:  // strings
			LOG_INFO("JOLuaUtils","[%d] '%s'", i , lua_tostring(_lua_State, i));
			break;
		case LUA_TBOOLEAN:  // booleans
			LOG_INFO("JOLuaUtils","[%d] %s", i ,lua_toboolean(_lua_State, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  // numbers
			LOG_INFO("JOLuaUtils","[%d] %g", i , lua_tonumber(_lua_State, i));
			break;
		default:  // other
			LOG_INFO("JOLuaUtils","[%d] %s", i , lua_typename(_lua_State, t));
			break;
		}

	}
#else
    LOG_WARN("JOLuaUtils", "not define JO_LUA");
#endif
     */
}

NS_JOFW_END

/*

#ifdef JO_LUA

extern "C" {
#include "lua.h"
#include "tolua++.h"
#include "lualib.h"
#include "lauxlib.h"
    
}
#include "tolua_fix.h"

#endif

static lua_State *_lua_State = nullptr;


int lua_println(lua_State * luastate)
{
    
#ifdef JO_LUA
	int nargs = lua_gettop(luastate);

	std::string t;
	for (int i=1; i <= nargs; i++)
	{
		if (lua_istable(luastate, i))
			t += "table";
		else if (lua_isnone(luastate, i))
			t += "none";
		else if (lua_isnil(luastate, i))
			t += "nil";
		else if (lua_isboolean(luastate, i))
		{
			if (lua_toboolean(luastate, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction(luastate, i))
			t += "function";
		else if (lua_islightuserdata(luastate, i))
			t += "lightuserdata";
		else if (lua_isthread(luastate, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring(luastate, i);
			//if(getLogLevelN(str)!=0) continue;
			if (str)
				t += lua_tostring(luastate, i);
			else
				t += lua_typename(luastate, lua_type(luastate, i));
		}
		if (i!=nargs)
			t += "\t";
	}
	lua_pop(luastate, 1);
	lua_settop(luastate, 0);
	JOLog::printLog(t.c_str());
	
#else
    LOG_WARN("JOLuaUtils", "not define JO_LUA");
    
#endif
    return 0;
}

void luaopen_JOLuaUtil( lua_State *L )
{
#ifdef JO_LUA
    _lua_State = L;
	const luaL_reg global_functions [] = {
		{"println", lua_println},
		{NULL, NULL}
	};
	luaL_register( L, "_G", global_functions);
#else
    LOG_WARN("JOLuaUtils", "not define JO_LUA");
#endif
}


*/


