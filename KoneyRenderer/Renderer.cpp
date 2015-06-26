#include "Renderer.hpp"
#include <assert.h>
#define CULL_BACK
#define SCANLINE
//#define SCANLINE
inline float f12(float x1, float y1,float x2, float y2,  float x, float y){
	return (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1;
}

inline void genCoe(float tmp, float x1, float y1, float x2, float y2, float x, float y, float &a, float &b, float &c){
	a = (y1 - y2) / tmp;
	b = (x2 - x1) / tmp;
	c = (x1 * y2 - x2 * y1) / tmp;
}

inline float f(float a, float b, float c, int x, int y){
	return a * x + b * y + c;
}


Renderer::Renderer(){}
DirectX::XMVECTOR tmpVec;

void Renderer::setSize(int width, int height){
	fbWidth = width, fbHeight = height;
	fbdata = new unsigned[width * height];
	zbuffer = new unsigned short[width * height];
}

Renderer::~Renderer()
{
	delete[] fbdata;
	delete[] zbuffer;
}

void Renderer::SetTriangleBuffer(Triangle *triBuffer, int size)
{
	bSize = size;
	for (int i = 0; i < bSize; i++)
		triangleBuffer[i] = triBuffer[i];
}

void Renderer::VertexShader(Triangle& triangle){
	XMStoreFloat4(&triangle.vert[0].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[0].position), mvp));
	XMStoreFloat4(&triangle.vert[1].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[1].position), mvp));
	XMStoreFloat4(&triangle.vert[2].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[2].position), mvp));
	DirectX::XMVECTOR diff0 = DirectX::XMVectorMax(DirectX::XMVector3Dot(XMLoadFloat3(&light), XMLoadFloat4(&triangle.vert[0].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	DirectX::XMVECTOR diff1 = DirectX::XMVectorMax(DirectX::XMVector3Dot(XMLoadFloat3(&light), XMLoadFloat4(&triangle.vert[1].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	DirectX::XMVECTOR diff2 = DirectX::XMVectorMax(DirectX::XMVector3Dot(XMLoadFloat3(&light), XMLoadFloat4(&triangle.vert[2].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	
	DirectX::XMVECTOR spec0 = DirectX::XMVectorMax(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(XMLoadFloat3(&eyePos), XMLoadFloat4(&triangle.vert[0].position)), XMLoadFloat4(&triangle.vert[0].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	DirectX::XMVECTOR spec1 = DirectX::XMVectorMax(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(XMLoadFloat3(&eyePos), XMLoadFloat4(&triangle.vert[0].position)), XMLoadFloat4(&triangle.vert[1].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	DirectX::XMVECTOR spec2 = DirectX::XMVectorMax(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(XMLoadFloat3(&eyePos), XMLoadFloat4(&triangle.vert[0].position)), XMLoadFloat4(&triangle.vert[2].normal)), { { 0.0f, 0.0f, 0.0f, 0.0f } });
	diff0 = DirectX::XMVectorMultiply(diff0, XMLoadFloat3(&lightColor));
	diff1 = DirectX::XMVectorMultiply(diff1, XMLoadFloat3(&lightColor));
	diff2 = DirectX::XMVectorMultiply(diff2, XMLoadFloat3(&lightColor));

	spec0 = DirectX::XMVectorMultiply(DirectX::XMVectorPow(spec0, XMLoadFloat3(&n)), XMLoadFloat3(&spec));
	spec1 = DirectX::XMVectorMultiply(DirectX::XMVectorPow(spec1, XMLoadFloat3(&n)), XMLoadFloat3(&spec));
	spec2 = DirectX::XMVectorMultiply(DirectX::XMVectorPow(spec2, XMLoadFloat3(&n)), XMLoadFloat3(&spec));

	spec0 = DirectX::XMVectorAdd(XMLoadFloat4(&triangle.vert[0].color), spec0);
	spec0 = DirectX::XMVectorAdd(XMLoadFloat4(&triangle.vert[1].color), spec1);
	spec0 = DirectX::XMVectorAdd(XMLoadFloat4(&triangle.vert[2].color), spec2);

	XMStoreFloat4(&triangle.vert[0].pcolor, DirectX::XMVectorSaturate(DirectX::XMVectorMultiplyAdd(diff0, spec0, XMLoadFloat3(&aColor))));
	XMStoreFloat4(&triangle.vert[1].pcolor, DirectX::XMVectorSaturate(DirectX::XMVectorMultiplyAdd(diff1, spec1, XMLoadFloat3(&aColor))));
	XMStoreFloat4(&triangle.vert[2].pcolor, DirectX::XMVectorSaturate(DirectX::XMVectorMultiplyAdd(diff2, spec2, XMLoadFloat3(&aColor))));
}

unsigned Renderer::PixelShader(Vertex vin){
	unsigned ret;
	unsigned char red, green, blue;
	red = vin.color.x * 255;
	green = vin.color.y * 255;
	blue = vin.color.z * 255;
	ret = blue | (green << 8) | (red << 16);
	return ret;
}

void Renderer::OutputMerge(unsigned c, unsigned short z, int x, int y){
	int idx = y * fbWidth + x;
	if (zbuffer[idx] > z){//ztest
		fbdata[idx] = c;
		zbuffer[idx] = z;
	}
}

void Renderer::Render(){
	count = 0;
	memset(zbuffer, 0xff, sizeof(short) * fbWidth * fbHeight);
	memset(fbdata, 0, sizeof(int) * fbWidth * fbHeight);
	for (int i = 0; i < bSize; i++){
		VertexShader(triangleBuffer[i]);
		RasterizeAndOutput(triangleBuffer[i]);
	}
	count = count * count;
}

void Renderer::RasterizeAndOutput(Triangle & triangle){
	int width = fbWidth - 1, height = fbHeight - 1;
	float wScale = width * 0.5f, hScale = height * 0.5f;
	//do divide
	DirectX::XMFLOAT4 p0 = triangle.vert[0].positionH;
	DirectX::XMFLOAT4 p1 = triangle.vert[1].positionH;
	DirectX::XMFLOAT4 p2 = triangle.vert[2].positionH;
	DirectX::XMFLOAT3 zz = { 1.0f / p0.w, 1.0f / p1.w, 1.0f / p2.w };
	XMStoreFloat4(&p0, DirectX::XMVectorScale(XMLoadFloat4(&p0), zz.x));
	XMStoreFloat4(&p1, DirectX::XMVectorScale(XMLoadFloat4(&p1), zz.y));
	XMStoreFloat4(&p2, DirectX::XMVectorScale(XMLoadFloat4(&p2), zz.z));
	triangle.vert[0].z = p0.z * 65535;
	triangle.vert[1].z = p1.z * 65535;
	triangle.vert[2].z = p2.z * 65535;
#ifdef CULL_BACK
	DirectX::XMVECTOR v0 = DirectX::XMVectorSubtract(XMLoadFloat4(&p1), XMLoadFloat4(&p0));
	DirectX::XMVECTOR v1 = DirectX::XMVectorSubtract(XMLoadFloat4(&p2), XMLoadFloat4(&p1));
	DirectX::XMFLOAT3 cross;
	XMStoreFloat3(&cross, DirectX::XMVector3Cross(v0, v1));
	if (cross.z >= 0) return;
#endif // CULL_BACK

#ifdef CULL_FRONT
	DirectX::XMVECTOR v0 = DirectX::XMVectorSubtract(XMLoadFloat4(&p1), XMLoadFloat4(&p0));
	DirectX::XMVECTOR v1 = DirectX::XMVectorSubtract(XMLoadFloat4(&p2), XMLoadFloat4(&p1));
	DirectX::XMFLOAT3 cross;
	XMStoreFloat3(&cross, DirectX::XMVector3Cross(v0, v1));
	if (cross.z <= 0) return;
#endif // CULL_BACK

	DirectX::XMFLOAT3 X = { p0.x, p1.x, p2.x };
	DirectX::XMFLOAT3 Y = { p0.y, p1.y, p2.y };
	DirectX::XMVECTOR vX = XMLoadFloat3(&X);
	DirectX::XMVECTOR vY = XMLoadFloat3(&Y);
	DirectX::XMVECTOR wsVec = { wScale, wScale, wScale };
	DirectX::XMVECTOR hsVec = { hScale, hScale, hScale };
	//to image coord
	vX = DirectX::XMVectorMultiplyAdd(vX, wsVec, wsVec);
	vY = DirectX::XMVectorMultiplyAdd(vY, hsVec, hsVec);
	XMStoreFloat3(&X, vX);
	XMStoreFloat3(&Y, vY);
	DirectX::XMFLOAT3 a, b, c;
	//coeff
	//float a0, a1, a2, b0, b1, b2, c0, c1, c2;
	float tmp = f12(X.y, Y.y, X.z, Y.z, X.x, Y.x);
	if (fabs(tmp) < EPS) return;
	genCoe(tmp, X.y, Y.y, X.z, Y.z, X.x, Y.x, a.x, b.x, c.x);
	genCoe(tmp, X.z, Y.z, X.x, Y.x, X.y, Y.y, a.y, b.y, c.y);
	genCoe(tmp, X.x, Y.x, X.y, Y.y, X.z, Y.z, a.z, b.z, c.z);
	//get bounding box
	int maxx = (int)ceilf(max(max(X.x, X.y), X.z));
	int maxy = (int)floorf(max(max(Y.x, Y.y), Y.z));
	//int minx = (int)floorf(min(min(X.x, X.y), X.z));
	int minx = (int)ceilf(min(min(X.x, X.y), X.z));
	int miny = (int)floorf(min(min(Y.x, Y.y), Y.z));
	int dx = maxx - minx, starty = miny + 1;
	DirectX::XMVECTOR va = XMLoadFloat3(&a);
	DirectX::XMVECTOR vb = XMLoadFloat3(&b);
	DirectX::XMVECTOR zvec = { triangle.vert[0].z, triangle.vert[1].z, triangle.vert[2].z };
	DirectX::XMVECTOR rvec = {
		(unsigned char)triangle.vert[0].c,
		(unsigned char)triangle.vert[1].c,
		(unsigned char)triangle.vert[2].c
	};
	DirectX::XMVECTOR gvec = {
		(unsigned char)(triangle.vert[0].c >> 8),
		(unsigned char)(triangle.vert[1].c >> 8),
		(unsigned char)(triangle.vert[2].c >> 8),
	};
	DirectX::XMVECTOR bvec = {
		(unsigned char)(triangle.vert[0].c >> 16),
		(unsigned char)(triangle.vert[1].c >> 16),
		(unsigned char)(triangle.vert[2].c >> 16),
	};
	DirectX::XMFLOAT4 abcybase = {
		f(a.x, b.x, c.x, minx, maxy),
		f(a.y, b.y, c.y, minx, maxy),
		f(a.z, b.z, c.z, minx, maxy),
		0
	};

	DirectX::XMFLOAT4 abcx = abcybase;
	DirectX::XMMATRIX pos, normal, uv, rgb;
	pos.r[0] = XMLoadFloat4(&triangle.vert[0].position);
	pos.r[1] = XMLoadFloat4(&triangle.vert[1].position);
	pos.r[2] = XMLoadFloat4(&triangle.vert[2].position);
	pos.r[3] = { { 0, 0, 0, 0 } };

	normal.r[0] = XMLoadFloat4(&triangle.vert[0].normal);
	normal.r[1] = XMLoadFloat4(&triangle.vert[1].normal);
	normal.r[2] = XMLoadFloat4(&triangle.vert[2].normal);
	normal.r[3] = { { 0, 0, 0, 0 } };

	uv.r[0] = XMLoadFloat2(&triangle.vert[0].uv);
	uv.r[1] = XMLoadFloat2(&triangle.vert[1].uv);
	uv.r[2] = XMLoadFloat2(&triangle.vert[2].uv);
	uv.r[3] = { { 0, 0, 0, 0 } };

	rgb.r[0] = XMLoadFloat4(&triangle.vert[0].pcolor);
	rgb.r[1] = XMLoadFloat4(&triangle.vert[1].pcolor);
	rgb.r[2] = XMLoadFloat4(&triangle.vert[2].pcolor);
	rgb.r[3] = { { 0, 0, 0, 0 } };
	DirectX::XMFLOAT4 tabcfloor, tabcCeil;
	DirectX::XMVECTOR abcz, abcdot, abcvec, abcyvec;
	Vertex tvert;
	abcyvec = XMLoadFloat4(&abcybase);
#ifdef SCANLINE
	for (int i = maxy; i > miny; i--){
		XMStoreFloat4(&abcx ,abcyvec);
		int start = -1, end = -1, cnt = 0;
		int rec[3] = { 0, 0, 0 };
		int mint, t;
		if (fabs(a.x) > EPS){
			t = ceilf(-abcx.x / a.x);
			if (t >= 0 && t <= dx){
				rec[cnt++] = t;
			}
		}
		if (fabs(a.y) > EPS){
			t = ceilf(-abcx.y / a.y);
			if (t >= 0 && t <= dx){
				rec[cnt++] = t;
			}
		}
		if (fabs(a.z) > EPS){
			t = ceilf(-abcx.z / a.z);
			if (t >= 0 && t <= dx){
				rec[cnt++] = t;
			}
		}
		if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
		if (cnt == 3){
			if (rec[1] > rec[2]) std::swap(rec[1], rec[2]);
			if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
			int k = -1;
			for (k = 0; k < 3; k++){
				XMStoreFloat4(&tabcCeil, DirectX::XMVectorAdd(abcyvec, DirectX::XMVectorScale(va, rec[k])));
				XMStoreFloat4(&tabcfloor, DirectX::XMVectorSubtract(XMLoadFloat4(&tabcCeil), va));
				if (tabcCeil.x >= 0 && tabcCeil.x <= 1.0f &&
					tabcCeil.y >= 0 && tabcCeil.y <= 1.0f &&
					tabcCeil.z >= 0 && tabcCeil.z <= 1.0f){
					if (start < 0){
						start = minx + rec[k];
						abcvec = XMLoadFloat4(&tabcCeil);
					}
					end = minx + rec[k];
				}
				else if (tabcfloor.x >= 0 && tabcfloor.x <= 1.0f &&
					tabcfloor.y >= 0 && tabcfloor.y <= 1.0f &&
					tabcfloor.z >= 0 && tabcfloor.z <= 1.0f){
					end = minx + rec[k];
				}
			}
			if (start < 0){
				start = end = 0;
			}
		}
		else {
			mint = rec[0];
			start = minx + rec[0];
			end = minx + rec[1];
			abcvec = DirectX::XMVectorAdd(abcyvec, DirectX::XMVectorScale(va, mint));
		}
		
		for (int j = start; j < end; j++){
#ifdef ZCORRECT
			abcz = DirectX::XMVectorDivide(DirectX::XMVectorMultiply(abcvec, XMLoadFloat3(&zz)),
				DirectX::XMVector3Dot(abcvec, XMLoadFloat3(&zz)));
			XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(abcz, uv));
			XMStoreFloat4(&tvert.color, DirectX::XMVector4Transform(abcz, rgb));
#else
			XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(abcvec, uv));
			XMStoreFloat4(&tvert.color, DirectX::XMVector4Transform(abcvec, rgb));
#endif
			//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), pos));
			//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), normal));
			tvert.z = DirectX::XMVector3Dot(abcvec, zvec).m128_f32[0];
			OutputMerge(PixelShader(tvert), tvert.z, j, i);
			abcvec = DirectX::XMVectorAdd(abcvec, va);
		}
		abcyvec = DirectX::XMVectorSubtract(abcyvec, vb);
	}
#else
	//brute force
	for (int i = maxy; i > miny; i--){
		for (int j = minx; j < maxx; j++){
			if (abcx.x >= 0 && abcx.x <= 1.0f &&
				abcx.y >= 0 && abcx.y <= 1.0f &&
				abcx.z >= 0 && abcx.z <= 1.0f)
			{
				//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), pos));
				//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), normal));				
				XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), uv));
				tvert.z = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), zvec).m128_f32[0];
				XMStoreFloat4(&tvert.color, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), rgb));
				OutputMerge(PixelShader(tvert), tvert.z, j, i);
			}
			XMStoreFloat4(&abcx, DirectX::XMVectorAdd(XMLoadFloat4(&abcx), va));
		}
		XMStoreFloat4(&abcybase, DirectX::XMVectorSubtract(XMLoadFloat4(&abcybase), vb));
		abcx = abcybase;
	}
#endif
}