# Introduction
#### PC的CPU为I7-8770，64位。

#### 速度测试时，跑了50个loop。

#### 测试速度时VS的配置图如下：

![](image/peizhi.jpg)

- speed_rgb2gray_sse.cpp 使用sse加速RGB和灰度图转换算法，相比于OpenCV系统函数有2-3倍加速。算法原理：https://blog.csdn.net/just_sort/article/details/94456945 。速度测试结果如下：

|优化方式|图像分辨率|速度|
|---------|----------|----|
|C语言实现+单线程|4032*3024|9.39ms|
|4次循环展开+单线程|4032*3024|8.74ms|
|SSE优化+单线程|4032*3024|4.57ms|

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

- speed_vibrance_algorithm.cpp 使用SSE加速自然饱和度算法，加速幅度巨大，算法原理请看：https://www.cnblogs.com/Imageshop/p/7234463.html。速度测试结果如下：

|优化方式|图像分辨率 |速度|
|---------|----------|-------|
|C语言实现+单线程|4032*3024|70.17ms|
|浮点数改成整形运算+单线程|4032*3024|36.30ms|
|SSE优化+单线程|4032*3024|8.72ms|

