#include "manager/JOWinMgr.h"
#include "manager/JOSnMgr.h"
#include "ui/JOUILayout.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOWinMgr::JOWinMgr()
: m_root(nullptr)
, m_grayBgLayer(nullptr)
, m_grayDirty(false)
, m_sn(0)
, m_curScene(nullptr)
, m_curHome(nullptr)
, m_curWait(nullptr)
{
	m_sn = JOSnMgr::Instance()->getSn();
}

JOWinMgr::~JOWinMgr()
{
	JOSnMgr::Instance()->dispose(m_sn);
	clearAll();
	clearReg();
	CC_SAFE_RELEASE(m_grayBgLayer);
	CC_SAFE_RELEASE(m_root);
}

void JOWinMgr::init(Scene* s /*= nullptr*/)
{
	clearAll();
	CC_SAFE_RELEASE(m_root);
	Size visibleSize = Director::getInstance()->getVisibleSize();
	if (s==nullptr){
		s = Scene::create();
	}
	
	//s->setContentSize(visibleSize);
	m_root = s;
	m_root->retain();
	
	for (unsigned int i = JOWinMgr::SCENE; i < JOWinMgr::END; i++){
		Node* layer = Node::create();
		layer->setContentSize(visibleSize);
		layer->setTag(i);
		m_layerMap[i] = layer;
		m_root->addChild(layer, i);
		JOUILayout::Instance()->relativePos(layer, m_root);
	}
	EventListenerKeyboard* listener = EventListenerKeyboard::create();
	listener->onKeyReleased = [](EventKeyboard::KeyCode kcode, Event* pEvent){
		if (kcode==EventKeyboard::KeyCode::KEY_ESCAPE){
			Director::getInstance()->end();
		}
		else if (kcode == EventKeyboard::KeyCode::KEY_MENU){
		}
		else if (kcode == EventKeyboard::KeyCode::KEY_F5){
		}
		else if (kcode == EventKeyboard::KeyCode::KEY_BACKSPACE){
		}
	};
	m_root->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, m_root);
	
	if (m_grayBgLayer == nullptr){
		m_grayBgLayer = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width * 10, visibleSize.height * 10);
		m_grayBgLayer->retain();
	}
	else{
		m_grayBgLayer->setContentSize(visibleSize);
	}
}

void JOWinMgr::tick()
{
	if (m_grayDirty){
		__refreshGrayBg();
	}
}

Node* JOWinMgr::getLayer(int layerTag)
{
	return m_layerMap[layerTag];
}

void JOWinMgr::showWin(Node* win, int layerTag)
{
	if (win == nullptr){
		return;
	}
	else if (win->getParent()){
		LOG_ERROR("JOWinMgr", "win add layer Tag [%d] is readly have parent!!!!", layerTag);
		return;
	}
	Node* layer = m_layerMap[layerTag];
	if (layer == nullptr){
		LOG_ERROR("JOWinMgr", "layer Tag [%d] no base layer in it!!!!", layerTag);
		return;
	}
	switch (layerTag)
	{
	case JOWinMgr::SCENE:
		if (m_curScene==win){
			return;
		}
		clear(layerTag);
		m_curScene = win;
		win->addExitCall(m_sn, [=](Node* sender){
			m_curScene = nullptr;
		});
		break;
	case JOWinMgr::HOME:
		if (m_curHome == win){
			return;
		}
		clear(layerTag);
		m_curHome = win;
		win->addExitCall(m_sn, [=](Node* sender){
			m_curHome = nullptr;
		});
		break;
	case JOWinMgr::WIN:		
		m_logicWinList.push_back(win);		
		_refreshGrayBg(win);
		
		win->addExitCall(m_sn, [=](Node* sender){
			m_logicWinList.remove(sender);
			_refreshGrayBg(sender);
		});
		break;
	case JOWinMgr::ALERT:		
		if (m_alertWinList.empty()){
			layer->addChild(win);
			JOUILayout::Instance()->relativePos(win, layer);
			_refreshGrayBg(win);
		}
		win->retain();
		m_alertWinList.push_back(win);
		win->addExitCall(m_sn, [=](Node* sender){
			m_alertWinList.remove(sender);
			if (!m_alertWinList.empty()){
				Node* nextAlert = m_alertWinList.front();
				if (!nextAlert->getParent()){
					layer->addChild(nextAlert);
					JOUILayout::Instance()->relativePos(nextAlert, layer);
				}
			}			
			_refreshGrayBg(sender);
			sender->release();
		});
		return;
		break;
	case JOWinMgr::TIPS:
		_refreshGrayBg(win);
		win->addExitCall(m_sn, [=](Node* sender){
			_refreshGrayBg(sender);
		});
		break;
	case JOWinMgr::GUIDE:
		_refreshGrayBg(win);
		win->addExitCall(m_sn, [=](Node* sender){
			_refreshGrayBg(sender);
		});
		break;
	case JOWinMgr::WAIT:
		if (m_curWait == win){
			return;
		}
		clear(layerTag);
		m_curWait = win;
		_refreshGrayBg(win);
		win->addExitCall(m_sn, [=](Node* sender){
			_refreshGrayBg(sender);
			m_curWait = nullptr;
		});
		break;
	case JOWinMgr::NOTICE:
		break;
	case JOWinMgr::DEBUG_L:
		if (m_debugWait == win){
			return;
		}
		clear(layerTag);
		m_debugWait = win;
		_refreshGrayBg(win);
		win->addExitCall(m_sn, [=](Node* sender){
			_refreshGrayBg(sender);
			m_debugWait = nullptr;
		});
		break;
	default:
		break;
	}
	layer->addChild(win);
	JOUILayout::Instance()->relativePos(win, layer);	
	
}


void JOWinMgr::clearAll()
{
	for (int i = JOWinMgr::END - 1; i >= JOWinMgr::SCENE; i--){
		clear(i);
	}
}

void JOWinMgr::clear(int layerTag)
{
	Node* layer = m_layerMap[layerTag];
	if (layer == nullptr){
		LOG_WARN("JOWinMgr", "layer Tag [%d] no base layer in it!!!!", layerTag);
		return;
	}
	switch (layerTag)
	{
	case JOWinMgr::SCENE:
		m_curScene = nullptr;
		break;
	case JOWinMgr::HOME:
		m_curHome = nullptr;
		break;
	case JOWinMgr::WIN:
		m_logicWinList.clear();
		m_grayDirty = true;
		break;
	case JOWinMgr::ALERT:
	{
		WIN_LIST::iterator itr = m_alertWinList.begin();
		while (itr != m_alertWinList.end())	{
			(*itr)->release();
			++itr;
		}
		m_alertWinList.clear();
		m_grayDirty = true;
		break;
	}		
	case JOWinMgr::TIPS:
		m_grayDirty = true;
		break;
	case JOWinMgr::GUIDE:
		m_grayDirty = true;
		break;
	case JOWinMgr::WAIT:
		m_curWait = nullptr;
		m_grayDirty = true;
		break;
	case JOWinMgr::NOTICE:
		break;
	case JOWinMgr::DEBUG_L:
		m_debugWait = nullptr;
		m_grayDirty = true;
		break;
	}

	Vector<Node*> children = layer->getChildren();
	Vector<Node*>::iterator itr = children.begin();
	while (itr != children.end()){
		(*itr)->removeExitCall(m_sn);
		++itr;
	}
	layer->removeAllChildren();
}

void JOWinMgr::showSingleWin(Node* win)
{
	if (win){
		clear(JOWinMgr::WIN);
		showWin(win, JOWinMgr::WIN);
	}	
}

void JOWinMgr::removeWin(Node* win)
{
	if (win){
		m_logicWinList.remove(win);
		win->removeExitCall(m_sn);
		win->removeFromParent();
	}	
}

void JOWinMgr::removeTopWin()
{
	Node* win = m_logicWinList.back();
	removeWin(win);
}

void JOWinMgr::removeAlert(Node* win)
{
	if (win){
		m_alertWinList.remove(win);
		win->removeExitCall(m_sn);
		win->removeFromParent();
		win->release();
	}
}

void JOWinMgr::removeTopAlert()
{
	Node* win = m_alertWinList.front();
	if (win){
		win->removeFromParent();
	}
}

void JOWinMgr::_refreshGrayBg(Node* win)
{
	if (win && win->isGrayBackgroundEnable()){
		m_grayDirty = true;
	}
	else{
		m_grayDirty = false;
	}
}

void JOWinMgr::__refreshGrayBg()
{
	m_grayDirty = false;
	m_grayBgLayer->setVisible(false);
	Node* layer = nullptr;
	Node* node = nullptr;
	Vector<Node*> children;

	for (int i = JOWinMgr::END - 1; i > JOWinMgr::HOME;i--){
		layer = m_layerMap[i];
		if (layer){
			children = layer->getChildren();
			Vector<Node*>::reverse_iterator itr = children.rbegin();
			while (itr != children.rend()){
				node = *itr;
				if (node && node->isGrayBackgroundEnable())	{
					if (m_grayBgLayer->getParent() != node){
						m_grayBgLayer->removeFromParent();
						node->addChild(m_grayBgLayer, -999);
						JOUILayout::Instance()->relativePos(m_grayBgLayer, node);
					}
					m_grayBgLayer->setVisible(true);
					return;
				}
				++itr;
			}
		}
	}
}

void JOWinMgr::reSize()
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	if (m_root){		
		m_root->setContentSize(visibleSize);
		/*
		if (m_root->getParent()){
			JOUILayout::Instance()->relativePos(m_root, m_root->getParent());
		}
		*/
		Node* layer = nullptr;
        for (unsigned int i = JOWinMgr::SCENE; i < JOWinMgr::END; i++){
            layer = m_layerMap[i];
            if (layer){
                layer->setContentSize(visibleSize);
                JOUILayout::Instance()->relativePos(layer, m_root);
            }
        }
	}
	if (m_grayBgLayer){
		m_grayBgLayer->setContentSize(visibleSize);
		if (m_grayBgLayer->getParent()){
			JOUILayout::Instance()->relativePos(m_grayBgLayer, m_grayBgLayer->getParent());
		}
	}
}

void JOWinMgr::reg(unsigned int key, Node* win)
{
	WIN_MAP::iterator itr = m_winMap.find(key);
	if (itr != m_winMap.end()){
		LOG_ERROR("JOWinMgr", "key [%d] is alread reg!!!!");
		return;
	}
	m_winMap[key] = win;
	win->retain();
}

void JOWinMgr::unReg(unsigned int key)
{
	WIN_MAP::iterator itr = m_winMap.find(key);
	if (itr != m_winMap.end()){
		itr->second->release();
		m_winMap.erase(key);
	}
}

Node* JOWinMgr::getWin(unsigned int key)
{
	return m_winMap[key];
}

void JOWinMgr::clearReg()
{
	WIN_MAP::iterator itr = m_winMap.begin();
	while(itr != m_winMap.end()){
		itr->second->release();		
		++itr;
	}
	m_winMap.clear();
}

NS_JOFW_END
