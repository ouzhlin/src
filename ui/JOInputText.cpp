#include "ui/JOInputText.h"
#include "utils/JOLuaUtils.h"
#include "utils/JOString.h"

#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"

NS_JOFW_BEGIN

JOInputText::JOInputText()
{
    _eventCallback = 0;
}

JOInputText * JOInputText::create(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    JOInputText *ret = new (std::nothrow) JOInputText();
    if(ret && ret->init(placeholder, fontName, fontSize))
    {
        ret->autorelease();
        if (placeholder.size()>0)
        {
            ret->setPlaceHolder(placeholder);
        }
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool JOInputText::init(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    setPlaceHolder(placeholder);
    setSystemFontName(fontName);
    setSystemFontSize(fontSize);
    return true;
}


void JOInputText::setInputEventCallback(LUA_FUNCTION eventCallback)
{
    _eventCallback = eventCallback;
}

bool JOInputText::canAttachWithIME()
{
    return true;
}

bool JOInputText::canDetachWithIME()
{
    return true;
}


void JOInputText::insertText(const char * text, size_t len)
{
    if (_eventCallback) {
		LuaStack* l = LuaEngine::getInstance()->getLuaStack();
		l->pushInt(INPUT_EVENT_INSERT_BEGIN);
		l->pushString(text);		
		bool ret = l->executeFunctionByHandler(_eventCallback, 2);
		if (ret==false)	{
			return;
		}
    }
    TextFieldTTF::insertText(text, len);
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_INSERT_END);
        stack->pushString(text, len);
        stack->executeFunctionByHandler(_eventCallback, 2);
		//stack->clean();
    }
}
    
void JOInputText::deleteBackward()
{
    TextFieldTTF::deleteBackward();
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_DELETE);
        stack->executeFunctionByHandler(_eventCallback, 1);
		//stack->clean();
    }
}

void JOInputText::didAttachWithIME()
{
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_ATTACH);
        stack->executeFunctionByHandler(_eventCallback, 1);
		//stack->clean();
    }
}
void JOInputText::didDetachWithIME()
{
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_DETACH);
        stack->executeFunctionByHandler(_eventCallback, 1);
		//stack->clean();
    }
}
    //////////////////////////////////////////////////////////////////////////
    // keyboard show/hide notification
    //////////////////////////////////////////////////////////////////////////
void JOInputText::keyboardWillShow(IMEKeyboardNotificationInfo& info)
{
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_KEYBOARDSHOW);
        stack->pushFloat(info.duration);
        
        stack->pushFloat(info.begin.origin.x);
        stack->pushFloat(info.begin.origin.y);
        stack->pushFloat(info.begin.size.width);
        stack->pushFloat(info.begin.size.height);
        
        stack->pushFloat(info.end.origin.x);
        stack->pushFloat(info.end.origin.y);
        stack->pushFloat(info.end.size.width);
        stack->pushFloat(info.end.size.height);

        stack->executeFunctionByHandler(_eventCallback, 10);
		//stack->clean();
    }
}
void JOInputText::keyboardDidShow(IMEKeyboardNotificationInfo& info)
{
    
}
void JOInputText::keyboardWillHide(IMEKeyboardNotificationInfo& info)
{
    if (_eventCallback) {
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
        stack->pushInt(INPUT_EVENT_KEYBOARDHIDE);
        stack->pushFloat(info.duration);
       
        stack->pushFloat(info.begin.origin.x);
        stack->pushFloat(info.begin.origin.y);
        stack->pushFloat(info.begin.size.width);
        stack->pushFloat(info.begin.size.height);
        
        stack->pushFloat(info.end.origin.x);
        stack->pushFloat(info.end.origin.y);
        stack->pushFloat(info.end.size.width);
        stack->pushFloat(info.end.size.height);
        
        stack->executeFunctionByHandler(_eventCallback, 10);
		//stack->clean();
    }
}
void JOInputText::keyboardDidHide(IMEKeyboardNotificationInfo& info)
{
    
}
    
NS_JOFW_END