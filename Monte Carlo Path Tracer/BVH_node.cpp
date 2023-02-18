#include "BVH_node.h"
#include <algorithm>
#include <iostream>
#define MAX_BVH_NODE_SIZE 5

bool x_comp(const Triangle* a, const Triangle* b)
{
	return a->bounding_box->p_min.coor[0] + a->bounding_box->p_max.coor[0] < b->bounding_box->p_min.coor[0] + b->bounding_box->p_max.coor[0];
}

bool y_comp(const Triangle* a, const Triangle* b)
{
	return a->bounding_box->p_min.coor[1] + a->bounding_box->p_max.coor[1] < b->bounding_box->p_min.coor[1] + b->bounding_box->p_max.coor[1];
}

bool z_comp(const Triangle* a, const Triangle* b)
{
	return a->bounding_box->p_min.coor[2] + a->bounding_box->p_max.coor[2] < b->bounding_box->p_min.coor[2] + b->bounding_box->p_max.coor[2];
}

BVH_node::BVH_node(vector<Triangle*> meshes, int axis)
{
	// 若剩余面片数量足够少，就放在一个Node中，不再分割
	if (meshes.size() < MAX_BVH_NODE_SIZE)
		triangles.assign(meshes.begin(), meshes.end());

	// 否则，继续分割Node
	else
	{	
		// 轮流按某一坐标轴对meshes排序
		if (axis % 3 == 0)
			sort(meshes.begin(), meshes.end(), x_comp);
		else if (axis % 3 == 1)
			sort(meshes.begin(), meshes.end(), y_comp);
		else
			sort(meshes.begin(), meshes.end(), z_comp);

		// 将meshes的前半部分到left中，后半部分到right中
		vector<Triangle*> left_meshes, right_meshes;
		vector<Triangle*>::iterator iter = meshes.begin() + meshes.size() / 2;
		left_meshes.assign(meshes.begin(), iter);
		right_meshes.assign(iter, meshes.end());
		left = new BVH_node(left_meshes, axis + 1);
		right = new BVH_node(right_meshes, axis + 1);
	}

	// 通过子节点来计算当前的包围盒
	//AABB aabb;
	if (triangles.size())
	{
		//aabb = box_union(triangles);
		bounding_box = new AABB(box_union(triangles));
		//if (aabb.p_min.coor[1] > aabb.p_max.coor[1])
			//cout << "error";
		//cout << aabb.p_min.coor[0] << ' ' << aabb.p_min.coor[1] << ' ' << aabb.p_min.coor[2] << endl;
		//cout << aabb.p_max.coor[0] << ' ' << aabb.p_max.coor[1] << ' ' << aabb.p_max.coor[2] << endl;
	}
		
	else
	{
		if (left != nullptr && right != nullptr)
			bounding_box = new AABB(box_union((left->bounding_box), (right->bounding_box)));
		//if (aabb.p_min.coor[1] > aabb.p_max.coor[1])
			//cout << "error" << endl;
		//cout << aabb.p_min.coor[0] << ' ' << aabb.p_min.coor[1] << ' ' << aabb.p_min.coor[2] << endl;
		//cout << aabb.p_max.coor[0] << ' ' << aabb.p_max.coor[1] << ' ' << aabb.p_max.coor[2] << endl;
	}
}

Hit_rec BVH_node::hit(Ray ray)
{
	if (bounding_box->intersect(ray))
	//if (1)
	{
		// 若不是叶子节点，则递归处理其左右儿子
		if (triangles.empty())
		{
			Hit_rec hit_left = left->hit(ray);
			Hit_rec hit_right = right->hit(ray);

			// 返回左右儿子中较好的一个
			if (hit_left.hit && hit_right.hit)
				if (hit_left.hit_time < hit_right.hit_time)
					return hit_left;
				else
					return hit_right;

			else if (hit_left.hit)
				return hit_left;

			else if (hit_right.hit)
				return hit_right;

			else
				return Hit_rec();
		}

		// 若是叶子节点，则需精密判断每个mesh
		else
		{
			Hit_rec res = Hit_rec();
			for (Triangle* triangle : triangles)
			{
				Hit_rec tmp = triangle->hit(ray);
				if (!res.hit)
					res = tmp;
				else if (tmp.hit && tmp.hit_time < res.hit_time)
					res = tmp;
			}
			return res;
		}
	}
	else
		return Hit_rec();
}