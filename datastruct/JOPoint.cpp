
#include "datastruct/JOPoint.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN


/** Clamp a value between from and to.
*/
inline float clampf(float value, float min_inclusive, float max_inclusive)
{
	if (min_inclusive > max_inclusive) {
		std::swap(min_inclusive, max_inclusive);
	}
	return value < min_inclusive ? min_inclusive : value < max_inclusive ? value : max_inclusive;
}

//其实就是获取中间值，传入s\e
// returns true if segment A-B intersects with segment C-D. S->E is the ovderlap part
bool isOneDimensionSegmentOverlap(float A, float B, float C, float D, float *S, float * E)
{
	float ABmin = std::min(A, B);
	float ABmax = std::max(A, B);
	float CDmin = std::min(C, D);
	float CDmax = std::max(C, D);

	if (ABmax < CDmin || CDmax < ABmin)
	{
		// ABmin->ABmax->CDmin->CDmax or CDmin->CDmax->ABmin->ABmax
		return false;
	}
	else
	{
		if (ABmin >= CDmin && ABmin <= CDmax)
		{
			// CDmin->ABmin->CDmax->ABmax or CDmin->ABmin->ABmax->CDmax
			if (S != nullptr) *S = ABmin;
			if (E != nullptr) *E = CDmax < ABmax ? CDmax : ABmax;
		}
		else if (ABmax >= CDmin && ABmax <= CDmax)
		{
			// ABmin->CDmin->ABmax->CDmax
			if (S != nullptr) *S = CDmin;
			if (E != nullptr) *E = ABmax;
		}
		else
		{
			// ABmin->CDmin->CDmax->ABmax
			if (S != nullptr) *S = CDmin;
			if (E != nullptr) *E = CDmax;
		}
		return true;
	}
}

//叉积
// cross procuct of 2 vector. A->B X C->D
float crossProduct2Vector(const JOPoint& A, const JOPoint& B, const JOPoint& C, const JOPoint& D)
{
	return (D.y - C.y) * (B.x - A.x) - (D.x - C.x) * (B.y - A.y);
}



JOPoint::JOPoint()
	: x(0.0f), y(0.0f)
{
}

JOPoint::JOPoint(float xx, float yy)
	: x(xx), y(yy)
{
}

JOPoint::JOPoint(const float* array)
{
	set(array);
}

JOPoint::JOPoint(const JOPoint& p1, const JOPoint& p2)
{
	set(p1, p2);
}

JOPoint::JOPoint(const JOPoint& copy)
{
	set(copy);
}

JOPoint::~JOPoint()
{
}

float JOPoint::angle(const JOPoint& v1, const JOPoint& v2)
{
	float dz = v1.x * v2.y - v1.y * v2.x;
	return atan2f(fabsf(dz) + MATH_FLOAT_SMALL, dot(v1, v2));
}

void JOPoint::add(const JOPoint& v1, const JOPoint& v2, JOPoint* dst)
{
	dst->x = v1.x + v2.x;
	dst->y = v1.y + v2.y;
}

void JOPoint::clamp(const JOPoint& min, const JOPoint& max)
{
	if (min.x > max.x || min.y > max.y)
	{
		LOG_ERROR("JOPoint", "(min.x > max.x || min.y > max.y)");
		return;
	}

	// Clamp the x value.
	if (x < min.x)
		x = min.x;
	if (x > max.x)
		x = max.x;

	// Clamp the y value.
	if (y < min.y)
		y = min.y;
	if (y > max.y)
		y = max.y;
}

JOPoint JOPoint::clamp(const JOPoint& v, const JOPoint& min, const JOPoint& max)
{
	LOG_ERROR("JOPoint", "(min.x > max.x || min.y > max.y)");
	JOPoint dst(v);
	dst.clamp(min, max);
	return dst;	
}

float JOPoint::distance(const JOPoint& v) const
{
	float dx = v.x - x;
	float dy = v.y - y;

	return sqrt(dx * dx + dy * dy);
}

float JOPoint::dot(const JOPoint& v1, const JOPoint& v2)
{
	return (v1.x * v2.x + v1.y * v2.y);
}

float JOPoint::length() const
{
	return sqrt(x * x + y * y);
}

void JOPoint::normalize()
{
	float n = x * x + y * y;
	// Already normalized.
	if (n == 1.0f)
		return;

	n = sqrt(n);
	// Too close to zero.
	if (n < MATH_TOLERANCE)
		return;

	n = 1.0f / n;
	x *= n;
	y *= n;
}

JOPoint JOPoint::getNormalized() const
{
	JOPoint v(*this);
	v.normalize();
	return v;
}

void JOPoint::rotate(const JOPoint& point, float angle)
{
	double sinAngle = sin(angle);
	double cosAngle = cos(angle);

	if (point.isZero())
	{
		float tempX = x * cosAngle - y * sinAngle;
		y = y * cosAngle + x * sinAngle;
		x = tempX;
	}
	else
	{
		float tempX = x - point.x;
		float tempY = y - point.y;

		x = tempX * cosAngle - tempY * sinAngle + point.x;
		y = tempY * cosAngle + tempX * sinAngle + point.y;
	}
}

void JOPoint::set(const float* array)
{
	x = array[0];
	y = array[1];
}

void JOPoint::subtract(const JOPoint& v1, const JOPoint& v2, JOPoint* dst)
{
	dst->x = v1.x - v2.x;
	dst->y = v1.y - v2.y;
}

bool JOPoint::equals(const JOPoint& target) const
{
	return (fabs(this->x - target.x) < FLT_EPSILON)
		&& (fabs(this->y - target.y) < FLT_EPSILON);
}

bool JOPoint::fuzzyEquals(const JOPoint& b, float var) const
{
	if (x - var <= b.x && b.x <= x + var)
		if (y - var <= b.y && b.y <= y + var)
			return true;
	return false;
}

float JOPoint::getAngle(const JOPoint& other) const
{
	JOPoint a2 = getNormalized();
	JOPoint b2 = other.getNormalized();
	float angle = atan2f(a2.cross(b2), a2.dot(b2));
	if (fabs(angle) < FLT_EPSILON) return 0.f;
	return angle;
}

JOPoint JOPoint::rotateByAngle(const JOPoint& pivot, float angle) const
{
	return pivot + (*this - pivot).rotate(JOPoint::forAngle(angle));
}

bool JOPoint::isLineIntersect(const JOPoint& A, const JOPoint& B,
	const JOPoint& C, const JOPoint& D,
	float *S, float *T)
{
	// FAIL: Line undefined
	if ((A.x == B.x && A.y == B.y) || (C.x == D.x && C.y == D.y))
	{
		return false;
	}

	const float denom = crossProduct2Vector(A, B, C, D);

	if (denom == 0)
	{
		// Lines parallel or overlap
		return false;
	}

	if (S != nullptr) *S = crossProduct2Vector(C, D, C, A) / denom;
	if (T != nullptr) *T = crossProduct2Vector(A, B, C, A) / denom;

	return true;
}

bool JOPoint::isLineParallel(const JOPoint& A, const JOPoint& B,
	const JOPoint& C, const JOPoint& D)
{
	// FAIL: Line undefined
	if ((A.x == B.x && A.y == B.y) || (C.x == D.x && C.y == D.y))
	{
		return false;
	}

	if (crossProduct2Vector(A, B, C, D) == 0)
	{
		// line overlap
		if (crossProduct2Vector(C, D, C, A) == 0 || crossProduct2Vector(A, B, C, A) == 0)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool JOPoint::isLineOverlap(const JOPoint& A, const JOPoint& B,
	const JOPoint& C, const JOPoint& D)
{
	// FAIL: Line undefined
	if ((A.x == B.x && A.y == B.y) || (C.x == D.x && C.y == D.y))
	{
		return false;
	}

	if (crossProduct2Vector(A, B, C, D) == 0 &&
		(crossProduct2Vector(C, D, C, A) == 0 || crossProduct2Vector(A, B, C, A) == 0))
	{
		return true;
	}

	return false;
}

bool JOPoint::isSegmentOverlap(const JOPoint& A, const JOPoint& B, const JOPoint& C, const JOPoint& D, JOPoint* S, JOPoint* E)
{

	if (isLineOverlap(A, B, C, D))
	{
		return isOneDimensionSegmentOverlap(A.x, B.x, C.x, D.x, &S->x, &E->x) &&
			isOneDimensionSegmentOverlap(A.y, B.y, C.y, D.y, &S->y, &E->y);
	}

	return false;
}

bool JOPoint::isSegmentIntersect(const JOPoint& A, const JOPoint& B, const JOPoint& C, const JOPoint& D)
{
	float S, T;

	if (isLineIntersect(A, B, C, D, &S, &T) &&
		(S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f))
	{
		return true;
	}

	return false;
}

JOPoint JOPoint::getIntersectPoint(const JOPoint& A, const JOPoint& B, const JOPoint& C, const JOPoint& D)
{
	float S, T;

	if (isLineIntersect(A, B, C, D, &S, &T))
	{
		// JOPoint of intersection
		JOPoint P;
		P.x = A.x + S * (B.x - A.x);
		P.y = A.y + S * (B.y - A.y);
		return P;
	}

	return JOPoint::ZERO;
}

JOPoint JOPoint::getClampPoint(const JOPoint& min_inclusive, const JOPoint& max_inclusive) const
{
	return JOPoint(clampf(x, min_inclusive.x, max_inclusive.x), clampf(y, min_inclusive.y, max_inclusive.y));
}

const JOPoint JOPoint::ZERO(0.0f, 0.0f);
const JOPoint JOPoint::ONE(1.0f, 1.0f);
const JOPoint JOPoint::UNIT_X(1.0f, 0.0f);
const JOPoint JOPoint::UNIT_Y(0.0f, 1.0f);
const JOPoint JOPoint::ANCHOR_MIDDLE(0.5f, 0.5f);
const JOPoint JOPoint::ANCHOR_BOTTOM_LEFT(0.0f, 0.0f);
const JOPoint JOPoint::ANCHOR_TOP_LEFT(0.0f, 1.0f);
const JOPoint JOPoint::ANCHOR_BOTTOM_RIGHT(1.0f, 0.0f);
const JOPoint JOPoint::ANCHOR_TOP_RIGHT(1.0f, 1.0f);
const JOPoint JOPoint::ANCHOR_MIDDLE_RIGHT(1.0f, 0.5f);
const JOPoint JOPoint::ANCHOR_MIDDLE_LEFT(0.0f, 0.5f);
const JOPoint JOPoint::ANCHOR_MIDDLE_TOP(0.5f, 1.0f);
const JOPoint JOPoint::ANCHOR_MIDDLE_BOTTOM(0.5f, 0.0f);

NS_JOFW_END