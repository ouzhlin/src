#include "module/animation/frame/JOFrameSprite.h"
#include "module/animation/frame/JOFrameAnimationCache.h"
#include "module/loader/vo/JOResConfigVO.h"
#include "module/loader/JOResConfig.h"
#include "module/loader/JOResMgr.h"

#include "manager/JOTickMgr.h"
#include "manager/JOSnMgr.h"

#include "utils/JOLog.h"

#define HEAD_OFFSET_Y_INTERVAL 32

NS_JOFW_BEGIN


JOFrameSprite::JOFrameSprite()
: m_bRepeat(true)
, m_aniSpeed(1)
, m_aniName(JO_EMTYP_STRING)
, m_touchRect(Rect::ZERO)
, m_headTitleOffsetY(HEAD_OFFSET_Y_INTERVAL)

, m_loadCall(nullptr)
, m_aniKey(JO_EMTYP_STRING)
, m_tmpAniKey(JO_EMTYP_STRING)

, m_isPlaying(false)
, m_curFrameIdx(0)
, m_curAnimation(nullptr)
, m_elapseTime(0)
{

}

JOFrameSprite::~JOFrameSprite()
{
    stop();
	JOFrameAnimationCache::Instance()->unQuoteRes(m_aniKey);
	m_aniKey.clear();
	clearSource();
}

JOFrameSprite* JOFrameSprite::create()
{
	JOFrameSprite* spr = new (std::nothrow) JOFrameSprite();
	if (spr && spr->init()){
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

	if (JOFrameAnimationCache::Instance()->haveAnimationsWithAniKey(aniKey)){
		m_tmpAniKey = aniKey;
		_clearTmp();
		_loadEnd();
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


bool JOFrameSprite::isSetupAniKey(const std::string& aniKey)
{
	if (m_aniKey == aniKey){
		return true;
	}
	return false;
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

	/*
	std::string tmpAniName = m_aniName;
	m_aniName = JO_EMTYP_STRING;
	_play(tmpAniName, m_bRepeat);
	*/
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
	m_touchRect = Rect::ZERO;
	m_headTitleOffsetY = HEAD_OFFSET_Y_INTERVAL;
	setTexture(nullptr);
}
//////////////////////////////////////////////////////////////////////////


bool JOFrameSprite::isCanPlay(const std::string& aniName)
{
	if (JOFrameAnimationCache::Instance()->getAnimation(aniName))
		return true;
	return false;
}

bool JOFrameSprite::play(const std::string& aniName, bool bRepeat /*= true*/)
{
	if (m_bAsynLoading || aniName.empty()){
		m_isPlaying = false;
		return false;
	}

	m_bRepeat = bRepeat;
	if (aniName==m_aniName){
		m_isPlaying = true;
		return true;
	}
	m_aniName = aniName;
		
	m_curAnimation = JOFrameAnimationCache::Instance()->getAnimation(m_aniName);
	if (m_curAnimation == nullptr){
		LOG_WARN("JOFrameSprite", "get animation with name[%s] fail !!!", m_aniName.c_str());
		m_isPlaying = false;
		return false;
	}

	const Vector<AnimationFrame*>& frames = m_curAnimation->getFrames();
	if (frames.empty()){
		m_touchRect = Rect::ZERO;
		return false;
	}
	m_splitTimes.clear();
	m_splitTimes.reserve(frames.size());

	float singleDuration = m_curAnimation->getDuration();
	float accumUnitsOfTime = 0;
	float newUnitOfTimeValue = singleDuration / m_curAnimation->getTotalDelayUnits();

	for (AnimationFrame* frame : frames)
	{		
		//m_splitTimes.push_back((accumUnitsOfTime * newUnitOfTimeValue) / singleDuration);
		m_splitTimes.push_back((accumUnitsOfTime * newUnitOfTimeValue));
		accumUnitsOfTime += frame->getDelayUnits();
	}	
	m_curFrameIdx = 0;
	m_elapseTime = 0;

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

	m_isPlaying = true;
	return true;
}

void JOFrameSprite::restart()
{
	m_isPlaying = true;
	m_curFrameIdx = 0;
	m_elapseTime = 0;
}

void JOFrameSprite::_tick()
{
	if (m_isPlaying == false || !m_curAnimation || (m_aniSpeed<0.001f && m_aniSpeed>-0.001f) ) return;
	
	if (m_curFrameIdx>0){
		m_elapseTime += JOTickMgr::Instance()->deltaTime()*m_aniSpeed;
	}

	const Vector<AnimationFrame*>& frames = m_curAnimation->getFrames();	
	short numberOfFrames = frames.size();

	for (int i = m_curFrameIdx; i < numberOfFrames; i++) {
		if (m_splitTimes.at(i) <= m_elapseTime) {
			setSpriteFrame(frames.at(i)->getSpriteFrame());
			//关键帧回调
			std::unordered_map<std::string, std::unordered_map<short, FrameEventCall > >::iterator itr = m_callMap.find(m_aniName);
			if (itr != m_callMap.end()) {
				std::unordered_map<short, FrameEventCall >::iterator idxItr = itr->second.find(m_curFrameIdx);
				if (idxItr!=itr->second.end()){
					idxItr->second(this, m_aniName, m_curFrameIdx);
				}
				//默认的结束回调
				else if (m_curFrameIdx + 1 == numberOfFrames){
					idxItr = itr->second.find(-1);
					if (idxItr != itr->second.end()){
						idxItr->second(this, m_aniName, m_curFrameIdx);
					}
				}
			}
			
			m_curFrameIdx = i + 1;
			if (m_curFrameIdx == numberOfFrames){
				m_curFrameIdx = 0;
				m_elapseTime = 0;
				if (!m_bRepeat){
					stop();
				}
			}
			break;
		}
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


void JOFrameSprite::setVisible(bool visible)
{
	Sprite::setVisible(visible);
	if (visible){
		JOTickMgr::Instance()->registerTick(m_sn, JO_CBACK_0(JOFrameSprite::_tick, this));
	}
	else{
		JOTickMgr::Instance()->unRegisterTick(m_sn);
	}
}


void JOFrameSprite::onEnter()
{
	Sprite::onEnter();
	if (this->isVisible()){
		JOTickMgr::Instance()->registerTick(m_sn, JO_CBACK_0(JOFrameSprite::_tick, this));
	}
}

void JOFrameSprite::onExit()
{
	JOTickMgr::Instance()->unRegisterTick(m_sn);
	Sprite::onExit();
}

void JOFrameSprite::setEventCall(const std::string& aniName, FrameEventCall call, short frameIdx /*= -1*/)
{
	std::unordered_map<std::string, std::unordered_map<short, FrameEventCall > >::iterator itr = m_callMap.find(aniName);
	if (itr!=m_callMap.end()){
		itr->second[frameIdx] = call;
	}
	else{
		std::unordered_map<short, FrameEventCall > tmpMap;
		tmpMap[frameIdx] = call;
		m_callMap[aniName] = tmpMap;
	}
}

void JOFrameSprite::removeCall(const std::string& aniName)
{
	m_callMap.erase(aniName);
}

void JOFrameSprite::removeCall(const std::string& aniName, short frameIdx)
{
	std::unordered_map<std::string, std::unordered_map<short, FrameEventCall > >::iterator itr = m_callMap.find(aniName);
	if (itr != m_callMap.end()){
		itr->second.erase(frameIdx);
		if (itr->second.empty()){
			m_callMap.erase(aniName);
		}
	}
}

void JOFrameSprite::removeAllCall()
{
	m_callMap.clear();
}

NS_JOFW_END




