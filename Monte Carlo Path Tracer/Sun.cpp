#include "Sun.h"

Sun::Sun(Triangle* triangle, Point avg_norm)
{
	bounding_box = triangle->bounding_box;
	mtl = triangle->mtl;
	p1 = triangle->p1;
	p2 = triangle->p2;
	p3 = triangle->p3;

	// 海伦公式求三角形面积
	float a = sqrt((p2 - p3) * (p2 - p3));
	float b = sqrt((p1 - p3) * (p1 - p3));
	float c = sqrt((p1 - p2) * (p1 - p2));
	float p = (a + b + c) / 2;
	S = sqrt(p * (p - a) * (p - b) * (p - c));

	normal = vec_cross(p2 - p1, p3 - p2).unitize();
	if (avg_norm * normal < 0)
		normal = Point(0, 0, 0) - normal;
}
