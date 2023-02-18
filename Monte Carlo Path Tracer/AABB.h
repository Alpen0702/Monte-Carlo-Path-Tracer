#pragma once
#include <cstdio>
#include <vector>
#include "Point.h"
#include "Ray.h"
using namespace std;

class AABB
{
public:
    Point p_min;
    Point p_max;
	AABB() {};
    AABB(Point p_min, Point p_max);
    bool intersect(Ray ray);
};

class Triangle;
AABB box_union(vector<Triangle*> triangles);
AABB box_union(AABB aabb1, AABB aabb2);
AABB box_union(AABB* aabb1, AABB* aabb2);