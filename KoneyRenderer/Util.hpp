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

inline int myCeilf(float v){
	float ff = floorf(v);
	if (v - ff < 0.00001f) return ff;
	else return (int)ff + 1;
}

#endif