# Monte Carlo Path Tracer

这里是浙江大学2022冬学期计算机图形学作业：实现一个Monte Carlo Path Tracer。

因为只剩两个小时就要交了而且还没渲完！所以就很简略地写一下文档⑧

## 程序结构与实现思路

首先，用读入要渲染的信息！所以在input.cpp中读入了XML、MTL和OBJ。

在渲染之前，准备一下要用到的几何信息和数据结构，并且顺带实现一些简单的几何求交啦向量乘法啦三角形求随机内点啦之类的。主要的类包括：
* Point - 空间中的点、三维向量（rgb）
* Triangle - 三角形
* AABB - 包围盒
* BVH_node - BVH树的节点，用于快速求光线与世界的交点
* Sun - 光源
* Ray - 光线，包括起始点和方向
* Hit_rec - 光线与物体相交的信息，包括时间、位置、法向、物体的材质等
* Texture - 纹理

紧接着，就开始渲染了，主要内容在Render.cpp中。主要思路是把光照分成直接光照和间接光照两部分。对直接光照，从相机射出光线打在物体上，然后直接与光源连接，用光源面积、距离等信息求亮度；对间接光照，从相机射出光线打在物体上，然后基于BRDF采样下一方向，递归求亮度。这里计算亮度用的是Phong模型，详见Reference。

渲染需要很久很久很久，我没有用GPU加速，不过试着用了一下CPU上的多线程，有一点点点的效果。渲染结束后，把直接光照和间接光照的结果加起来，并对全图做一次伽马校正，输出结果图，结束！

## 使用说明

本项目用到的外部（就是说要下载的那种）库主要有：
* stb_image.h - 用来读入纹理图
* tiny_obj_loader.h - 用来加载obj文件
* tinyxml.h - 用来加载xml文件
* GL/freeglut.h - 其实没用，就是用来呈现一下不做光照的场景长啥样，跟光追做个对比

如果想要使用本项目，你应该先下载它，然后安装相应的库，并在编译器中选择开启openmp（用来多线程，如果不用的话也可以忽略），然后编译运行。如果失败，也可以在项目文件夹中找到Monte Carlo Path Tracer.exe试着运行，但要记得把path.txt移到同一路径下。

path.txt是用来引导程序找到资源文件的，资源文件打包在example-scenes-cg22.zip中。path.txt包含四行，第一行是xml文件位置，第二行是obj文件位置，第三行是mtl文件所位于的路径，第四行是纹理文件所位于的路径。如果没有纹理，则第四行为"NULL"。若要使用资源包，应该解压之，然后放在项目文件夹的父级文件夹中。

例如：

```
../example-scenes-cg22\staircase\staircase.xml
../example-scenes-cg22\staircase\stairscase.obj
../example-scenes-cg22\staircase
../example-scenes-cg22\staircase
```

运行项目前，你可以在render.cpp中修改一些参数，以达到你想要的目的。这些参数包括：
* MAX_REFLECT - 间接光照的最大反射次数，超过则截断
* DIRECT_RAY_PER_PIXEL - 每个像素的直接光照光线数量
* INDIRECT_RAY_PER_PIXEL - 每个像素的间接光照光线数量
* CONTINUE_RATIO - 在间接光照的俄罗斯轮盘赌中，光线继续反射的概率
* MAX_SUNS - 在直接光照中，光源三角形的数量上限，超过则随机选择其中MAX_SUNS个，这是为了在一些特殊情况下提高运行效率

运行完毕后，程序会在../results目录下生成一个imgae.ppm文件。这是一个位图文件，可以用Photoshop等软件打开查看渲染结果。

## 看看结果

* veach-mis (DIRECT_RAY_PER_PIXEL = 12, INDIRECT_RAY_PER_PIXEL = 108, CONTINUE_RATIO = 0.8, MAX_SUNS = 6, RUN_TIME ≈ 17min)

* cornell-box (DIRECT_RAY_PER_PIXEL = 12, INDIRECT_RAY_PER_PIXEL = 108, CONTINUE_RATIO = 0.8, RUN_TIME ≈ 56min)

* staircase (DIRECT_RAY_PER_PIXEL = 24, INDIRECT_RAY_PER_PIXEL = 24, CONTINUE_RATIO = 0.8, MAX_SUNS = ?, RUN_TIME ≈ 1h23min)

## 做得好的地方
* 用BVH树做加速结构，减少光线与世界求交用时。
* 直接光照和间接光照分别处理，体现了蒙特卡洛采样的思想。有些场景下应该还能再在间接光照中多去直接找光源，可以更快地降低方差。
* 对直接光照和间接光照选取不同数量的光线，因为实验中发现直接光照很快就收敛了，可以节约时间给间接光照更多机会。
* 间接光照用了基于BRDF函数的采样来代替半球随机采样，也是蒙特卡洛采样的思想体现，对镜面反射效果尤其好。
* 在staircase中，针对光源的特性，选择对数量大的小光源减少采样，对数量少的大光源增加采样，用尽量少的资源达到相同的效果。
* 尝试用openmp来加速渲染。
* ~~挺努力的~~

## 做得烂的地方
* 最后一天才写纹理和折射，写了24h到最后还是肉眼可见地一无所有:sob:
* 纹理的问题在于好像会把整个纹理的颜色平均后贴上去一样，还没查出来是哪里出错。
* 折射的问题其实我也不知道具体在哪，没贴反射（我觉得是不是提供文件里面错了，glass不应该是漫反射而应该是镜面反射），而且玻璃后面的台阶一部分有一部分无，我猜测是直接光照没有让它透过玻璃的问题，间接光照因为折射算成多次递归，被轮盘赌筛出掉所以看不出来。
* 在多线程中用随机数的时候好像有点bug，现在我是用伪随机替代的，以后应该用trng库，结合一些厉害点的并行库去做。
* 程序效率还挺差的，应该有很多小优化可以做但还没机会做。
* 就是因为上一条，所以渲都没时间渲，渲出来也糊糊的。

## Reference

* 射线与三角形求交

https://blog.csdn.net/linolzhang/article/details/55548707

* 光线追踪相关

https://raytracing.github.io/books/RayTracingInOneWeekend.html

https://blog.csdn.net/qq_35312463/category_10343418.html

* BRDF采样及Phong模型 Jason Lawrence - Importance Sampling of the Phong Reflectance Model

https://www.cs.princeton.edu/courses/archive/fall16/cos526/papers/importance.pdf

* 三角形内点的均匀随机采样 Robert Osada, Thomas Funkhouser, Bernard Chazelle, David Dobkin - Shape Distributions

https://www.cs.princeton.edu/~funk/tog02.pdf

## 联系作者

有任何问题，欢迎通过llzju@zju.edu.cn联系我。
