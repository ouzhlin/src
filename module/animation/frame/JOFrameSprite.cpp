#include "module/animation/frame/JOFrameSprite.h"
#include "module/animation/frame/JOFrameAnimationCache.h"
#include "module/loader/JOAsynchMultLoader.h"
#include "module/loader/vo/JOResConfigVO.h"
#include "module/loader/JOResConfig.h"
#include "module/loader/JOResMgr.h"

#include "manager/JOSnMgr.h"

#include "utils/JOLog.h"

#define HEAD_OFFSET_Y_INTERVAL 32

NS_JOFW_BEGIN

JOFrameSprite::JOFrameSprite()
:m_curAct(nullptr)
, m_bRepeat(true)
, m_aniSpeed(1)
, m_aniName(JO_EMTYP_STRING)
, m_finishCall(nullptr)
, m_touchRect(Rect::ZERO)
, m_headTitleOffsetY(HEAD_OFFSET_Y_INTERVAL)

, m_loadCall(nullptr)
, m_aniKey(JO_EMTYP_STRING)
, m_tmpAniKey(JO_EMTYP_STRING)
{

}

JOFrameSprite::~JOFrameSprite()
{
    stopAllActions();
	CC_SAFE_RELEASE_NULL(m_curAct);
	m_finishCall = nullptr;
	JOFrameAnimationCache::Instance()->unQuoteRes(m_aniKey);
	m_aniKey.clear();
	clearSource();
}

JOFrameSprite* JOFrameSprite::create()
{
	JOFrameSprite* spr = new (std::nothrow) JOFrameSprite();
	if (spr && spr->init()){
		spr->setLoadType(JOAsynchBaseLoader::MULT);
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

JOFrameSprite* JOFrameSprite::create(const std::string& aniKey, bool isAsyn /*= true*/, LoadSpriteCall call /*= nullptr*/)
{
	JOFrameSprite* spr = JOFrameSprite::create();
	if (spr){
		spr->setLoadType(JOAsynchBaseLoader::MULT);
		spr->setCallback(call);
		spr->setAniKey(aniKey, isAsyn);
	}
	return spr;
}
//////////////////////////////////////////////////////////////////////////
void JOFrameSprite::setAniKey(const std::string& aniKey, bool isAsyn /*= true*/)
{
	if (m_aniKey == aniKey){
		return;
	}
	if (m_bAsynLoading && m_tmpAniKey == aniKey){
		return;
	}
	
    std::list<std::string> outList;
    if (!JOResConfig::Instance()->srcPlistWithAniKey(aniKey, outList)){
		LOG_WARN("JOFrameSprite", "can't find ani plist with key[%s] !!!!", aniKey.c_str());
		return;
	}
	std::unordered_map<std::string, short> tmpMap;
	std::list<std::string> tmpList;    
    const JOResSrcVO* srcVo=nullptr;
    std::list<std::string>::iterator itr = outList.begin();
    while(itr != outList.end())
    {
        srcVo = JOResConfig::Instance()->srcVOWithPlist((*itr));
        if(srcVo){
			tmpList.push_back(srcVo->getSrc());
			tmpMap[srcVo->getSrc()] = srcVo->pixelFormat;
        }
        else{
            LOG_WARN("JOFrameSprite", "key[%s] plist[%s] can't find src !!!!", aniKey.c_str(), (*itr).c_str());
            return;
        }
        ++itr;
    }

	m_tmpAniKey = aniKey;
	setSource(tmpList, isAsyn, &tmpMap);
}


void JOFrameSprite::setCallback(LoadSpriteCall call)
{
	m_loadCall = call;
}

void JOFrameSprite::_loadStart()
{
	m_bAsynLoading = true;
}

void JOFrameSprite::_loadEnd()
{
	m_bAsynLoading = false;
	JOFrameAnimationCache::Instance()->unQuoteRes(m_aniKey);
	m_aniKey = m_tmpAniKey;
	JOFrameAnimationCache::Instance()->addAnimationsWithKey(m_aniKey);
	JOFrameAnimationCache::Instance()->quoteRes(m_aniKey);

	std::string tmpAniName = m_aniName;
	m_aniName = JO_EMTYP_STRING;
	_play(tmpAniName, m_bRepeat);

	//回调处理
	if (m_loadCall)	{
		m_loadCall(this, m_aniKey, m_aniName);
	}
}

bool JOFrameSprite::_isLoading()
{
	return m_bAsynLoading;
}

void JOFrameSprite::_loadCancel()
{
	m_bAsynLoading = false;
}

void JOFrameSprite::_emptyTexture()
{
	stopAllActions();
	m_touchRect = Rect::ZERO;
	m_headTitleOffsetY = HEAD_OFFSET_Y_INTERVAL;
	setTexture(nullptr);
}
//////////////////////////////////////////////////////////////////////////

bool JOFrameSprite::play(const std::string& name, bool bRepeat/*=true*/, std::function<void(JOFrameSprite*)> finishCall/*=nullptr*/)
{
	m_finishCall = finishCall;
	return _play(name, bRepeat);
}

void JOFrameSprite::setSpeed(float speed)
{
	if (m_aniSpeed>speed-0.001f && m_aniSpeed<speed+0.001f){
		return;
	}
	m_aniSpeed = speed;

	stopAllActions();
	if (m_aniSpeed < 0.01f){		
		return;
	}
	if (m_curAct){
		if (m_aniSpeed > 0.99f && m_aniSpeed < 1.01f){
			runAction(m_curAct);
		}
		else{
			runAction(Speed::create(m_curAct, m_aniSpeed));
		}
	}
}



bool JOFrameSprite::_play(const std::string& aniName, bool bRepeat /*= true*/)
{
	if (aniName==m_aniName && m_bRepeat==bRepeat){
		return true;
	}
	m_aniName = aniName;
	m_bRepeat = bRepeat;
	if (m_bAsynLoading){
		return true;
	}
	if (m_aniName == JO_EMTYP_STRING){
		return true;
	}
	
	Animation* animation = JOFrameAnimationCache::Instance()->getAnimation(m_aniName);
	if (animation == nullptr){
		LOG_WARN("JOFrameSprite", "get animation with name[%s] fail !!!", m_aniName.c_str());
		return false;
	}

	stopAllActions();
	Animate* ani = Animate::create(animation);
	CC_SAFE_RELEASE_NULL(m_curAct);
	m_curAct = Sequence::create(ani, CallFunc::create(JO_CBACK_0(JOFrameSprite::_finish, this)), nullptr);
    
	if (m_bRepeat){
		m_curAct = RepeatForever::create(m_curAct);
	}
    m_curAct->retain();

	if (m_aniSpeed>0.99f && m_aniSpeed<1.01f){
		runAction(m_curAct);
	}
	else if (m_aniSpeed < 0.01f){
		stopAllActions();
	}
	else{
		runAction(Speed::create(m_curAct, m_aniSpeed));
	}
	
	Vector<AnimationFrame*> frames = animation->getFrames();
	if (frames.empty()){
		m_touchRect = Rect::ZERO;
		return false;
	}
	SpriteFrame* frame = frames.at(0)->getSpriteFrame();
	
	Point offset = frame->getOffsetInPixels(); // 图片中心点的偏移量
	Rect rc = frame->getRectInPixels(); // 切图信息
	/*
	Size aniSize = frame->getOriginalSizeInPixels(); // 原始图片大小
	Point seq = Point::ZERO;	
	
	seq.setPoint(aniSize.width * 0.5f, aniSize.height * 0.5f); // 先和原图正中心对齐
	seq.x += offset.x - rc.size.width * 0.5f; // 然后处理偏移量
	seq.y += offset.y - rc.size.height * 0.5f;
	*/
	m_touchRect.origin = offset;// seq - aniSize*0.5f;
	m_touchRect.size = rc.size;

	m_headTitleOffsetY = m_touchRect.size.height + m_touchRect.origin.y + HEAD_OFFSET_Y_INTERVAL;
	
	return true;
}

void JOFrameSprite::_finish()
{
	if (m_finishCall){
		m_finishCall(this);
	}
}


Rect JOFrameSprite::getTouchRect()
{
	return m_touchRect;
}

float JOFrameSprite::getHeadTitleOffsetY()
{
	return m_headTitleOffsetY;
}



NS_JOFW_END