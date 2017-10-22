#include "module/loader/JOResMgr.h"
#include "module/loader/vo/JOResConfigVO.h"
#include "module/loader/JOResConfig.h"
#include "module/loader/vo/JOResRecordVO.h"
#include "module/loader/JOAsynchBaseLoader.h"

#include "manager/JOCachePoolMgr.h"
#include "manager/JOTickMgr.h"
#include "utils/JOString.h"
#include "utils/JOPath.h"
#include "utils/JOTime.h"

#include "cocos2d.h"
USING_NS_CC;

//#include "editor-support/cocostudio/CocoStudio.h"

NS_JOFW_BEGIN

JOResMgr::JOResMgr() :checkInterval(180)
{
}

JOResMgr::~JOResMgr()
{
}

bool JOResMgr::isResLoaded(std::string& resKey)
{
	RECORD_MAP::iterator itr = recordMap.find(resKey);
	if (itr != recordMap.end())
	{
		return true;
	}
	return false;
}

void JOResMgr::quoteRes(std::string& resKey)
{
	RECORD_MAP::iterator itr = recordMap.find(resKey);
	if (itr != recordMap.end())
	{
		itr->second->increase();
	}
}

void JOResMgr::unQuoteRes(std::string& resKey)
{
	RECORD_MAP::iterator itr = recordMap.find(resKey);
	if (itr != recordMap.end())
	{
		itr->second->decrease();
	}
}

void JOResMgr::setRecord(std::string& srcPath, cocos2d::Texture2D* tex, short resType)
{
	RECORD_MAP::iterator itr = recordMap.find(srcPath);
	if (itr == recordMap.end())
	{
		std::string srcKey = JOPath::getFileName(srcPath);
		JOResRecordVO *vo = POOL_GET(JOResRecordVO, "JOResMgr");
		vo->setRecord(srcKey, tex, resType);
		recordMap[srcPath] = vo;
		
		const JOResSrcVO* srcVo = JOResConfig::Instance()->srcVOWithSrc(srcKey);
		if (srcVo && resType == JOAsynchBaseLoader::RES_IMG){
			if (srcVo->plist!=""){
				std::string plist = srcVo->getPlist();
				if (!SpriteFrameCache::getInstance()->isSpriteFramesWithFileLoaded(plist)){
					SpriteFrameCache::getInstance()->addSpriteFramesWithFile(plist, tex);
				}
				/*
				const JOResArmatureVO* armVo = JOResConfig::Instance()->armVOWithSrc(srcKey);
				if (armVo){
					cocostudio::ArmatureDataManager::getInstance()->addArmatureFileInfo(armVo->getArmPath());
				}
				*/
			}
		}
	}
}

void JOResMgr::tick()
{
	static float interval = 0;
	interval += JOTickMgr::Instance()->deltaTime();
	if (interval > checkInterval){
		interval = 0;
		_checkDispose();		
	}
}

void JOResMgr::_checkDispose()
{
	JOResConfig* resConfig = JOResConfig::Instance();
	SpriteFrameCache* frameCache = SpriteFrameCache::getInstance();
	TextureCache* texCache = Director::getInstance()->getTextureCache();

	clock_t curTime = JOTime::getTimeofday();
	RECORD_MAP::iterator itr = recordMap.begin();
	while (itr != recordMap.end())
	{
		JOResRecordVO *vo = itr->second;
		if (vo->count <= 0 && (curTime - vo->noQuoteTime) > checkInterval)
		{			
			if (vo->resType == JOAsynchBaseLoader::RES_IMG){
				const JOResSrcVO* srcVo = resConfig->srcVOWithSrc(vo->srcPath);
				if (srcVo && srcVo->plist != ""){
					std::string plist = srcVo->getPlist();
					frameCache->removeSpriteFramesFromFile(plist);
					/*
					const JOResArmatureVO* armVo = JOResConfig::Instance()->armVOWithSrc(srcVo->srcKey);
					if (armVo){
						cocostudio::ArmatureDataManager::getInstance()->removeArmatureFileInfo(armVo->getArmPath());
					}
					*/
				}
			}
			
			vo->dispose();
			texCache->removeTextureForKey(itr->first);
			POOL_RECOVER(vo, JOResRecordVO, "JOResMgr");
			itr = recordMap.erase(itr);
		}
		else
		{
			++itr;
		}
	}
	//texCache->removeUnusedTextures();
}

NS_JOFW_END