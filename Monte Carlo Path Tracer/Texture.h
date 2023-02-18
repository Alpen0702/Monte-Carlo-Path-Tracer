#pragma once
#include <string>
#include "Triangle.h"
#include "Point.h"

using namespace std;

class Texture
{
public:
	string name = "";
	string dir = "";
	int imageWidth, imageHeight, imageChannels;
	unsigned char* img;
	Texture(string name, string dir);
	Point tex_col(Triangle* triangle, Point point);
};

