#include "Hit_rec.h"

Hit_rec::Hit_rec(Point hit_pos, float hit_time, material_t* mtl, Point normal, Triangle* hit_triangle)
{
	hit = true;
	this->hit_pos = hit_pos;
	this->hit_time = hit_time;
	this->mtl = mtl;
	this->normal = normal;
	this->hit_triangle = hit_triangle;
}