#include "Texture.h"
#include <iostream>
#include "global.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

Texture::Texture(string name, string dir)
{
	this->name = name;
	this->dir = TextureFolder + '/' + dir;
	stbi_info(this->dir.c_str(), &imageWidth, &imageHeight, &imageChannels);
	img = stbi_load(this->dir.c_str(), &imageWidth, &imageHeight, &imageChannels, 0);
	if(!img)
		cerr << "Error: Failed to load texture file - " << name << endl;
}

Point Texture::tex_col(Triangle* triangle, Point point)
{
	// 计算重心坐标w
	Point v0 = triangle->p2 - triangle->p1;
	Point v1 = triangle->p3 - triangle->p1;
	Point v2 = point - triangle->p1;
	float d00 = v0 * v0;
	float d01 = v0 * v1;
	float d11 = v1 * v1;
	float d20 = v2 * v0;
	float d21 = v2 * v1;
	float denom = d00 * d11 - d01 * d01;
	float w_A = (d11 * d20 - d01 * d21) / denom;
	float w_B = (d00 * d21 - d01 * d20) / denom;
	float w_C = 1.0f - w_A - w_B;

	// 计算对应纹理坐标
	float u_P = w_A * triangle->t1.coor[0] + w_B * triangle->t2.coor[0] + w_C * triangle->t3.coor[0];
	float v_P = w_A * triangle->t1.coor[1] + w_B * triangle->t2.coor[1] + w_C * triangle->t3.coor[1];

	// 计算纹理贴图中的位置，考虑超出(0,1)范围的坐标
	int x = int(u_P * imageWidth) % imageWidth;
	int y = int(v_P * imageHeight) % imageHeight;
	if (x < 0)
		x += imageWidth;
	if (y < 0)
		y += imageHeight;

	// 获得纹理贴图的rgb信息
	int pixelIndex = (y * imageWidth + x) * imageChannels;
	float red = img[pixelIndex] / 255.0f;
	float green = img[pixelIndex + 1] / 255.0f;
	float blue = img[pixelIndex + 2] / 255.0f;

	return Point(red, green, blue);
}
