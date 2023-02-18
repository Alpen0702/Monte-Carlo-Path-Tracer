#pragma once
#include <tiny_obj_loader.h>
#include "Point.h"
using namespace tinyobj;

class Triangle;
class Hit_rec
{
public:
	bool hit = false;
	Point hit_pos;
	float hit_time;
	material_t* mtl = nullptr;
	Triangle* hit_triangle = nullptr;
	Point normal;
	Hit_rec() {};
	Hit_rec(Point hit_pos, float hit_time, material_t* mtl, Point normal, Triangle* hit_triangle);
};

