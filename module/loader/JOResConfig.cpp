#include "module/loader/JOResConfig.h"
#include "module/loader/JOAsynchBaseLoader.h"
#include "module/loader/vo/JOResConfigVO.h"

#include "manager/JOCachePoolMgr.h"
#include "manager/JOFileMgr.h"

#include "core/datautils/JODataPool.h"
#include "core/datautils/JODataCoder.h"

#include "utils/JOTime.h"
#include "utils/JOString.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"

#include "cocos2d.h"
USING_NS_CC;


NS_JOFW_BEGIN

JOResConfig::JOResConfig()
: m_isDebug(false)
{
	srand(JOTime::getTimeofday());
}

JOResConfig::~JOResConfig()
{
	clear();
}

void JOResConfig::clear()
{
	clearSrc();
	clearAni();

	RES_ARM_MAP::iterator armItr = m_arm_armMap.begin();
	while (armItr != m_arm_armMap.end()){
		POOL_RECOVER(armItr->second, JOResAnimationVO, "JOResConfig");
		++armItr;
	}
	m_arm_armMap.clear();
	m_src_armMap.clear();
}

bool JOResConfig::addSrcKey(const std::string& imgKey, const std::string& basePath, const std::string& srcPath, short pf /*= PF_565*/, const std::string& plistPath /*= JO_EMTYP_STRING*/)
{
	std::string srcName = JOPath::getFileName(srcPath);
	std::string plistKey = JOPath::getFileName(plistPath);

	if (m_isDebug){
		RES_SRC_MAP::iterator itr = m_src_srcMap.find(srcName);
		if (itr != m_src_srcMap.end()){
			LOG_WARN("JOResConfig", "have same srcName [%s] in src_srcMap !!!!", srcName.c_str());
			return false;
		}
		itr = m_key_srcMap.find(imgKey);
		if (itr != m_key_srcMap.end()){
			LOG_WARN("JOResConfig", "have same imgKey [%s] in key_srcMap !!!!", imgKey.c_str());
			return false;
		}
		if (plistPath != JO_EMTYP_STRING){
			itr = m_plist_srcMap.find(plistKey);
			if (itr != m_plist_srcMap.end()){
				LOG_WARN("JOResConfig", "have same plist [%s] in plist_srcMap !!!!", plistKey.c_str());
				return false;
			}
		}
	}

	JOResSrcVO* vo = POOL_GET(JOResSrcVO, "JOResConfig");
	vo->setData(srcPath, srcName, basePath, pf, plistPath);
	m_src_srcMap[srcName] = vo;	
	m_key_srcMap[imgKey] = vo;

	if (plistPath != JO_EMTYP_STRING){
		m_plist_srcMap[plistKey] = vo;
	}

	return true;
}

bool JOResConfig::loadSrcConfig(const std::string& fPath, const std::string& basePath, const std::string& keyname)
{
	JOData* data = JOFileMgr::Instance()->getFileData(fPath);
	if (data == nullptr){
		return false;
	}
	JODataCoder* dc = JODataPool::Instance()->getDataCoder();
	dc->init((unsigned char*)data->bytes(), data->length());

	BACK_INDEX::iterator backItr = m_back_srcMap.find(keyname);
	if (backItr == m_back_srcMap.end()){
		std::vector<std::string> vec;
		m_back_srcMap[keyname] = vec;
	}
	backItr = m_back_plistMap.find(keyname);
	if (backItr == m_back_plistMap.end()){
		std::vector<std::string> vec;
		m_back_plistMap[keyname] = vec;
	}
	backItr = m_back_keyMap.find(keyname);
	if (backItr == m_back_keyMap.end()){
		std::vector<std::string> vec;
		m_back_keyMap[keyname] = vec;
	}
	std::vector<std::string> srcVec = m_back_srcMap[keyname];
	std::vector<std::string> plistVec = m_back_plistMap[keyname];
	std::vector<std::string> keyVec = m_back_keyMap[keyname];
	bool ret = true;
	do 
	{
		/************************************************************************/
		/* 单图
		/************************************************************************/
		std::string plistPath = JO_EMTYP_STRING;		
		std::string srcPath = JO_EMTYP_STRING;

		std::string plistKey = JO_EMTYP_STRING;
		std::string srcKey = JO_EMTYP_STRING;
		RES_SRC_MAP::iterator srcItr;
		int pfVal = 0;
		unsigned int len = 0;
		for (unsigned int cdx = 0; cdx < 4; cdx++){
			pfVal = dc->readShort();
			len = dc->readUInt();
			for (unsigned int i = 0; i < len; i++){
				srcPath = dc->readString();
				srcKey = JOPath::getFileName(srcPath);
				if (m_isDebug){
					srcItr = m_src_srcMap.find(srcKey);
					if (srcItr != m_src_srcMap.end()){
						LOG_WARN("JOResConfig", "have same srcPath [%s] in srcDataMap !!!!", srcKey.c_str());
						ret = false;
						break;
					}
				}
				
				JOResSrcVO* vo = POOL_GET(JOResSrcVO, "JOResConfig");
				vo->setData(srcPath, srcKey, basePath, pfVal);
				m_src_srcMap[srcKey] = vo;
				srcVec.push_back(srcKey);
			}
		}
		/************************************************************************/
		/* 图集
		/************************************************************************/
		pfVal = 0;
		len = 0;
		for (unsigned int cdx = 0; cdx < 4; cdx++){
			pfVal = dc->readShort();
			len = dc->readUInt();
			for (unsigned int i = 0; i < len; i++){
				srcPath = dc->readString();
				plistPath = dc->readString();

				srcKey = JOPath::getFileName(srcPath);
				plistKey = JOPath::getFileName(plistPath);
				if (m_isDebug){
					srcItr = m_src_srcMap.find(srcKey);
					if (srcItr != m_src_srcMap.end()){
						LOG_WARN("JOResConfig", "have same srcPath [%s] in srcDataMap !!!!", srcKey.c_str());
						ret = false;
						break;
					}
					srcItr = m_plist_srcMap.find(plistKey);
					if (srcItr != m_plist_srcMap.end()){
						LOG_WARN("JOResConfig", "have same plist [%s] in srcDataMap !!!!", plistKey.c_str());
						ret = false;
						break;
					}
				}

				JOResSrcVO* vo = POOL_GET(JOResSrcVO, "JOResConfig");
				vo->setData(srcPath, srcKey, basePath, pfVal, plistPath);
				m_src_srcMap[srcKey] = vo;
				m_plist_srcMap[plistKey] = vo;
				srcVec.push_back(srcKey);
				plistVec.push_back(plistKey);
			}
		}

		std::string imgKey = JO_EMTYP_STRING;
		len = dc->readUInt();						
		for (unsigned int i = 0; i < len; i++){
			imgKey = dc->readString();
			srcKey = dc->readString();
			if (m_isDebug){
				srcItr = m_key_srcMap.find(imgKey);
				if (srcItr != m_key_srcMap.end()){
					LOG_WARN("JOResConfig", "have same imgKey [%s] in imgKeyMap !!!!", imgKey.c_str());
					ret = false;
					break;
				}				
			}
			srcItr = m_src_srcMap.find(srcKey);
			if (srcItr == m_src_srcMap.end()){
				LOG_WARN("JOResConfig", "can't find src[%s] with imgKey [%s]!!!!", srcKey.c_str(), imgKey.c_str());
				ret = false;
				break;
			}
			m_key_srcMap[imgKey] = srcItr->second;
			keyVec.push_back(imgKey);
		}
	} while (0);
	
	JODataPool::Instance()->recover(dc);
	if (ret==false)
	{
		removeConfig(keyname);
	}
	return ret;
}

void JOResConfig::removeConfig(const std::string& keyname)
{
	std::vector<std::string>::iterator vecItr;
	BACK_INDEX::iterator backItr = m_back_srcMap.find(keyname);
	if (backItr != m_back_srcMap.end()){
		std::vector<std::string> vec = backItr->second;
		vecItr = vec.begin();
		while (vecItr != vec.end())
		{
			m_src_srcMap.erase((*vecItr));
			vecItr++;
		}
	}
	backItr = m_back_plistMap.find(keyname);
	if (backItr != m_back_plistMap.end()){
		std::vector<std::string> vec = backItr->second;
		vecItr = vec.begin();
		while (vecItr != vec.end())
		{
			m_plist_srcMap.erase((*vecItr));
			vecItr++;
		}
	}
	backItr = m_back_keyMap.find(keyname);
	if (backItr != m_back_keyMap.end()){
		std::vector<std::string> vec = backItr->second;
		vecItr = vec.begin();
		while (vecItr != vec.end())
		{
			m_key_srcMap.erase((*vecItr));
			vecItr++;
		}
	}
	m_back_srcMap.erase(keyname);
	m_back_plistMap.erase(keyname);
	m_back_keyMap.erase(keyname);
}

const JOResSrcVO* JOResConfig::srcVOWithKey(const std::string& imgKey)
{
	RES_SRC_MAP::iterator itr = m_key_srcMap.find(imgKey);
	if (itr != m_key_srcMap.end()){
		return itr->second;
	}
	return nullptr;
}

const JOResSrcVO* JOResConfig::srcVOWithSrc(const std::string& srcName)
{
	RES_SRC_MAP::iterator itr = m_src_srcMap.find(srcName);
	if (itr != m_src_srcMap.end()){
		return itr->second;
	}
	return nullptr;
}

const JOResSrcVO* JOResConfig::srcVOWithPlist(const std::string& plistName)
{
	RES_SRC_MAP::iterator itr = m_plist_srcMap.find(plistName);
	if (itr != m_plist_srcMap.end()){
		return itr->second;
	}
	return nullptr;
}

void JOResConfig::clearSrc()
{
	RES_SRC_MAP::iterator srcItr = m_src_srcMap.begin();
	while (srcItr != m_src_srcMap.end()){
		POOL_RECOVER(srcItr->second, JOResSrcVO, "JOResConfig");
		++srcItr;
	}
	m_src_srcMap.clear();
	m_plist_srcMap.clear();
	m_key_srcMap.clear();
}
//////////////////////////////////////////////////////////////////////////

void JOResConfig::setPFVal(short pf, short val)
{
	pfMap[pf] = val;
}

short JOResConfig::getPFVal(short pf)
{
	PIX_FORMAT_MAP::iterator itr = pfMap.find(pf);
	if (itr != pfMap.end()){
		return itr->second;
	}
	return 0;
}


void JOResConfig::description()
{
	std::unordered_map<std::string, std::list<std::string> > tmpFramesMap;
	std::unordered_map<std::string, std::list<std::string> >::iterator frameItr;
	const JOResSrcVO* vo = nullptr;
	unsigned int count = 0;
	RES_SRC_MAP::iterator itr = m_key_srcMap.begin();
	while (itr != m_key_srcMap.end()){
		vo = srcVOWithKey(itr->first);
		if (vo->plist == JO_EMTYP_STRING)	{
			LOG_INFO("JOResConfig", "key:%s  ==> path:%s", itr->first.c_str(), vo->srcPath.c_str());
			++count;
		}
		else{
			frameItr = tmpFramesMap.find(vo->plist);
			if (frameItr == tmpFramesMap.end())	{
				std::list<std::string> list;
				tmpFramesMap[vo->plist] = list;
			}
			tmpFramesMap[vo->plist].push_back(itr->first);
		}
		++itr;
	}
	LOG_INFO("JOResConfig", "total img is %d\n\n", count);

	std::list<std::string>* tmpFrameList = nullptr;
	std::list<std::string>::iterator tmpListItr;
	frameItr = tmpFramesMap.begin();
	while (frameItr != tmpFramesMap.end()){
		tmpFrameList = &frameItr->second;
		tmpListItr = tmpFrameList->begin();
		while (tmpListItr != tmpFrameList->end()){
			LOG_INFO("JOResConfig", "key:%s  ==> path:%s", (*tmpListItr).c_str(), frameItr->first.c_str());
			++tmpListItr;
		}
		LOG_INFO("JOResConfig", "total frames %d in %s\n\n", tmpFrameList->size(), frameItr->first.c_str());
		++frameItr;
	}
}

std::string JOResConfig::randomKey()
{
 	unsigned int count = 0;
	//int idx = ((int)rand() / RAND_MAX)*(configVOMap.size() - 1) + 1;/*产生区间[a,b]上的随机数*/
	int idx = (double)rand() / ((double)RAND_MAX / (m_key_srcMap.size() - 1)) + 1;
	RES_SRC_MAP::iterator itr = m_key_srcMap.begin();
	while (itr != m_key_srcMap.end()){
		++count;
		if (count == idx){
			return itr->first;
		}
		++itr;
	}
	return "";
}

//GENERATE////////////////////////////////////////////////////////////////////////
bool JOResConfig::generateSrcConfig(const std::string& srcPath, const std::string& destPath, const std::string& ellipsis, const std::string& outName)
{
	//std::string imgPath = UserDefault::getInstance()->getStringForKey("IMG_PATH");
	//std::string imgPath = JOFileMgr::Instance()->getFileExplorerDir(srcPath);
	//UserDefault::getInstance()->setStringForKey("IMG_PATH", dir.c_str());
	std::string imgPath = JOPath::standardisePath(srcPath, true);
	std::list<std::string> fileList;
	JOFileMgr::Instance()->getFilePathInDIR(imgPath.c_str(), fileList);
	if (fileList.empty()){
		LOG_ERROR("JOResConfig", "srcPath [%s] no need files!!!!!", srcPath.c_str());
		return false;
	}
	std::string tmpEllipsis = srcPath;
	if (ellipsis != JO_EMTYP_STRING){
		tmpEllipsis = ellipsis;
	}
	tmpEllipsis = JOPath::standardisePath(tmpEllipsis);
	std::list<std::string> pf565List;
	std::list<std::string> pf4444List;
	std::list<std::string> pf5551List;
	std::list<std::string> pf8888List;

	std::unordered_map< std::string, std::list<std::string> > pf565PlistMap;
	std::unordered_map< std::string, std::list<std::string> > pf4444PlistMap;
	std::unordered_map< std::string, std::list<std::string> > pf5551PlistMap;
	std::unordered_map< std::string, std::list<std::string> > pf8888PlistMap;

	std::unordered_map<std::string, std::string> plistSrcMap;
	std::string suffix = JO_EMTYP_STRING;
	std::list<std::string>::iterator itr = fileList.begin();


	itr = fileList.begin();
	while (itr != fileList.end()){
		suffix = JOPath::getFileSuffix(*itr);
        JOString::toLowerCase(suffix);
		if (suffix == "plist"){
			//std::string fullPath = FileUtils::getInstance()->fullPathForFilename(*itr);
			//if (fullPath.size() > 0){
			ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(*itr);
			if (!dict.empty()){			
				//ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(fullPath);
                if (dict["frames"].getType() != cocos2d::Value::Type::MAP){
                    ++itr;
					continue;
                }
				std::string texturePath;
				if (dict.find("metadata") != dict.end()){
					ValueMap& metadataDict = dict["metadata"].asValueMap();
					// try to read  texture file name from meta data
					texturePath = metadataDict["textureFileName"].asString();
				}

				if (!texturePath.empty()){
					// build texture path relative to plist file
					texturePath = FileUtils::getInstance()->fullPathFromRelativeFile(texturePath, *itr);
				}
				else{
					// build texture path by replacing file extension
					texturePath = *itr;
					// remove .xxx
					size_t startPos = texturePath.find_last_of(".");
					texturePath = texturePath.erase(startPos);
					// append .png
					texturePath = texturePath.append(".png");
					CCLOG("cocos2d: SpriteFrameCache: Trying to use file %s as texture", texturePath.c_str());
				}

				ValueMap& framesDict = dict["frames"].asValueMap();
				if (framesDict.empty()){
                    ++itr;
					continue;
				}
				std::string plistPath = JOString::replaceAll((*itr).c_str(), tmpEllipsis.c_str(), "");
				std::unordered_map<std::string, std::string>::iterator checkPsItr = plistSrcMap.find(plistPath);
				if (checkPsItr != plistSrcMap.end()){
					LOG_ERROR("", "have same plist [%s]", (*itr).c_str());
					return false;
				}
				plistSrcMap[plistPath] = JOString::replaceAll(texturePath.c_str(), tmpEllipsis.c_str(), "");

				std::list<std::string> frameNameList;
				for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
				{
					frameNameList.push_back(iter->first);
				}
				if (JOString::isContains(plistPath, "/565/")){
					pf565PlistMap[plistPath] = frameNameList;
				}
				else if (JOString::isContains(plistPath, "/4444/")){
					pf4444PlistMap[plistPath] = frameNameList;
				}
				else if (JOString::isContains(*itr, "/5551/")){
					pf5551PlistMap[plistPath] = frameNameList;
				}
				else if (JOString::isContains(plistPath, "/8888/")){
					pf8888PlistMap[plistPath] = frameNameList;
				}
				else{
					pf8888PlistMap[plistPath] = frameNameList;
				}
			}
		}
        else if (suffix == "png" || suffix == "jpg" ||
                 suffix == "pvr" || suffix == "pvr.ccz" ||
                 suffix == "block")
		{
			std::string srcPath = JOString::replaceAll((*itr).c_str(), tmpEllipsis.c_str(), "");
			if (JOString::isContains(srcPath, "/565/")){
				pf565List.push_back(srcPath);
			}
			else if (JOString::isContains(srcPath, "/4444/")){
				pf4444List.push_back(srcPath);
			}
			else if (JOString::isContains(srcPath, "/5551/")){
				pf5551List.push_back(srcPath);
			}
			else if (JOString::isContains(srcPath, "/8888/")){
				pf8888List.push_back(srcPath);
			}
			else{
				pf8888List.push_back(srcPath);
			}
		}
		++itr;
	}

	std::list<std::string>::iterator checkSrcListItr;
	std::unordered_map<std::string, std::string>::iterator checkPsItr = plistSrcMap.begin();
	while (checkPsItr != plistSrcMap.end()){
		pf565List.remove(checkPsItr->second);
		pf4444List.remove(checkPsItr->second);
		pf5551List.remove(checkPsItr->second);
		pf8888List.remove(checkPsItr->second);
		++checkPsItr;
	}



	/************************************************************************/
	/* 用于检查有没有重名的
	/************************************************************************/
	std::unordered_map<std::string, std::string> checkKeyMap;
	std::unordered_map<std::string, std::string> checkSrcMap;

	JODataCoder* srcCoder = JODataPool::Instance()->getDataCoder();
	JODataCoder* keyCoder = JODataPool::Instance()->getDataCoder();
	JODataCoder* plistCoder = JODataPool::Instance()->getDataCoder();

	/************************************************************************/
	/* plist->src
	/************************************************************************/
	unsigned int len = plistSrcMap.size();
	plistCoder->writeUInt(len);
	std::unordered_map<std::string, std::string>::iterator pSrcItr = plistSrcMap.begin();
	while (pSrcItr != plistSrcMap.end()){
		if (pSrcItr->first == JO_EMTYP_STRING || pSrcItr->second == JO_EMTYP_STRING){
			LOG_ERROR("JOResConfig", "plist data error [%s] [%s]", pSrcItr->first.c_str(), pSrcItr->second.c_str());
			return false;
		}
		plistCoder->writeString(pSrcItr->first);
		plistCoder->writeString(pSrcItr->second);
		++pSrcItr;
	}
	/************************************************************************/
	/* 单图
	/************************************************************************/
	srcCoder->writeShort(getPFVal(PF_565));
	if (!_setSrcKey(pf565List, checkSrcMap, checkKeyMap, srcCoder, keyCoder))
		return false;
	srcCoder->writeShort(getPFVal(PF_4444));
	if (!_setSrcKey(pf4444List, checkSrcMap, checkKeyMap, srcCoder, keyCoder))
		return false;
	srcCoder->writeShort(getPFVal(PF_5551));
	if (!_setSrcKey(pf5551List, checkSrcMap, checkKeyMap, srcCoder, keyCoder))
		return false;
	srcCoder->writeShort(getPFVal(PF_8888));
	if (!_setSrcKey(pf8888List, checkSrcMap, checkKeyMap, srcCoder, keyCoder))
		return false;

	/************************************************************************/
	/* 图集
	/************************************************************************/
	srcCoder->writeShort(getPFVal(PF_565));
	if (!_setPlistMap(pf565PlistMap, plistSrcMap, checkSrcMap, checkKeyMap, srcCoder, keyCoder)){
		return false;
	}
	srcCoder->writeShort(getPFVal(PF_4444));
	if (!_setPlistMap(pf4444PlistMap, plistSrcMap, checkSrcMap, checkKeyMap, srcCoder, keyCoder)){
		return false;
	}
	srcCoder->writeShort(getPFVal(PF_5551));
	if (!_setPlistMap(pf5551PlistMap, plistSrcMap, checkSrcMap, checkKeyMap, srcCoder, keyCoder)){
		return false;
	}
	srcCoder->writeShort(getPFVal(PF_8888));
	if (!_setPlistMap(pf8888PlistMap, plistSrcMap, checkSrcMap, checkKeyMap, srcCoder, keyCoder)){
		return false;
	}

	srcCoder->writeUInt(checkKeyMap.size()); // add the key count

	JOData* d = srcCoder->data();
	//d->appendData(srcCoder->data());
	d->appendData(keyCoder->data());

	std::string outPath = JOPath::standardisePath(destPath, true);
	bool ret = JOFileMgr::wirteFile((outPath + outName).c_str(), d->bytes(), d->length());
	JODataPool::Instance()->recover(plistCoder);
	JODataPool::Instance()->recover(srcCoder);
	JODataPool::Instance()->recover(keyCoder);

	return ret;
}

bool JOResConfig::generateAniConfig(const std::string& srcPath, const std::string& destPath, const std::string& ellipsis, const std::string& outName)
{
	std::string imgPath = JOPath::standardisePath(srcPath, true);
	std::list<std::string> fileList;
	JOFileMgr::Instance()->getFilePathInDIR(imgPath.c_str(), fileList, "plist");
	if (fileList.empty()){
		LOG_ERROR("JOResConfig", "srcPath [%s] no need files!!!!!", srcPath.c_str());
		return false;
	}
	std::string tmpEllipsis = srcPath;
	if (ellipsis != JO_EMTYP_STRING){
		tmpEllipsis = ellipsis;
	}
	tmpEllipsis = JOPath::standardisePath(tmpEllipsis);

	JODataCoder* srcCoder = JODataPool::Instance()->getDataCoder();
	std::string fKey = JO_EMTYP_STRING;
	std::string tmpPath = JO_EMTYP_STRING;
	std::unordered_map<std::string, std::string> tmpMap;
	std::list<std::string>::iterator itr = fileList.begin();

	unsigned int len = fileList.size();
	srcCoder->writeUInt(len);
	while (itr != fileList.end()){
		fKey = JOPath::getFileName(*itr);
		std::unordered_map<std::string, std::string>::iterator aniItr = tmpMap.find(fKey);
		if (aniItr != tmpMap.end())	{
			LOG_ERROR("JOResConfig", "have same aniKey [%s] !!!!!", fKey.c_str());
			JODataPool::Instance()->recover(srcCoder);
			return false;
		}
		tmpPath = JOString::replaceAll((*itr).c_str(), tmpEllipsis.c_str(), "", false);
		tmpMap[fKey] = tmpPath;
		srcCoder->writeString(fKey);
		srcCoder->writeString(tmpPath);
		++itr;
	}
		
	JOData* d = srcCoder->data();
	std::string outPath = JOPath::standardisePath(destPath, true);
	bool ret = JOFileMgr::wirteFile((outPath + outName).c_str(), d->bytes(), d->length());	
	JODataPool::Instance()->recover(srcCoder);
	return ret;
}


bool JOResConfig::_setSrcKey(std::list<std::string> &list, std::unordered_map<std::string, std::string> &checkSrcMap, std::unordered_map<std::string, std::string> &checkKeyMap, JODataCoder* srcCoder, JODataCoder* keyCoder)
{
	std::string fKey = JO_EMTYP_STRING;
	unsigned int len = list.size();
	srcCoder->writeUInt(len);
	for (auto itr = list.begin(); itr != list.end(); itr++)
	{
		fKey = JOPath::getFileName(*itr);
		std::unordered_map<std::string, std::string>::iterator checkItr = checkSrcMap.find(*itr);
		if (checkItr != checkSrcMap.end()){
			LOG_ERROR("JOResConfig", "have same src path [%s]", (*itr).c_str());
			LOG_ERROR("JOResConfig", checkItr->second.c_str());
			return false;
		}
		checkItr = checkKeyMap.find(fKey);
		if (checkItr != checkKeyMap.end()){
			LOG_ERROR("JOResConfig", "have same key [%s]", fKey.c_str());
			LOG_ERROR("JOResConfig", checkItr->second.c_str());
			LOG_ERROR("JOResConfig", (*itr).c_str());
			return false;
		}
		if ((*itr)==JO_EMTYP_STRING || fKey==JO_EMTYP_STRING){
			LOG_ERROR("JOResConfig", "src data error src[%s] key[%s]", (*itr).c_str(), fKey.c_str());
			return false;
		}
		checkSrcMap[(*itr)] = JO_EMTYP_STRING;
		checkKeyMap[fKey] = (*itr);
		srcCoder->writeString(*itr);

		keyCoder->writeString(fKey);
		keyCoder->writeString(fKey);
	}
	return true;
}

bool JOResConfig::_setPlistMap(std::unordered_map< std::string, std::list<std::string> > &pFrameMap, std::unordered_map< std::string, std::string > &pMap, std::unordered_map<std::string, std::string> &checkSrcMap, std::unordered_map<std::string, std::string> &checkKeyMap, JODataCoder* srcCoder, JODataCoder* keyCoder)
{
	std::string fKey = JO_EMTYP_STRING;
	std::string srcKey = JO_EMTYP_STRING;
	unsigned int len = pFrameMap.size();
	srcCoder->writeUInt(len);
	
	for (auto itr = pFrameMap.begin(); itr != pFrameMap.end(); itr++){
		std::unordered_map<std::string, std::string>::iterator pItr = pMap.find(itr->first);
		if (pItr == pMap.end())	{
			LOG_ERROR("JOResConfig", "plist [%s] is not path !!!!", (itr->first).c_str());
			return false;
		}
		
		std::unordered_map<std::string, std::string>::iterator checkItr = checkSrcMap.find(pItr->second);
		if (checkItr != checkSrcMap.end()){
			LOG_ERROR("JOResConfig", "have same src path [%s]", (pItr->second).c_str());
			LOG_ERROR("JOResConfig", checkItr->second.c_str());
			return false;
		}
		checkSrcMap[pItr->second] = JO_EMTYP_STRING;
		if (pItr->second == JO_EMTYP_STRING || itr->first == JO_EMTYP_STRING){
			LOG_ERROR("JOResConfig", "src data error src[%s] key[%s]", pItr->second.c_str(), itr->first.c_str());
			return false;
		}
		srcCoder->writeString(pItr->second); //src path
		srcCoder->writeString(itr->first); //plist path
		srcKey = JOPath::getFileName(pItr->second);
		checkItr = checkKeyMap.find(srcKey);
		if (checkItr != checkKeyMap.end()){
			LOG_ERROR("JOResConfig", "have same key [%s]", srcKey.c_str());
			LOG_ERROR("JOResConfig", checkItr->second.c_str());
			LOG_ERROR("JOResConfig", pItr->second.c_str());
		}
		checkKeyMap[srcKey] = pItr->second;
		keyCoder->writeString(srcKey); //key
		keyCoder->writeString(srcKey); // src key

		for (auto fItr = itr->second.begin(); fItr != itr->second.end(); fItr++){
			fKey = *fItr;
			
			checkItr = checkKeyMap.find(fKey);
			if (checkItr != checkKeyMap.end()){
				LOG_ERROR("JOResConfig", "have same key [%s]", fKey.c_str());
				LOG_ERROR("JOResConfig", checkItr->second.c_str());
				LOG_ERROR("JOResConfig", itr->first.c_str());
				return false;
			}
			if (fKey == JO_EMTYP_STRING || pItr->second == JO_EMTYP_STRING){
				LOG_ERROR("JOResConfig", "key data error src[%s] key[%s]", pItr->second.c_str(), fKey.c_str());
				return false;
			}
			checkKeyMap[fKey] = itr->first;
			keyCoder->writeString(fKey); //key
			keyCoder->writeString(srcKey); // src key
		}		
	}
	return true;
}

//ARM////////////////////////////////////////////////////////////////////////


bool JOResConfig::loadArmConfig(const std::string& fPath, const std::string& basePath)
{
	return false;
}

const JOResArmatureVO* JOResConfig::armVOWithSrc(const std::string& srcName)
{
	RES_ARM_MAP::iterator itr = m_src_armMap.find(srcName);
	if (itr != m_src_armMap.end()){
		return itr->second;
	}
	return nullptr;
}

const JOResArmatureVO* JOResConfig::armVOWithArm(const std::string& armName)
{
	RES_ARM_MAP::iterator itr = m_arm_armMap.find(armName);
	if (itr != m_arm_armMap.end()){
		return itr->second;
	}
	return nullptr;
}

//ANI////////////////////////////////////////////////////////////////////////

bool JOResConfig::srcPlistWithAniKey(const std::string& aniKey, std::list<std::string>& outList)
{
    RES_ANI_MAP::iterator itr = m_aniMap.find(aniKey);
    if (itr!=m_aniMap.end()) {
        outList = itr->second;
        return true;
    }
	JOData* aniData = getAniDataWithAniKey(aniKey);
	if (aniData == nullptr) {
		LOG_ERROR("JOResConfig", "aniKey[%s] file data could not be found!!!!", aniKey.c_str());
		return false;
	}
	ValueMap dictionary = FileUtils::getInstance()->getValueMapFromData((const char*)aniData->bytes(), aniData->length());
    //ValueMap dictionary = FileUtils::getInstance()->getValueMapFromFile(m_aniBasePath+aniKey);
    
    if (dictionary.empty())
    {
        LOG_ERROR("JOResConfig", "aniKey[%s] file could not be found!!!!", aniKey.c_str());
        return false;
    }
    
    if (dictionary.find("animations") == dictionary.end())
    {
        LOG_ERROR("JOResConfig", "aniKey[%s: No animations were found in provided dictionary.", aniKey.c_str());
        return false;
    }
    std::list<std::string> tmpList;
    if (dictionary.find("properties") != dictionary.end())
    {
        const ValueMap& properties = dictionary.at("properties").asValueMap();
        const ValueVector& spritesheets = properties.at("spritesheets").asValueVector();
        
        for (const auto &value : spritesheets) {
            tmpList.push_back(value.asString());
        }
    }
    else{
        tmpList.push_back(aniKey);
    }
    if (tmpList.empty()) {
        LOG_WARN("JOResConfig", "can't find ani plist with key[%s] !!!!", aniKey.c_str());
        return false;
    }
    outList = tmpList;
    m_aniMap[aniKey] = tmpList;
    return true;
}



void JOResConfig::addAniBasePath(const std::string& aniPath, const std::string& keyname)
{
	m_aniPaths.push_back(aniPath);
	BACK_INDEX::iterator itr = m_back_aniPathMap.find(keyname);
	if (itr == m_back_aniPathMap.end()){
		std::vector<std::string> vec;
		vec.push_back(aniPath);
		m_back_aniPathMap[keyname] = vec;
	}
	else{
		itr->second.push_back(aniPath);
	}
}

void sophia_framework::JOResConfig::removeAniBasePath(const std::string& keyname)
{
	BACK_INDEX::iterator itr = m_back_aniPathMap.find(keyname);
	if (itr != m_back_aniPathMap.end()){
		std::vector<std::string> vec = itr->second;
		std::vector<std::string>::iterator vecItr = vec.begin();
		while (vecItr != vec.end())
		{
			m_aniPaths.remove((*vecItr));
			vecItr++;
		}
	}
}

JOData* JOResConfig::getAniDataWithAniKey(const std::string& aniKey)
{
	JOData* aniData = nullptr;
	std::list<std::string>::iterator itr = m_aniPaths.begin();
	while (itr != m_aniPaths.end()){
		aniData = JOFileMgr::Instance()->getFileData((*itr) + aniKey);
		if (aniData){
			break;
		}
		++itr;
	}
	return aniData;
}

void JOResConfig::clearAni()
{
	m_back_aniPathMap.clear();
	m_aniPaths.clear();
	m_aniMap.clear();
}

void JOResConfig::generateFilePathMap1(const std::string& filepath)
{
	JODataCoder* dc = JODataPool::Instance()->getDataCoder();	
	if (dc->readFile(filepath)){
		unsigned int len = dc->readUInt();
		std::string norpath;
		std::string md5path;
		for (unsigned int i = 0; i < len; i++){
			norpath = dc->readString();
			md5path = dc->readString();
			m_selfFilecodeMap[norpath] = md5path;
		}
	}
	JODataPool::Instance()->recover(dc);
}

void JOResConfig::generateFilePathMap2(const std::string& filepath)
{
	JODataCoder* dc = JODataPool::Instance()->getDataCoder();
	if (dc->readFile(filepath)){
		unsigned int len = dc->readUInt();
		std::string norpath;
		std::string md5path;
		for (unsigned int i = 0; i < len; i++){
			norpath = dc->readString();
			md5path = dc->readString();
			m_norFilecodeMap[norpath] = md5path;
		}
	}
	JODataPool::Instance()->recover(dc);
}

std::string JOResConfig::getRealFilePath(const std::string& filepath)
{
	std::unordered_map<std::string, std::string>::iterator itr = m_selfFilecodeMap.find(filepath);
	if (itr != m_selfFilecodeMap.end()){
		return "data/" + itr->second;
	}
	itr = m_norFilecodeMap.find(filepath);
	if (itr != m_norFilecodeMap.end()){
		return "data/" + itr->second;
	}
	return "";
}




NS_JOFW_END
