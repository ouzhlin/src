
#include "module/map/rpg/vo/JOMapRPGVO.h"
#include "utils/JOJsonHelper.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOMapRPGVO::JOMapRPGVO() :cellW(0), cellH(0), row(0), col(0), mapW(0), mapH(0), sliceW(0), sliceH(0), sliceWC(0), sliceHC(0), cellData(nullptr)
{
}

JOMapRPGVO::~JOMapRPGVO()
{
	JO_SAFE_DELETE_ARRAY(cellData);
}

void JOMapRPGVO::setMapVal(rapidjson::Document &root, unsigned short id)
{
	mapId = id;
	mapW = JOJsonHelper::Instance()->getIntValue(root, "mapW", 0);
	mapH = JOJsonHelper::Instance()->getIntValue(root, "mapH", 0);

	cellW = JOJsonHelper::Instance()->getIntValue(root, "mapGridW", 0); 
	cellH = JOJsonHelper::Instance()->getIntValue(root, "mapGridH", 0); 
	row = mapW / cellW + ((mapW%cellW==0) ? 0:1);
	col = mapH / cellH + ((mapH%cellH==0) ? 0:1);

	sliceW = JOJsonHelper::Instance()->getIntValue(root, "divideBlockW", 0);
	sliceH = JOJsonHelper::Instance()->getIntValue(root, "divideBlockH", 0);
	sliceWC = mapW / sliceW + ((mapW%sliceW==0) ? 0:1);
	sliceHC = mapH / sliceH + ((mapH%sliceH==0) ? 0:1);

	std::string	mapFlagArr = JOJsonHelper::Instance()->getStringValue(root, "mapFlagArr", "");
	/*
	std::vector<std::string > vc = JOString::split(mapFlagArr, ",");
	unsigned int dataSize = vc.size();

	JO_SAFE_DELETE_ARRAY(cellData);
	cellData = new unsigned char[dataSize];
	for (unsigned int i = 0; i < dataSize; i++)
	{
		cellData[i] = JOString::toUChat(vc[i].c_str());
	}
	cellDataSize = dataSize;
	*/
	unsigned char* arr = (unsigned char*)mapFlagArr.c_str();
	unsigned int dataSize = (mapFlagArr.size() + 1)*0.5;
	JO_SAFE_DELETE_ARRAY(cellData);
	cellData = new unsigned char[dataSize];
	unsigned int i = 0;
	while (i<dataSize)
	{
		memccpy(cellData + i, arr + i * 2, 1, 1);
		i++;
	}	
	cellDataSize = dataSize;
}

void JOMapRPGVO::addBlock(unsigned int index)
{
	if (index >= cellDataSize)
	{
		LOG_WARN("JOMapRPGVO", "idx[%d] >= dataSize[%d]", index, cellDataSize);
		return;
	}

	if (cellData[index] == BLOCK)
	{
		return;
	}

	ORG_FLAG_MAP::iterator itr = orgFlagMap.find(index);
	if (itr == orgFlagMap.end())
	{
		orgFlagMap[index] = cellData[index];
	}
	cellData[index] = BLOCK;
	
}

void JOMapRPGVO::removeBlock(unsigned int index)
{
	if (index >= cellDataSize)
	{
		LOG_WARN("JOMapRPGVO", "idx[%d] >= dataSize[%d]", index, cellDataSize);
		return;
	}

	if (cellData[index] != BLOCK)
	{
		return;
	}

	ORG_FLAG_MAP::iterator itr = orgFlagMap.find(index);
	if (itr == orgFlagMap.end())
	{
		orgFlagMap[index] = cellData[index];
	}
	cellData[index] = PASS;
}

void JOMapRPGVO::restoreDatas()
{
	ORG_FLAG_MAP::iterator itr = orgFlagMap.begin();
	while (itr != orgFlagMap.end())
	{
		cellData[itr->first] = itr->second;
		++itr;
	}
	orgFlagMap.clear();
}

unsigned char JOMapRPGVO::cellDataWithIndex(unsigned short index)
{
	if (index >= cellDataSize)
	{
		LOG_WARN("JOMapRPGVO", "idx[%d] >= dataSize[%d]", index, cellDataSize);
		return PASS;
	}
	return cellData[index];
}

NS_JOFW_END
