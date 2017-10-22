#include "ui/JOButton.h"
#include "ui/JOSprite.h"
#include "ui/JOScale9Sprite.h"
#include "ui/JOLabel.h"
#include "ui/JOUILayout.h"
#include "ui/JOUIDef.h"

#include "manager/JOShaderMgr.h"
#include "utils/JOLog.h"

#include "cocos2d.h"
USING_NS_CC;
#include "extensions/GUI/CCScrollView/CCScrollView.h"

NS_JOFW_BEGIN

short		JOButton::s_zoomMode = JOButton::ZOOM_NONE;
std::string	JOButton::s_norShaderKey = "";
std::string	JOButton::s_selShaderKey = "gamma_btn_sel";
std::string	JOButton::s_disShaderKey = "gray";
float		JOButton::s_fsize = 24;
std::string	JOButton::s_falias = "";
Color3B		JOButton::s_fcolor = Color3B::WHITE;
short		JOButton::s_boldsize = 2;
Color4B		JOButton::s_outcolor = Color4B::WHITE;


void JOButton::setDefaultBase(short zoomMode)
{
	s_zoomMode = zoomMode;
}

void JOButton::setDefaultShader(const std::string& norKey, const std::string& selKey, const std::string& disKey)
{
	s_norShaderKey = norKey;
	s_selShaderKey = selKey;
	s_disShaderKey = disKey;
}

void JOButton::setDefaultTitleBase(float fsize, const std::string& falias, Color3B fcolor)
{
	s_fsize = fsize;
	s_falias = falias;
	s_fcolor = fcolor;
}

void JOButton::setDefaultTitleOutline(short boldsize, Color4B outcolor)
{
	s_boldsize = boldsize;
	s_outcolor = outcolor;
}
//////////////////////////////////////////////////////////////////////////
JOButton::JOButton()
: m_img(nullptr)
, m_scaleImg(nullptr)
, m_call(nullptr)
, m_bScale(false)
, m_title(nullptr)
, m_curState(JOButton::BTN_NOR)
, m_curKey("")
, m_norKey("")
, m_selKey("")
, m_disKey("")
, m_norShaderKey(s_norShaderKey)
, m_selShaderKey(s_selShaderKey)
, m_disShaderKey(s_disShaderKey)
, m_norShaderValCall(nullptr)
, m_selShaderValCall(nullptr)
, m_disShaderValCall(nullptr)
, m_bCanTouch(true)
, m_fTouchOrgScaleX(1)
, m_fTouchOrgScaleY(1)
, m_zoomBegan(nullptr)
, m_zoomEnd(nullptr)
, m_zoomMode(s_zoomMode)
, m_bHandleAllTouch(false)
{
	Node::setCascadeColorEnabled(true);
}

JOButton::~JOButton()
{
	CC_SAFE_RELEASE(m_zoomBegan);
	CC_SAFE_RELEASE(m_zoomEnd);
	//CC_SAFE_RELEASE(m_img);
	//CC_SAFE_RELEASE(m_scaleImg);
	//CC_SAFE_RELEASE(m_title);
}


JOButton* JOButton::create(const std::string& key /*= nullptr*/)
{
	JOButton* spr = new (std::nothrow) JOButton();
	if (spr){
		spr->setKey(key);
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

JOButton* JOButton::create(const std::string& key, BUTTON_CALL call, bool bScale/*=false*/, const std::string& title /*= nullptr*/)
{
	JOButton* spr = new (std::nothrow) JOButton();
	if (spr && spr->init(key, call, bScale, title)){
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

bool JOButton::init(const std::string& key, BUTTON_CALL call, bool bScale /*= false*/, const std::string& title /*= nullptr*/)
{
	setAnchorPoint(Point(0.5f, 0.5f));
	m_bScale = bScale;
	m_call = call;
	setKey(key);
	setTitle(title);
	setCatchTouch(true);
	return true;
}

void JOButton::setKey(const std::string& key)
{
	/*
	if (key==""){
		LOG_WARN("JOButton", "key==nullptr");
	}
	*/
	if (m_norKey==key){
		return;
	}
	m_norKey = key;
	
	if (m_selKey.empty()){
		m_selKey = m_norKey;
	}
	if (m_disKey.empty()){
		m_disKey = m_norKey;
	}
	if (m_curState==JOButton::BTN_NOR){
		_setKey(m_norKey);
	}
	else if (m_curState == JOButton::BTN_SEL){
		_setKey(m_selKey);
	}
	else if (m_curState == JOButton::BTN_DIS){
		_setKey(m_disKey);
	}
}

void JOButton::setScaleEnable(bool enable)
{
	if (m_bScale == enable){
		return;
	}
	m_bScale = enable;
	int tmp = m_curState;
	m_curState = -1;
	setBtnState(tmp);
	//setKey(m_norKey.c_str());
}

void JOButton::setTitle(const std::string& title)
{
	if (!title.empty()){
		if (m_title == nullptr)	{
			m_title = JOLabel::create(title, s_fsize, s_fcolor, s_falias);
			m_title->setOutline(s_boldsize, s_outcolor);
			addChild(m_title, 10);
		}
		else{
			m_title->setString(title);
		}
		m_title->setVisible(true);
	}
	else{
		if (m_title){
			m_title->setVisible(false);
		}
	}
	_layout();
}

void JOButton::setTitleArg(const std::string& fName, short fSize, Color3B fColor /*= Color3B::WHITE*/, short blodSize /*= 0*/, Color4B outColor /*= Color4B::WHITE*/)
{
	if (m_title == nullptr)	{
		m_title = JOLabel::create("", fSize, fColor, fName);
		addChild(m_title, 10);
	}
	else{
		m_title->init(m_title->getString(), fSize, fColor, fName);
	}	
	m_title->setOutline(blodSize, outColor);
	_layout();
}

std::string JOButton::getTitleString()
{
	if (m_title){
		return m_title->getString();
	}
	return "";
}

void JOButton::setBtnState(short state)
{
	if (m_curState==state){
		return;
	}
	m_curState = state;
	std::string tmpKey;
	std::string tmpShaderKey;
	std::function<void(Node*, cocos2d::GLProgramState*)> tmpShaderValCall = nullptr;
	if (m_curState==JOButton::BTN_NOR){
		tmpKey = m_norKey;
		tmpShaderKey = m_norShaderKey;
		tmpShaderValCall = m_norShaderValCall;
	}
	else if (m_curState==JOButton::BTN_SEL){
		tmpKey = m_selKey;
		tmpShaderKey = m_selShaderKey;
		tmpShaderValCall = m_selShaderValCall;
	}
	else if (m_curState==JOButton::BTN_DIS){
		tmpKey = m_disKey;
		tmpShaderKey = m_disShaderKey;	 
		tmpShaderValCall = m_disShaderValCall;
	}
	_setKey(tmpKey);
	_touchScaleCancelHandle();
	if (m_zoomMode!=JOButton::ZOOM_NONE){
		if (m_curState==JOButton::BTN_NOR){
			_touchScaleEndHandle();
		}
		else if(m_curState == JOButton::BTN_SEL){
			_touchScaleBeginHandle();
		}
	}
	else{
		_setShader(tmpShaderKey, true, tmpShaderValCall);
	}
}

void JOButton::setZoomMode(short zoomMode)
{
	m_zoomMode = zoomMode;
}

//////////////////////////////////////////////////////////////////////////
void JOButton::setContentSize(const Size& contentSize)
{
	Node::setContentSize(contentSize);
	if (m_bScale && m_scaleImg){
		m_scaleImg->setContentSize(contentSize);
	}
	_layout();
}

bool JOButton::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	bool ret = Node::onTouchBegan(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return false;
	}
	if (ret){
        Node* tmpParent = getParent();
        while (tmpParent){
             cocos2d::extension::ScrollView* layoutParent = dynamic_cast<cocos2d::extension::ScrollView*>(tmpParent);
             if (layoutParent){
                 if (!layoutParent->isTouchInside(pTouch)){
                     return false;
                 }
             }
            tmpParent = tmpParent->getParent();
        }
         
		setBtnState(JOButton::BTN_SEL);
		if (m_bHandleAllTouch && m_call){
			m_call(this, JOTouch::BEGAN, pTouch);
		}
	}
	else{
		setBtnState(JOButton::BTN_NOR);
	}
	return ret;
}

void JOButton::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	Node::onTouchMoved(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return ;
	}
	if (isTouchInside(pTouch)){
		setBtnState(JOButton::BTN_SEL);
	}
	else{
		setBtnState(JOButton::BTN_NOR);
	}
	if (m_bHandleAllTouch && m_call){
		m_call(this, JOTouch::MOVE, pTouch);
	}
}

void JOButton::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	Node::onTouchEnded(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	setBtnState(JOButton::BTN_NOR);
	if (isTouchInside(pTouch) && m_call){
		m_call(this, JOTouch::ENDED, pTouch);
	}
}

void JOButton::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	Node::onTouchCancelled(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	setBtnState(JOButton::BTN_NOR);
	if (m_bHandleAllTouch && m_call){
		m_call(this, JOTouch::CANCELLED, pTouch);
	}
	/*
	if (isTouchInside(pTouch) && m_call){
	m_call(this);
	}
	*/
}
//////////////////////////////////////////////////////////////////////////
void JOButton::_layout()
{
	if (m_scaleImg){
		JOUILayout::Instance()->relativePos(m_scaleImg, this);
	}
	if (m_img){
		JOUILayout::Instance()->relativePos(m_img, this);
	}
	if (m_title){
		JOUILayout::Instance()->relativePos(m_title, this);
	}
}

void JOButton::setOtherStateKey(const std::string& selKey, const std::string& disKey)
{
	if (m_selKey==selKey && m_disKey==disKey){
		return;
	}
	m_selKey = selKey;
	m_disKey = disKey;
	if (m_curState!=JOButton::BTN_NOR){
		short tmpState = m_curState;
		m_curState = -1;
		setBtnState(tmpState);
	}
}

void JOButton::setNorShaderKey(const std::string& key, std::function<void(Node*, cocos2d::GLProgramState*)> setValCall/* = nullptr*/)
{
	m_norShaderValCall = setValCall;
	if (m_norShaderKey==key){
		return;
	}
	m_norShaderKey = key;
	
	if (m_curState==JOButton::BTN_NOR){
		short tmpState = m_curState;
		m_curState = -1;
		setBtnState(tmpState);
	}
}

void JOButton::setSelShaderKey(const std::string& key, std::function<void(Node*, cocos2d::GLProgramState*)> setValCall/* = nullptr*/)
{
	m_selShaderValCall = setValCall;
	if (m_selShaderKey == key){
		return;
	}
	m_selShaderKey = key;

	if (m_curState == JOButton::BTN_SEL){
		short tmpState = m_curState;
		m_curState = -1;
		setBtnState(tmpState);
	}
}

void JOButton::setDisShaderKey(const std::string& key, std::function<void(Node*, cocos2d::GLProgramState*)> setValCall/* = nullptr*/)
{
	m_disShaderValCall = setValCall;
	if (m_disShaderKey == key){
		return;
	}
	m_disShaderKey = key;

	if (m_curState == JOButton::BTN_DIS){
		short tmpState = m_curState;
		m_curState = -1;
		setBtnState(tmpState);
	}
}

void JOButton::_setKey(std::string& key)
{
	if (m_curKey == key){
		return;
	}
	m_curKey = key;
	if (m_scaleImg){
		m_scaleImg->setVisible(false);
	}
	if (m_img){
		m_img->setVisible(false);
	}
	if (m_bScale){
		if (m_scaleImg == nullptr){
			m_scaleImg = JOScale9Sprite::create();
			addChild(m_scaleImg, 1);
		}
		m_scaleImg->setVisible(true);
		m_scaleImg->setKey(m_curKey,false);
		if (getContentSize().equals(Size::ZERO)){
			Node::setContentSize(m_scaleImg->getContentSize());
		}
		else{
			m_scaleImg->setContentSize(getContentSize());
		}
	}
	else{
		if (m_img == nullptr){
			m_img = JOSprite::create();
			addChild(m_img, 1);
		}
		m_img->setVisible(true);
		m_img->setKey(m_curKey,false);
		if (getContentSize().equals(Size::ZERO)){
			Node::setContentSize(m_img->getContentSize());
		}
	}
	_layout();
}

void JOButton::_setShader(const std::string& key, bool bRestore, std::function<void(Node*, cocos2d::GLProgramState*)> setValCall)
{
	if (m_scaleImg){
		m_scaleImg->setVisible(false);
	}
	if (m_img){
		m_img->setVisible(false);
	}
	Node* tmpShaderNode = nullptr;
	Size tmpSize = getContentSize();
	if (m_bScale){
		if (m_scaleImg == nullptr){
			m_scaleImg = JOScale9Sprite::create();
			addChild(m_scaleImg, 1);
			m_scaleImg->setKey(m_curKey, false);
		}
		m_scaleImg->setVisible(true);		
		if (tmpSize.equals(Size::ZERO)){
			Node::setContentSize(m_scaleImg->getContentSize());
		}
		else{
			m_scaleImg->setContentSize(tmpSize);
		}
		tmpShaderNode = m_scaleImg;
	}
	else{
		if (m_img == nullptr){
			m_img = JOSprite::create();
			addChild(m_img, 1);
			m_img->setKey(m_curKey, false);
		}
		m_img->setVisible(true);		
		if (tmpSize.equals(Size::ZERO)){
			Node::setContentSize(m_img->getContentSize());
		}
		tmpShaderNode = m_img;
	}
	_layout();
	if (!key.empty()){
		JOShaderMgr::Instance()->shader(tmpShaderNode, key.c_str(), setValCall);
	}
	else if (bRestore){
		JOShaderMgr::Instance()->restore(tmpShaderNode);
	}
}


void JOButton::_touchScaleBeginHandle()
{
	if (m_bCanTouch && m_zoomMode!=JOButton::ZOOM_NONE){
		m_bCanTouch = false;
		float tmpScaleX = getScaleX();
		float tmpScaleY = getScaleY();
		if (m_fTouchOrgScaleX != tmpScaleX || m_fTouchOrgScaleY != tmpScaleY || m_zoomBegan==nullptr || m_zoomEnd==nullptr){
			m_fTouchOrgScaleX = tmpScaleX;
			m_fTouchOrgScaleY = tmpScaleY;

			CC_SAFE_RELEASE(m_zoomBegan);
			if (m_zoomMode == JOButton::ZOOM_BIG)	{
				m_zoomBegan = ScaleTo::create(0.1f, m_fTouchOrgScaleX + m_fTouchOrgScaleX*0.05f, m_fTouchOrgScaleY + m_fTouchOrgScaleY*0.05f);
			}
			else if (m_zoomMode == JOButton::ZOOM_SMALL){
				m_zoomBegan = ScaleTo::create(0.1f, m_fTouchOrgScaleX - m_fTouchOrgScaleX*0.05f, m_fTouchOrgScaleY - m_fTouchOrgScaleY*0.05f);
			}
			m_zoomBegan->retain();

			CC_SAFE_RELEASE(m_zoomEnd);
			m_zoomEnd = Sequence::create(ScaleTo::create(0.1f, m_fTouchOrgScaleX, m_fTouchOrgScaleY), CallFunc::create([=](){
				m_bCanTouch = true;
			}), nullptr);
			m_zoomEnd->retain();
		}
		
		runAction(m_zoomBegan);		
	}
}

void JOButton::_touchScaleEndHandle()
{
	if (m_zoomMode != JOButton::ZOOM_NONE && m_zoomEnd){
		runAction(m_zoomEnd);
	}
}

void JOButton::_touchScaleCancelHandle()
{
	if (m_zoomBegan){
		stopAction(m_zoomBegan);
	}
	if (m_zoomEnd){
		stopAction(m_zoomEnd);
	}
	if (m_zoomMode != JOButton::ZOOM_NONE){
		setScaleX(m_fTouchOrgScaleX);
		setScaleY(m_fTouchOrgScaleY);
	}
	m_bCanTouch = true;
}



NS_JOFW_END
