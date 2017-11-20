#include "ui/SpecialEffects/JOLaser.h"
#include "utils/JOLog.h"

#include "manager/JOSnMgr.h"

NS_JOFW_BEGIN

JOLaser::JOLaser()
:m_halfLaser1(nullptr)
,m_halfLaser2(nullptr)
,m_slimHalfLaser1(nullptr)
,m_slimHalfLaser2(nullptr)
,m_start(Point::ZERO)
, m_end(Point::ZERO)
{
}


JOLaser::~JOLaser()
{
	CC_SAFE_RELEASE_NULL(m_halfLaser1);
	CC_SAFE_RELEASE_NULL(m_halfLaser2);
	CC_SAFE_RELEASE_NULL(m_slimHalfLaser1);
	CC_SAFE_RELEASE_NULL(m_slimHalfLaser2);
}

JOLaser* JOLaser::create(const string &Laser, const string &mask, const string &mask2, const string &noise, const string &noise2)
{
	JOLaser *laser = new (std::nothrow) JOLaser();
	if (laser && laser->init(Laser, mask, mask2, noise, noise2)){
		return laser;
	}
	CC_SAFE_DELETE(laser);
	return nullptr;
}
bool JOLaser::init(const string &Laser, const string &mask, const string &mask2, const string &noise, const string &noise2)
{
	//halfLaser1
	m_halfLaser1 = JOHalfLaser::create(Laser, mask, mask2, noise, noise2);
	m_halfLaser1->setPorN(1);
	m_halfLaser1->setBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	m_halfLaser1->setScaleY(1.3);//(1.0);
	m_halfLaser1->m_kGlowColor = 1.6;//1.0;
	m_halfLaser1->m_kLightColor = 0.0;
	m_halfLaser1->m_noiseScale = 2.2;//0.5;
	addChild(m_halfLaser1);
	//halfLaser2
	m_halfLaser2 = JOHalfLaser::create(Laser, mask, mask2, noise, noise2);
	m_halfLaser2->setPorN(-1);
	m_halfLaser2->setBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	m_halfLaser2->setScaleY(1.3);//(1.0);
	m_halfLaser2->m_kGlowColor = 1.6;//1.0;
	m_halfLaser2->m_kLightColor = 0.0;
	m_halfLaser2->m_noiseScale = 2.2;//0.5;
	addChild(m_halfLaser2);
	//slimHalfLaser1
	m_slimHalfLaser1 = JOHalfLaser::create(Laser, mask, mask2, noise, noise2);;
	m_slimHalfLaser1->setPorN(1);
	m_slimHalfLaser1->setBlendFunc(GL_ONE, GL_ONE);
	m_slimHalfLaser1->setScaleY(0.4);//(0.6);//(0.1);
	m_slimHalfLaser1->m_kGlowColor = 2.5;//1.5;
	m_slimHalfLaser1->m_kLightColor = 0.5;//0.4;//0.3;
	m_slimHalfLaser1->m_noiseScale = 1.5;
	addChild(m_slimHalfLaser1);
	//slimHalfLaser2
	m_slimHalfLaser2 = JOHalfLaser::create(Laser, mask, mask2, noise, noise2);
	m_slimHalfLaser2->setPorN(-1);
	m_slimHalfLaser2->setBlendFunc(GL_ONE, GL_ONE);
	m_slimHalfLaser2->setScaleY(0.4);//(0.6);//(0.1);
	m_slimHalfLaser2->m_kGlowColor = 2.5;//1.5;
	m_slimHalfLaser2->m_kLightColor = 0.5;//0.4;//0.3;
	m_slimHalfLaser2->m_noiseScale = 1.5;
	addChild(m_slimHalfLaser2);

	setContentSize(m_halfLaser1->getContentSize());
	m_halfLaser1:setPosition(m_halfLaser1->getContentSize().width*0.5, m_halfLaser1->getContentSize().height*0.5);
	m_halfLaser2:setPosition(m_halfLaser1->getContentSize().width*0.5, m_halfLaser1->getContentSize().height*0.5);
	m_slimHalfLaser1:setPosition(m_halfLaser1->getContentSize().width*0.5, m_halfLaser1->getContentSize().height*0.5);
	m_slimHalfLaser2:setPosition(m_halfLaser1->getContentSize().width*0.5, m_halfLaser1->getContentSize().height*0.5);

	return true;
}


void JOLaser::setStart(const Point&start)
{
	if (start.equals(m_start)) return;
	
	m_start = start;
	m_halfLaser1->setStart(m_start);
	m_halfLaser2->setStart(m_start);
	m_slimHalfLaser1->setStart(m_start);
	m_slimHalfLaser2->setStart(m_start);

	_layout();
	
}
void JOLaser::setEnd(const Point&end)
{
	if (end.equals(m_end)) return;

	m_end = end;
	m_halfLaser1->setEnd(m_end);
	m_halfLaser2->setEnd(m_end);
	m_slimHalfLaser1->setEnd(m_end);
	m_slimHalfLaser2->setEnd(m_end);

	_layout();
}

void JOLaser::_layout()
{
	setPosition(ccpMult(m_start + m_end, 0.5));
}




JOHalfLaser::JOHalfLaser() :m_program(nullptr), m_maskTex(nullptr), m_maskTex2(nullptr), m_noiseTex(nullptr), m_noiseTex2(nullptr),
m_highlight(1.0f), m_time(0), m_PorN(1), m_kLightColor(1.0f), m_kGlowColor(1.0f), m_noiseScale(1.0f)
{
}

JOHalfLaser::~JOHalfLaser()
{
	CC_SAFE_RELEASE_NULL(m_program);
	CC_SAFE_RELEASE_NULL(m_maskTex);
	CC_SAFE_RELEASE_NULL(m_maskTex2);
	CC_SAFE_RELEASE_NULL(m_noiseTex);
	CC_SAFE_RELEASE_NULL(m_noiseTex2);
}

JOHalfLaser* JOHalfLaser::create(const string &Laser, const string &mask, const string &mask2, const string &noise, const string &noise2)
{
	JOHalfLaser *halfLaser = new (std::nothrow) JOHalfLaser();
	if (halfLaser && halfLaser->init(Laser, mask, mask2, noise, noise2))
	{
		return halfLaser;
	}
	CC_SAFE_DELETE(halfLaser);
	return nullptr;
}
bool JOHalfLaser::init(const string &Laser, const string &mask, const string &mask2, const string &noise, const string &noise2)
{
	//lightTex
	this->setKey(Laser);
	{
		Texture2D::TexParams texParams = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		this->getTexture()->setTexParameters(&texParams);
	}

	//maskTex
	m_maskTex = CCTextureCache::sharedTextureCache()->addImage(mask);
	m_maskTex->retain();
	{
		Texture2D::TexParams texParams = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		m_maskTex->setTexParameters(&texParams);
	}
	//maskTex2
	m_maskTex2 = CCTextureCache::sharedTextureCache()->addImage(mask2);
	m_maskTex2->retain();
	{
		Texture2D::TexParams texParams = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		m_maskTex2->setTexParameters(&texParams);
	}

	//noiseTex
	m_noiseTex = CCTextureCache::sharedTextureCache()->addImage(noise);
	m_noiseTex->retain();
	{
		Texture2D::TexParams texParams = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		m_noiseTex->setTexParameters(&texParams);
	}
	//noiseTex2
	m_noiseTex2 = CCTextureCache::sharedTextureCache()->addImage(noise2);
	m_noiseTex2->retain();
	{
		Texture2D::TexParams texParams = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		m_noiseTex2->setTexParameters(&texParams);
	}

	//create and set shader program
	{
		GLchar * fragSource = (GLchar*)CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("shaders/laser.fsh").c_str())->getCString();
		ens::CGLProgramWithUnifos* program = new ens::CGLProgramWithUnifos();
		program->autorelease();
		program->initWithVertexShaderByteArray(ccPositionTextureColor_noMVP_vert, fragSource);//use ccPositionTextureColor_noMVP_vert instead of ccPositionTextureColor_vert, see: http://www.cnblogs.com/wantnon/p/4190341.html
		//            --below code is no longer needed, because bindPredefinedVertexAttribs() is called in link() in 3.x
		//            --bind attribute
		//            --program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		//            --program->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		//            --program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
		//link  (must after bindAttribute)
		program->link();
		//get cocos2d-x build-in uniforms
		program->updateUniforms();
		//get my own uniforms
		program->attachUniform("u_maskTex");
		program->attachUniform("u_maskTex2");
		program->attachUniform("u_noiseTex");
		program->attachUniform("u_noiseTex2");
		program->attachUniform("u_spriteSize");
		program->attachUniform("u_LUPos");
		program->attachUniform("u_noiseSize");
		program->attachUniform("u_highlight");
		program->attachUniform("u_time");
		program->attachUniform("u_PorN");
		program->attachUniform("u_scale");
		program->attachUniform("u_kLightColor");
		program->attachUniform("u_kGlowColor");
		program->attachUniform("u_noiseScale");
		program->attachUniform("u_color_r");
		program->attachUniform("u_color_g");
		program->attachUniform("u_color_b");

		//set program
		m_program = program;
		m_program->retain();

		//check gl error
		CHECK_GL_ERROR_DEBUG();
	}
	//update
	this->scheduleUpdate();


	return true;
}
void JOHalfLaser::onPassUnifoAndBindTex(const Mat4 &transform, uint32_t flags)
{

}

void JOHalfLaser::update(float dt)
{

}

void JOHalfLaser::draw(Renderer* renderer, const Mat4 &transform, uint32_t flags)
{

}

void JOHalfLaser::setEnd(const Point&end)
{

}

void JOHalfLaser::setStart(const Point&start)
{

}

void JOHalfLaser::updateStartAndEnd()
{

}

void JOHalfLaser::setBlendFunc(GLenum src, GLenum dst)
{

}




NS_JOFW_END
