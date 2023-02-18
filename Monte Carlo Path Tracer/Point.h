#pragma once
// 三维的点或向量都由Point表示
class Point
{
public:
	/*float x;
	float y;
	float z;*/
	float coor[3];
	Point() {}
	//Point(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	Point(float x, float y, float z);
	Point unitize();
	void print();
};

Point vec_minus(Point p1, Point p2);
Point vec_cross(Point p1, Point p2);
Point vec_multi(double d, Point p);

Point operator+(Point p1, Point p2);
Point operator-(Point p1, Point p2);
Point operator*(float f, Point p);
float operator*(Point p1, Point p2);
