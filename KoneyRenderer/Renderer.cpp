#include "Renderer.hpp"
#define CULL_BACK
#define BARYCENTRIC
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


inline int min3(float a, float b, float c){

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
}

unsigned Renderer::PixelShader(Vertex vin, int x, int y){
	unsigned ret;
#ifdef NOTEX
	ret = vin.c;
#else

#endif

#ifdef LIGHT
	return 0;
#else
	return ret;
#endif 

}

void Renderer::OutputMerge(unsigned c, unsigned short z, int x, int y){
	count++;
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
	/*int t = 0;
	for (int i = 0; i < fbHeight / 4; i++){
	for (int j = 0; j < fbWidth / 4; j++){
	fbdata[t + j] = 0x00ff0000;
	}
	t += fbWidth;
	}*/
	for (int i = 0; i < bSize; i++){
		VertexShader(triangleBuffer[i]);
		RasterizeAndOutput(triangleBuffer[i]);
	}
	count = count * count;
}

void Renderer::UpRaster(float xl, float yl, float xr, float yr, float xu, float yu, Vertex&l, Vertex&r, Vertex& u){

}

void Renderer::DowneRaster(float xl, float yl, float xr, float yr, float xd, float yd, Vertex&l, Vertex&r, Vertex& d){

}

void ::Renderer::BarycentricRaster(Triangle & triangle){
	int width = fbWidth - 1, height = fbHeight - 1;
	float wScale = width * 0.5f, hScale = height * 0.5f;
	//do divide
	DirectX::XMFLOAT4 p0 = triangle.vert[0].positionH;
	DirectX::XMFLOAT4 p1 = triangle.vert[1].positionH;
	DirectX::XMFLOAT4 p2 = triangle.vert[2].positionH;

	float z0 = 1.0f / p0.w;
	float z1 = 1.0f / p1.w;
	float z2 = 1.0f / p2.w;
	XMStoreFloat4(&p0, DirectX::XMVectorScale(XMLoadFloat4(&p0), z0));
	XMStoreFloat4(&p1, DirectX::XMVectorScale(XMLoadFloat4(&p1), z1));
	XMStoreFloat4(&p2, DirectX::XMVectorScale(XMLoadFloat4(&p2), z2));
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

	DirectX::XMFLOAT3 X = { p0.x, p1.x, p2.x};
	DirectX::XMFLOAT3 Y = { p0.y, p1.y, p2.y};
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
	int maxx = (int)(max(max(X.x, X.y), X.z));
	int maxy = (int)(max(max(Y.x, Y.y), Y.z));
	int minx = (int)(min(min(X.x, X.y), X.z));
	int miny = (int)(min(min(Y.x, Y.y), Y.z));
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
		f(a.x, b.x, c.x, minx, miny),
		f(a.y, b.y, c.y, minx, miny),
		f(a.z, b.z, c.z, minx, miny),
		0
	};

	DirectX::XMFLOAT4 abcx = abcybase;
	DirectX::XMMATRIX pos, normal, uv;
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
	Vertex tvert;
	unsigned char red, green, blue;
	if (abcybase.x >= 0 && abcybase.x <= 1.0f &&
		abcybase.y >= 0 && abcybase.y <= 1.0f &&
		abcybase.z >= 0 && abcybase.z <= 1.0f)
	{
		//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), pos));
		//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), normal));
		XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), uv));
		tvert.z = DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), zvec).m128_f32[0];
		red =	DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), rvec).m128_f32[0];
		green =	DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), gvec).m128_f32[0];
		blue =	DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), bvec).m128_f32[0];
		tvert.c = red || (green << 8) || (blue << 16);
		OutputMerge(PixelShader(tvert, minx, miny), tvert.z, minx, miny);
	}
	for (int j = minx + 1; j <= maxx; j++){
		XMStoreFloat4(&abcx, DirectX::XMVectorAdd(XMLoadFloat4(&abcx), va));
		if (abcx.x >= 0 && abcx.x <= 1.0f &&
			abcx.y >= 0 && abcx.y <= 1.0f &&
			abcx.z >= 0 && abcx.z <= 1.0f)
		{
			//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), pos));
			//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), normal));
			XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), uv));
			tvert.z = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), zvec).m128_f32[0];
			red = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), rvec).m128_f32[0];
			green = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), gvec).m128_f32[0];
			blue = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), bvec).m128_f32[0];
			tvert.c = red || (green << 8) || (blue << 16);
			OutputMerge(PixelShader(tvert, minx, miny), tvert.z, j, miny);
		}
	}
	for (int i = miny + 1; i <= maxy; i++){
		XMStoreFloat4(&abcybase, DirectX::XMVectorAdd(XMLoadFloat4(&abcybase), vb));
		if (abcybase.x >= 0 && abcybase.x <= 1.0f
			&& abcybase.y >= 0 && abcybase.y <= 1.0f
			&& abcybase.z >= 0 && abcybase.z <= 1.0f
			)
		{
			//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), pos));
			//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), normal));
			XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(XMLoadFloat4(&abcybase), uv));
			tvert.z = DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), zvec).m128_f32[0];
			red = DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), rvec).m128_f32[0];
			green = DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), gvec).m128_f32[0];
			blue = DirectX::XMVector3Dot(XMLoadFloat4(&abcybase), bvec).m128_f32[0];
			tvert.c = red || (green << 8) || (blue << 16);
			OutputMerge(PixelShader(tvert, minx, miny), tvert.z, minx, i);
		}
		abcx = abcybase;
		for (int j = minx + 1; j <= maxx; j++){
			XMStoreFloat4(&abcx, DirectX::XMVectorAdd(XMLoadFloat4(&abcx), va));
			if (abcx.x >= 0 && abcx.x <= 1.0f &&
				abcx.y >= 0 && abcx.y <= 1.0f &&
				abcx.z >= 0 && abcx.z <= 1.0f)
			{
				//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), pos));
				//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), normal));				
				XMStoreFloat2(&tvert.uv, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), uv));
				tvert.z = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), zvec).m128_f32[0];
				red = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), rvec).m128_f32[0];
				green = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), gvec).m128_f32[0];
				blue = DirectX::XMVector3Dot(XMLoadFloat4(&abcx), bvec).m128_f32[0];
				tvert.c = red | (green << 8) | (blue << 16);
				OutputMerge(PixelShader(tvert, minx, miny), tvert.z, j, i);
			}
		}
	}
}

void Renderer::ScanLineRaster(Triangle & triangle){
	int width = fbWidth - 1, height = fbHeight - 1;
	float wScale = width * 0.5f, hScale = height * 0.5f;
	//do divide
	DirectX::XMFLOAT4 p0 = triangle.vert[0].positionH;
	DirectX::XMFLOAT4 p1 = triangle.vert[1].positionH;
	DirectX::XMFLOAT4 p2 = triangle.vert[2].positionH;

	float z0 = 1.0f / p0.w;
	float z1 = 1.0f / p1.w;
	float z2 = 1.0f / p2.w;
	XMStoreFloat4(&p0, DirectX::XMVectorScale(XMLoadFloat4(&p0), z0));
	XMStoreFloat4(&p1, DirectX::XMVectorScale(XMLoadFloat4(&p1), z1));
	XMStoreFloat4(&p2, DirectX::XMVectorScale(XMLoadFloat4(&p2), z2));
	triangle.vert[0].z = p0.z;
	triangle.vert[1].z = p1.z;
	triangle.vert[2].z = p2.z;
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
	DirectX::XMVECTOR wsVec = { wScale, wScale, wScale };
	DirectX::XMVECTOR hsVec = { hScale, hScale, hScale };
	//to image coord
	XMStoreFloat3(&X, DirectX::XMVectorMultiplyAdd(XMLoadFloat3(&X), wsVec, wsVec));
	XMStoreFloat3(&Y, DirectX::XMVectorMultiplyAdd(XMLoadFloat3(&Y), hsVec, hsVec));
	float x[3] = { X.x, X.y, X.z }, y[3] = { Y.x, Y.y, Y.z };
}

void Renderer::RasterizeAndOutput(Triangle & triangle){
#ifdef BARYCENTRIC
	BarycentricRaster(triangle);
#endif // BARYCENTRIC

#ifdef SCANLINE
	ScanLineRaster(triangle);
#endif
}