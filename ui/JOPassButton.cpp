#include "ui/JOPassButton.h"
#include "ui/JOSprite.h"
#include "ui/JOUIDef.h"
#include "ui/JOUILayout.h"
#include "manager/JOTickMgr.h"
#include "manager/JOSnMgr.h"

NS_JOFW_BEGIN


JOPassButton::JOPassButton()
: m_passTime(1)
, m_curTime(0)
, m_clip(nullptr)
, m_stencil(nullptr)
, m_passImg(nullptr)
, m_direct(JOPassButton::PASS_RIGHT)
, m_bPassing(false)
{
	m_sn = JOSnMgr::Instance()->getSn();
}

JOPassButton::~JOPassButton()
{
	//CC_SAFE_RELEASE(m_clip);
	//CC_SAFE_RELEASE(m_stencil);
	JOTickMgr::Instance()->unRegisterTick(m_sn);
	JOSnMgr::Instance()->dispose(m_sn);
}

JOPassButton* JOPassButton::create(const std::string& key, const std::string& passKey, BUTTON_CALL call, const std::string& title /*= nullptr*/)
{
	JOPassButton* spr = new (std::nothrow) JOPassButton();
	if (spr && spr->init(key, call, title)){
		spr->setPassImgKey(passKey);
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

void JOPassButton::setDirect(short direct)
{
	m_direct = direct;
	if (m_bPassing){
		_passHandle();
	}
}

void JOPassButton::setPassImgKey(const std::string& key)
{
	if (m_clip==nullptr){
		m_clip = ClippingNode::create();
		addChild(m_clip,2);
		m_clip->setAlphaThreshold(0.5f);
		m_clip->setInverted(false);
	}
	if (m_stencil==nullptr){
		m_stencil = JOSprite::create(key, false);
	}
	else{
		m_stencil->setKey(key,false);
	}
	m_clip->setContentSize(getContentSize());
	m_clip->setStencil(m_stencil);

	if (m_passImg==nullptr){
		m_passImg = JOSprite::create(key, false);
		m_clip->addChild(m_passImg);
	}
	else{
		m_passImg->setKey(key);
	}

	m_clip->setPosition(Point::ZERO);
	JOUILayout::Instance()->relativePos(m_stencil, this);	
	JOUILayout::Instance()->relativePos(m_passImg, m_clip);

	m_clip->setVisible(false);
}

void JOPassButton::startPassing()
{
	setCatchTouch(false);
	m_bPassing = true;
	JOTickMgr::Instance()->unRegisterTick(m_sn);
	m_clip->setVisible(true);

	if (m_curTime<=0){
		m_curTime = m_passTime;
	}
	JOTickMgr::Instance()->registerTick(m_sn, JO_CBACK_0(JOPassButton::_passTick, this));
}

void JOPassButton::stopPassing()
{
	m_bPassing = false;
	JOTickMgr::Instance()->unRegisterTick(m_sn);
	m_clip->setVisible(false);
	m_curTime = 0;
	JOUILayout::Instance()->relativePos(m_stencil, this);
	setCatchTouch(true);
}

void JOPassButton::_passHandle()
{
	float x = 0;
	float y = 0;
	Size rootsize = getContentSize();	
	x += rootsize.width*0.5f;
	y += rootsize.height*0.5f;
	//m_passImg->setPosition(x, y);

	Size cs = m_stencil->getContentSize();
	switch (m_direct)
	{
	case JOPassButton::PASS_LEFT:
		x -= cs.width / m_passTime*(m_passTime-m_curTime);
		break;
	case JOPassButton::PASS_RIGHT:
		x += cs.width / m_passTime*(m_passTime - m_curTime);
		break;
	case JOPassButton::PASS_TOP:
		y += cs.height / m_passTime*(m_passTime - m_curTime);
		break;
	case JOPassButton::PASS_BOTTOM:
		y -= cs.height / m_passTime*(m_passTime - m_curTime);
		break;
	default:
		break;
	}
	m_stencil->setPosition(x, y);

	/*
	m_stencil->setPosition(Point(-w*0.5 + passX, y));//x
	m_stencil->setPosition(Point(w*1.5 - passX, y));
	*/
}

void JOPassButton::_passTick()
{
	m_curTime -= JOTickMgr::Instance()->deltaTime();
	if (m_curTime < 0){
		stopPassing();
		return;
	}
	_passHandle();
}

void JOPassButton::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	if (isTouchInside(pTouch)){
		startPassing();
	}
	JOButton::onTouchEnded(pTouch, pEvent);
}

void JOPassButton::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	if (isTouchInside(pTouch)){
		startPassing();
	}
	JOButton::onTouchEnded(pTouch, pEvent);
}

void JOPassButton::onContentSizeChange()
{
	if (m_clip){
		m_clip->setContentSize(getContentSize());
		m_clip->setPosition(Point::ZERO);

		if (m_passImg){
			JOUILayout::Instance()->relativePos(m_passImg, m_clip);
		}
	}
}



NS_JOFW_END