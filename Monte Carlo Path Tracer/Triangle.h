#pragma once
#include <tiny_obj_loader.h>
#include "Point.h"
#include "AABB.h"
using namespace tinyobj;

class Triangle
{
public:
	Point p1;
	Point p2;
	Point p3;
	// ÎÆÀí×ø±ê
	Point t1;
	Point t2;
	Point t3;
	AABB* bounding_box = nullptr;
	material_t* mtl = nullptr;
	Triangle(){};
	Triangle(Point p1, Point p2, Point p3);
	Hit_rec hit(Ray ray);
};

