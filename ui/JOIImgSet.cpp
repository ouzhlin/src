#include "ui/JOIImgSet.h"
#include "module/loader/JOAsynchQueueLoader.h"
#include "module/loader/JOAsynchMultLoader.h"
#include "module/loader/JOResMgr.h"
#include "module/loader/JOResConfig.h"
#include "module/loader/vo/JOResConfigVO.h"
#include "utils/JOLog.h"

#include "manager/JOSnMgr.h"

NS_JOFW_BEGIN

JOIImgSet::JOIImgSet()
: m_loadDirty(false)
, m_loadType(JOAsynchBaseLoader::QUEUE)
{
	m_sn = JOSnMgr::Instance()->getSn();
	m_comLeteCall = JO_CBACK_6(JOIImgSet::_loadComplete, this);
}

JOIImgSet::~JOIImgSet()
{
	JOSnMgr::Instance()->dispose(m_sn);
	m_comLeteCall = nullptr;
}


void JOIImgSet::setSource(const std::list<std::string>& srcList, bool isAsyn /*= true*/, const std::unordered_map<std::string, short>* pixelMap/*=nullptr*/)
{
	if (_isLoading()){
		if (m_loadDirty){
			JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
			JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
			m_loadDirty = false;
		}
		else{
			if (m_loadType == JOAsynchBaseLoader::QUEUE){
                JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
			}
			else{
				JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
			}
		}
	}
	_loadStart();
	m_tmpSrcList = srcList;

	// 异步加载
	if (isAsyn)	{
		JOAsynchMultLoader::Instance()->load(m_sn, m_tmpSrcList, JOAsynchBaseLoader::RES_IMG, m_comLeteCall);
	}
	// 即时加载
	else{
		int tmpSrcCount = m_tmpSrcList.size();
		std::list<std::string>::const_iterator itr = m_tmpSrcList.begin();
		int idx = 1;
		Texture2D::PixelFormat currentPixelFormat;
		const JOResSrcVO* srcVo = nullptr;
		while (itr != m_tmpSrcList.end())
		{
			srcVo = JOResConfig::Instance()->srcVOWithPlist((*itr));
			std::string source = srcVo->getSrc();

			// 加载纹理
			Texture2D* texture = Director::getInstance()->getTextureCache()->getTextureForKey((*itr));
			if (texture == nullptr){
				currentPixelFormat = Texture2D::getDefaultAlphaPixelFormat();
				Texture2D::setDefaultAlphaPixelFormat((Texture2D::PixelFormat)JOResConfig::Instance()->getPFVal(srcVo->pixelFormat));
				texture = Director::getInstance()->getTextureCache()->addImage(source);
				Texture2D::setDefaultAlphaPixelFormat(currentPixelFormat);
			}
			if (texture == nullptr){
				_clearTmp();
				return;
			}
			JOResMgr::Instance()->setRecord(source, texture, JOAsynchBaseLoader::RES_IMG);
			_loadComplete(texture, source, JOAsynchBaseLoader::RES_IMG, nullptr, idx, tmpSrcCount);
			++idx;
			++itr;
		}
	}
}


void JOIImgSet::_loadComplete(Texture2D* tex, std::string source, short resType, JODataCoder* dataCoder, int index, int totalCount)
{
	if (tex == nullptr){
		LOG_ERROR("JOFrameSprite", "[%s]; %s not the need resource!!!!", source.c_str());
		return;
	}
	if (totalCount <= index) {
		m_loadDirty = false;
		//减去当前资源引用
		std::list<std::string>::iterator itr = m_srcList.begin();
		while (itr != m_srcList.end()){
			JOResMgr::Instance()->unQuoteRes((*itr));
			++itr;
		}
		itr = m_tmpSrcList.begin();
		m_srcList.clear();
		//引用处理
		while (itr != m_tmpSrcList.end()){
			JOResMgr::Instance()->quoteRes((*itr));
			m_srcList.push_back((*itr));
			++itr;
		}

		_clearTmp();

		_loadEnd();
	}
}

void JOIImgSet::clearSource()
{
	if (m_loadDirty){
		JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
		JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
		m_loadDirty = false;
	}
	else{
		if (m_loadType == JOAsynchBaseLoader::QUEUE){
            JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
		}
		else{
			JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
		}
	}

	//减去当前资源引用
	std::list<std::string>::iterator itr = m_srcList.begin();
	while (itr != m_srcList.end()){
		JOResMgr::Instance()->unQuoteRes((*itr));
		++itr;
	}
	m_srcList.clear();
	m_tmpSrcList.clear();
	_emptyTexture();
}


void JOIImgSet::setLoadType(short loadType)
{
	if (m_loadType==loadType){
		return;
	}
	m_loadType = loadType;
	if (_isLoading() == true){
		m_loadDirty = true;
	}
}

void JOIImgSet::_clearTmp()
{
	m_tmpSrcList.clear();
	_loadCancel();
}


NS_JOFW_END
