#include "module/map/rpg/JOMapRPGAsset.h"
#include "module/map/rpg/vo/JOMapRPGVO.h"
#include "utils/JOJsonHelper.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOMapRPGAsset::JOMapRPGAsset() :curMapVO(nullptr), mapPath(""),
maxSliceLen(0), maxCellLen(0)
{
}

JOMapRPGAsset::~JOMapRPGAsset()
{
	VO_MAP::iterator itr = voMap.begin();
	while (itr != voMap.end())
	{
		delete itr->second;
		itr++;
	}
	voMap.clear();
}

JOMapRPGVO* JOMapRPGAsset::loadMap(unsigned short mapId)
{
	curMapVO = _getMapVO(mapId);

	static JOAStar::QueryFunction query = [&](const JOAStar::Vec2 &pos)->bool
	{
		return (curMapVO->cellDatas()[pos.x + pos.y] == JOMapRPGVO::BLOCK);
	};

	static JOAStar::GValFunction gVal = [&](const JOAStar::Vec2 &pos)->bool
	{
		return 0;
	};
	if (curMapVO)
	{
		astarMachine.setSize(curMapVO->rows(), curMapVO->cols());
		astarMachine.setAllowCorner(true);
		astarMachine.setCanReachCall(query);
		//astarMachine.setGValCall(gVal);

		maxSliceLen = curMapVO->sliceWidthC()*curMapVO->sliceHeightC();
		maxCellLen = curMapVO->rows()*curMapVO->cols();
	}

	return curMapVO;
}


bool JOMapRPGAsset::createPath(JOPoint& pos, JOPoint& targetPos, std::deque<JOPoint>& path)
{
	if (curMapVO)
	{
		path = astarMachine.search(pos, targetPos);
		return (!path.empty());		
	}
	return false;
}

JOMapRPGVO* JOMapRPGAsset::_getMapVO(unsigned short mapId)
{
	VO_MAP::iterator itr = voMap.find(mapId);
	if (itr != voMap.end())
	{
		return itr->second;
	}
	rapidjson::Document root;
	bool ret = JOJsonHelper::Instance()->getFileDictionary(root, JOString::formatString("%s/%d/%d.json", mapPath.c_str(), mapId, mapId).c_str());
	if (!ret)
	{
		LOG_ERROR("JOMapRPGAsset", "int [%s] id[%d] can't load map data", mapPath.c_str(), mapId);
		return nullptr;
	}
	JOMapRPGVO* vo = new JOMapRPGVO();
	vo->setMapVal(root, mapId);
	voMap[mapId] = vo;

	return vo;
}

JOPoint JOMapRPGAsset::coord2Map(unsigned short cellx, unsigned short celly)
{
	if (curMapVO == nullptr)
	{
		return JOPoint::ONE;
	}
	if (cellx > curMapVO->rows() || celly > curMapVO->cols())
	{
		return JOPoint::ONE;
	}
	float x = floor((cellx - 1)*curMapVO->cellWidth() + curMapVO->cellWidth()*0.5);
	float y = floor((celly - 1)*curMapVO->cellHeight() + curMapVO->cellHeight()*0.5);
	return JOPoint(x, y);
}

JOPoint JOMapRPGAsset::map2Coord(unsigned short mapx, unsigned short mapy)
{
	if (curMapVO == nullptr)
	{
		return JOPoint::ZERO;
	}
	int cellx = 1;
	int celly = 1;
	if (mapx)
	{
		cellx = floor(mapx / curMapVO->cellWidth()) + ((mapx%curMapVO->cellWidth() == 0) ? 0 : 1);
	}
	if (mapy)
	{
		celly = floor(mapy / curMapVO->cellHeight()) + ((mapy%curMapVO->cellHeight() == 0) ? 0 : 1);
	}
	
	if (cellx > curMapVO->rows() || celly > curMapVO->cols())
	{
		return JOPoint::ONE;
	}
	return JOPoint(cellx, celly);
}

JOPoint JOMapRPGAsset::slice2Map(unsigned short slicex, unsigned short slicey)
{
	if (curMapVO == nullptr)
	{
		return JOPoint::ONE;
	}
	if (slicex > curMapVO->sliceWidthC() || slicey > curMapVO->sliceHeightC())
	{
		return JOPoint::ONE;
	}
	float x = floor((slicex - 1)*curMapVO->sliceWidth() + curMapVO->sliceWidth()*0.5);
	float y = floor((slicey - 1)*curMapVO->sliceHeight() + curMapVO->sliceHeight()*0.5);
	return JOPoint(x, y);
}

JOPoint JOMapRPGAsset::map2slice(unsigned short mapx, unsigned short mapy)
{
	if (curMapVO == nullptr)
	{
		return JOPoint::ZERO;
	}
	int slicex = 1;
	int slicey = 1;
	if (mapx)
	{
		slicex = floor(mapx / curMapVO->sliceWidth()) + ((mapx%curMapVO->sliceWidth() == 0) ? 0 : 1);
	}
	if (mapy)
	{
		slicey = floor(mapy / curMapVO->sliceHeight()) + ((mapy%curMapVO->sliceHeight() == 0) ? 0 : 1);
	}
	
	if (slicex > curMapVO->sliceWidthC() || slicey > curMapVO->sliceHeightC())
	{
		return JOPoint::ONE;
	}
	return JOPoint(slicex, slicey);
}


unsigned short JOMapRPGAsset::getSliceTileIdx(unsigned short tileRow, unsigned short tileCol)
{
	unsigned short idx = -1;
	if (curMapVO)
	{
		idx = curMapVO->sliceWidthC()*(tileCol-1) + tileRow;	
		if (idx > maxSliceLen)
		{
			idx = maxSliceLen;
		}
	}
	return idx;
}

unsigned short JOMapRPGAsset::getCellTileIdx(unsigned short tileRow, unsigned short tileCol)
{
	unsigned short idx = -1;
	if (curMapVO)
	{
		idx = curMapVO->rows()*(tileCol - 1) + tileRow;		
		if (idx > maxCellLen)
		{
			idx = maxCellLen;
		}
	}
	return idx;
}

unsigned char JOMapRPGAsset::cellStateWithIndex(unsigned short index)
{
	if (curMapVO)
	{
		return curMapVO->cellDataWithIndex(index-1);
	}
	return JOMapRPGVO::PASS;
}

unsigned char JOMapRPGAsset::cellStateWithCoord(unsigned short cellx, unsigned short celly)
{
	unsigned short idx = getCellTileIdx(cellx, celly);
	return cellStateWithIndex(idx);
}

unsigned char JOMapRPGAsset::cellStateWithMapXY(unsigned short mapx, unsigned short mapy)
{
	JOPoint coord = map2Coord(mapx, mapy);
	return cellStateWithCoord(coord.x, coord.y);
}

void JOMapRPGAsset::filterSameLine(std::deque<JOPoint>& oldPath, std::deque<JOPoint>& newPath)
{
	newPath.clear();
	unsigned int oldPathCount = oldPath.size();
	if (oldPathCount < 2)
	{		
		return;
	}
	
	JOPoint sPoint = oldPath[0];
	JOPoint tempPoint = oldPath[1];
	newPath.push_back(sPoint);
	
	/*
	用于标志x1-x2是否等于0， 0为true
	*/
	bool disX = true;
	/*
	斜率
	*/
	float k = 0;
	float tempk = 0;
	if (tempPoint.x != sPoint.x)
	{
		disX = false;
		k = (tempPoint.y - sPoint.y) / (tempPoint.x - sPoint.x);
	}	

	for (unsigned int i = 2; i < oldPathCount; i++)
	{
		/*
		在同一直线时
		*/
		if (sPoint.x == oldPath[i].x && disX == true)
		{
			tempPoint = oldPath[i];			
		}
		/*
		是在另一直线上
		*/
		else if (sPoint.x == oldPath[i].x && disX == false)
		{			
			newPath.push_back(tempPoint);
			sPoint = tempPoint;
			tempPoint = oldPath[i];

			disX = true;
			if (tempPoint.x != sPoint.x)
			{
				disX = false;
				k = (tempPoint.y - sPoint.y) / (tempPoint.x - sPoint.x);
			}
		}
		else
		{
			disX = false;
			tempk = (oldPath[i].y - sPoint.y) / (oldPath[i].x - sPoint.x);
			/*
			在同一直线时
			*/
			if (tempk == k)
			{
				tempPoint = oldPath[i];				
			}
			/*
			是在另一直线上
			*/
			else
			{
				k = tempk;
				newPath.push_back(tempPoint);
				sPoint = tempPoint;
				tempPoint = oldPath[i];
			}
		}
	}
}

NS_JOFW_END
