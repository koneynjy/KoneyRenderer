#ifndef _UTIL
#define _UTIL

#define INTRIANGLE(x)				\
	(x.m128_f32[0] >= -IMGEPS &&		\
	 x.m128_f32[1] >= -IMGEPS &&		\
	 x.m128_f32[2] >= -IMGEPS )


inline float f12(float dx, float dy, float dxy, float x, float y){
	return dy * x - dx * y + dxy;
}

inline double f12(double dx, double dy, double dxy, double x, double y){
	return dy * x - dx * y + dxy;
}

inline float quickPow(float a, int b){
	float c = 1.0f, d = a;
	while (b > 0){
		if (b & 1)
			c *= d;
		b = b >> 1;
		d = d * d;
	}
	return c;
}
const int nSize = 200;
const int lutSize = 0x10000;
const float lutsf = lutSize - 1;
float pLUT[lutSize + 1][nSize];

void initLUT(){
	float dy = 1.0f / lutSize, dx = 0.0f;
	for (int i = 0; i <= lutSize; i++){
		pLUT[i][0] = 1.0f;
		for (int j = 1; j < nSize; j++)
			pLUT[i][j] = pLUT[i][j - 1] * dx;
		dx += dy;
	}
}

inline float lutPow(float a, int b){
	return pLUT[int(a * lutsf)][b];
}

// inline void genCoe(float tmp,
// 	float dx, float dy, float dxy,
// 	float x, float y,
// 	float &a, float &b, float &c){
// 	a = dy / tmp;
// 	b = -dx / tmp;
// 	c = dxy / tmp;
// }

inline void genCoe(double tmp,
	double dx, double dy, double dxy,
	double x, double y,
	float &a, float &b, float &c){
	a = dy / tmp;
	b = -dx / tmp;
	c = dxy / tmp;
}



inline void genCoe(float tmp,
	float x1, float y1,
	float x2, float y2,
	float x, float y,
	float &a, float &b, float &c){
	a = (y1 - y2) / tmp;
	b = (x2 - x1) / tmp;
	c = (x1 * y2 - x2 * y1) / tmp;
}

inline float f(float a, float b, float c, int x, int y){
	return a * x + b * y + c;
}

#endif