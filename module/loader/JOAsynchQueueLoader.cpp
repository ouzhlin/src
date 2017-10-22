#include "module/loader/JOAsynchQueueLoader.h"
#include "module/loader/vo/JOAsynchLoaderVO.h"
#include "manager/JOCachePoolMgr.h"
#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

void JOAsynchQueueLoader::startLoadImg()
{
	if (allLoadList.empty()){
		return;
	}
	
	JOAsynchLoaderVO* vo = allLoadList.front();
	allLoadList.pop_front();

	Texture2D *tex = Director::getInstance()->getTextureCache()->getTextureForKey(vo->source);
	if (tex)
	{
		vo->exec(tex);
		POOL_RECOVER(vo, JOAsynchLoaderVO, "JOAsynchBaseLoader");
	}
	else
	{
		LOADERVO_MAP::iterator itr = curLoadMap.find(vo->source);
		if (itr != curLoadMap.end())
		{
			/*
			当前正在加载
			*/
			itr->second->samePathList.push_back(vo);
			return;
		}
		else
		{
			curLoadMap[vo->source] = vo;			
			_handleCallAsynch(vo);
		}
	}	
}


NS_JOFW_END
