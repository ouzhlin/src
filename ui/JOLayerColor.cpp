#include "ui/JOLayerColor.h"
#include "ui/JOUIDef.h"

NS_JOFW_BEGIN


JOLayerColor::JOLayerColor()
{

}

JOLayerColor::~JOLayerColor()
{

}

JOLayerColor* JOLayerColor::create()
{
	JOLayerColor* ret = new (std::nothrow) JOLayerColor();
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

JOLayerColor* JOLayerColor::create(const Color4B& color)
{
	JOLayerColor * layer = new (std::nothrow) JOLayerColor();
	if (layer && layer->initWithColor(color))
	{
		layer->autorelease();
		return layer;
	}
	CC_SAFE_DELETE(layer);
	return nullptr;
}

void JOLayerColor::setLayerColor(const Color4B& color)
{
	// default blend function
	_blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;

	_displayedColor.r = _realColor.r = color.r;
	_displayedColor.g = _realColor.g = color.g;
	_displayedColor.b = _realColor.b = color.b;
	_displayedOpacity = _realOpacity = color.a;

	for (size_t i = 0; i < sizeof(_squareVertices) / sizeof(_squareVertices[0]); i++)
	{
		_squareVertices[i].x = 0.0f;
		_squareVertices[i].y = 0.0f;
	}

	updateColor();
	Size size = getContentSize();
	_squareVertices[1].x = size.width;
	_squareVertices[2].y = size.height;
	_squareVertices[3].x = size.width;
	_squareVertices[3].y = size.height;

	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP));
}


NS_JOFW_END