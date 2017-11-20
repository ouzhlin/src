#include "module/loader/JOAsynchBaseLoader.h"
#include "module/loader/JOAsynchSoundLoader.h"
#include "module/loader/JOResMgr.h"
#include "module/loader/vo/JOAsynchLoaderVO.h"

#include "module/loader/JOResConfig.h"
#include "module/loader/vo/JOResConfigVO.h"

#include "core/datautils/JODataCoder.h"
#include "core/datautils/JODataPool.h"

#include "utils/JOLog.h"
#include "manager/JOCachePoolMgr.h"
#include "manager/JOSnMgr.h"

#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

JOAsynchBaseLoader::JOAsynchBaseLoader()
{
}


JOAsynchBaseLoader::~JOAsynchBaseLoader()
{
	sn_curIdx_map.clear();
	sn_loaderVos_map.clear();

	std::list<JOAsynchLoaderVO*>::iterator allItr = allLoadList.begin();
	while (allItr != allLoadList.end())
	{
		POOL_RECOVER((*allItr), JOAsynchLoaderVO, "JOAsynchBaseLoader");
		++allItr;
	}
	allLoadList.clear();

	LOADERVO_MAP::iterator itr = curLoadMap.begin();
	std::list<JOAsynchLoaderVO*>* tmpList = nullptr;
	while (itr != curLoadMap.end())
	{
		JOAsynchLoaderVO *vo = itr->second;	
		tmpList = &vo->samePathList;
		while (!tmpList->empty())
		{
			JOAsynchLoaderVO* sameVo = tmpList->front();
			tmpList->pop_front();
			POOL_RECOVER(sameVo, JOAsynchLoaderVO, "JOAsynchBaseLoader");
		}		
		POOL_RECOVER(vo, JOAsynchLoaderVO, "JOAsynchBaseLoader");
		++itr;
	}
	curLoadMap.clear();
}

void JOAsynchBaseLoader::_handleComple(Texture2D* tex, JOAsynchLoaderVO* vo)
{
	switch (vo->resType)
	{
	case JOAsynchBaseLoader::RES_MUSIC:
		break;
	case JOAsynchBaseLoader::RES_SOUND:
		break;
	case JOAsynchBaseLoader::RES_IMG:
	default:
		if (!tex){
			LOG_ERROR("JOAsynchBaseLoader", "can't load [%s] texture!!!!", vo->source.c_str());
			return;
		}
		JOResMgr::Instance()->setRecord(vo->source, tex, vo->resType);
		break;
	}
}


void JOAsynchBaseLoader::_load(JOAsynchLoaderVO* vo)
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
	allLoadList.push_back(vo);
}


bool JOAsynchBaseLoader::_loadVo(const std::string& srcPath, short resType, CompLeteCall loadCompleteCall, LUA_FUNCTION luaCompleteCall, JODataCoder* dataCoder, unsigned short idx, unsigned short total)
{
	if (srcPath.empty() || srcPath.length() < 1){
		LOG_WARN("JOAsynchBaseLoader", "srcPath empty!!!!!");
		return false;
	}
	switch (resType)
	{
	case JOAsynchBaseLoader::RES_MUSIC:
		break;
	case JOAsynchBaseLoader::RES_SOUND:
		break;
	case JOAsynchBaseLoader::RES_IMG:
	default:
		Texture2D* tex = Director::getInstance()->getTextureCache()->getTextureForKey(srcPath);
		if (tex)
		{
			if (dataCoder){
				dataCoder->seek(0);
			}
			if (loadCompleteCall){
				loadCompleteCall(tex, srcPath, resType, dataCoder, idx, total);
			}
//			else if (luaCompleteCall != -1){
//				JOAsynchLoaderVO::asynchExecLuaCall(luaCompleteCall, tex, srcPath, resType, dataCoder, idx, total);
//			}
			if (idx == total){
				JODataPool::Instance()->recover(dataCoder);
			}
			return false;
		}
		break;
	}
	return true;
}



void JOAsynchBaseLoader::load(unsigned int sn, const std::string& srcPath, short resType, const CompLeteCall loadCompleteCall, JODataCoder* dataCoder /*= nullptr*/, Texture2D::PixelFormat pixel /*= Texture2D::PixelFormat::NONE*/)
{
	if (_loadVo(srcPath, resType, loadCompleteCall, -1, dataCoder, 1, 1)){
		JOAsynchLoaderVO* vo = POOL_GET(JOAsynchLoaderVO, "JOAsynchBaseLoader");
		vo->setData(srcPath, resType, pixel, loadCompleteCall, dataCoder, sn);
		_addSnVoRecord(vo);
		_load(vo);
		startLoadImg();
	}
}


void JOAsynchBaseLoader::load(unsigned int sn, std::list<std::string> srcPathList, short resType, const CompLeteCall loadCompleteCall, JODataCoder* dataCoder /*= nullptr*/, Texture2D::PixelFormat pixel /*= Texture2D::PixelFormat::NONE*/)
{
	unsigned int totalCount = srcPathList.size();
	unsigned int idx = 1;
	bool bHandle = false;
	std::list<std::string>::iterator itr = srcPathList.begin();
	while (itr != srcPathList.end())
	{
		if (_loadVo((*itr), resType, loadCompleteCall, -1, dataCoder, idx, totalCount)){
			bHandle = true;
			JOAsynchLoaderVO* vo = POOL_GET(JOAsynchLoaderVO, "JOAsynchBaseLoader");
			vo->setData((*itr), resType, pixel, loadCompleteCall, dataCoder, sn, totalCount);
			_addSnVoRecord(vo);
			_load(vo);
		}
		else{
			++idx;
		}
		++itr;
	}
	if (bHandle){
		sn_curIdx_map[sn] = idx;
		startLoadImg();
	}
}


void JOAsynchBaseLoader::cancelLoad(unsigned int sn)
{
	SN_LOADERVOs_MAP::iterator itr = sn_loaderVos_map.find(sn);
	if (itr != sn_loaderVos_map.end()){
		std::list<JOAsynchLoaderVO*> tmpList = itr->second;
		std::list<JOAsynchLoaderVO*>::iterator voItr = tmpList.begin();
		while (voItr != tmpList.end()){
			(*voItr)->unBindCall();
			++voItr;
		}
		sn_loaderVos_map.erase(itr);
	}
}

/*

void JOAsynchBaseLoader::load(unsigned int sn, const std::string srcPath, short resType, LUA_FUNCTION luaCompleteCall, JODataBundle* dataBundle )
{
	if (_loadVo(srcPath, resType, nullptr, luaCompleteCall, dataBundle, 1, 1)){
		JOAsynchLoaderVO* vo = POOL_GET(JOAsynchLoaderVO, "JOAsynchBaseLoader");
		vo->setData(srcPath, resType, luaCompleteCall, dataBundle);
		_load(vo);
		startLoadImg();
	}
}

void JOAsynchBaseLoader::load(unsigned int sn, std::list<std::string> srcPathList, short resType, LUA_FUNCTION luaCompleteCall, JODataBundle* dataBundle)
{
	unsigned int totalCount = srcPathList.size();
	unsigned int idx = 1;
	bool bHandle = false;
	unsigned int sn = JOSnMgr::Instance()->getSn();
	std::list<std::string>::iterator itr = srcPathList.begin();
	while (itr != srcPathList.end())
	{
		if (_loadVo((*itr), resType, nullptr, luaCompleteCall, dataBundle, idx, totalCount)){
			bHandle = true;
			JOAsynchLoaderVO* vo = POOL_GET(JOAsynchLoaderVO, "JOAsynchBaseLoader");
			vo->setData((*itr), resType, luaCompleteCall, dataBundle, sn, totalCount);
			_load(vo);
		}
		else{
			++idx;
		}
		++itr;
	}
	if (bHandle){
		sn_curIdx_map[sn] = idx;
		startLoadImg();
	}
	else{
		JOSnMgr::Instance()->dispose(sn);
	}
}
*/

void JOAsynchBaseLoader::asyncLoadCompleteCall(Texture2D* tex, const std::string& imgPath)
{
    //LOG_WARN("JOAsynchBaseLoader", "asyncLoadCompleteCall %s", imgPath.c_str());
	LOADERVO_MAP::iterator itr = curLoadMap.find(imgPath);
	if (itr != curLoadMap.end())
	{
		JOAsynchLoaderVO *vo = itr->second;
		_handleComple(tex, vo);
		unsigned int curIdx = _snFindCurIdx(vo->sn, vo->totalCount);
		vo->exec(tex, curIdx);
		std::list<JOAsynchLoaderVO*> tmpList = vo->samePathList;
        std::list<JOAsynchLoaderVO*>::iterator tmpItr = tmpList.begin();
		while (tmpItr!=tmpList.end())
		{
            JOAsynchLoaderVO* sameVo = *tmpItr;//tmpList.front();
			//tmpList.pop_front();
			curIdx = _snFindCurIdx(sameVo->sn, sameVo->totalCount);
			sameVo->exec(tex, curIdx);
			_removeSnVoRecord(sameVo);
			POOL_RECOVER(sameVo, JOAsynchLoaderVO, "JOAsynchBaseLoader");
            tmpItr++;
		}
        tmpList.clear();
        _removeSnVoRecord(vo);
		POOL_RECOVER(vo, JOAsynchLoaderVO, "JOAsynchBaseLoader");
        curLoadMap.erase(itr);
	}
    else{
        LOG_WARN("JOAsynchBaseLoader", "loading map can't find %s", imgPath.c_str());
    }
	startLoadImg();
}

void JOAsynchBaseLoader::_handleCallAsynch(JOAsynchLoaderVO* vo)
{
	if (JOAsynchBaseLoader::RES_MUSIC == vo->resType)
	{
		JOAsynchSoundLoader::Instance()->addSoundAsync(vo->source, true, [=](void){
			asyncLoadCompleteCall(nullptr, vo->source);
		});
	}
	else if (JOAsynchBaseLoader::RES_SOUND == vo->resType)
	{
		JOAsynchSoundLoader::Instance()->addSoundAsync(vo->source, false, [=](void){
			asyncLoadCompleteCall(nullptr, vo->source);
		});
	}
	else
	{
        std::string source = vo->source;
		Texture2D::PixelFormat pixel = vo->pixel;
		if (pixel==Texture2D::PixelFormat::NONE)
		{
			const JOResSrcVO* srcVo = JOResConfig::Instance()->srcVOWithSrc(vo->baseFileName);
			if (srcVo){
				pixel = (Texture2D::PixelFormat)srcVo->pixelFormat;
			}
		}
        //LOG_WARN("JOAsynchBaseLoader", "_handleCallAsynch %s", source.c_str());
		Director::getInstance()->getTextureCache()->addImageAsync(source, [=](Texture2D* tex2d){
			asyncLoadCompleteCall(tex2d, source);
		}, pixel);
	}
}

unsigned int JOAsynchBaseLoader::_snFindCurIdx(unsigned int sn, unsigned int total)
{
	if (sn > 0){
		SN_CURIDX_MAP::iterator itr = sn_curIdx_map.find(sn);
		if (itr != sn_curIdx_map.end()){
			unsigned int curIdx = itr->second;
			if (curIdx+1>=total){
				sn_curIdx_map.erase(itr);
				JOSnMgr::Instance()->dispose(sn);
			}
			else{
				sn_curIdx_map[sn] = curIdx + 1;
			}			
			return curIdx;
		}
	}	
	return 1;
}

void JOAsynchBaseLoader::_addSnVoRecord(JOAsynchLoaderVO* vo)
{
	SN_LOADERVOs_MAP::iterator itr = sn_loaderVos_map.find(vo->sn);
	if (itr == sn_loaderVos_map.end()){
		std::list<JOAsynchLoaderVO*> list;
        list.push_back(vo);
		sn_loaderVos_map[vo->sn] = list;
        return;
	}
	sn_loaderVos_map[vo->sn].push_back(vo);
}

void JOAsynchBaseLoader::_removeSnVoRecord(JOAsynchLoaderVO* vo)
{
	SN_LOADERVOs_MAP::iterator itr = sn_loaderVos_map.find(vo->sn);
	if (itr != sn_loaderVos_map.end()){		
		itr->second.remove(vo);
		if (itr->second.empty()){
			sn_loaderVos_map.erase(itr);
		}
	}
}


NS_JOFW_END
