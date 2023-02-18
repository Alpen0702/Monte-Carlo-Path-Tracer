#include "Point.h"
#include <cmath>
#include <iostream>
using namespace std;

Point operator+(Point p1, Point p2)
{
	return Point(p1.coor[0] + p2.coor[0], p1.coor[1] + p2.coor[1], p1.coor[2] + p2.coor[2]);
}

Point operator-(Point p1, Point p2)
{
	return Point(p1.coor[0] - p2.coor[0], p1.coor[1] - p2.coor[1], p1.coor[2] - p2.coor[2]);
}

Point operator*(float f, Point p)
{
	return Point(f * p.coor[0], f * p.coor[1], f * p.coor[2]);
}

float operator*(Point p1, Point p2)
{
	return p1.coor[0] * p2.coor[0] + p1.coor[1] * p2.coor[1] + p1.coor[2] * p2.coor[2];
}

Point vec_cross(Point p1, Point p2)
{
	double x, y, z;
	x = p1.coor[1] * p2.coor[2] - p1.coor[2] * p2.coor[1];
	y = p1.coor[2] * p2.coor[0] - p1.coor[0] * p2.coor[2];
	z = p1.coor[0] * p2.coor[1] - p1.coor[1] * p2.coor[0];
	return Point(x, y, z);
}

Point::Point(float x, float y, float z)
{
	coor[0] = x; 
	coor[1] = y;
	coor[2] = z; 
}

Point Point::unitize()
{
	double l = sqrt(coor[0] * coor[0] + coor[1] * coor[1] + coor[2] * coor[2]);
	return Point(coor[0] / l, coor[1] / l, coor[2] / l);
}

void Point::print()
{
	cout << coor[0] << ' ' << coor[1] << ' ' << coor[2] << endl;
}

