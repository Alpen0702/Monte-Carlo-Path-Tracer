#pragma once
#include <string>
#include <vector>
#include <tiny_obj_loader.h>
#include "Triangle.h"
#include "Sun.h"
#include "Texture.h"

using namespace std;
using namespace tinyobj;

struct Camera {
    string type;
    int width;
    int height;
    double fovy;
    double eye_x;
    double eye_y;
    double eye_z;
    double lookat_x;
    double lookat_y;
    double lookat_z;
    double up_x;
    double up_y;
    double up_z;
};
extern Camera camera;

struct Light {
    string mtlname;
    double radiance_r;
    double radiance_g;
    double radiance_b;
};
extern vector<Light> lights;

extern attrib_t attrib;
extern vector<shape_t> shapes;
extern vector<material_t> materials;
extern vector<Triangle*> world;
extern vector<Sun*> suns;
extern vector<Texture> textures;
extern string TextureFolder;

int inputXML(string XMLpath);
int inputOBJ_MTL(string OBJpath, string MTLroot);
void output(int argc, char** argv);
void RayTracing();
