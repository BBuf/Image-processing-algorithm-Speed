//近似值
union Approximation
{
	double Value;
	int X[2];
};

// 函数1: 将数据截断在Byte数据类型内。
// 参考: http://www.cnblogs.com/zyl910/archive/2012/03/12/noifopex1.html
// 简介: 用位掩码做饱和处理，用带符号右移生成掩码。
unsigned char ClampToByte(int Value){
	return ((Value | ((signed int)(255 - Value) >> 31)) & ~((signed int)Value >> 31));
}

//函数2: 将数据截断在指定范围内
//参考: 无
//简介: 无
int ClampToInt(int Value, int Min, int Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}