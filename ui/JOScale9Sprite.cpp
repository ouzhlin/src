#include "ui/JOScale9Sprite.h"

#include "utils/JOLog.h"
#include "ui/JOSprite.h"

NS_JOFW_BEGIN


JOScale9Sprite::JOScale9Sprite()
: m_loadCall(nullptr)
, m_srcSprite(nullptr)
{
	m_srcSprite = JOSprite::create();
	m_srcSprite->retain();
}

JOScale9Sprite::~JOScale9Sprite()
{	
	clearSource();
	if (m_srcSprite){
		m_srcSprite->release();
		m_srcSprite = nullptr;
	}
	m_loadCall = nullptr;
}

JOScale9Sprite* JOScale9Sprite::create()
{
	JOScale9Sprite* spr = new (std::nothrow) JOScale9Sprite();
	if (spr && spr->init()){
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

JOScale9Sprite* JOScale9Sprite::create(const std::string& srcKey, bool isAsyn/*=true*/, LoadSpriteCall call/*=nullptr*/)
{
	JOScale9Sprite* spr = JOScale9Sprite::create();
	if (spr){
		spr->setCallback(call);
		spr->setKey(srcKey, isAsyn);
	}
	return spr;
}

JOScale9Sprite* JOScale9Sprite::create(const std::string& filePath, const std::string& imgName, bool isAsyn /*= true*/, short pixelFormat /*= (short)Texture2D::PixelFormat::RGBA8888*/, LoadSpriteCall call /*= nullptr*/)
{
	JOScale9Sprite* spr = JOScale9Sprite::create();
	if (spr){
		spr->setCallback(call);
		spr->setSource(filePath, imgName, isAsyn, pixelFormat);
	}
	return spr;
}

void JOScale9Sprite::setCallback(LoadSpriteCall call)
{
	m_loadCall = call;
}

void JOScale9Sprite::_resetInset(Rect& org)
{
    
//	float w = org.size.width*0.33f;
//	float h = org.size.height*0.33f;
//	setInsetLeft(w);
//	setInsetRight(w);
//	setInsetTop(h);
//	setInsetBottom(h);
    setContentSize(org.size);
//    org.origin.x = w;
//    org.origin.y = h;
//    org.size.width = w;
//    org.size.height = h;
//    
//    setCapInsets(org);
    
}

void JOScale9Sprite::_loadStart()
{
	m_bAsynLoading = true;
    //LOG_DEBUG("JOScale9Sprite", "m_bAsynLoading = true %d", m_sn);
}

void JOScale9Sprite::_loadEnd()
{
    //LOG_DEBUG("JOScale9Sprite", "m_bAsynLoading = false _loadEnd %d", m_sn);
	m_bAsynLoading = false;

	Rect tmpRect = Rect::ZERO;
	Size orgSize = getContentSize();
	bool bFrame = false;

    if (!m_imgName.empty() && strcmp(m_imgName.c_str(),m_sourcePath.c_str())!=0)
	//if (!m_imgName.empty())//  != JO_EMTYP_STRING)
	{
		SpriteFrame* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(m_imgName);
		if (frame){
			setSpriteFrame(frame);
			tmpRect = frame->getRect();
			bFrame = true;
        }
//        else{
//            LOG_WARN("JOScale9Sprite", "cannot get spriteFrame with name [%s]", m_imgName.c_str());
//        }
	}
	if (bFrame == false){
		m_srcSprite->setSource(m_sourcePath, JO_EMTYP_STRING, false);
		tmpRect.size = m_srcSprite->getTexture()->getContentSize();
		setSpriteFrame(m_srcSprite->getSpriteFrame());
	}
	
	if (!orgSize.equals(Size::ZERO)){
		tmpRect.size = orgSize;
	}

	_resetInset(tmpRect);

	//回调处理
	if (m_loadCall)	{
		m_loadCall(this, m_sourcePath, m_imgName);
	}	
}

bool JOScale9Sprite::_isLoading()
{
	return m_bAsynLoading;
}

void JOScale9Sprite::_loadCancel()
{
	m_bAsynLoading = false;
    //LOG_DEBUG("JOScale9Sprite", "m_bAsynLoading = false _loadCancel %d", m_sn);
}

void JOScale9Sprite::_emptyTexture()
{
	m_srcSprite->clearSource();

	updateWithSprite(nullptr, Rect::ZERO, false, Rect::ZERO);
}

NS_JOFW_END
