#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include "global.h"

using namespace std;

int main(int argc, char** argv)
{
    ifstream file("path.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file\n";
        return 1;
    }
    string XMLpath;
    string OBJpath;
    string MTLroot;
    if (getline(file, XMLpath))
        cout << "Get XML path: " << XMLpath << endl;
    if (getline(file, OBJpath))
        cout << "Get OBJ path: " << OBJpath << endl;
    if (getline(file, MTLroot))
        cout << "Get MTL root: " << MTLroot << endl;
    if (getline(file, TextureFolder))
    {
        if (TextureFolder == "NULL")
            cout << "No texture will be used." << endl;
        else
            cout << "Get Texture Folder: " << TextureFolder << endl;
    }
    file.close();

    if (inputXML(XMLpath) && inputOBJ_MTL(OBJpath, MTLroot))
    {
        cout << "Input succeeded." << endl;
        srand(time(NULL));
        RayTracing();
        cout << "Rendering succeeded." << endl;

        // 绘制未开启光追功能的草图，校验用
        //output(argc, argv);
    }
}