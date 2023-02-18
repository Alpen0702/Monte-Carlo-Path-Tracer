#pragma once
#include <vector>
#include "AABB.h"
#include "Triangle.h"
#include "Hit_rec.h"
using namespace std;

class BVH_node
{
public:
	BVH_node* left = nullptr;
	BVH_node* right = nullptr;
	AABB* bounding_box = nullptr;
	vector<Triangle*> triangles;
	BVH_node(vector<Triangle*> meshes, int axis);
	bool intersect(Ray ray);
	Hit_rec hit(Ray ray);
};

