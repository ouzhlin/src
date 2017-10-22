#include "module/dyer/vo/JODyerVO.h"
#include "module/dyer/JODyerParser.h"

#include "manager/JOCachePoolMgr.h"
#include "utils/JOPath.h"
#include "ui/JOSprite.h"

#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN


JODyerBlockVO::JODyerBlockVO()
: m_block(nullptr)
, m_time(0)
{
}

JODyerBlockVO::~JODyerBlockVO()
{
	done();
}

void JODyerBlockVO::init(cocos2d::Sprite* spr, double time, const std::string& srcPath)
{
	CC_SAFE_RELEASE_NULL(m_block);
	spr->retain();
	m_block = spr;
	m_time = time;
	m_srcPath = srcPath;
}

void JODyerBlockVO::done()
{
	CC_SAFE_RELEASE_NULL(m_block);
	m_time = 0;
}


JODyerVO::JODyerVO()
: m_dd(nullptr)
, m_ori(nullptr)
{
}

JODyerVO::~JODyerVO()
{
	done();
}

void JODyerVO::init(cocos2d::Sprite* spr, DyerData* dd, const std::string& srcPath)
{
	CC_SAFE_RELEASE_NULL(m_ori);
	spr->retain();
	m_ori = spr;
	m_dd = dd;
	m_srcPath = srcPath;
}

void JODyerVO::done()
{
	CC_SAFE_RELEASE_NULL(m_ori);
	m_dd = nullptr;
}

NS_JOFW_END