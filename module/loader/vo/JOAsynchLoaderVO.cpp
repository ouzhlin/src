#include "module/loader/vo/JOAsynchLoaderVO.h"

extern "C" {
#include "lua.h"
#include "tolua++.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"

#include "cocos2d.h"
USING_NS_CC;

#include "core/datautils/JODataCoder.h"
#include "core/datautils/JODataPool.h"
#include "manager/JOCachePoolMgr.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

void JOAsynchLoaderVO::asynchExecLuaCall(int luaHandle, cocos2d::Texture2D* tex, std::string source, int resType, JODataCoder* dataCoder, unsigned int index, unsigned int totalCount)
{
	auto engine = LuaEngine::getInstance();
	LuaStack *stack = engine->getLuaStack();
	lua_State* L = stack->getLuaState();
	//1
	if (tex)
	{
		int ID = (tex) ? (int)tex->_ID : -1;
		int* luaID = (tex) ? &tex->_luaID : nullptr;
		toluafix_pushusertype_ccobject(L, ID, luaID, (void*)tex, "cc.Texture2D");
	}
	else
	{
		stack->pushNil();
	}
	//2
	stack->pushString(source.c_str());
	//3
	stack->pushInt(resType);
	//4
	stack->pushInt(index);
	//5
	stack->pushInt(totalCount);
	//6
	if (dataCoder)
	{
		tolua_pushusertype(stack->getLuaState(), (void*)dataCoder, "JODataCoder");
	}
	else
	{
		stack->pushNil();
	}

	stack->executeFunctionByHandler(luaHandle, 6);
}


void JOAsynchLoaderVO::exec(Texture2D* tex, unsigned int index/* = 1*/)
{
    //LOG_WARN("JOAsynchLoaderVO", "exec %d", sn);
	if (dataCoder){
		dataCoder->seek(0);
	}
	if (loadComplete)
	{
		loadComplete(tex, source, resType, dataCoder, index, totalCount);
	}
//	else if (luaLoadComplete != -1)
//	{
//		asynchExecLuaCall(luaLoadComplete, tex, source, resType, dataCoder, index, totalCount);
//	}
	if (index == totalCount){
		JODataPool::Instance()->recover(dataCoder);
	}
	//RECOVER_POOL_OBJ(dataBundle, "JODataBundle", "JOEventDispatcher");
}


void JOAsynchLoaderVO::init()
{
	source = "";
	plist = "";	
	sn = 0;
	totalCount = 1;
	luaLoadComplete = -1;
	loadComplete = nullptr;
	dataCoder = nullptr;
	
	samePathList.clear();
}


void JOAsynchLoaderVO::setData(const std::string src, int resType, Texture2D::PixelFormat pixel, int luaHandle, JODataCoder* dataCoder, unsigned int sn, unsigned int totalCount/* = 1*/)
{
	source = src;	
	baseFileName = JOPath::getFileWithoutSuffix(source);
	this->resType = resType;
	this->sn = sn;	
	this->totalCount = totalCount;
	this->pixel = pixel;
	luaLoadComplete = luaHandle;
	loadComplete = nullptr;
	this->dataCoder = dataCoder;
	samePathList.clear();
}

void JOAsynchLoaderVO::setData(const std::string src, int resType, Texture2D::PixelFormat pixel, CompLeteCall handle, JODataCoder* dataCoder, unsigned int sn, unsigned int totalCount/* = 1*/)
{
    //LOG_WARN("JOAsynchLoaderVO", "setData %d", sn);
	source = src;
	baseFileName = JOPath::getFileWithoutSuffix(source);
	this->resType = resType;
	this->sn = sn;
	this->totalCount = totalCount;
	this->pixel = pixel;
	luaLoadComplete = -1;
	loadComplete = handle;
	this->dataCoder = dataCoder;

	samePathList.clear();
}

void JOAsynchLoaderVO::unBindCall()
{
	loadComplete = nullptr;
	luaLoadComplete = -1;
}

NS_JOFW_END
