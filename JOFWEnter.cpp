#include "JOFWEnter.h"
#include "JOFrameWork.h"
#include "core/socket/JOSocket.h"
#include "manager/commandqueue/JOCommandVO.h"
#include "manager/vo/JOFileCacheVO.h"
#include "ui/vo/JOUILayoutVO.h"

#include "module/map/rpg/vo/JOMapRPGTileVO.h"

//#include "module/dragonBones/JODragonBonesDataVO.h"

NS_JOFW_BEGIN

JOFWEnter::JOFWEnter()
{

}

JOFWEnter::~JOFWEnter()
{
	JOWinMgr::Destroy();
	JOUILayout::Destroy();
	// JO_EVENT BEGIN
	{
		JOSocketMgr::Destroy();
	}
	// JO_EVENT END

	JOFrameAnimationCache::Destroy();

//#ifdef JO_ASYNCH_LOADER
	JOAsynchMultLoader::Destroy();
	JOAsynchQueueLoader::Destroy();
	JOAsynchSoundLoader::Destroy();
	JOResMgr::Destroy();
//#endif // JO_ASYNCH_LOADER

#ifdef JO_SQL
	JOSQL::Destroy();
#endif //JO_SQL

#ifdef JO_MAP_RPG
	JOMapRPGAsset::Destroy();
	JOSceneRPGMgr::Destroy();
#endif // JO_MAP_RPG


	JODyerParser::Destroy();
	JOEventDispatcher::Destroy();


	JOTaskMgr::Destroy();
	JOCommandMgr::Destroy();
	JODataPool::Destroy();
	JOFileMgr::Destroy();	

	//JODBFactory::Destroy();
	
	JOTickMgr::Destroy();
	JOSnMgr::Destroy();

	JOClsMemoryPool::Destroy();
	JOCachePoolMgr::Destroy();
}

void JOFWEnter::init()
{
	_regPoolVO();
}

void JOFWEnter::tick(float dt)
{
	
	JOTickMgr::Instance()->tick(dt);
	JOCommandMgr::Instance()->tick();
	JODataPool::Instance()->tick();
	JOCachePoolMgr::Instance()->tick();
	JOFileMgr::Instance()->tick();

	JOEventDispatcher::Instance()->tick();
	JOWinMgr::Instance()->tick();
	JOUILayout::Instance()->tick();
	JODyerParser::Instance()->tick();
	JOLog::tick();
	
    JOFrameAnimationCache::Instance()->tick();
	JOFrameUnpackPng::Instance()->tick();

	//JODBFactory::Instance()->tick();
	
//#ifdef JO_ASYNCH_LOADER
	JOResMgr::Instance()->tick();
//#endif //JO_ASYNCH_LOADER    

#ifdef JO_SQL
	JOSQL::Instance()->tick();
#endif //JO_SQL

	JOClsMemoryPool::Instance()->tick();

}

void JOFWEnter::_regPoolVO()
{
	POOL_REG(JOSocket);
	POOL_REG(JOCommandVO);
	POOL_REG(JOEventBindVO);
	POOL_REG(JOAsynchLoaderVO);
	POOL_REG(JOAsynchSoundVO);
	
	POOL_REG(JOResSrcVO);
	POOL_REG(JOResArmatureVO);
	POOL_REG(JOFrameAnimationVO);

	POOL_REG(JOResRecordVO);
	POOL_REG(TickFunctInThreadVO);
	POOL_REG(JOTickVO);
	POOL_REG(JOFileCacheVO);
	POOL_REG(JOUILayoutVO);
	POOL_REG(JODyerVO);
	POOL_REG(JODyerBlockVO);

    POOL_REG(JOMapRPGTileVO);

	POOL_REG(JOClsMemoryVO);
}

NS_JOFW_END
