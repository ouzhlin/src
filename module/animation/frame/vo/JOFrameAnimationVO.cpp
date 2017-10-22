#include "module/animation/frame/vo/JOFrameAnimationVO.h"
#include "utils/JOLog.h"


NS_JOFW_BEGIN

JOFrameAnimationVO::JOFrameAnimationVO()
: m_count(0)
, m_noQuoteTime(0)
{
    
}
JOFrameAnimationVO::~JOFrameAnimationVO()
{
    clear();
}

void JOFrameAnimationVO::increase()
{
    ++m_count;
    m_noQuoteTime = 0;
}

void JOFrameAnimationVO::decrease()
{
    m_count--;
    if (m_count <= 0)
    {
        m_count = 0;
        m_noQuoteTime = JOTime::getTimeofday();
    }
}

void JOFrameAnimationVO::addAnimation(const std::string &key, Animation* ani)
{
    ani->retain();
    std::unordered_map<std::string, Animation*>::iterator itr = m_aniMap.find(key);
    if (itr != m_aniMap.end())
    {
        itr->second->release();
    }
    m_aniMap[key] = ani;
}

void JOFrameAnimationVO::clear()
{
    std::unordered_map<std::string, Animation*>::iterator itr = m_aniMap.begin();
    while (itr != m_aniMap.end()) {
        itr->second->release();
        ++itr;
    }
    m_aniMap.clear();
}

std::unordered_map<std::string, Animation*> JOFrameAnimationVO::getAnimations()
{
    return m_aniMap;
}

NS_JOFW_END