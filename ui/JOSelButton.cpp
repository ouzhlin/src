#include "ui/JOSelButton.h"
#include "ui/JOUIDef.h"

NS_JOFW_BEGIN


JOSelButton* JOSelButton::create(const std::string& key, BUTTON_CALL call, const std::string& title /*= nullptr*/)
{
	JOSelButton* spr = new (std::nothrow) JOSelButton();
	if (spr && spr->init(key, call, title)){
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

bool JOSelButton::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	if (m_curState == JOButton::BTN_DIS){
		return false;
	}
	m_shouldState = m_curState;
	bool ret = Node::onTouchBegan(pTouch, pEvent);
	if (ret){
		if (m_curState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_curState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
	}
	return ret;
}
void JOSelButton::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	Node::onTouchMoved(pTouch, pEvent);
	if ( isTouchInside(pTouch) ){
		if (m_shouldState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_shouldState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
	}
	else{
		setBtnState(m_shouldState);
	}
}

void JOSelButton::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	Node::onTouchEnded(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	if (isTouchInside(pTouch)){
		if (m_shouldState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_shouldState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
		if (m_call){
			m_call(this, JOTouch::ENDED, pTouch);
		}
	}
	else{
		setBtnState(m_shouldState);
	}
}

void JOSelButton::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	Node::onTouchCancelled(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	if (isTouchInside(pTouch)){
		if (m_shouldState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_shouldState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
		if (m_call){
			m_call(this, JOTouch::ENDED, pTouch);
		}
	}
	else{
		setBtnState(m_shouldState);
	}
}

NS_JOFW_END