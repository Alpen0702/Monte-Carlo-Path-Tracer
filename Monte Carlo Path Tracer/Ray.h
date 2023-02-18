#pragma once
#include "Point.h"
#include "Hit_rec.h"

class Ray
{
public:
	Point origin;
	Point dir;
	Ray(Point origin, Point dir);
};

