#include "ximage.h"
#include "ximath.h"
#include <math.h>

//this module should contain some classes for geometrical transformations
//usable with selections, etc... once it's done, that is. :)

CxPoint2::CxPoint2()
{
	x = y = 0.0f;
}

CxPoint2::CxPoint2(const float x_, const float y_)
{
	x = x_;
	y = y_;
}

CxPoint2::CxPoint2(const CxPoint2& p)
{
	x = p.x;
	y = p.y;
}

float CxPoint2::Distance(const CxPoint2 p2)
{
	return static_cast<float>(sqrt((x - p2.x) * (x - p2.x) + (y - p2.y) * (y - p2.y)));
}

float CxPoint2::Distance(const float x_, const float y_)
{
	return static_cast<float>(sqrt((x - x_) * (x - x_) + (y - y_) * (y - y_)));
}

CxRect2::CxRect2()
{
}

CxRect2::CxRect2(const float x1_, const float y1_, const float x2_, const float y2_)
{
	botLeft.x = x1_;
	botLeft.y = y1_;
	topRight.x = x2_;
	topRight.y = y2_;
}

CxRect2::CxRect2(const CxRect2& p)
{
	botLeft = p.botLeft;
	topRight = p.topRight;
}

float CxRect2::Surface() const
/*
 * Returns the surface of rectangle.
 */
{
	return (topRight.x - botLeft.x) * (topRight.y - botLeft.y);
}

CxRect2 CxRect2::CrossSection(const CxRect2& r2) const
/*
 * Returns crossection with another rectangle.
 */
{
	CxRect2 cs;
	cs.botLeft.x = max(botLeft.x, r2.botLeft.x);
	cs.botLeft.y = max(botLeft.y, r2.botLeft.y);
	cs.topRight.x = min(topRight.x, r2.topRight.x);
	cs.topRight.y = min(topRight.y, r2.topRight.y);
	if (cs.botLeft.x <= cs.topRight.x && cs.botLeft.y <= cs.topRight.y)
	{
		return cs;
	}
	return CxRect2(0, 0, 0, 0);
	//if
}

CxPoint2 CxRect2::Center() const
/*
 * Returns the center point of rectangle.
 */
{
	return CxPoint2((topRight.x + botLeft.x) / 2.0f, (topRight.y + botLeft.y) / 2.0f);
}

float CxRect2::Width() const
//returns rectangle width
{
	return topRight.x - botLeft.x;
}

float CxRect2::Height() const
//returns rectangle height
{
	return topRight.y - botLeft.y;
}
