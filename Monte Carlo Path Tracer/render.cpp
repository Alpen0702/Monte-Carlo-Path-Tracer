#include "global.h"
#include "Point.h"
#include "Ray.h"
#include "BVH_node.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <chrono>
#include <omp.h>
#define MAX_REFLECT 50
#define DIRECT_RAY_PER_PIXEL 24
#define INDIRECT_RAY_PER_PIXEL 24
#define RAY_PER_PIXEL DIRECT_RAY_PER_PIXEL + INDIRECT_RAY_PER_PIXEL
#define CONTINUE_RATIO 0.8
#define MAX_SUNS 6
#define PI 3.1415926536
using namespace std;

Point eye, lookat, up;
Point lower_left_corner, horizontal, vertical;

// 以屏幕左下角为(0,0)，获得屏幕坐标为(u,v)的光线
Ray get_ray(float u, float v)
{
    return Ray(eye, (lower_left_corner + u * horizontal + v * vertical - eye).unitize());
}

// 获取纹理，若无纹理则返回单位向量
Point get_tex_col(Hit_rec rec)
{
    if (!rec.mtl->diffuse_texname.empty())
        for (Texture texture : textures)
            if (texture.name == rec.mtl->name)
                return texture.tex_col(rec.hit_triangle, rec.hit_pos);
    return Point(1, 1, 1);
}

Point Phong(Point in_dir, Point out_dir, Point col, Hit_rec rec)
{
    // 漫反射
    float cos_d = out_dir * rec.normal;
    Point diffuse = cos_d / PI * Point(rec.mtl->diffuse[0] * col.coor[0], rec.mtl->diffuse[1] * col.coor[1], rec.mtl->diffuse[2] * col.coor[2]);

    // 镜面反射
    Point mirror(in_dir - 2 * (in_dir * rec.normal) * rec.normal);
    float cos_s = mirror.unitize() * out_dir;
    float cos_s_n = pow(cos_s, rec.mtl->shininess);
    Point specular = cos_s_n * (rec.mtl->shininess + 2) / (2 * PI) * Point(rec.mtl->specular[0] * col.coor[0], rec.mtl->specular[1] * col.coor[1], rec.mtl->specular[2] * col.coor[2]);
        
    return diffuse + specular;
    //return specular;
}

Point refraction_dir(Point in_dir, Hit_rec rec, bool in_glass)
{
    float eta = rec.mtl->ior;
    if (!in_glass)
        eta = 1.0f / eta;
    float c1 = in_dir * rec.normal;
    float c2 = sqrt(1 - eta * eta * (1 - c1 * c1));
    return (eta * (in_dir + c1 * rec.normal) - c2 * rec.normal).unitize();
}

// 递归返回采样颜色（已废弃）
Point color(BVH_node* root, Ray ray, int depth)
{
    Hit_rec rec = root->hit(ray);
    if (rec.hit)
    {
        // 若是光源，则结束
        for (Light light : lights)
            if (rec.mtl->name == light.mtlname)
            {
                // 计算光线出射方向与光源法向的夹角
                float cos = ray.dir * rec.normal;
                return -cos * Point(light.radiance_r, light.radiance_g, light.radiance_b);
            }

        // 若不是光源：
        // 若超过最大反射次数，则强制结束反射
        if (depth > MAX_REFLECT)
            return Point(0, 0, 0);

        Point next_dir;
        
        // 轮盘赌决定是否继续反射
        float r = rand() / double(RAND_MAX);
        if (r < CONTINUE_RATIO)
        {
            // 若未到达最大反射次数，则继续自由反射
            if (depth < MAX_REFLECT)
            {
                // 均匀半球上随机生成下一方向
                float x = rand() / double(RAND_MAX);
                float y = rand() / double(RAND_MAX);
                float z = rand() / double(RAND_MAX);
                next_dir = Point(x, y, z).unitize();
            }
            // 若到达最大反射次数，则自动引导至某一光源
            else
            {
                int idx = int(rand() % suns.size());
                // 在三角形上均匀随机取点 https://www.cs.princeton.edu/~funk/tog02.pdf
                float r1 = rand() / double(RAND_MAX);
                float r2 = rand() / double(RAND_MAX);
                Point rp = (1 - sqrt(r1)) * suns[idx]->p1 + sqrt(r1) * (1 - r2) * suns[idx]->p2 + sqrt(r1) * r2 * suns[idx]->p3;
                //cout << rp.coor[0] <<' '<< rp.coor[1] <<' '<< rp.coor[2] << endl;
                //cout << r1 <<' '<< r2 << endl;
                next_dir = (rp - rec.hit_pos).unitize();
                //next_dir = (suns[idx]->p1 - rec.hit_pos).unitize();
            }

            // 计算下一方向与法向的夹角
            // 考虑面法向朝内和朝外
            if (ray.dir * rec.normal > 0)
                rec.normal = Point(0, 0, 0) - rec.normal;
            float cos = next_dir * rec.normal;
            if (cos < 0)
            {
                cos = -cos;
                next_dir = Point(0, 0, 0) - next_dir;
                //Point(-next_dir.coor[0], -next_dir.coor[1], -next_dir.coor[2]);
            }

            Ray next_ray(rec.hit_pos, next_dir);
            Point next_col = color(root, next_ray, depth + 1);
            return 1.0f / CONTINUE_RATIO * Phong(ray.dir, next_dir, next_col, rec);
            //return 1.0f / CONTINUE_RATIO * cos * Point(rec.mtl->diffuse[0] * next_col.coor[0], rec.mtl->diffuse[1] * next_col.coor[1], rec.mtl->diffuse[2] * next_col.coor[2]);
        }
        else
            return Point(0, 0, 0);
    }
    return Point(0, 0, 0);
}

Point direct_col(BVH_node* root, Ray ray, bool in_glass)
{
    Hit_rec rec = root->hit(ray);
    if (rec.hit)
    {
        // 若是光源，则结束
        for (Light light : lights)
            if (rec.mtl->name == light.mtlname)
            {
                // 计算光线出射方向与光源法向的夹角
                float cos = ray.dir * rec.normal;
                return -cos * Point(light.radiance_r, light.radiance_g, light.radiance_b);
            }

        // 若不是光源，计算直接光照
        Point tot_direct(0, 0, 0);
        float total_r = 0;
        float total_g = 0;
        float total_b = 0;

        // 折射
        /*Point refraction(0, 0, 0);
        if (abs(rec.mtl->ior - 1) > .0001)
        {
            Point next_dir = refraction_dir(ray.dir, rec, in_glass);
            Ray next_ray(rec.hit_pos, next_dir);
            Point next_col;
            if (in_glass)
                next_col = direct_col(root, next_ray, 0);
            else
                next_col = direct_col(root, next_ray, 1);

            refraction = Point(rec.mtl->transmittance[0] * next_col.coor[0], rec.mtl->transmittance[1] * next_col.coor[1], rec.mtl->transmittance[2] * next_col.coor[2]);

            // 添加纹理
            Point tex = get_tex_col(rec);
            refraction = Point(refraction.coor[0] * tex.coor[0], refraction.coor[1] * tex.coor[1], refraction.coor[2] * tex.coor[2]);

            return refraction;
        }*/

        // 多线程伪随机
        int ompid = omp_get_thread_num() * 10;

        vector<Sun*> chosen_suns = suns;
        // 特殊照顾
        if (suns.size() == 496)
        {
            chosen_suns.clear();
            for (int i = 0; i < 3; i++)
                chosen_suns.push_back(suns[rand() * static_cast<unsigned long long>(ompid) % 480]);
            chosen_suns.push_back(suns[480 + rand() * static_cast<unsigned long long>(ompid) % 8]);
            for (int i = 488; i < 496; i++)
                if((rand() * ompid % RAND_MAX) / double(RAND_MAX) <0.5)
                    chosen_suns.push_back(suns[i]);
        }
        // 随机选择一些光源
        else if (suns.size() > MAX_SUNS)
        {
            chosen_suns.clear();
            for (int i = 0; i < MAX_SUNS; i++)
                //chosen_suns.push_back(suns[rand() % suns.size()]);
                chosen_suns.push_back(suns[rand() * static_cast<unsigned long long>(ompid) % suns.size()]);// , cout << rand() * static_cast<unsigned long long>(ompid) % suns.size() << endl;
        }

        // 并行进行光源运算（已废弃）
        //#pragma omp parallel for reduction(+:total_r,total_g,total_b)

        for (int id = 0; id < chosen_suns.size(); id++)
        {
            Sun* sun = chosen_suns[id];

            // 在三角形上均匀随机取点 https://www.cs.princeton.edu/~funk/tog02.pdf
            //float r1 = rand() / double(RAND_MAX);
            //float r2 = rand() / double(RAND_MAX);
            float r1 = (rand() * ompid % RAND_MAX) / double(RAND_MAX);
            float r2 = (rand() * ompid % RAND_MAX) / double(RAND_MAX);
            Point rp = (1 - sqrt(r1)) * sun->p1 + sqrt(r1) * (1 - r2) * sun->p2 + sqrt(r1) * r2 * sun->p3;
            Point next_dir = rp - rec.hit_pos;
            if (rec.normal * next_dir <= 0)
                continue;

            float cosine = -(next_dir.unitize() * sun->normal);
            if (cosine <= 0)
                continue;
            float dist2 = next_dir * next_dir;
            float pdf = 1.0f / sun->S;
            
            Ray to_sun(rec.hit_pos, next_dir);
            Hit_rec rec_to_sun = root->hit(to_sun);
            Point error = rec_to_sun.hit_pos - rp;

            // 从物体发射的光线成功连接到了光源
            if (rec_to_sun.hit && error * error < .0001)
            {
                Point tot = 1.0f / pdf * cosine / dist2 * Phong(ray.dir, next_dir.unitize(), sun->radiance, rec);
                total_r += tot.coor[0];
                total_g += tot.coor[1];
                total_b += tot.coor[2];
            }
        }
        // 添加纹理
        Point tex = get_tex_col(rec);

        // 特殊照顾
        if (suns.size() == 496)
            return 2 * Point(total_r * tex.coor[0], total_g * tex.coor[1], total_b * tex.coor[2]);

        return suns.size() / chosen_suns.size() * Point(total_r * tex.coor[0], total_g * tex.coor[1], total_b * tex.coor[2]);
    }
    return Point(0, 0, 0);
}

Point indirect_col(BVH_node* root, Ray ray, int depth, bool in_glass)
{
    Hit_rec rec = root->hit(ray);
    if (rec.hit)
    {
        // 若是光源，则结束
        for (Light light : lights)
            if (rec.mtl->name == light.mtlname)
            {
                // 若深度为0，意味着直接照射光源；若深度为1，意味着打到的物体直接连接光源。这两种情况都包括在direct_col中，故在此不应重复添加。
                if (depth <= 1)
                    return Point(0, 0, 0);
                return Point(light.radiance_r, light.radiance_g, light.radiance_b);
            }

        // 若不是光源：
        // 若超过最大反射次数，则强制结束反射
        if (depth >= MAX_REFLECT)
            return Point(0, 0, 0);

        // 多线程伪随机
        int ompid = (omp_get_thread_num() + 1) * 10;

        // 轮盘赌决定是否继续反射
        //float r = rand() / double(RAND_MAX);
        float r = (rand() * ompid % RAND_MAX) / double(RAND_MAX);
        //cout << r << endl;
        if (r < CONTINUE_RATIO)
        {
            
            /*
            // （已废弃）均匀半球上随机生成下一方向
            float x = rand() / double(RAND_MAX);
            float y = rand() / double(RAND_MAX);
            float z = rand() / double(RAND_MAX);
            next_dir = Point(x, y, z).unitize();
            */

            // 考虑面法向朝内和朝外
            if (ray.dir * rec.normal > 0)
                rec.normal = Point(0, 0, 0) - rec.normal;

            // 折射
            Point refraction(0, 0, 0);
            if (abs(rec.mtl->ior - 1) > .0001)
            {
                Point next_dir = refraction_dir(ray.dir, rec, in_glass);
                Ray next_ray(rec.hit_pos, next_dir);
                Point next_col;
                if (in_glass)
                    next_col = indirect_col(root, next_ray, depth + 1, 0);
                else
                    next_col = indirect_col(root, next_ray, depth + 1, 1);

                refraction = 1.0f / CONTINUE_RATIO * Point(rec.mtl->transmittance[0] * next_col.coor[0], rec.mtl->transmittance[1] * next_col.coor[1], rec.mtl->transmittance[2] * next_col.coor[2]);
                
                // 添加纹理
                Point tex = get_tex_col(rec);
                refraction = Point(refraction.coor[0] * tex.coor[0], refraction.coor[1] * tex.coor[1], refraction.coor[2] * tex.coor[2]);
                
                return refraction;
            }

            // 基于BRDF采样下一方向
            //float u0 = rand() / double(RAND_MAX);
            float u0 = (rand() * ompid % RAND_MAX) / double(RAND_MAX);
            float sum_d = rec.mtl->diffuse[0] + rec.mtl->diffuse[1] + rec.mtl->diffuse[2];
            float sum_s = rec.mtl->specular[0] + rec.mtl->specular[1] + rec.mtl->specular[2];
            float sum_ds = sum_d + sum_s;
            float ds_ratio = sum_ds / 3;
            
            float pdf;
            Point next_dir;
            //float u1 = rand() / double(RAND_MAX);
            //float u2 = rand() / double(RAND_MAX);
            float u1 = (rand() * ompid % RAND_MAX) / float(RAND_MAX);
            float u2 = (rand() * ompid % RAND_MAX) / float(RAND_MAX);
            //cout << u1 << ' ' << u2 << endl;
            
            // 漫反射
            if (u0 < sum_d / sum_ds)
            {
                float x, y, z, theta, phi;
                theta = acos(sqrt(u1));
                phi = 2 * PI * u2;
                x = sin(theta) * cos(phi);
                y = sin(theta) * sin(phi);
                z = cos(theta);
                next_dir = Point(x, y, z).unitize();

                // 转移到局部坐标系
                Point vup(1, 0, 0);
                if (abs(rec.normal * Point(1, 0, 0)) > 0.9)
                    vup = Point(0, 1, 0);
                Point t(vec_cross(vup, rec.normal).unitize());
                Point s(vec_cross(t, rec.normal).unitize());
                next_dir = next_dir.coor[0] * s + next_dir.coor[1] * t + next_dir.coor[2] * rec.normal;

                // 递归到下一条光线
                Ray next_ray(rec.hit_pos, next_dir);
                Point next_col = indirect_col(root, next_ray, depth + 1, 0);
                
                /*
                pdf = 1.0f / PI * cos(theta);
                return 1.0f / CONTINUE_RATIO * ds_ratio / pdf / PI * cos(theta) * Point(rec.mtl->diffuse[0] * next_col.coor[0], rec.mtl->diffuse[1] * next_col.coor[1], rec.mtl->diffuse[2] * next_col.coor[2]);
                上两式经约分，等价于下式
                */
                Point indirect_diffuse = 1.0f / CONTINUE_RATIO * ds_ratio * Point(rec.mtl->diffuse[0] * next_col.coor[0], rec.mtl->diffuse[1] * next_col.coor[1], rec.mtl->diffuse[2] * next_col.coor[2]);
                
                // 添加纹理
                Point tex = get_tex_col(rec);
                indirect_diffuse = Point(indirect_diffuse.coor[0] * tex.coor[0], indirect_diffuse.coor[1] * tex.coor[1], indirect_diffuse.coor[2] * tex.coor[2]);

                return indirect_diffuse;
            }

            // 镜面反射
            else
            {
                float x, y, z, alpha, phi;
                alpha = acos(pow(u1, 1 / (1 + rec.mtl->shininess)));
                phi = 2 * PI * u2;
                x = sin(alpha) * cos(phi);
                y = sin(alpha) * sin(phi);
                z = cos(alpha);
                next_dir = Point(x, y, z).unitize();

                // 转移到局部坐标系
                Point perfect_angle(ray.dir - 2 * (ray.dir * rec.normal) * rec.normal);
                Point vup(1, 0, 0);
                if (perfect_angle * Point(1, 0, 0) > 0.9)
                    vup = Point(0, 1, 0);
                Point t(vec_cross(vup, perfect_angle).unitize());
                Point s(vec_cross(t, perfect_angle).unitize());
                next_dir = next_dir.coor[0] * s + next_dir.coor[1] * t + next_dir.coor[2] * perfect_angle;

                // 递归到下一条光线
                Ray next_ray(rec.hit_pos, next_dir);
                Point next_col = indirect_col(root, next_ray, depth + 1, 0);

                /*
                pdf = (rec.mtl->shininess + 1) / PI / 2 * pow(cos(alpha), rec.mtl->shininess);
                return 1.0f / CONTINUE_RATIO * ds_ratio / pdf / (2 * PI) * (rec.mtl->shininess + 2) * pow(cos(alpha), rec.mtl->shininess) * Point(rec.mtl->specular[0] * next_col.coor[0], rec.mtl->specular[1] * next_col.coor[1], rec.mtl->specular[2] * next_col.coor[2]);
                上两式经约分，等价于下式
                */
                Point indirect_specular = 1.0f / CONTINUE_RATIO * ds_ratio / (rec.mtl->shininess + 1) * (rec.mtl->shininess + 2) * Point(rec.mtl->specular[0] * next_col.coor[0], rec.mtl->specular[1] * next_col.coor[1], rec.mtl->specular[2] * next_col.coor[2]);
                
                // 添加纹理
                Point tex = get_tex_col(rec);
                indirect_specular = Point(indirect_specular.coor[0] * tex.coor[0], indirect_specular.coor[1] * tex.coor[1], indirect_specular.coor[2] * tex.coor[2]);

                return indirect_specular;
            }

        }
        else
            return Point(0, 0, 0);
    }
    return Point(0, 0, 0);
}

void RayTracing()
{
    // 记录开始时间
    auto t1 = std::chrono::high_resolution_clock::now();

    BVH_node* root = new BVH_node(world, 0);
    
    eye = Point(camera.eye_x, camera.eye_y, camera.eye_z);
    lookat = Point(camera.lookat_x, camera.lookat_y, camera.lookat_z);
    up = Point(camera.up_x, camera.up_y, camera.up_z);

    // 刻画距相机一单位距离的屏幕
    float theta = camera.fovy * PI / 180;
    float half_height = tan(theta / 2);
    float half_width = float(camera.width) / camera.height * half_height;

    // w是屏幕反方向向量，u是屏幕向右向量，v是屏幕向上向量
    Point w = (eye - lookat).unitize();
    Point u = vec_cross(up, w).unitize();
    Point v = vec_cross(w, u);

    // lower_left_corner是距相机一单位距离屏幕的左下角位置，通过加上0-1之间倍率的horizontal向量和vertical向量以获取屏幕任意位置
    lower_left_corner = eye - (half_width * u) - (half_height * v) - w;
    horizontal = 2 * half_width * u;
    vertical = 2 * half_height * v;

    ofstream OutImage;
    OutImage.open("../results/Image.ppm");

    //OutImage << "P3\n" << 100 << ' ' << 100 << "\n255\n";
    OutImage << "P3\n" << camera.width << ' ' << camera.height << "\n255\n";

    // 检查是否支持omp多线程运作。若顺利，控制台将输出可使用线程数个"parallel run!!!"
    #pragma omp parallel
    {
        cout << "parallel run!!!\n";
    }

    int tens1 = int(camera.height / 10 * 9);
    int tens2 = int(camera.height / 10 * 8);
    int tens3 = int(camera.height / 10 * 7);
    int tens4 = int(camera.height / 10 * 6);
    int tens5 = int(camera.height / 10 * 5);
    int tens6 = int(camera.height / 10 * 4);
    int tens7 = int(camera.height / 10 * 3);
    int tens8 = int(camera.height / 10 * 2);
    int tens9 = int(camera.height / 10 * 1);

    // 以像素左下角坐标作为像素坐标
    //for (int j = 300; j > 200; j--)
    for (int j = camera.height - 1; j >= 0; j--)
    {
        if (j == tens1)
            cout << "10% done." << endl;
        if (j == tens2)
            cout << "20% done." << endl;
        if (j == tens3)
            cout << "30% done." << endl;
        if (j == tens4)
            cout << "40% done." << endl;
        if (j == tens5)
            cout << "50% done." << endl;
        if (j == tens6)
            cout << "60% done." << endl;
        if (j == tens7)
            cout << "70% done." << endl;
        if (j == tens8)
            cout << "80% done." << endl;
        if (j == tens9)
            cout << "90% done." << endl;

        //for (int i = 600; i < 700; i++)
        for (int i = 0; i < camera.width; i++)
        {
            Point total_col(0, 0, 0);
            float direct_r = 0;
            float direct_g = 0;
            float direct_b = 0;
            float indirect_r = 0;
            float indirect_g = 0;
            float indirect_b = 0;
            
            // 每像素采样RAY_PER_PIXEL条光线，并加一个小随机项以抗锯齿
            float u[RAY_PER_PIXEL];
            float v[RAY_PER_PIXEL];
            for (int k = 0; k < RAY_PER_PIXEL; k++)
            {
                u[k] = float(i + rand() / double(RAND_MAX)) / camera.width;
                v[k] = float(j + rand() / double(RAND_MAX)) / camera.height;
            }

            /* 对direct和indirect采样相同数量的光线（已废弃）
            #pragma omp parallel for reduction(+:total_r,total_g,total_b)
            for (int k = 0; k < RAY_PER_PIXEL; k++)
            {
                // 递归返回采样颜色（已废弃）
                //float u = float(i) / camera.width;
                //float v = float(j) / camera.height;
                //Point col = color(root, get_ray(u, v), 0);
                
                //Point direct = direct_col(root, get_ray(u[k], v[k]));
                Point indirect = indirect_col(root, get_ray(u[k], v[k]), 0);
                Point col = indirect;// + indirect;

                total_r += col.coor[0];
                total_g += col.coor[1];
                total_b += col.coor[2];
            }*/

            #pragma omp parallel for reduction(+: direct_r, direct_g, direct_b)
            for (int k = 0; k < DIRECT_RAY_PER_PIXEL; k++)
            {
                Point direct = direct_col(root, get_ray(u[k], v[k]), 0);

                direct_r += direct.coor[0];
                direct_g += direct.coor[1];
                direct_b += direct.coor[2];
            }

            #pragma omp parallel for reduction(+: indirect_r, indirect_g, indirect_b)
            for (int k = DIRECT_RAY_PER_PIXEL; k < RAY_PER_PIXEL; k++)
            {
                Point indirect = indirect_col(root, get_ray(u[k], v[k]), 0, 0);

                indirect_r += indirect.coor[0];
                indirect_g += indirect.coor[1];
                indirect_b += indirect.coor[2];
            }
            
            // 伽马校正
            float gamma = 1.0f / 2.2;

            int r = int(pow(direct_r / DIRECT_RAY_PER_PIXEL + indirect_r / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int g = int(pow(direct_g / DIRECT_RAY_PER_PIXEL + indirect_g / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int b = int(pow(direct_b / DIRECT_RAY_PER_PIXEL + indirect_b / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);

            // 调试：仅显示direct
            /*int r = int(pow(direct_r / DIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int g = int(pow(direct_g / DIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int b = int(pow(direct_b / DIRECT_RAY_PER_PIXEL, gamma) * 255.99);*/

            // 调试：仅显示indirect
            /*int r = int(pow(indirect_r / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int g = int(pow(indirect_g / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);
            int b = int(pow(indirect_b / INDIRECT_RAY_PER_PIXEL, gamma) * 255.99);*/
            
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            
            OutImage << r << ' ' << g << ' ' << b << '\n';
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    float seconds = duration / 1000000.0;
    printf("Time(s): %f\n", seconds);
}