#include "datastruct/JOSize.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

JOSize::JOSize(void) : width(0), height(0)
{
}

JOSize::JOSize(float w, float h) : width(w), height(h)
{
}

JOSize::JOSize(const JOSize& other) : width(other.width), height(other.height)
{
}

JOSize::JOSize(const JOPoint& point) : width(point.x), height(point.y)
{
}

JOSize& JOSize::operator= (const JOSize& other)
{
	setSize(other.width, other.height);
	return *this;
}

JOSize& JOSize::operator= (const JOPoint& point)
{
	setSize(point.x, point.y);
	return *this;
}

JOSize JOSize::operator+(const JOSize& right) const
{
	return JOSize(this->width + right.width, this->height + right.height);
}

JOSize JOSize::operator-(const JOSize& right) const
{
	return JOSize(this->width - right.width, this->height - right.height);
}

JOSize JOSize::operator*(float a) const
{
	return JOSize(this->width * a, this->height * a);
}

JOSize JOSize::operator/(float a) const
{
	if (a != 0)
	{
		return JOSize(this->width / a, this->height / a);
	}
	LOG_ERROR("JOSize", "Size division by 0.");
	return ZERO;	
}

void JOSize::setSize(float w, float h)
{
	this->width = w;
	this->height = h;
}

bool JOSize::equals(const JOSize& target) const
{
	return (fabs(this->width - target.width) < FLT_EPSILON)
		&& (fabs(this->height - target.height) < FLT_EPSILON);
}

const JOSize JOSize::ZERO = JOSize(0, 0);

NS_JOFW_END