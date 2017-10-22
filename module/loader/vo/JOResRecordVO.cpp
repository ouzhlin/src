#include "module/loader/vo/JOResRecordVO.h"
#include "utils/JOTime.h"
#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN
void JOResRecordVO::setRecord(const std::string srcPath, cocos2d::Texture2D *texture, int resType)
{
	count = 0;
	noQuoteTime = JOTime::getTimeofday();
	this->resType = resType;
	this->srcPath = srcPath;
	tex = texture;
	tex->retain();
}

void JOResRecordVO::dispose()
{
	CC_SAFE_RELEASE_NULL(tex);
}


void JOResRecordVO::increase()
{
	++count;
	//noQuoteTime = 0;
}

void JOResRecordVO::decrease()
{
	count--;
	if (count <= 0)
	{
		count = 0;
		noQuoteTime = JOTime::getTimeofday();
	}
}

NS_JOFW_END