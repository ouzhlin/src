#include "manager/vo/JOFileCacheVO.h"

NS_JOFW_BEGIN

JOFileCacheVO::JOFileCacheVO()
: m_fileData(nullptr)
, m_useTime(0)
{

}

JOFileCacheVO::~JOFileCacheVO()
{

}

void JOFileCacheVO::init(JOData* d, clock_t time)
{
	m_fileData = d;
	m_useTime = time;
}

NS_JOFW_END