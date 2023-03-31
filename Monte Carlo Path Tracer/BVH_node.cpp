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
	// ��ʣ����Ƭ�����㹻�٣��ͷ���һ��Node�У����ٷָ�
	if (meshes.size() < MAX_BVH_NODE_SIZE)
		triangles.assign(meshes.begin(), meshes.end());

	// ���򣬼����ָ�Node
	else
	{	
		// ������ĳһ�������meshes����
		if (axis % 3 == 0)
			sort(meshes.begin(), meshes.end(), x_comp);
		else if (axis % 3 == 1)
			sort(meshes.begin(), meshes.end(), y_comp);
		else
			sort(meshes.begin(), meshes.end(), z_comp);

		// ��meshes��ǰ�벿�ֵ�left�У���벿�ֵ�right��
		vector<Triangle*> left_meshes, right_meshes;
		vector<Triangle*>::iterator iter = meshes.begin() + meshes.size() / 2;
		left_meshes.assign(meshes.begin(), iter);
		right_meshes.assign(iter, meshes.end());
		left = new BVH_node(left_meshes, axis + 1);
		right = new BVH_node(right_meshes, axis + 1);
	}

	// ͨ���ӽڵ������㵱ǰ�İ�Χ��
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
		// ������Ҷ�ӽڵ㣬��ݹ鴦�������Ҷ���
		if (triangles.empty())
		{
			Hit_rec hit_left = left->hit(ray);
			Hit_rec hit_right = right->hit(ray);

			// �������Ҷ����нϺõ�һ��
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

		// ����Ҷ�ӽڵ㣬���辫���ж�ÿ��mesh
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