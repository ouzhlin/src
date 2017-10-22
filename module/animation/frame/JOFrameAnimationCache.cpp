#include "module/animation/frame/JOFrameAnimationCache.h"
#include "module/animation/frame/vo/JOFrameAnimationVO.h"
#include "module/loader/JOResConfig.h"
#include "manager/JOCachePoolMgr.h"
#include "manager/JOTickMgr.h"

#include "core/datautils/JOData.h"

#include "utils/JOTime.h"
#include "utils/JOLog.h"


NS_JOFW_BEGIN

JOFrameAnimationCache::JOFrameAnimationCache()
: m_checkInterval(120)
{

}

JOFrameAnimationCache::~JOFrameAnimationCache()
{

}

void JOFrameAnimationCache::quoteRes(std::string& aniKey)
{
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.find(aniKey);
    if (itr != m_aniMap.end()){
        itr->second->increase();
    }
}
void JOFrameAnimationCache::unQuoteRes(std::string& aniKey)
{
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.find(aniKey);
    if (itr != m_aniMap.end()){
        itr->second->decrease();
    }
}
void JOFrameAnimationCache::addAnimationsWithKey(const std::string& aniKey)
{
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.find(aniKey);
    if (itr==m_aniMap.end())
    {
        
		JOData* aniData = JOResConfig::Instance()->getAniDataWithAniKey(aniKey);
		if (aniData == nullptr) {
			LOG_ERROR("JOFrameAnimationCache", "aniKey[%s] file data could not be found!!!!", aniKey.c_str());
			return;
		}
		ValueMap dict = FileUtils::getInstance()->getValueMapFromData((const char*)aniData->bytes(), aniData->length());

        //ValueMap dict =  FileUtils::getInstance()->getValueMapFromFile(JOResConfig::Instance()->getAniPath()+aniKey);
        
        CCASSERT( !dict.empty(), "CCAnimationCache: File could not be found");
        
        if ( dict.find("animations") == dict.end() )
        {
            CCLOG("cocos2d: AnimationCache: No animations were found in provided dictionary.");
            return;
        }
        
        const Value& animations = dict.at("animations");
        unsigned int version = 1;
        
        if( dict.find("properties") != dict.end() )
        {
            const ValueMap& properties = dict.at("properties").asValueMap();
            version = properties.at("format").asInt();
        }
		JOFrameAnimationVO* vo = POOL_GET(JOFrameAnimationVO, "JOFrameAnimationCache");
        vo->clear();

        switch (version) {
            case 1:
                _parseVersion1(animations.asValueMap(), vo->m_aniMap);
                break;
            case 2:
                _parseVersion2(animations.asValueMap(), vo->m_aniMap);
                break;
            default:
                CCASSERT(false, "Invalid animation format");
        }
        std::unordered_map<std::string, Animation*>::iterator itr = vo->m_aniMap.begin();
        while (itr!=vo->m_aniMap.end()) {
            m_animations[itr->first] = itr->second;
            ++itr;
        }
		m_aniMap[aniKey] = vo;
    }
    
}
void JOFrameAnimationCache::removeAnimationsWithKey(const std::string& aniKey)
{
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.find(aniKey);
    if (itr!=m_aniMap.end()){
		JOFrameAnimationVO* vo = itr->second;
		std::unordered_map<std::string, Animation*>* tmpMap = &vo->m_aniMap;
		std::unordered_map<std::string, Animation*>::iterator aniItr = tmpMap->begin();
		while (aniItr != tmpMap->end()) {
            m_animations.erase(aniItr->first);
            ++aniItr;
        }
        vo->clear();
		POOL_RECOVER(vo, JOFrameAnimationVO, "JOFrameAnimationCache");
        m_aniMap.erase(aniKey);
    }
}

Animation* JOFrameAnimationCache::getAnimation(const std::string& name)
{
    return m_animations[name];
}

bool JOFrameAnimationCache::getAnimationsWithAniKey(const std::string& aniKey, std::unordered_map<std::string, Animation*> &outMap)
{
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.find(aniKey);
    if (itr!=m_aniMap.end())
    {
        outMap = m_aniMap[aniKey]->getAnimations();
        return true;
    }
    return false;
}

void JOFrameAnimationCache::tick()
{
    static float interval = 0;
    interval += JOTickMgr::Instance()->deltaTime();
    if (interval > m_checkInterval){
        interval = 0;
        _checkDispose();
    }
}


void JOFrameAnimationCache::_checkDispose()
{
    clock_t curTime = JOTime::getTimeofday();
	std::unordered_map<std::string, JOFrameAnimationVO*>::iterator itr = m_aniMap.begin();
	JOFrameAnimationVO* vo = nullptr;
	std::unordered_map<std::string, Animation*>* tmpMap = nullptr;
	std::unordered_map<std::string, Animation*>::iterator aniItr;
    while (itr!=m_aniMap.end()){
		vo = itr->second;
        if (vo->m_count <= 0 && (curTime - vo->m_noQuoteTime ) > m_checkInterval )
        {
            //removeAnimationsWithKey(itr->first);
			tmpMap = &vo->m_aniMap;
			aniItr = tmpMap->begin();
			while (aniItr != tmpMap->end()) {
				m_animations.erase(aniItr->first);
				++aniItr;
			}
			vo->clear();
			POOL_RECOVER(vo, JOFrameAnimationVO, "JOFrameAnimationCache");

            itr = m_aniMap.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

void JOFrameAnimationCache::_parseVersion1(const ValueMap& animations, std::unordered_map<std::string, Animation*> &outMap)
{
    SpriteFrameCache *frameCache = SpriteFrameCache::getInstance();
    
    for (auto iter = animations.cbegin(); iter != animations.cend(); ++iter)
    {
        const ValueMap& animationDict = iter->second.asValueMap();
        const ValueVector& frameNames = animationDict.at("frames").asValueVector();
        float delay = animationDict.at("delay").asFloat();
        Animation* animation = nullptr;
        
        if ( frameNames.empty() )
        {
            LOG_WARN("JOFrameAnimationCache", "Animation '%s' found in dictionary without any frames - cannot add to animation cache.", iter->first.c_str());
            continue;
        }
        
        ssize_t frameNameSize = frameNames.size();
        Vector<AnimationFrame*> frames(frameNameSize);
        
        for (auto& frameName : frameNames)
        {
            SpriteFrame* spriteFrame = frameCache->getSpriteFrameByName(frameName.asString());
            
            if ( ! spriteFrame ) {
                LOG_WARN("JOFrameAnimationCache", "Animation '%s' refers to frame '%s' which is not currently in the SpriteFrameCache. This frame will not be added to the animation.", iter->first.c_str(), frameName.asString().c_str());
                
                continue;
            }
            
            AnimationFrame* animFrame = AnimationFrame::create(spriteFrame, 1, ValueMap());
            frames.pushBack(animFrame);
        }
        
        if ( frames.empty() )
        {
            LOG_WARN("JOFrameAnimationCache", "None of the frames for animation '%s' were found in the SpriteFrameCache. Animation is not being added to the Animation Cache.", iter->first.c_str());
            continue;
        }
        else if ( frames.size() != frameNameSize )
        {
            LOG_WARN("JOFrameAnimationCache", "An animation in your dictionary refers to a frame which is not in the SpriteFrameCache. Some or all of the frames for the animation '%s' may be missing.", iter->first.c_str());
        }
        
        animation = Animation::create(frames, delay, 1);
        animation->retain();
        outMap[iter->first] = animation;
        //AnimationCache::getInstance()->addAnimation(animation, iter->first);
    }
}

void JOFrameAnimationCache::_parseVersion2(const ValueMap& animations, std::unordered_map<std::string, Animation*> &outMap)
{
    SpriteFrameCache *frameCache = SpriteFrameCache::getInstance();
    
    for (auto iter = animations.cbegin(); iter != animations.cend(); ++iter)
    {
        std::string name = iter->first;
        ValueMap& animationDict = const_cast<ValueMap&>(iter->second.asValueMap());
        
        const Value& loops = animationDict["loops"];
        bool restoreOriginalFrame = animationDict["restoreOriginalFrame"].asBool();
        
        ValueVector& frameArray = animationDict["frames"].asValueVector();
        
        if ( frameArray.empty() )
        {
            LOG_WARN("JOFrameAnimationCache", "Animation '%s' found in dictionary without any frames - cannot add to animation cache.", name.c_str());
            continue;
        }
        
        // Array of AnimationFrames
        Vector<AnimationFrame*> array(static_cast<int>(frameArray.size()));
        
        for (auto& obj : frameArray)
        {
            ValueMap& entry = obj.asValueMap();
            std::string spriteFrameName = entry["spriteframe"].asString();
            SpriteFrame *spriteFrame = frameCache->getSpriteFrameByName(spriteFrameName);
            
            if( ! spriteFrame ) {
                LOG_WARN("JOFrameAnimationCache", "Animation '%s' refers to frame '%s' which is not currently in the SpriteFrameCache. This frame will not be added to the animation.", name.c_str(), spriteFrameName.c_str());
                
                continue;
            }
            
            float delayUnits = entry["delayUnits"].asFloat();
            Value& userInfo = entry["notification"];
            
            AnimationFrame *animFrame = AnimationFrame::create(spriteFrame, delayUnits, userInfo.getType() == Value::Type::MAP ? userInfo.asValueMap() : ValueMapNull);
            
            array.pushBack(animFrame);
        }
        
        float delayPerUnit = animationDict["delayPerUnit"].asFloat();
        Animation *animation = Animation::create(array, delayPerUnit, loops.getType() != Value::Type::NONE ? loops.asInt() : 1);
        
        animation->setRestoreOriginalFrame(restoreOriginalFrame);
        animation->retain();
        outMap[iter->first] = animation;
        //AnimationCache::getInstance()->addAnimation(animation, name);
    }
}
NS_JOFW_END
