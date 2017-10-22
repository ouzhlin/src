#include "ui/JOUILayout.h"
#include "ui/vo/JOUILayoutVO.h"
#include "manager/JOCachePoolMgr.h"
#include "utils/JOLog.h"

#include "extensions/cocos-ext.h"
USING_NS_CC_EXT;

static unsigned int s_layoutFlagCnt = 0;

NS_JOFW_BEGIN
/*
template<typename T, typename TBase> class TIsDerived
{
public:
	static int t(TBase* base)
	{
		return 1;
	}
	static char t(void* t2)
	{
		return 0;
	}

	enum
	{
		Result = (sizeof(int) == sizeof(t((T*)NULL))),
	};
};
bool AISDerviedFromB = TIsDerived<Node, ScrollView>::Result;
*/
JOUILayout::JOUILayout()
:m_beDirect(false)
{

}

JOUILayout::~JOUILayout()
{
	std::list<JOUILayoutVO*>::iterator itr = m_layoutList.begin();
	while (itr != m_layoutList.end())
	{
		POOL_RECOVER(*itr, JOUILayoutVO, "JOUILayout");
		++itr;
	}
	m_layoutList.clear();
}

bool JOUILayout::relativePos(Node* src, Node* target, unsigned int layoutType /*= JOUILayout::CENTER*/, Point pos /*= Point::ZERO*/)
{
	if (layoutType == JOUILayout::NONE){
		return false;
	}
	if (src==nullptr){
		LOG_WARN("JOUILayout", "relativePosition src == nullptr");
		return false;
	}
	else if (target==nullptr){
		target = src->getParent();
	}
	if (target==nullptr){
		LOG_WARN("JOUILayout", "relativePosition target == nullptr");
		return false;
	}
	if (m_beDirect == true){
		_rp(src, target, layoutType, pos);
	}
	else{
		//if (src->isAsynLoading() || target->isAsynLoading()){
		JOUILayoutVO* vo = POOL_GET(JOUILayoutVO, "JOUILayout");
		vo->init(src, target, layoutType, pos);
		m_layoutList.push_back(vo);
		//return false;
		//}
	}

	//_rp(src, target, layoutType, pos);
	return true;
}

void JOUILayout::tick()
{
	JOUILayoutVO* vo = nullptr;
	std::list<JOUILayoutVO*>::iterator itr = m_layoutList.begin();
	while (itr != m_layoutList.end()){
		vo = *itr;
		if (vo->isLoaded())	{
			vo->exec();
			vo->done();
			POOL_RECOVER(vo, JOUILayoutVO, "JOUILayout");
			itr = m_layoutList.erase(itr);
            s_layoutFlagCnt=0;
		}
		else{
            if(s_layoutFlagCnt>10){
                LOG_WARN("JOUILayout", "JOUILayoutVO unLoaded");
                return;
            }
            s_layoutFlagCnt++;
			return;
		}		
	}
	m_layoutList.clear();
}


void JOUILayout::_rp(Node* src, Node* target, unsigned int layoutType, Point& pos)
{
	if (layoutType==JOUILayout::NONE){
		return;
	}
	Point tarAnchor = Point::ZERO;
	Point tarPos = Point::ZERO;
	
	Size tarSize = target->getViewSize();
	if (target != src->getParent()){
		tarPos = target->getPosition();
		if (!target->isIgnoreAnchorPointForPosition()){
			tarAnchor = target->getAnchorPoint();
		}
		tarSize.width = tarSize.width*target->getScaleX();
		tarSize.height = tarSize.height*target->getScaleY();
	}

	Point srcAnchor = Point::ZERO;
	if (!src->isIgnoreAnchorPointForPosition()){
		srcAnchor = src->getAnchorPoint();
	}
	Size srcSize = src->getViewSize();
	
	srcSize.width = srcSize.width*src->getScaleX();
	srcSize.height = srcSize.height*src->getScaleY();

	src->setPosition(_setPostionBase(srcSize, srcAnchor, tarSize, tarAnchor, tarPos, pos, layoutType));
}


Point JOUILayout::_setPostionBase(Size &srcSize, Point &srcAnchor, Size &targetSize, Point &targetAnchor, Point &targetPos, Point &offset, unsigned int relative)
{
	targetPos.x -= targetSize.width*targetAnchor.x;
	targetPos.y -= targetSize.height*targetAnchor.y;
	Point targetCenter = Point(targetPos.x + targetSize.width*0.5f, targetPos.y + targetSize.height*0.5f);
	Point retPos = Point::ZERO;
	if (srcSize.width==0 || srcSize.height==0){
		srcSize = Size(1, 1);
	}
	switch (relative)
	{
	case JOUILayout::CENTER:
		retPos.x = targetCenter.x + srcSize.width*(srcAnchor.x - 0.5f);
		retPos.y = targetCenter.y + srcSize.height*(srcAnchor.y - 0.5f);
		break;
	case JOUILayout::IN_T:
		retPos.x = targetCenter.x + srcSize.width*(srcAnchor.x - 0.5f);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::IN_L:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x);
		retPos.y = targetCenter.y + srcSize.height*(srcAnchor.y - 0.5f);
		break;
	case JOUILayout::IN_R:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetCenter.y + srcSize.height*(srcAnchor.y - 0.5f);
		break;
	case JOUILayout::IN_B:
		retPos.x = targetCenter.x + srcSize.width*(srcAnchor.x - 0.5f);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::IN_TL:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::IN_TR:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::IN_BL:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::IN_BR:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_T:
		retPos.x = targetCenter.x + srcSize.width*(srcAnchor.x - 0.5f);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_L:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetCenter.y + srcSize.height*(srcAnchor.y - 0.5f);
		break;
	case JOUILayout::OUT_B:
		retPos.x = targetCenter.x + srcSize.width*(srcAnchor.x - 0.5f);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_R:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x);
		retPos.y = targetCenter.y + srcSize.height*(srcAnchor.y - 0.5f);
		break;
	case JOUILayout::OUT_TL:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_TR:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_BL:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_BR:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_T_IN_L:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_T_IN_R:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_B_IN_L:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_B_IN_R:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_L_IN_T:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_L_IN_B:
		retPos.x = targetPos.x + srcSize.width*(srcAnchor.x - 1);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y);
		break;
	case JOUILayout::OUT_R_IN_T:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + targetSize.height + srcSize.height*(srcAnchor.y - 1);
		break;
	case JOUILayout::OUT_R_IN_B:
		retPos.x = targetPos.x + targetSize.width + srcSize.width*(srcAnchor.x);
		retPos.y = targetPos.y + srcSize.height*(srcAnchor.y);
		break;
	default:

		break;
	}

	retPos.x += offset.x;
	retPos.y += offset.y;
	return retPos;
}

NS_JOFW_END
