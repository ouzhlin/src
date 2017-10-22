#include "ui/JOLabel.h"
#include "utils/JOString.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

float			JOLabel::s_fontSize = 0;
std::string		JOLabel::s_fontAlias = "Helvetica";
Color3B			JOLabel::s_fontColor = Color3B::WHITE;

short			JOLabel::s_boldSize = 0;
Color4B			JOLabel::s_outColor = Color4B::WHITE;

Size			JOLabel::s_shadowOffset = Size::ZERO;
Color4B			JOLabel::s_shadowColor = Color4B::WHITE;


void JOLabel::setDefaultBase(float fsize, const std::string& falias, Color3B fcolor)
{
	s_fontSize = fsize;
	s_fontAlias = falias;
	s_fontColor = fcolor;
}

void JOLabel::setDefaultOutline(int boldsize, Color4B outcolor)
{
	s_boldSize = boldsize;
	s_outColor = outcolor;
}

void JOLabel::setDefaultShadow(Size offset, Color4B shadowcolor)
{
	s_shadowOffset = offset;
	s_shadowColor = shadowcolor;
}
//////////////////////////////////////////////////////////////////////////

JOLabel::JOLabel()
:m_labelType(JOLabel::NOR)
, m_fontSize(0)
, m_fontAlias("")
, m_fontColor(Color3B::WHITE)
, m_boldSize(0)
, m_outColor(Color4B::WHITE)
, m_shadowColor(Color4B::WHITE)
, m_shadowOffset(Size::ZERO)
{

}

JOLabel::~JOLabel()
{

}


JOLabel* JOLabel::create(const std::string& str/*=nullptr*/, float fSize/*=s_fontSize*/, Color3B fColor/*=m_fontColor*/, const std::string& alias/*=s_fontAlias*/)
{
	JOLabel* spr = new (std::nothrow) JOLabel();
	if (spr && spr->init(str, fSize, fColor, alias)){
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return nullptr;
}

bool JOLabel::init(const std::string& str/*=nullptr*/, float fSize/*=s_fontSize*/, Color3B fColor/*=m_fontColor*/, const std::string& alias/*=s_fontAlias*/)
{	
	if (fSize<0){
		m_fontSize = s_fontSize;
	}
	else{
		m_fontSize = fSize;
	}
	
	if (alias==""){
		m_fontAlias = s_fontAlias;
	}
	else{
		m_fontAlias = alias;
	}	
	
	std::string suffix = JOPath::getFileSuffix(m_fontAlias);
	JOString::toLowerCase(suffix);
	if (suffix=="ttf"){
		m_labelType = JOLabel::TTF;
		TTFConfig tc;
		tc.fontFilePath = m_fontAlias;
		tc.fontSize = m_fontSize;
		setTTFConfig(tc);
	}
	else if (suffix == "fnt"){
		m_labelType = JOLabel::BMF;
		setBMFontFilePath(m_fontAlias);
	}
	else{
		m_labelType = JOLabel::NOR;
		setSystemFontName(m_fontAlias);
		setSystemFontSize(m_fontSize);
	}
	setLabelColor(fColor);

	_loadEffectStatus();

	setString(str);
	return true;
}

void JOLabel::setFontSize(float fSize)
{
	if (m_fontSize==fSize){
		return;
	}
	m_fontSize = fSize;
	if (m_labelType==TTF){
		TTFConfig tc;
		tc.fontFilePath = m_fontAlias;
		tc.fontSize = m_fontSize;
		setTTFConfig(tc);
	}
	else if (m_labelType == BMF){
		setBMFontSize(fSize);
	}
	else{
		setSystemFontSize(m_fontSize);
	}
}

void JOLabel::setFontName(const std::string& fName)
{
	if (m_fontAlias == fName){
		return;
	}
	if (fName==""){
		m_fontAlias = s_fontAlias;
	}
	else{
		m_fontAlias = fName;
	}
	
	std::string suffix = JOPath::getFileSuffix(m_fontAlias);
	JOString::toLowerCase(suffix);
	if (suffix == "ttf"){
		m_labelType = JOLabel::TTF;
		TTFConfig tc;
		tc.fontFilePath = m_fontAlias;
		tc.fontSize = m_fontSize;
		setTTFConfig(tc);
	}
	else if (suffix == "fnt"){
		m_labelType = JOLabel::BMF;
		setBMFontFilePath(m_fontAlias);
	}
	else{
		m_labelType = JOLabel::NOR;
		setSystemFontName(m_fontAlias);
		setSystemFontSize(m_fontSize);
	}
	_loadEffectStatus();
}

void JOLabel::setLabelColor(Color3B color)
{
	if (m_fontColor==color){
		return;
	}
	m_fontColor = color;
	if (m_labelType==JOLabel::BMF || m_labelType==JOLabel::CHARMAP){
		setColor(m_fontColor);
	}
	else{
		setTextColor(Color4B(m_fontColor));
	}
}

void JOLabel::setOutline(int boldSize, Color4B color/*=s_outColor*/)
{
	if (m_boldSize == boldSize && m_outColor == color){
		return;
	}
	m_boldSize = boldSize;
	m_outColor = color;
	if (m_labelType==JOLabel::TTF){
		if (m_boldSize > 0){
			enableOutline(m_outColor, m_boldSize);
		}
		else{
			disableEffect(LabelEffect::OUTLINE);
		}
	}
}

void JOLabel::setShadow(Color4B color, Size offset, int blurRadius /*= 0*/)
{
	if (m_shadowColor==color && m_shadowOffset.equals(offset)){
		return;
	}
	m_shadowColor = color;
	m_shadowOffset = offset;
	if (m_labelType==JOLabel::TTF || m_labelType==JOLabel::BMF || m_labelType==JOLabel::CHARMAP){
		if (m_shadowOffset.equals(Size::ZERO)){
			disableEffect(LabelEffect::SHADOW);
		}
		else{
			enableShadow(m_shadowColor, m_shadowOffset, blurRadius);
		}
	}
}

bool JOLabel::setCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
{
	m_labelType = JOLabel::CHARMAP;
	bool ret = Label::setCharMap(charMapFile, itemWidth, itemHeight, startCharMap);
	_loadEffectStatus();
	return ret;
}

bool JOLabel::setCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
{
	m_labelType = JOLabel::CHARMAP;
	bool ret = Label::setCharMap(texture, itemWidth, itemHeight, startCharMap);
	_loadEffectStatus();
	return ret;
}

bool JOLabel::setCharMap(const std::string& plistFile)
{
	m_labelType = JOLabel::CHARMAP;
	bool ret = Label::setCharMap(plistFile);
	_loadEffectStatus();
	return ret;
}

void JOLabel::_loadEffectStatus()
{
	if (m_labelType == JOLabel::TTF){
		if (m_boldSize > 0){
			enableOutline(m_outColor, m_boldSize);
		}
		else{
			disableEffect(LabelEffect::OUTLINE);
		}
	}
	if (m_labelType == JOLabel::TTF || m_labelType == JOLabel::BMF || m_labelType == JOLabel::CHARMAP){
		if (m_shadowOffset.equals(Size::ZERO)){
			disableEffect(LabelEffect::SHADOW);
		}
		else{
			enableShadow(m_shadowColor, m_shadowOffset, 0);
		}
	}
}


NS_JOFW_END