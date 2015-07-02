#ifndef _UTIL
#define _UTIL

#include "DirectXMath.h"

__forceinline float f12(float dx, float dy, float dxy, float x, float y){
	return dy * x - dx * y + dxy;
}

__forceinline double f12(double dx, double dy, double dxy, double x, double y){
	return dy * x - dx * y + dxy;
}

__forceinline float quickPow(float a, int b){
	float c = 1.0f, d[] = { 1.0f, a };
	//float d = a;
	while (b){
		//if (b & 1)
		c *= d[b&1];
		b >>= 1;
		d[1] *= d[1];
	}
	return c;
}


const int nSize = 200;
const int lutSize = 0x10000;
const float lutsf = lutSize - 1;
float pLUT[lutSize + 1];
float gLUT[lutSize + 1];
unsigned char gammaLUT[0x10000];

void initLUT(int n){
	float dy = 1.0f / lutSize, dx = 0.0f;
	for (int i = 0; i <= lutSize; i++){
		pLUT[i] = quickPow(dx, n);
		dx += dy;
	}

	dy = 1.0f / lutSize, dx = 0.0f;
	for (int i = 0; i <= lutSize; i++){
		gammaLUT[i] = unsigned char (powf(dx, 0.45f) * 255.0f);
		dx += dy;
	}
}


__forceinline float lutPow(float a, int b){
	return pLUT[int(a * lutsf)];
}

__forceinline int GammaCorrect(DirectX:: XMVECTOR &c){
	DirectX::XMVECTOR cf = DirectX::XMVectorScale(c, lutsf);
	return	gammaLUT[int(cf.m128_f32[2])] |
		(gammaLUT[int(cf.m128_f32[1])] << 8) |
		(gammaLUT[int(cf.m128_f32[0])] << 16);
}

__forceinline float Q_rsqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long *)&y;                       // evil floating point bit level hacking（对浮点数的邪恶位级hack）
	i = 0x5f375a86 - (i >> 1);               // what the fuck?（这他妈的是怎么回事？）
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));   // 1st iteration （第一次牛顿迭代）
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed（第二次迭代，可以删除）
	return y;
}

__forceinline DirectX::XMVECTOR QuickNormalize(DirectX::XMVECTOR& v){
	return DirectX::XMVectorScale(v, Q_rsqrt(DirectX::XMVector3Dot(v, v).m128_f32[0]));
}




// inline void genCoe(float tmp,
// 	float dx, float dy, float dxy,
// 	float x, float y,
// 	float &a, float &b, float &c){
// 	a = dy / tmp;
// 	b = -dx / tmp;
// 	c = dxy / tmp;
// }

__forceinline void genCoe(double tmp,
	double dx, double dy, double dxy,
	double x, double y,
	float &a, float &b, float &c){
	a = dy / tmp;
	b = -dx / tmp;
	c = dxy / tmp;
}



__forceinline void genCoe(float tmp,
	float x1, float y1,
	float x2, float y2,
	float x, float y,
	float &a, float &b, float &c){
	a = (y1 - y2) / tmp;
	b = (x2 - x1) / tmp;
	c = (x1 * y2 - x2 * y1) / tmp;
}

__forceinline float f(float a, float b, float c, int x, int y){
	return a * x + b * y + c;
}

#endif