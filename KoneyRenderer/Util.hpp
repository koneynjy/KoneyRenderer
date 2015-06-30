#ifndef _UTIL
#define _UTIL

#define INTRIANGLE(x)				\
	(x.m128_f32[0] >= -IMGEPS &&		\
	 x.m128_f32[1] >= -IMGEPS &&		\
	 x.m128_f32[2] >= -IMGEPS )


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

void initLUT(int n){
	float dy = 1.0f / lutSize, dx = 0.0f;
	for (int i = 0; i <= lutSize; i++){
		pLUT[i] = quickPow(dx, n);
		dx += dy;
	}
}

__forceinline float lutPow(float a, int b){
	return pLUT[int(a * lutsf)];
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