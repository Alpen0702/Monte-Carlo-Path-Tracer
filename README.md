# Monte Carlo Path Tracer

Reference

射线与三角形求交
https://blog.csdn.net/linolzhang/article/details/55548707

光线追踪相关
https://raytracing.github.io/books/RayTracingInOneWeekend.html
https://blog.csdn.net/qq_35312463/category_10343418.html

BRDF采样及Phong模型 Jason Lawrence - Importance Sampling of the Phong Reflectance Model
https://www.cs.princeton.edu/courses/archive/fall16/cos526/papers/importance.pdf

三角形内点的均匀随机采样 Robert Osada, Thomas Funkhouser, Bernard Chazelle, David Dobkin - Shape Distributions
https://www.cs.princeton.edu/~funk/tog02.pdf

debug
cornellbox看起来颜色对不上 伽马
veachmis的中间灯周围曝光很亮 pdf
veachmis镜面反射不够亮 pdf
cornellbox加上消隐后噪点多 解决了 因为求交的时候求到发射点自己身上了 应要求t>.0001
cornellbox间接光照颜色断层 解决了 因为溢出int范围 改成了先除法后乘法 pdf错了