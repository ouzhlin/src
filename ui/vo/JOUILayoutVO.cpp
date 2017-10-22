#include "ui/vo/JOUILayoutVO.h"
#include "ui/JOUILayout.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOUILayoutVO::JOUILayoutVO()
: m_src(nullptr)
, m_target(nullptr)
{
}

JOUILayoutVO::~JOUILayoutVO()
{
	done();
}

bool JOUILayoutVO::init(Node* src, Node* target, unsigned int layoutType /*= JOUILayout::CENTER*/, Point pos /*= Point::ZERO*/)
{
	if (src==nullptr || target==nullptr){
		return false;			 
	}
	m_src = src;
	m_target = target;
	m_src->retain();
	m_target->retain();
	m_src->setLayouVisible(false);
	m_layoutType = layoutType;
	m_offset = pos;
	return true;
}
bool JOUILayoutVO::isLoaded()
{
	if (m_src==nullptr || m_target==nullptr){
		LOG_ERROR("JOUILayoutVO", "the layou obj is null!!!!");
		return false;
	}
	if (m_src->isAsynLoading() || m_target->isAsynLoading()){
		return false;
	}
	if (m_src->getParent() != m_target && m_target->getParent()==nullptr){
		return false;
	}
	return true;
}

void JOUILayoutVO::exec()
{
	if (m_src == nullptr || m_target == nullptr){
		LOG_ERROR("JOUILayoutVO", "the layou obj is null!!!!");
		return ;
	}
	JOUILayout::Instance()->_rp(m_src, m_target, m_layoutType, m_offset);
}

void JOUILayoutVO::done()
{
	if (m_src){
		m_src->setLayouVisible(true);
		m_src->release();
		m_src = nullptr;
	}
	if (m_target){
		m_target->release();
		m_target = nullptr;
	}
}



NS_JOFW_END