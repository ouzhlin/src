#include "module/animation/frame/JOFrameModel.h"
#include "module/animation/frame/JOFrameSprite.h"
#include "ui/JOUILayout.h"

NS_JOFW_BEGIN

JOFrameModel::JOFrameModel()
: m_bRepeat(true)
, m_aniSpeed(1.0f)
, m_aniSpriteCount(0)

, m_touchCall(nullptr)
, m_isPlaying(true)
, m_aniDir(JOFrameModel::CLOCK_30)
, m_aniLastDir(0)
, m_bPicFlip(false)
, m_mainPartIdx(0)
, m_aniName("")
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
	}
	return ret;
}


bool JOFrameModel::playRoot(const std::string& aniName, bool bRepeat /*= true*/)
{
	if (m_aniSpriteCount>0){
		m_aniSprites[0]->play(aniName, bRepeat);
		m_aniSprites[0]->setVisible(true);
		return true;
	}
	return false;
}

bool JOFrameModel::playPart(const std::string& aniName, unsigned int idx, bool bRepeat /*= true*/)
{
	if (m_aniSpriteCount>idx){
		
		m_aniSprites[idx]->setVisible(true);
		m_aniSprites[idx]->play(aniName, bRepeat);
		return true;
	}
	return false;
}

void JOFrameModel::stop()
{
	m_isPlaying = false;
	for (unsigned int i = 0; i < m_aniSpriteCount; i++){
		m_aniSprites[i]->stop();
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
		fs->setCallback(JO_CBACK_3(JOFrameModel::_frameSpriteLoadCall, this));
		m_aniSprites[i] = fs;
		addChild(fs);		
		//JOUILayout::Instance()->relativePos(fs, this, JOUILayout::CENTER);
		//fs->setVisible(false);
	}
	setFrameAnchor(Point(0.5f, 0));
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
		m_aniSprites[idx]->setLocalZOrder(zorder);
	}
}

bool JOFrameModel::isAnimValidate(unsigned int idx)
{
	if (m_aniSpriteCount <= idx){
		return false;
	}
	return true;
}

void JOFrameModel::setAnimationVisible(unsigned int idx, bool visible)
{
	if (m_aniSpriteCount > idx){
		m_aniSprites[idx]->setVisible(visible);
	}
}

void JOFrameModel::setFrameAnchor(Point anchor)
{
	std::vector<JOFrameSprite*>::iterator itr = m_aniSprites.begin();
	while (itr != m_aniSprites.end()){
		(*itr)->setAnchorPoint(anchor);
		++itr;
	}
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

void JOFrameModel::setAniData(std::string& aniName, unsigned short dir, unsigned short partIdx, std::string& aniKey)
{	
	ANI_NAME_MAP::iterator itr = m_aniDataMap.find(aniName);
	if (itr != m_aniDataMap.end()){
		DIR_ANI_MAP::iterator dirItr = itr->second.find(dir);
		if (dirItr != itr->second.end()){
			dirItr->second[partIdx] = aniKey;
		}
		else{
			PART_ANI_MAP tmpMap;
			tmpMap[partIdx] = aniKey;
			itr->second[dir] = tmpMap;
		}
	}
	else{
		PART_ANI_MAP tmpKeyMap;
		tmpKeyMap[partIdx] = aniKey;

		DIR_ANI_MAP tmpMap;
		tmpMap[dir] = tmpKeyMap;
		m_aniDataMap[aniName] = tmpMap;
	}
}


void JOFrameModel::setMainPart(unsigned short partIdx)
{
	if (m_mainPartIdx == partIdx) return;
	if (m_aniSpriteCount <= partIdx) return;

	
	if (m_aniSpriteCount > m_mainPartIdx){
		m_aniSprites[m_mainPartIdx]->removeAllCall();
	}
	m_mainPartIdx = partIdx;
	JOFrameSprite* spr = m_aniSprites[m_mainPartIdx];
	unordered_map<std::string, unordered_map<short, std::function<void(JOFrameModel*)> > >::iterator itr = m_eventCallMap.begin();
	while (itr!=m_eventCallMap.end())
	{
		unordered_map<short, std::function<void(JOFrameModel*)> >::iterator frameItr = itr->second.begin();
		while ( frameItr!=itr->second.end() )
		{
			spr->setEventCall(itr->first, JO_CBACK_3(JOFrameModel::_frameEventCall, this), frameItr->first);
			frameItr++;
		}
		itr++;
	}
}

void JOFrameModel::setAniCall(std::string& aniName, std::function<void(JOFrameModel*)> call, short frameIdx/* = -1*/)
{
	if (m_aniSpriteCount>m_mainPartIdx)
	{
		m_aniSprites[m_mainPartIdx]->setEventCall(aniName, JO_CBACK_3(JOFrameModel::_frameEventCall, this), frameIdx);
	}
	unordered_map<std::string, unordered_map<short, std::function<void(JOFrameModel*)> > >::iterator itr = m_eventCallMap.find(aniName);
	if (itr != m_eventCallMap.end()){
		itr->second[frameIdx] = call;
	}
	else{
		unordered_map<short, std::function<void(JOFrameModel*)> > tmpMap;
		tmpMap[frameIdx] = call;
		m_eventCallMap[aniName] = tmpMap;
	}
}

void JOFrameModel::removeAniCall(std::string& aniName, short frameIdx)
{
	if (m_aniSpriteCount > m_mainPartIdx){
		m_aniSprites[m_mainPartIdx]->removeCall(aniName, frameIdx);
	}
	std::unordered_map<std::string, std::unordered_map<short, std::function<void(JOFrameModel*)> > >::iterator itr = m_eventCallMap.find(aniName);
	if (itr != m_eventCallMap.end()){
		itr->second.erase(frameIdx);
		if (itr->second.empty()){
			m_eventCallMap.erase(aniName);
		}
	}
}

void JOFrameModel::removeAniCall(std::string& aniName)
{
	if (m_aniSpriteCount > m_mainPartIdx){
		m_aniSprites[m_mainPartIdx]->removeCall(aniName);
	}
	m_eventCallMap.erase(aniName);
}

void JOFrameModel::removeAllCall()
{
	if (m_aniSpriteCount > m_mainPartIdx){
		m_aniSprites[m_mainPartIdx]->removeAllCall();
	}
	m_eventCallMap.clear();
}

void JOFrameModel::playAni(std::string& aniName, bool bRepeat /*= true*/)
{
	m_isPlaying = true;
	if (bRepeat == m_bRepeat && aniName == m_aniName && m_aniLastDir==m_aniDir) return;

	m_aniLastDir = m_aniDir;
	ANI_NAME_MAP::iterator itr = m_aniDataMap.find(aniName);
	if (itr != m_aniDataMap.end()){
		DIR_ANI_MAP::iterator dirItr = itr->second.find(m_aniDir);
		if (dirItr != itr->second.end()){
			m_needLoadCount = dirItr->second.size();
			m_loadingIdx = 0;
			m_aniName = aniName;
			m_bRepeat = bRepeat;
			PART_ANI_MAP::iterator partItr = dirItr->second.begin();
			while (partItr != dirItr->second.end()){
				JOFrameSprite* spr = m_aniSprites[partItr->first];
				if (spr->isSetupAniKey(partItr->second)){
					_frameSpriteLoadCall(spr, partItr->second, aniName);
				}
				else{
					spr->setAniKey(partItr->second);
				}
				partItr++;
			}
		}
	}
}


void JOFrameModel::setDir(unsigned short clockType)
{
	if (m_isPlaying == false) return;
	
	if (clockType<JOFrameModel::CLOCK_00 || clockType>JOFrameModel::CLOCK_105){
		LOG_WARN("JOFrameModel", "set dir fail error dir[%d] ", clockType);
		return;
	}
	if (clockType == m_aniDir) return;
	
	m_aniDir = clockType;
	bool tmpFlip;
	switch (m_aniDir)
	{
	case JOFrameModel::CLOCK_00:
	case JOFrameModel::CLOCK_15:
	case JOFrameModel::CLOCK_30:
	case JOFrameModel::CLOCK_45:
	case JOFrameModel::CLOCK_60:
		tmpFlip = false;
		break;
	case JOFrameModel::CLOCK_75:
	case JOFrameModel::CLOCK_90:
	case JOFrameModel::CLOCK_105:
		tmpFlip = true;
		break;
	default:
		break;
	}
	if (tmpFlip != m_bPicFlip){
		m_bPicFlip = tmpFlip;
		std::vector<JOFrameSprite*>::iterator itr = m_aniSprites.begin();
		while (itr != m_aniSprites.end()){
			(*itr)->setFlippedX(m_bPicFlip);
			++itr;
		}
	}
	
	std::string tmp = m_aniName;
	m_aniName = "";
	this->playAni(tmp, m_bRepeat);
}

bool JOFrameModel::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	//bool ret = Node::onTouchBegan(pTouch, pEvent);

	Node* tmpParent = getParent();
	if (!isVisible() || tmpParent == nullptr){
		return false;
	}
	while (tmpParent){
		if (!tmpParent->isVisible()){
			return false;
		}
		tmpParent = tmpParent->getParent();
	}
	return _inTouchRect(pTouch);
	
}

void JOFrameModel::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	//Node::onTouchEnded(pTouch, pEvent);
	if (_inTouchRect(pTouch)){
		if (m_touchCall){
			m_touchCall(this);
		}
	}
}


bool JOFrameModel::_inTouchRect(Touch* pTouch)
{
	if (m_aniSpriteCount > m_mainPartIdx)	{
		Rect rc = m_aniSprites[m_mainPartIdx]->getTouchRect();
		//rc.origin = rc.origin*getScale();
		rc.size = rc.size*getScale();
		rc.origin = Point(-rc.size.width*0.5f, 0);//因为frameModel其实大小为0

		return rc.containsPoint(convertToNodeSpace(pTouch->getLocation()));
	}
	return false;
}

void JOFrameModel::_frameSpriteLoadCall(cocos2d::Node* sender, std::string& aniKey, std::string& aniName)
{
	if (m_isPlaying == false) return;

	m_loadingIdx++;
	if (m_needLoadCount>0 && m_needLoadCount == m_loadingIdx)
	{
		unsigned short partIdx = 0;
		std::vector<JOFrameSprite*>::iterator itr = m_aniSprites.begin();
		while (itr != m_aniSprites.end()){
			(*itr)->play(getRealAniName(m_aniName,m_aniDir,partIdx), m_bRepeat);
			++partIdx;
			++itr;
		}
		m_needLoadCount = 0;
		m_loadingIdx = 0;
	}
}

void JOFrameModel::_frameEventCall(JOFrameSprite* frameSpr, std::string& aniName, short frameIdx)
{
	unordered_map<std::string, unordered_map<short, std::function<void(JOFrameModel*)> > >::iterator itr = m_eventCallMap.find(aniName);
	if (itr != m_eventCallMap.end()){
		unordered_map<short, std::function<void(JOFrameModel*)> >::iterator frameItr = itr->second.find(frameIdx);
		if (frameItr!=itr->second.end()){
			frameItr->second(this);
		}
	}
}

void JOFrameModel::setAniNamePair(std::string& aniName, unsigned short dir, unsigned short partIdx, std::string& realAniName)
{
	ANI_NAME_MAP::iterator itr = m_aniNamePair.find(aniName);
	if (itr != m_aniNamePair.end()){
		DIR_ANI_MAP::iterator dirItr = itr->second.find(dir);
		if (dirItr != itr->second.end()){
			dirItr->second[partIdx] = realAniName;
		}
		else{
			PART_ANI_MAP tmpMap;
			tmpMap[partIdx] = realAniName;
			itr->second[dir] = tmpMap;
		}
	}
	else{
		PART_ANI_MAP tmpKeyMap;
		tmpKeyMap[partIdx] = realAniName;

		DIR_ANI_MAP tmpMap;
		tmpMap[dir] = tmpKeyMap;
		m_aniNamePair[aniName] = tmpMap;
	}
}

void JOFrameModel::clearAniNamePair()
{
	m_aniNamePair.clear();
}

std::string JOFrameModel::getRealAniName(std::string& aniName, unsigned short dir, unsigned short partIdx)
{
	ANI_NAME_MAP::iterator itr = m_aniNamePair.find(aniName);
	if (itr != m_aniNamePair.end()){
		DIR_ANI_MAP::iterator dirItr = itr->second.find(dir);
		if (dirItr != itr->second.end()){
			PART_ANI_MAP::iterator partItr = dirItr->second.find(partIdx);
			if (partItr!=dirItr->second.end()){
				return partItr->second;
			}
		}
	}
	return aniName;
}

JOFrameSprite* JOFrameModel::getRoot()
{
	if (m_aniSpriteCount>m_mainPartIdx){
		return m_aniSprites[m_mainPartIdx];
	}
	return nullptr;
}


NS_JOFW_END