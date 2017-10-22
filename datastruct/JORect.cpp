#include "datastruct/JORect.h"
#include "datastruct/JOPoint.h"

NS_JOFW_BEGIN

JORect::JORect(void)
{
	setRect(0.0f, 0.0f, 0.0f, 0.0f);
}

JORect::JORect(float x, float y, float width, float height)
{
	setRect(x, y, width, height);
}

JORect::JORect(const JORect& other)
{
	setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
}

JORect& JORect::operator= (const JORect& other)
{
	setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
	return *this;
}

void JORect::setRect(float x, float y, float width, float height)
{
	// CGJORect can support width<0 or height<0
	// CCASSERT(width >= 0.0f && height >= 0.0f, "width and height of JORect must not less than 0.");

	origin.x = x;
	origin.y = y;

	size.width = width;
	size.height = height;
}

bool JORect::equals(const JORect& rect) const
{
	return (origin.equals(rect.origin) &&
		size.equals(rect.size));
}

float JORect::getMaxX() const
{
	return origin.x + size.width;
}

float JORect::getMidX() const
{
	return origin.x + size.width / 2.0f;
}

float JORect::getMinX() const
{
	return origin.x;
}

float JORect::getMaxY() const
{
	return origin.y + size.height;
}

float JORect::getMidY() const
{
	return origin.y + size.height / 2.0f;
}

float JORect::getMinY() const
{
	return origin.y;
}

bool JORect::containsPoint(const JOPoint& point) const
{
	bool bRet = false;

	if (point.x >= getMinX() && point.x <= getMaxX()
		&& point.y >= getMinY() && point.y <= getMaxY())
	{
		bRet = true;
	}

	return bRet;
}

bool JORect::intersectsJORect(const JORect& rect) const
{
	return !(getMaxX() < rect.getMinX() ||
		rect.getMaxX() < getMinX() ||
		getMaxY() < rect.getMinY() ||
		rect.getMaxY() < getMinY());
}

bool JORect::intersectsCircle(const JOPoint &center, float radius) const
{
	float w = size.width / 2;
	float h = size.height / 2;

	float dx = fabs(center.x - (origin.x + size.width * 0.5f));
	float dy = fabs(center.y - (origin.y + size.height * 0.5f));

	if (dx > (radius + w) || dy > (radius + h))
	{
		return false;
	}

	dx = fabs(center.x - origin.x - w);
	dy = fabs(center.y - origin.y - h);
	

	if (dx <= (w))
	{
		return true;
	}

	if (dy <= (h))
	{
		return true;
	}

	float cornerDistanceSq = powf(dx - w, 2) + powf(dy - h, 2);

	return (cornerDistanceSq <= (powf(radius, 2)));
}

void JORect::merge(const JORect& rect)
{
	float top1 = getMaxY();
	float left1 = getMinX();
	float right1 = getMaxX();
	float bottom1 = getMinY();

	float top2 = rect.getMaxY();
	float left2 = rect.getMinX();
	float right2 = rect.getMaxX();
	float bottom2 = rect.getMinY();
	origin.x = std::min(left1, left2);
	origin.y = std::min(bottom1, bottom2);
	size.width = std::max(right1, right2) - origin.x;
	size.height = std::max(top1, top2) - origin.y;
}

JORect JORect::unionWithJORect(const JORect & rect) const
{
	float thisLeftX = origin.x;
	float thisRightX = origin.x + size.width;
	float thisTopY = origin.y + size.height;
	float thisBottomY = origin.y;

	if (thisRightX < thisLeftX)
	{
		std::swap(thisRightX, thisLeftX);   // This JORect has negative width
	}

	if (thisTopY < thisBottomY)
	{
		std::swap(thisTopY, thisBottomY);   // This JORect has negative height
	}

	float otherLeftX = rect.origin.x;
	float otherRightX = rect.origin.x + rect.size.width;
	float otherTopY = rect.origin.y + rect.size.height;
	float otherBottomY = rect.origin.y;

	if (otherRightX < otherLeftX)
	{
		std::swap(otherRightX, otherLeftX);   // Other JORect has negative width
	}

	if (otherTopY < otherBottomY)
	{
		std::swap(otherTopY, otherBottomY);   // Other JORect has negative height
	}

	float combinedLeftX = std::min(thisLeftX, otherLeftX);
	float combinedRightX = std::max(thisRightX, otherRightX);
	float combinedTopY = std::max(thisTopY, otherTopY);
	float combinedBottomY = std::min(thisBottomY, otherBottomY);

	return JORect(combinedLeftX, combinedBottomY, combinedRightX - combinedLeftX, combinedTopY - combinedBottomY);
}

const JORect JORect::ZERO = JORect(0, 0, 0, 0);

NS_JOFW_END