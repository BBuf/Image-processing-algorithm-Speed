# Introduction

## speed_histogram_algorithm_framework 

- 局部直方图加速框架，内部使用了一些近似计算及指令集加速(SSE)，可以快速处理中值滤波、最大值滤波、最小值滤波、表面模糊等算法。

## resources
- SSE优化相关的资源。

#### PC的CPU为I5-3230，64位。

#### OpenCV版本为3.4.0



- sse_implementation_of_common_functions_in_image_processing.cpp 多个图像处理中常用函数的SSE实现。
- speed_rgb2gray_sse.cpp 使用sse加速RGB和灰度图转换算法，相比于原始实现有接近5倍加速。算法原理：https://mp.weixin.qq.com/s/SagVQ5gfXWWA7NATv-zvBQ  速度测试结果如下：

>测试CPU型号：Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz

| 分辨率    | 优化                                     | 循环次数 | 速度 |
| --------- | ---------------------------------------- | -------- | ---- |
| 4032x3024 | 原始实现                                 | 1000      |  12.139ms    |
| 4032x3024 | 第一版优化（float->INT）                 | 1000      |   7.629ms   |
| 4032x3024 | OpenCV 自带函数                          | 1000      |   4.287ms   |
| 4032x3024 | 第二版优化（手动4路并行）                | 1000      |   10.528ms   |
| 4032x3024 | 第三版优化（OpenMP4线程）                | 1000      |   7.632ms   |
| 4032x3024 | 第四版优化（SSE优化，一次处理12个像素）  | 1000      |   5.579ms   |
| 4032x3024 | 第五版优化（SSE优化，一次处理15个像素）  | 1000      |  5.843ms    |
| 4032x3024 | 第六版优化（AVX2优化，一次处理10个像素） | 1000      |   3.576ms   |
| 4032x3024 | 第七版优化（AVX2优化+std::async）        | 1000      |   2.626ms   |



- speed_vibrance_algorithm.cpp 使用SSE加速自然饱和度算法，加速9倍，算法原理请看： https://mp.weixin.qq.com/s/26UVvqMNLgnquXY21Xu3OQ 。速度测试结果如下：

|分辨率|优化|循环次数|速度|
|----|----|----|----|
|4032x3024|原始实现|100|115.36ms|
|4032x3024|第一版优化|100|62.43ms|
|4032x3024|第二版优化(4线程)|100|28.89ms|
|4032x3024|第三版优化(SSE)|100|12.69ms|



- speed_sobel_edgedetection_sse.cpp 使用SSE加速Sobel边缘检测算法，加速幅度巨大，算法原理请看：https://mp.weixin.qq.com/s/5lCfO_jmSfP7DbsgM7qbpg。速度测试结果如下：

|分辨率|算法优化|循环次数|速度|
|-|-|-|-|
|4032x3024|普通实现|1000|126.54 ms|
|4032x3024|Float->INT+查表法|1000|81.62 ms|
|4032x3024|SSE优化版本1|1000|34.95 ms|
|4032x3024|SSE优化版本2|1000|28.87 ms|
|4032x3024|AVX2优化版本1|1000|15.42 ms  |
|4032x3024|AVX2优化+std::async|1000| 5.69 ms |

- speed_skin_detection_sse.cpp 使用SSE加速肤色检测算法，加速幅度较大，算法原理请看：https://mp.weixin.qq.com/s/UFzY1s6ohTM-dnNg0P4kkw。速度测试结果如下：

|分辨率|算法优化|循环次数|速度|
|-|-|-|-|
|4272x2848|普通实现|1000|41.40ms|
|4272x2848|OpenMP 4线程|1000|36.54ms|
|4272x2848|SSE第一版|1000|6.77ms|
|4272x2848|SSE第二版(std::async)|1000|4.73ms|

- speed_rgb2yuv_sse.cpp SSE极致优化RGB和YUV图像空间互转，算法原理请看：https://mp.weixin.qq.com/s/ryGocz-0YpqZ1CjYXJbd7Q，速度测试结果如下：

|分辨率|算法优化|循环次数|速度|
|-|-|-|-|
|4032x3024|普通实现|1000|150.58ms|
|4032x3024|去掉浮点数，除法用位运算代替|1000|76.70ms|
|4032x3024|OpenMP 4线程|1000|50.48ms|
|4032x3024|普通SSE向量化|1000|48.92ms|
|4032x3024|_mm_madd_epi16二次优化|1000|33.04ms|
|4032x3024|SSE+4线程|1000|23.70ms|



- speed_median_filter_3x3_sse.cpp 极致优化3*3中值滤波，算法原理请看：https://blog.csdn.net/just_sort/article/details/98617050 。速度测试效果如下：
|分辨率|算法优化|循环次数|速度|
|-|-|-|-|
|4032x3024|普通实现|10||
|4032x3024|逻辑优化，更好的流水|10||
|4032x3024|SSE优化|10||
|4032x3024|AVX优化|1000||




----------------------------------------------------------------------------------

- speed_gaussian_filter_sse.cpp 使用sse加速高斯滤波算法。算法原理：https://blog.csdn.net/just_sort/article/details/95212099 。速度测试效果如下：

| 优化方式| 图像分辨率 | 速度 |
| ------------------- | ---------- | ---- |
| C语言普通实现+单线程 | 4032*3024  | 290.43ms |
| SSE优化+单线程      | 4032*3024  | 265.96ms |

- speed_integral_graph_sse.cpp 使用SSE加速积分图运算，但是在PC上并没有速度提升，算法原理请看：https://www.cnblogs.com/Imageshop/p/6897233.html 。速度测试结果如下：

|优化方式|图像分辨率 |速度|
|---------|----------|-------|
|C语言实现+单线程|4032*3024|66.66ms|
|C语言实现+4线程|4032*3024|65.34ms|
|SSE优化+单线程|4032*3024|66.10ms|
|SSE优化+4线程|4032*3024|66.20ms|


- speed_common_functions.cpp 对图像处理的一些常用函数的快速实现，个别使用了SSE优化。
- speed_max_filter_sse.cpp 使用speed_histogram_algorithm_framework框架实现最大值滤波，半径越大越明显。原理请看：https://blog.csdn.net/just_sort/article/details/97280807 。运行的时候记得把工程属性中的sdl检查关掉，不然会报一个变量未初始化的错误。速度测试效果如下:

|优化方式|图像分辨率 |半径|速度|
|---------|----------|-------|-------|
|C语言实现+单线程|4272*2848|7|9445.90ms|
|SSE优化+单线程|4272*2848|7|2234.55ms|
|C语言实现+单线程|4272*2848|9|14468.76ms|
|SSE优化+单线程|4272*2848|9|2221.68ms|
|C语言实现+单线程|4272*2848|11|23069.10ms|
|SSE优化+单线程|4272*2848|11|2180.95ms|

- speed_box_filter_sse.cpp 使用speed_histogram_algorithm框架实现O(1)最大值滤波，使用了SSE优化，算法原理请看：https://blog.csdn.net/just_sort/article/details/98075712 。运行方法和speed_max_filter_sse.cpp相同，速度测试结果如下：

|优化方式|图像分辨率 |半径|速度|
|---------|----------|-------|-------|
|C语言实现+单线程|4272*2848|11|163.16ms|
|SSE优化+单线程|4272*2848|11|123.83ms|
|C语言实现+单线程|4272*2848|21|167.81ms|
|SSE优化+单线程|4272*2848|21|126.98ms|
|C语言实现+单线程|4272*2848|31|168.62ms|
|SSE优化+单线程|4272*2848|31|126.17ms|

- speed_multi_scale_detail_boosting_see.cpp 在speed_box_filter_sse.cpp提供的盒子滤波sse优化的基础上，进一步使用指令集实现了对论文《DARK IMAGE ENHANCEMENT BASED ON PAIRWISE TARGET CONTRAST AND MULTI-SCALE DETAIL BOOSTING》的算法优化。算法原理请看：https://blog.csdn.net/just_sort/article/details/98485746  。在CoreI7-3770速度测试结果如下：

|优化方式|图像分辨率 |半径|速度|
|---------|----------|-------|-------|
|C语言实现+单线程|4272*2848|7|206.00ms|
|SSE优化+单线程|4272*2848|7|57.12ms|

- speed_bicubic_zoom_sse.cpp SSE优化三次立方插值算法，算法原理请看：https://blog.csdn.net/just_sort/article/details/100119653 。速度测试结果如下：

|优化方式|图像分辨率 |插值后大小|速度|
|---------|----------|-------|-------|
|C语言原始算法实现|4272*2848|长宽均为原始1.5倍|1856.29ms|
|C语言实现+查表优化+边界优化|4272*2848|长宽均为原始1.5倍|839.10ms|
|SSE优化+边界优化|4272*2848|长宽均为原始1.5倍|315.70ms|
|OpenCV3.1.0自带的函数|4272*2848|长宽均为原始1.5倍|118.77ms|




# 维护了一个微信公众号，分享论文，算法，比赛，生活，欢迎加入。

- 图片要是没加载出来直接搜GiantPandaCV 就好。

![](image/weixin.jpg)
