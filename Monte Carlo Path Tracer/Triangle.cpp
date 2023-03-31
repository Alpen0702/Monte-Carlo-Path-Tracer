#include "Triangle.h"
#include <iostream>
#define INF 2147483647

Triangle::Triangle(Point p1, Point p2, Point p3)
{
	this->p1 = p1;
	this->p2 = p2;
	this->p3 = p3;
    Point p_min(INF, INF, INF), p_max(-INF, -INF, -INF);
    for (int i = 0; i < 3; i++)
    {
        if (p1.coor[i] < p_min.coor[i])
            p_min.coor[i] = p1.coor[i];
        if (p1.coor[i] > p_max.coor[i])
            p_max.coor[i] = p1.coor[i];
        if (p2.coor[i] < p_min.coor[i])
            p_min.coor[i] = p2.coor[i];
        if (p2.coor[i] > p_max.coor[i])
            p_max.coor[i] = p2.coor[i];
        if (p3.coor[i] < p_min.coor[i])
            p_min.coor[i] = p3.coor[i];
        if (p3.coor[i] > p_max.coor[i])
            p_max.coor[i] = p3.coor[i];
    }
    bounding_box = new AABB(p_min, p_max);
}

Hit_rec Triangle::hit(Ray ray)
{
    // �����ε�����������
    Point u = p2 - p1;
    Point v = p3 - p1;

    //if (v.coor[0] < -500)
        //cout << "hh";

    // ����������ƽ��ķ�����
    Point n = vec_cross(u, v).unitize();
    if (ray.dir * n > 0)
        n = Point(0, 0, 0) - n;

    // ��������߷���ĵ��
    float n_multi_dir = n * ray.dir;

    // �����Ϊ0��ƽ�������ƽ��
    if (abs(n_multi_dir) < 0.0001)
        return Hit_rec();

    // ������ߵ�ƽ��ʱ��t
    float t = n * (p1 - ray.origin) / n_multi_dir;
    if (t < 0.0001)
        return Hit_rec();

    // ���������ƽ�潻��
    Point p = ray.origin + t * ray.dir;

    // �жϽ����Ƿ����������ڣ������������εı�������ʾ��������
    float uu = u * u;
    float uv = u * v;
    float vv = v * v;
    Point w = p - p1;
    float wu = w * u;
    float wv = w * v;
    float D = uv * uv - uu * vv;
    float alpha = (uv * wv - vv * wu) / D;
    float beta = (uv * wu - uu * wv) / D;
    
    // ��ʾ�ɹ�
    if (alpha >= 0 && beta >= 0 && alpha + beta <= 1)
        return Hit_rec(p, t, mtl, n, this);
    
    // ��ʾʧ��
    return Hit_rec();
}