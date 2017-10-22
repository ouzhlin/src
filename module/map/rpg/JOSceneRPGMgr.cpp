#include "module/map/rpg/JOSceneRPGMgr.h"
#include "module/map/rpg/JOMapRPGAsset.h"
#include "module/map/rpg/vo/JOMapRPGVO.h"
#include "module/map/rpg/vo/JOMapRPGTileVO.h"

#include "utils/JOString.h"
#include "manager/JOCachePoolMgr.h"
#include "manager/JOSnMgr.h"
#include "core/datautils/JODataPool.h"
#include "core/datautils/JODataCoder.h"

#include "module/loader/JOAsynchMultLoader.h"

NS_JOFW_BEGIN

JOSceneRPGMgr::~JOSceneRPGMgr()
{
    JOSnMgr::Instance()->dispose(m_sn);
	clearAllTile();
}

JOSceneRPGMgr::JOSceneRPGMgr() :origin(JOPoint::ZERO), displaySize(JOSize::ZERO),
_removeTileCall(nullptr), _loadTileCall(nullptr)
{
    m_sn = JOSnMgr::Instance()->getSn();
}

void JOSceneRPGMgr::setDisplaySize(unsigned short w, unsigned short h)
{
	displaySize.width = w*1.25; displaySize.height = h*1.25;	
}

void JOSceneRPGMgr::setOrigin(float x, float y)
{
	origin.x = x; origin.y = y;
	_refreshRect();
}

void JOSceneRPGMgr::_refreshRect()
{
	JOMapRPGVO* mapVo = JOMapRPGAsset::Instance()->getMapVO();
	if (!mapVo)
	{
		clearAllTile();
		return ;
	}
		
	JOPoint offset(displaySize.width*0.5, displaySize.height*0.5);
	float tx = origin.x-offset.x;
	float ty = origin.y-offset.y;
	if (origin.x < offset.x)
	{
		tx = 0;
	}
	else if (origin.x >= mapVo->mapWidth() - offset.x)
	{
		tx = mapVo->mapWidth() - displaySize.width;
	}

	if (origin.y < offset.y)
	{
		ty = 0;
	}
	else if (origin.y >= mapVo->mapHeight() - offset.y)
	{
		ty = mapVo->mapHeight() - displaySize.height;
	}
	
	static JOPoint static_slice(-1, -1);
	JOPoint slice = JOMapRPGAsset::Instance()->map2slice(tx, ty);
	JOPoint temp(slice);
	if (!static_slice.equals(slice))
	{
		static_slice = slice;
		/*
		图块显示的行列数
		*/
		unsigned short tempW = displaySize.width - (slice.x*mapVo->sliceWidth() - tx);
		unsigned short tempH = displaySize.height - (slice.y*mapVo->sliceHeight() - ty);

		unsigned short colBlocks = tempH / mapVo->sliceHeight() + (((int)tempH % mapVo->sliceHeight()) ? 1 : 0)+1;
		unsigned short rowBlocks = tempW / mapVo->sliceWidth() + (((int)tempW % mapVo->sliceWidth()) ? 1 : 0)+1;
		for (short i = 0; i < rowBlocks; i++)
		{
			for (short j = 0; j < colBlocks; j++)
			{
				JOPoint::add(slice, JOPoint(i, j), &temp);
				unsigned short idx = JOMapRPGAsset::Instance()->getSliceTileIdx(temp.x, temp.y);
				TILE_MAP::iterator itr = tileMap.find(idx);
				if (itr != tileMap.end())
				{
					itr->second->bCurRound = true;
				}
				else
				{
					JOMapRPGTileVO* vo = POOL_GET(JOMapRPGTileVO, "JOSceneRPGMgr");
					JOPoint mapSlice = JOMapRPGAsset::Instance()->slice2Map(temp.x, temp.y);
					vo->x = mapSlice.x - mapVo->sliceWidth()*0.5;
					vo->y = mapSlice.y - mapVo->sliceHeight()*0.5;
					vo->colX = temp.x;
					vo->rowY = temp.y;
					vo->mapId = mapVo->getMapId();
					vo->key = JOString::formatString("%d_%d_%d.jpg", vo->mapId, vo->rowY - 1, vo->colX - 1);
					vo->index = idx;
					vo->bLoaded = false;
					vo->bCurRound = true;
					tileMap[idx] = vo;
				}
			}
		}
		_load();
	}

	
	

	if (_debugCall)
	{
		
		static JOPoint static_cell(-1, -1);
		JOPoint cell = JOMapRPGAsset::Instance()->map2Coord(tx, ty);		
		if (!static_cell.equals(cell))
		{
			static_cell = cell;
			unsigned short tempW = displaySize.width - (cell.x*mapVo->cellWidth() - tx);
			unsigned short tempH = displaySize.height - (cell.y*mapVo->cellHeight() - ty);
			unsigned short colBlocks = tempH / mapVo->cellHeight() + (((int)tempH % mapVo->cellHeight()) ? 1 : 0) + 1;
			unsigned short rowBlocks = tempW / mapVo->cellWidth() + (((int)tempW % mapVo->cellWidth()) ? 1 : 0) + 1;
			for (short i = 0; i < rowBlocks; i++)
			{
				for (short j = 0; j < colBlocks; j++)
				{
					JOPoint::add(cell, JOPoint(i, j), &temp);
					unsigned short idx = JOMapRPGAsset::Instance()->getCellTileIdx(temp.x, temp.y);
					TILE_MAP::iterator itr = debugTileMap.find(idx);
					if (itr != debugTileMap.end())
					{
						itr->second->bCurRound = true;
					}
					else
					{
						JOMapRPGTileVO* vo = POOL_GET(JOMapRPGTileVO, "JOSceneRPGMgr");
						JOPoint mapCell = JOMapRPGAsset::Instance()->coord2Map(temp.x, temp.y);
						vo->x = mapCell.x - mapVo->cellWidth()*0.5;
						vo->y = mapCell.y - mapVo->cellHeight()*0.5;
						vo->index = idx;
						vo->bLoaded = false;
						vo->bCurRound = true;
						debugTileMap[idx] = vo;
					}
				}
			}

			TILE_MAP::iterator itr = debugTileMap.begin();
			while (itr != debugTileMap.end())
			{
				JOMapRPGTileVO* vo = itr->second;
				if (!vo->bCurRound)
				{
					_debugCall(vo->index, 0, 0, 0, 0, 0);
                    POOL_RECOVER(vo, JOMapRPGTileVO, "JOSceneRPGMgr");
					itr = debugTileMap.erase(itr);
					continue;
				}

				vo->bCurRound = false;
				if (!vo->bLoaded)
				{
					vo->bLoaded = true;
					unsigned char state = JOMapRPGAsset::Instance()->cellStateWithIndex(vo->index);
					_debugCall(vo->index, vo->x, vo->y, mapVo->cellWidth(), mapVo->cellHeight(), state);
				}
				++itr;
			}
		}		
	}
}


void JOSceneRPGMgr::_load()
{
	TILE_MAP::iterator itr = tileMap.begin();
	while (itr != tileMap.end())
	{
		JOMapRPGTileVO* vo = itr->second;
		if (!vo->bCurRound)
		{
			if (_removeTileCall)
			{
				_removeTileCall(vo->index);
			}
            POOL_RECOVER(vo, JOMapRPGTileVO, "JOSceneRPGMgr");

			itr = tileMap.erase(itr);
			continue;
		}

		vo->bCurRound = false;
		if (!vo->bLoaded)
		{
			CompLeteCall query = [&](cocos2d::Texture2D* tex, std::string source, short resType, JODataCoder* dCoder, unsigned short idx, unsigned short total){
				if (_loadTileCall)
				{
                    unsigned int x = dCoder->readUInt();
					unsigned int y = dCoder->readUInt();
					unsigned int idx = dCoder->readUInt();
					_loadTileCall(tex, x, y, idx);
				}
                JODataPool::Instance()->recover(dCoder);
			};
			vo->bLoaded = true;
            
			std::string imgPath = JOString::formatString("%s/%d/%s", JOMapRPGAsset::Instance()->getMapConfigPath().c_str(), vo->mapId, vo->key.c_str());
            JODataCoder* dCoder = JODataPool::Instance()->getDataCoder();
            dCoder->writeUInt(vo->x);
            dCoder->writeUInt(vo->y);
            dCoder->writeUInt(vo->index);
						
            JOAsynchMultLoader::Instance()->load(m_sn, imgPath, JOAsynchBaseLoader::RES_IMG, query, dCoder);
            
            void load(unsigned int sn, const std::string srcPath, short resType, const CompLeteCall loadCompleteCall, JODataCoder* dataCoder = nullptr, cocos2d::Texture2D::PixelFormat pixel = cocos2d::Texture2D::PixelFormat::NONE);
		}
		++itr;
	}	
}

void JOSceneRPGMgr::clearAllTile()
{
	TILE_MAP::iterator itr = tileMap.begin();
	while (itr != tileMap.end())
	{
		if (_removeTileCall)
		{
			_removeTileCall(itr->second->index);
		}
        POOL_RECOVER(itr->second, JOMapRPGTileVO, "JOSceneRPGMgr");
		++itr;
	}
	tileMap.clear();

	itr = debugTileMap.begin();
	while (itr != debugTileMap.end())
	{
		if (_debugCall)
		{
			_debugCall(itr->second->index, 0, 0, 0, 0, 0);
		}
        POOL_RECOVER(itr->second, JOMapRPGTileVO, "JOSceneRPGMgr");
		++itr;
	}
	debugTileMap.clear();

}

void JOSceneRPGMgr::setRemoveTileCall(std::function<void(unsigned short)>& call)
{
	_removeTileCall = call;
}

void JOSceneRPGMgr::setLoadTileCall(std::function<void(void*, float, float, unsigned short)>& call)
{
	_loadTileCall = call;
}

void JOSceneRPGMgr::setDebugCall(std::function<void(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned char)>& call)
{
	_debugCall = call;
}

NS_JOFW_END
