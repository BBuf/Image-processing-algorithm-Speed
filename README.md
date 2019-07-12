# Introduction

- speed_rgb2gray_sse.cpp 使用sse加速RGB和灰度图转换算法，相比于OpenCV系统函数有2-3倍加速。算法原理：https://blog.csdn.net/just_sort/article/details/94456945
- speed_gaussian_filter_sse.cpp 使用sse加速高斯滤波算法，但很遗憾因为未知原因没有带来加速，需要进一步研究。算法原理：https://blog.csdn.net/just_sort/article/details/95212099
- speed_integral_graph_sse.cpp 使用SSE加速积分图运算，算法原理请看：https://www.cnblogs.com/Imageshop/p/6897233.html