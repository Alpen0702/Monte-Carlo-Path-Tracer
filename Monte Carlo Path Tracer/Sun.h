#pragma once
#include "Triangle.h"
class Sun :
    public Triangle
{
public:
    Point radiance;
    Point normal;
    float S;
    Sun(Triangle* triangle, Point avg_norm);
};

