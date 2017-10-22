#include "module/animation/frame/JOFrameModel.h"
#include "module/animation/frame/JOFrameSprite.h"
#include "ui/JOUILayout.h"

NS_JOFW_BEGIN

JOFrameModel::JOFrameModel()
: m_bRepeat(true)
, m_aniSpeed(1.0f)
, m_rootAniName(JO_EMTYP_STRING)
, m_aniSpriteCount(0)
, m_tmpCallVal(nullptr)
, m_finishCall(nullptr)
, m_touchCall(nullptr)
, m_touchRectDirty(false)
{
	
}

JOFrameModel::~JOFrameModel()
{
    _clearAnimations();
	m_aniSpriteCount = 0;
}

JOFrameModel* JOFrameModel::create()
{
	JOFrameModel* spr = new (std::nothrow) JOFrameModel();
	if (spr && spr->init()){	
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}


bool JOFrameModel::init()
{
	bool ret = Node::init();
	if (ret){
		setAnchorPoint(Point(0.5f, 0.5f));
		setContentSize(Size(1, 1));
		setCatchTouch(true);
		m_tmpCallVal = JO_CBACK_1(JOFrameModel::_onPlayFinish, this);
	}
	return ret;
}


bool JOFrameModel::playRoot(const std::string& aniName, bool bRepeat /*= true*/, std::function<void(JOFrameModel*)> finishCall /*= nullptr*/)
{
	m_finishCall = finishCall;
	if (m_aniSpriteCount>0){
		m_aniSprites[0]->play(aniName, bRepeat, m_tmpCallVal);
		m_aniSprites[0]->setVisible(true);
		m_rootAniName = aniName;
		m_touchRectDirty = true;
		return true;
	}
	return false;
}

bool JOFrameModel::playPart(const std::string& aniName, unsigned int idx, bool bRepeat /*= true*/)
{
	if (m_aniSpriteCount>idx){
		m_aniSprites[idx]->play(aniName, bRepeat);
		m_aniSprites[idx]->setVisible(true);
		return true;
	}
	return false;
}

void JOFrameModel::stop()
{
	for (unsigned int i = 0; i < m_aniSpriteCount; i++){
		m_aniSprites[i]->stopAllActions();
	}
}

void JOFrameModel::setSpeed(float speed)
{
	for (unsigned int i = 0; i < m_aniSpriteCount; i++){
		m_aniSprites[i]->setSpeed(speed);
	}
}

void JOFrameModel::initAnimations(size_t count)
{
	_clearAnimations();
	m_aniSprites.resize(count);
	JOFrameSprite* fs = nullptr;
	for (unsigned int i = 0; i < count;i++){
		fs = JOFrameSprite::create();
		m_aniSprites[i] = fs;
		addChild(fs);		
		JOUILayout::Instance()->relativePos(fs, this, JOUILayout::CENTER);
		fs->setVisible(false);
	}
	m_aniSpriteCount = count;
}

bool JOFrameModel::setAnimation(unsigned int idx, const std::string& aniKey)
{
	if (m_aniSpriteCount>idx){
		m_aniSprites[idx]->setAniKey(aniKey);
		return true;
	}
	return false;
}

bool JOFrameModel::setAnimationColor(unsigned int idx, Color3B color)
{
	if (m_aniSpriteCount > idx){
		m_aniSprites[idx]->setColor(color);
		return true;
	}
	return false;
}

void JOFrameModel::setAnimationZorder(unsigned int idx, int zorder)
{
	if (m_aniSpriteCount > idx){
		m_aniSprites[idx]->setZOrder(zorder);
	}
}

bool JOFrameModel::isAnimValidate(unsigned int idx)
{
	if (m_aniSpriteCount <= idx){
		return false;
	}
	return true;
}

void JOFrameModel::_clearAnimations()
{
	std::vector<JOFrameSprite*>::iterator itr = m_aniSprites.begin();
	while (itr != m_aniSprites.end()){
		(*itr)->removeFromParent();
		++itr;
	}
	m_aniSprites.clear();
	m_aniSpriteCount = 0;
}

bool JOFrameModel::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	bool ret = Node::onTouchBegan(pTouch, pEvent);
	/*
	if (m_touchRectDirty){
		
	}
	return false;
	*/
	return _inTouchRect(pTouch);
	
}

void JOFrameModel::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	Node::onTouchMoved(pTouch, pEvent);
}

void JOFrameModel::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	Node::onTouchEnded(pTouch, pEvent);
	if (_inTouchRect(pTouch)){
		if (m_touchCall){
			m_touchCall(this);
		}
	}
}

void JOFrameModel::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	Node::onTouchCancelled(pTouch, pEvent);
}

void JOFrameModel::_onPlayFinish(JOFrameSprite* fsprite)
{
	if (m_finishCall){
		m_finishCall(this);
	}
}

bool JOFrameModel::_inTouchRect(Touch* pTouch)
{
	if (m_aniSpriteCount > 0)	{
		Rect rc = m_aniSprites[0]->getTouchRect();
		//rc.origin = rc.origin*getScale();
		rc.size = rc.size*getScale();
		rc.origin = Point(-rc.size.width*0.5f, 0);
		//Point p1 = m_aniSprites[0]->convertToWorldSpace(rc.origin);
		//rc.origin = convertToNodeSpace(p1);
		return rc.containsPoint(convertToNodeSpace(pTouch->getLocation()));
	}
	return false;
}

JOFrameSprite* JOFrameModel::getRoot()
{
	if (m_aniSpriteCount>0){
		return m_aniSprites[0];
	}
	return nullptr;
}

void JOFrameModel::setTouchCall(std::function<void(JOFrameModel*)> touchCall)
{
	m_touchCall = touchCall;
}

NS_JOFW_END