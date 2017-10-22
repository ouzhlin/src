#include "ui/JOSelButton.h"
#include "ui/JOUIDef.h"

NS_JOFW_BEGIN


JOSelButton* JOSelButton::create(const std::string& key, BUTTON_CALL call, bool bScale /*= false*/, const std::string& title /*= nullptr*/)
{
	JOSelButton* spr = new (std::nothrow) JOSelButton();
	if (spr && spr->init(key, call, bScale, title)){
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

bool JOSelButton::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	return Node::onTouchBegan(pTouch, pEvent);
}
void JOSelButton::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	Node::onTouchMoved(pTouch, pEvent);
}

void JOSelButton::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	Node::onTouchEnded(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	if (isTouchInside(pTouch)){
		if (m_curState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_curState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
		if (m_call){
			m_call(this, JOTouch::ENDED, pTouch);
		}
	}
}

void JOSelButton::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	Node::onTouchCancelled(pTouch, pEvent);
	if (m_curState == JOButton::BTN_DIS){
		return;
	}
	if (isTouchInside(pTouch)){
		if (m_curState == JOButton::BTN_NOR){
			setBtnState(JOButton::BTN_SEL);
		}
		else if (m_curState == JOButton::BTN_SEL){
			setBtnState(JOButton::BTN_NOR);
		}
		if (m_call){
			m_call(this, JOTouch::ENDED, pTouch);
		}
	}
}

NS_JOFW_END