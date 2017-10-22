#include "module/map/entity/JOMapEntity.h"

NS_JOFW_BEGIN

JOMapEntity::JOMapEntity()
{

}

JOMapEntity::~JOMapEntity()
{

}

void JOMapEntity::setDirectionWithPoints(JOPoint &sPoint, JOPoint &ePoint)
{
	if (sPoint.equals(ePoint))
	{
		return;
	}
	if (sPoint.x == ePoint.x)
	{
		if (sPoint.y > ePoint.y)
		{
			setDirection(BOTTOM);
		}
		else
		{
			setDirection(TOP);
		}
	}
	else if (sPoint.x > ePoint.x)
	{
		if (sPoint.y > ePoint.y)
		{
			setDirection(LEFT_BOTTOM);
		}
		else if (sPoint.y == ePoint.y)
		{
			setDirection(LEFT);
		}
		/*
		如果有左上的，这里设置左上
		*/
		else
		{
			setDirection(LEFT);
		}
	}
	else if (sPoint.x < ePoint.x)
	{
		if (sPoint.y > ePoint.y)
		{
			setDirection(RIGHT_BOTTOM);
		}
		else if (sPoint.y == ePoint.y)
		{
			setDirection(RIGHT);
		}
		/*
		如果有右上的，这里设置右上
		*/
		else
		{
			setDirection(RIGHT);
		}
	}
}

void JOMapEntity::setDirection(short type)
{
	switch (type)
	{
	case TOP:
		break;
	case LEFT:
		break;
	case LEFT_BOTTOM:
		break;
	case RIGHT:
		break;
	case RIGHT_BOTTOM:
		break;
	case BOTTOM:
		break;
	default:
		break;
	}
}

NS_JOFW_END
