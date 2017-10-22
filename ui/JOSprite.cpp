#include "ui/JOSprite.h"
#include "utils/JOLog.h"


NS_JOFW_BEGIN


JOSprite::JOSprite() 
: m_loadCall(nullptr)
{
}

JOSprite::~JOSprite()
{
	clearSource();
	m_loadCall = nullptr;
}

JOSprite* JOSprite::create()
{
	JOSprite* spr = new (std::nothrow) JOSprite();
	if (spr && spr->init()){
		spr->autorelease();		
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

JOSprite* JOSprite::create(const std::string& srcKey, bool isAsyn/*=true*/, LoadSpriteCall call/*=nullptr*/)
{
	JOSprite* spr = JOSprite::create();
	if (spr){
		spr->setCallback(call);
		spr->setKey(srcKey, isAsyn);
	}
	return spr;
}

JOSprite* JOSprite::create(const std::string& filePath, const std::string& imgName, bool isAsyn /*= true*/, short pixelFormat /*= (short)Texture2D::PixelFormat::RGBA8888*/, LoadSpriteCall call /*= nullptr*/)
{
	JOSprite* spr = JOSprite::create();
	if (spr){
		spr->setCallback(call);
		spr->setSource(filePath, imgName, isAsyn, pixelFormat);
	}
	return spr;
}


void JOSprite::setCallback(LoadSpriteCall call)
{
	m_loadCall = call;
}

void JOSprite::_loadStart()
{
	m_bAsynLoading = true;
    //LOG_DEBUG("JOSprite", "m_bAsynLoading = true %d", m_sn);
}

void JOSprite::_loadEnd()
{
	m_bAsynLoading = false;
    //LOG_DEBUG("JOSprite", "m_bAsynLoading = false _loadEnd %d", m_sn);
	if (!m_imgName.empty() && strcmp(m_imgName.c_str(),m_sourcePath.c_str())!=0) // != JO_EMTYP_STRING)
	{
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(m_imgName);
		if (frame){
			setSpriteFrame(frame);

			//回调处理
			if (m_loadCall)	{
				m_loadCall(this, m_sourcePath, m_imgName);
			}
            return;
		}
//        else{
//            LOG_WARN("JOSprite", "cannot get spriteFrame with name [%s]", m_imgName.c_str());
//        }
	}

	setTexture(m_sourcePath);

	//回调处理
	if (m_loadCall)	{
		m_loadCall(this, m_sourcePath, m_imgName);
	}
}

bool JOSprite::_isLoading()
{
	return m_bAsynLoading;
}

void JOSprite::_loadCancel()
{
	m_bAsynLoading = false;
    //LOG_DEBUG("JOSprite", "m_bAsynLoading = false _loadCancel %d", m_sn);
}

void JOSprite::_emptyTexture()
{
	setTexture(nullptr);
}


NS_JOFW_END
