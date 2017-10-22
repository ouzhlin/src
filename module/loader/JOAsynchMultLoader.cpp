#include "module/loader/JOAsynchMultLoader.h"
#include "module/loader/vo/JOAsynchLoaderVO.h"
#include "manager/JOCachePoolMgr.h"

#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

JOAsynchMultLoader::JOAsynchMultLoader() :asynchMaxNum(6), loadedImgNum(1)
{
}

void JOAsynchMultLoader::startLoadImg()
{
	while (!allLoadList.empty() && loadedImgNum < asynchMaxNum)
	{
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
				++loadedImgNum;

				_handleCallAsynch(vo);				
			}
		}
	}
}

void JOAsynchMultLoader::asyncLoadCompleteCall(Texture2D* tex, const std::string& imgPath)
{
	--loadedImgNum;
	JOAsynchBaseLoader::asyncLoadCompleteCall(tex, imgPath);
}

NS_JOFW_END