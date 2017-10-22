#include "module/loader/vo/JOResConfigVO.h"
#include "utils/JOPath.h"

NS_JOFW_BEGIN


void JOResConfigVO::setConfig(const std::string srcPath, int resType /*= JOAsynchBaseLoader::RES_IMG*/, int pFormat /*= JOResConfig::PF_565*/, const std::string plist/*=JO_EMTYP_STRING*/, const std::string aniPlist/*=JO_EMTYP_STRING */)
{
	pixelFormat = pFormat;
	this->resType = resType;
	this->srcPath = srcPath;
	this->plistPath = plist;
	this->aniPlistPath = aniPlist;
}


void JOResSrcVO::setData(const std::string& srcPath, const std::string& srcKey, const std::string& basePath, int pFormat /*= JOResConfig::PF_565*/, const std::string& plist /*= JO_EMTYP_STRING*/)
{
	this->srcPath = srcPath;
	this->basePath = basePath;
	this->plist = plist;
	this->srcKey = srcKey;
	pixelFormat = pFormat;
}

std::string JOResSrcVO::getSrc() const
{
	return basePath + srcPath;
}

std::string JOResSrcVO::getPlist() const
{
	return basePath + plist;
}

void JOResArmatureVO::setData(const std::string& arm, const std::string& basePath, const std::string& srcKey)
{
	armature = arm;
	this->srcKey = srcKey;
	this->basePath = basePath;
}

std::string JOResArmatureVO::getArmPath() const
{
	return basePath + armature;
}

NS_JOFW_END