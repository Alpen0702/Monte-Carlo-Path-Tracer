#include "AABB.h"
#include "Triangle.h"
#include <iostream>
#define INF 2147483647

/*AABB::AABB(float x_min, float y_min, float z_min, float x_max, float y_max, float z_max)
{
	this->x_min = x_min;
	this->y_min = y_min;
	this->z_min = z_min;
	this->x_max = x_max;
	this->y_max = y_max;
	this->z_max = z_max;
}*/

AABB::AABB(Point p_min, Point p_max)
{
	this->p_min = p_min;
	this->p_max = p_max;
}

AABB box_union(vector<Triangle*> triangles)
{
    Point new_p_min(INF, INF, INF), new_p_max(-INF, -INF, -INF);
    for (Triangle* &triangle : triangles)
        for (int i = 0; i < 3; i++)
        {
            if (triangle->bounding_box->p_min.coor[i] < new_p_min.coor[i])
                new_p_min.coor[i] = triangle->bounding_box->p_min.coor[i];
            if (triangle->bounding_box->p_max.coor[i] > new_p_max.coor[i])
                new_p_max.coor[i] = triangle->bounding_box->p_max.coor[i];
            //if (triangle->bounding_box->p_min.coor[1] > triangle->bounding_box->p_max.coor[1])
              //  cout << "what?" << endl;
        }
    return AABB(new_p_min, new_p_max);
}

AABB box_union(AABB aabb1, AABB aabb2)
{
    //Point new_p_min = aabb1.p_min, new_p_max = aabb1.p_max;
    //Point new_p_min(aabb1.p_min.coor[0], aabb1.p_min.coor[1], aabb1.p_min.coor[2]), new_p_max(aabb1.p_max.coor[0], aabb1.p_max.coor[1], aabb1.p_max.coor[2]);
    Point new_p_min(INF, INF, INF), new_p_max(-INF, -INF, -INF);

    for (int i = 0; i < 3; i++)
    {
        if (aabb1.p_min.coor[i] < new_p_min.coor[i])
            new_p_min.coor[i] = aabb1.p_min.coor[i];
        if (aabb1.p_max.coor[i] > new_p_max.coor[i])
            new_p_max.coor[i] = aabb1.p_max.coor[i];
        if (aabb2.p_min.coor[i] < new_p_min.coor[i])
            new_p_min.coor[i] = aabb2.p_min.coor[i];
        if (aabb2.p_max.coor[i] > new_p_max.coor[i])
            new_p_max.coor[i] = aabb2.p_max.coor[i];
    }
    //if (new_p_max.coor[1] > -100)
    //    cout << "error";
    return AABB(new_p_min, new_p_max);
}

AABB box_union(AABB* aabb1, AABB* aabb2)
{
    //Point new_p_min = aabb1.p_min, new_p_max = aabb1.p_max;
    //Point new_p_min(aabb1.p_min.coor[0], aabb1.p_min.coor[1], aabb1.p_min.coor[2]), new_p_max(aabb1.p_max.coor[0], aabb1.p_max.coor[1], aabb1.p_max.coor[2]);
    Point new_p_min(INF, INF, INF), new_p_max(-INF, -INF, -INF);

    //if (aabb1->p_min.coor[1] > aabb1->p_max.coor[1])
        //cout << "what?" << endl;

    for (int i = 0; i < 3; i++)
    {
        if (aabb1->p_min.coor[i] < new_p_min.coor[i])
            new_p_min.coor[i] = aabb1->p_min.coor[i];
        if (aabb1->p_max.coor[i] > new_p_max.coor[i])
            new_p_max.coor[i] = aabb1->p_max.coor[i];
        if (aabb2->p_min.coor[i] < new_p_min.coor[i])
            new_p_min.coor[i] = aabb2->p_min.coor[i];
        if (aabb2->p_max.coor[i] > new_p_max.coor[i])
            new_p_max.coor[i] = aabb2->p_max.coor[i];
    }
    //if (new_p_min.coor[1] > new_p_max.coor[1])
        //cout << "strange" << endl;
    return AABB(new_p_min, new_p_max);
}

bool AABB::intersect(Ray ray)
{
    float t_min = 0, t_max = INF;
    for (int i = 0; i < 3; i++)
    {
        //if (ray.dir.coor[i] == 0)

        float invD = 1.0f / ray.dir.coor[i];
        float t0 = (p_min.coor[i] - ray.origin.coor[i]) * invD;
        float t1 = (p_max.coor[i] - ray.origin.coor[i]) * invD;
        if (invD < 0.0f)
        {
            swap(t0, t1);
            //float tmp = t0;
            //t0 = t1;
            //t1 = tmp;
        }
        t_max = t1 < t_max ? t1 : t_max;
        t_min = t0 > t_min ? t0 : t_min;
        if (t_max < t_min)
            return false;
    }
    return true;
}

