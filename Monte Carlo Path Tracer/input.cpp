#include "global.h"
#include <tinyxml.h>
#include <tiny_obj_loader.h>
#include <vector>
using namespace std;
using namespace tinyobj;

int inputXML(string XMLpath)
{
    TiXmlDocument doc(XMLpath);
    if (!doc.LoadFile()) {
        cerr << "Failed to load file\n";
        return 0;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if (root == nullptr) {
        cerr << "Failed to find root element\n";
        doc.Clear();
        return 0;
    }

    TiXmlElement* camera_elem = root;// ->FirstChildElement("camera");
    if (camera_elem == nullptr) {
        cerr << "Failed to find camera element\n";
        doc.Clear();
        return 0;
    }

    //Camera camera;
    camera.type = camera_elem->Attribute("type");
    camera.width = stoi(camera_elem->Attribute("width"));
    camera.height = stoi(camera_elem->Attribute("height"));
    camera.fovy = stod(camera_elem->Attribute("fovy"));

    TiXmlElement* eye_elem = camera_elem->FirstChildElement("eye");
    camera.eye_x = stod(eye_elem->Attribute("x"));
    camera.eye_y = stod(eye_elem->Attribute("y"));
    camera.eye_z = stod(eye_elem->Attribute("z"));

    TiXmlElement* lookat_elem = camera_elem->FirstChildElement("lookat");
    camera.lookat_x = stod(lookat_elem->Attribute("x"));
    camera.lookat_y = stod(lookat_elem->Attribute("y"));
    camera.lookat_z = stod(lookat_elem->Attribute("z"));

    TiXmlElement* up_elem = camera_elem->FirstChildElement("up");
    camera.up_x = stod(up_elem->Attribute("x"));
    camera.up_y = stod(up_elem->Attribute("y"));
    camera.up_z = stod(up_elem->Attribute("z"));

    TiXmlElement* light_elem = root->NextSiblingElement();
    while (light_elem)
    {
        Light light;
        light.mtlname = light_elem->Attribute("mtlname");
        string radiance_str = light_elem->Attribute("radiance");
        vector<string> radiance_parts;
        size_t pos = 0;
        while ((pos = radiance_str.find(',')) != string::npos) {
            radiance_parts.push_back(radiance_str.substr(0, pos));
            radiance_str.erase(0, pos + 1);
        }
        radiance_parts.push_back(radiance_str);
        light.radiance_r = stod(radiance_parts[0]);
        light.radiance_g = stod(radiance_parts[1]);
        light.radiance_b = stod(radiance_parts[2]);
        lights.push_back(light);
        light_elem = light_elem->NextSiblingElement();
    }
    return 1;
}

int inputOBJ_MTL(string OBJpath, string MTLroot)
{
    string inputfile = OBJpath;
    ObjReaderConfig reader_config;
    reader_config.mtl_search_path = MTLroot;

    ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) 
    {
        if (!reader.Error().empty())
            cerr << "TinyObjReader: " << reader.Error();
        exit(-1);
    }

    if (!reader.Warning().empty()) 
        cout << "TinyObjReader: " << reader.Warning();

    attrib = reader.GetAttrib();
    shapes = reader.GetShapes();
    materials = reader.GetMaterials();

    // 遍历体
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        // 遍历面
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            Point pts[3];
            Point txs[3];
            Point avg_norm(0, 0, 0);

            // 遍历点
            for (size_t v = 0; v < fv; v++) 
            {
                index_t idx = shapes[s].mesh.indices[index_offset + v];
                real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                
                pts[v].coor[0] = vx;
                pts[v].coor[1] = vy;
                pts[v].coor[2] = vz;

                if (idx.normal_index >= 0) 
                {
                    real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    avg_norm = avg_norm + Point(nx, ny, nz);
                }

                if (idx.texcoord_index >= 0) 
                {
                    real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    txs[v].coor[0] = tx;
                    txs[v].coor[1] = ty;
                }

            }

            Triangle* triangle = new Triangle(pts[0], pts[1], pts[2]);
            triangle->t1 = txs[0];
            triangle->t2 = txs[1];
            triangle->t3 = txs[2];
            world.push_back(triangle);

            index_offset += fv;

            // 面的材料
            triangle->mtl = &materials[shapes[s].mesh.material_ids[f]];
            
            // 收集光源
            for (Light light : lights)
                if (triangle->mtl->name == light.mtlname)
                {
                    Sun* sun = new Sun(triangle, avg_norm);
                    sun->radiance = Point(light.radiance_r, light.radiance_g, light.radiance_b);
                    suns.push_back(sun);
                    break;
                }
            
            // 收集纹理
            if (triangle->mtl->diffuse_texname != "")
            {
                bool inside = 0;
                for (Texture texture : textures)
                    if (triangle->mtl->name == texture.name)
                    {
                        inside = 1;
                        break;
                    }
                if (!inside)
                    textures.push_back(Texture(triangle->mtl->name, triangle->mtl->diffuse_texname));
            }

            // 处理透射读不进来的问题
            if (triangle->mtl->ior == 1.5)
            {
                triangle->mtl->transmittance[0] = 0.8;
                triangle->mtl->transmittance[1] = 1;
                triangle->mtl->transmittance[2] = 0.95;
            }
        }
    }
    return 1;
}