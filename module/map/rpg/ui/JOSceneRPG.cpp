
#include "module/map/rpg/ui/JOSceneRPG.h"
#include "module/map/rpg/JOMapRPGAsset.h"
#include "module/map/rpg/JOSceneRPGMgr.h"
#include "module/map/rpg/vo/JOMapRPGVO.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

enum{ BG_LAYER=1, DEBUG_LAYER, PLAYER_LAYER};
JOSceneRPG::JOSceneRPG() :centerX(0), centerY(0),
m_debugLayer(nullptr), m_bDebug(false),
bgLayer(nullptr), playerLayer(nullptr), hero(nullptr)
{

}

JOSceneRPG::~JOSceneRPG()
{
	
}


JOSceneRPG * JOSceneRPG::create()
{
	JOSceneRPG * ret = new (std::nothrow) JOSceneRPG();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}


bool JOSceneRPG::init()
{
	bool ret = Node::init();
	if (ret)
	{
		bgLayer = Node::create();
		addChild(bgLayer, BG_LAYER);
		playerLayer = Node::create();
		addChild(playerLayer, PLAYER_LAYER);

		EventListenerTouchOneByOne* touchListener = EventListenerTouchOneByOne::create();
		touchListener->setSwallowTouches(true);
		touchListener->onTouchBegan = CC_CALLBACK_2(JOSceneRPG::onTouchBegan, this);
		touchListener->onTouchMoved = CC_CALLBACK_2(JOSceneRPG::onTouchMoved, this);
		touchListener->onTouchEnded = CC_CALLBACK_2(JOSceneRPG::onTouchEnded, this);
		touchListener->onTouchCancelled = CC_CALLBACK_2(JOSceneRPG::onTouchEnded, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

		std::function<void(unsigned short)> removeCall = [&](unsigned short idx){
			bgLayer->removeChildByTag(idx);
		};
		std::function<void(void*, float, float, unsigned short)> loadCall = [&](void* tex, float x, float y, unsigned short idx){
			//bgLayer->removeChildByTag(idx);
			Sprite* s = Sprite::createWithTexture((Texture2D*)tex);
			s->setAnchorPoint(Point(0, 0));
			s->setPosition(x, y);
			s->setTag(idx);
			bgLayer->addChild(s);
		};
		JOSceneRPGMgr::Instance()->setRemoveTileCall(removeCall);
		JOSceneRPGMgr::Instance()->setLoadTileCall(loadCall);	
		Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
	}
	return ret;
}

void JOSceneRPG::loadMap(unsigned short mapId)
{
	JOSceneRPGMgr::Instance()->clearAllTile();
	JOMapRPGAsset::Instance()->loadMap(mapId);
	JOMapRPGVO* vo = JOMapRPGAsset::Instance()->getMapVO();
	if (!vo)
	{
		LOG_ERROR("JOSceneRPG", "mapVo is nil!");
		return;
	}

	setViewCenter(vo->mapWidth()*0.5, vo->mapHeight()*0.5);
	
	hero = LayerColor::create(Color4B::ORANGE);
	hero->setAnchorPoint(Point(0.5, 0.5));
	hero->setContentSize(Size(50, 50));
	hero->setPosition(vo->mapWidth()*0.5, vo->mapHeight()*0.5);
	playerLayer->addChild(hero);
}

void JOSceneRPG::setViewCenter(int x, int y)
{
	JOMapRPGVO* vo = JOMapRPGAsset::Instance()->getMapVO();
	if (!vo)
	{
		LOG_ERROR("JOSceneRPG", "mapVo is nil!");
		return;
	}
	if (centerX == x && centerY == y)
	{
		return;
	}
	centerX = x;
	centerY = y;

	JOSceneRPGMgr::Instance()->setOrigin(x, y);

	Size vs = Director::getInstance()->getVisibleSize();
	x -= vs.width*0.5;
	y -= vs.height*0.5;
	x = MAX(x, 0);
	y = MAX(y, 0);
	x = MIN(x, vo->mapWidth() - vs.width);
	y = MIN(y, vo->mapHeight() - vs.height);
	setPosition(-x, -y);
}

void JOSceneRPG::onEnter()
{
	Node::onEnter();
}

void JOSceneRPG::onExit()
{
	Node::onExit();
}

bool JOSceneRPG::onTouchBegan(Touch* touch, Event* event)
{
	return true;
}

void JOSceneRPG::onTouchMoved(Touch* touch, Event* event)
{

}

void JOSceneRPG::onTouchEnded(Touch* touch, Event* event)
{
	Point nsp = convertToNodeSpace(touch->getLocation());
	hero->stopAllActions();
	Point pos = hero->getPosition();
	float dis = pos.distance(nsp);
	float dur = (dis / 10 + (((int)dis % 10) ? 0 : 1))*0.01;
	MoveTo* mt = MoveTo::create(dur, nsp);
	hero->runAction(mt);	
}

void JOSceneRPG::update(float dt)
{
	if (hero)
	{
		Point pos = hero->getPosition();
		setViewCenter(pos.x, pos.y);
	}
}

void JOSceneRPG::setDebug(bool bDebug)
{
	m_bDebug = bDebug;
	
	if (!m_bDebug && m_debugLayer)
	{
		m_debugLayer->removeFromParent();
		m_debugLayer = nullptr;
	}
	else if (m_bDebug && !m_debugLayer)
	{
		m_debugLayer = Node::create();
		addChild(m_debugLayer, DEBUG_LAYER);
	}

	if (!m_debugLayer)
	{
		std::function<void(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned char)> call = nullptr;
		JOSceneRPGMgr::Instance()->setDebugCall(call);
	}
	else
	{
		std::function<void(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned char)>
			call = [&](unsigned short index, unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned char state){
			if (m_debugLayer)
			{
				if (w == 0 || h == 0)
				{
					m_debugLayer->removeChildByTag(index);
				}
				else
				{
					Color4B c4b;
					if (JOMapRPGVO::PASS == state)
					{
						c4b = Color4B::Color4B(10, 255, 55, 100);
					}
					else if (JOMapRPGVO::BLOCK == state)
					{
						c4b = Color4B::Color4B(255, 127, 0, 100);						
					}
					else
					{
						c4b = Color4B::Color4B(10, 55, 255, 100);
					}
					LayerColor* lc = LayerColor::create(c4b);					
					lc->ignoreAnchorPointForPosition(false);
					lc->setAnchorPoint(Point(0, 0));
					lc->setContentSize(Size::Size(w-2, h-2));
					lc->setPosition(x+1, y+1);
					lc->setTag(index);
					m_debugLayer->addChild(lc);
				}
			}			
		};
		JOSceneRPGMgr::Instance()->setDebugCall(call);
	}
}

NS_JOFW_END
