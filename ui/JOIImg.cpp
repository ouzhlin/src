#include "ui/JOIImg.h"
#include "module/loader/JOAsynchQueueLoader.h"
#include "module/loader/JOAsynchMultLoader.h"
#include "module/loader/JOResMgr.h"
#include "module/loader/JOResConfig.h"
#include "module/loader/vo/JOResConfigVO.h"
#include "utils/JOLog.h"

#include "manager/JOSnMgr.h"

NS_JOFW_BEGIN


JOIImg::JOIImg()
: m_sourcePath("")
, m_imgName("")
, m_tmpSourcePath("")
, m_tmpImgName("")
, m_loadDirty(false)
, m_loadType(JOAsynchBaseLoader::QUEUE)
{
	m_sn = JOSnMgr::Instance()->getSn();
	m_comLeteCall = JO_CBACK_6(JOIImg::_loadComplete, this);
}

JOIImg::~JOIImg()
{
	JOSnMgr::Instance()->dispose(m_sn);
	m_comLeteCall = nullptr;
}

void JOIImg::setKey(const std::string& imgName, bool isAsyn/*=true*/)
{
	//if (m_imgName != imgName){
		const JOResSrcVO* vo = JOResConfig::Instance()->srcVOWithKey(imgName);
		if (vo==nullptr){
			vo = JOResConfig::Instance()->srcVOWithSrc(imgName);
		}
		
		if (vo == nullptr){
			LOG_WARN("JOImgInterface", "can't find source with key[%s] !!!!", imgName.c_str());
			return;
		}
		if (vo->plist == "")
			setSource(vo->getSrc(), "", isAsyn, vo->pixelFormat);
		else
			setSource(vo->getSrc(), imgName, isAsyn, vo->pixelFormat);
	//}
}


void JOIImg::setSource(const std::string& filePath, const std::string& imgName, bool isAsyn /*= true*/, short pixelFormat /*= (short)Texture2D::PixelFormat::RGBA8888*/)
{
	if ( strcmp(m_tmpSourcePath.c_str(), filePath.c_str())==0 && strcmp(m_imgName.c_str(), imgName.c_str())==0){
		return;
	}
	if (_isLoading()){
		if (m_loadDirty){
			JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
			JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
			m_loadDirty = false;
		}
		else{
			if (m_loadType == JOAsynchBaseLoader::QUEUE)	{
                JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
			}
			else{
				JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
			}
		}
	}
    if ( strcmp(m_sourcePath.c_str(), filePath.c_str())!=0 || strcmp(m_imgName.c_str(), imgName.c_str())!=0){
	//if (m_sourcePath!=filePath || m_imgName != imgName){
		_loadStart();
		m_tmpSourcePath.assign(filePath);
		m_tmpImgName.assign(imgName);
		
		// 异步加载
		if (isAsyn)	{
			if (m_loadType == JOAsynchBaseLoader::QUEUE)	{
				JOAsynchQueueLoader::Instance()->load(m_sn, m_tmpSourcePath, JOAsynchBaseLoader::RES_IMG, m_comLeteCall);
			}
			else{
                JOAsynchMultLoader::Instance()->load(m_sn, m_tmpSourcePath, JOAsynchBaseLoader::RES_IMG, m_comLeteCall);
			}
		}
		// 即时加载
		else{
			// 加载纹理
			Texture2D* texture = Director::getInstance()->getTextureCache()->getTextureForKey(m_tmpSourcePath);
			if (texture == nullptr){
				const Texture2D::PixelFormat currentPixelFormat = Texture2D::getDefaultAlphaPixelFormat();
				Texture2D::setDefaultAlphaPixelFormat((Texture2D::PixelFormat)JOResConfig::Instance()->getPFVal(pixelFormat));
				texture = Director::getInstance()->getTextureCache()->addImage(m_tmpSourcePath);
				Texture2D::setDefaultAlphaPixelFormat(currentPixelFormat);
			}
			if (texture == nullptr){
				//_loadCancel();
				LOG_WARN("JOImgInterface", "can't find create texture with path [%s] !!!!", m_tmpSourcePath.c_str());
				_clearTmp();
				return;
			}
			JOResMgr::Instance()->setRecord(m_tmpSourcePath, texture, JOAsynchBaseLoader::RES_IMG);
			_loadComplete(texture, m_tmpSourcePath, JOAsynchBaseLoader::RES_IMG, nullptr, 1, 1);
		}
	}
}

void JOIImg::_loadComplete(Texture2D* tex, std::string source, short resType, JODataCoder* dataCoder, int index, int totalCount)
{
    if (tex == nullptr || source.length() < 1) {
        LOG_WARN("JOImgInterface", "tex==nil tmp[%s]; source[%s]!!!!", m_tmpSourcePath.c_str(), source.c_str());
        return;
    }
    if ( strcmp(m_tmpSourcePath.c_str(), source.c_str())!=0 ){
	//if (m_tmpSourcePath != source){
		LOG_WARN("JOImgInterface", "need [%s]; %s not the need resource!!!!", m_tmpSourcePath.c_str(), source.c_str());
		return;
	}
	m_loadDirty = false;	

	//减去当前资源引用
	if (m_sourcePath.length() > 0){
		JOResMgr::Instance()->unQuoteRes(m_sourcePath);
	}
	//引用处理
	JOResMgr::Instance()->quoteRes(source);

    m_sourcePath.assign(m_tmpSourcePath);
    m_imgName.assign(m_tmpImgName);
	
	_loadEnd();
	_clearTmp();
}

void JOIImg::clearSource()
{
	if (m_loadDirty){
		JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
		JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
		m_loadDirty = false;
	}
	else{
		if (m_loadType == JOAsynchBaseLoader::QUEUE)	{
			JOAsynchQueueLoader::Instance()->cancelLoad(m_sn);
		}
		else{
            JOAsynchMultLoader::Instance()->cancelLoad(m_sn);
		}
	}
	
	if (m_sourcePath.length()>0){
		JOResMgr::Instance()->unQuoteRes(m_sourcePath);
	}
	m_sourcePath.clear();
	m_imgName.clear();

	m_tmpSourcePath.clear();
	m_tmpImgName.clear();

	_emptyTexture();
}


void JOIImg::setLoadType(short loadType)
{
	if (m_loadType==loadType){
		return;
	}
	m_loadType = loadType;
	if (_isLoading() == true){
		m_loadDirty = true;
	}
}

void JOIImg::_clearTmp()
{
	m_tmpImgName.clear();
	m_tmpSourcePath.clear();
	_loadCancel();
}


NS_JOFW_END
