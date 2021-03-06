#include "Renderer.hpp"
#include "Config.hpp"
#include <assert.h>
using namespace DirectX;
#define INTRIANGLE(x)											\
	x.m128_f32[0] > -EPS && x.m128_f32[0] < 1.0f + EPS &&		\
	x.m128_f32[1] > -EPS && x.m128_f32[1] < 1.0f + EPS &&		\
	x.m128_f32[2] > -EPS && x.m128_f32[2] < 1.0f + EPS			

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

Renderer::Renderer():fbdata(NULL),zbuffer(NULL){}
void Renderer::setSize(int width, int height){
	fbWidth = width, fbHeight = height;
	fbdata = new unsigned[width * height];
	zbuffer = new unsigned short[width * height];
	texture.Load("Texture\\wood.bmp");
}

Renderer::~Renderer()
{
	if(fbdata) delete[] fbdata;
	if(zbuffer) delete[] zbuffer;
}

void Renderer::SetTriangleBuffer(Triangle *triBuffer, int size)
{
	bSize = size;
	for (int i = 0; i < bSize; i++)
		triangleBuffer[i] = triBuffer[i];
}

__forceinline XMFLOAT4 Renderer::lightShader(Vertex& vert){
	XMFLOAT4 res;
	XMVECTOR pos = XMLoadFloat4(&vert.position);
	XMVECTOR normal = XMLoadFloat4(&vert.normal);
	XMVECTOR color = XMLoadFloat4(&vert.color);
	XMVECTOR lightc = XMLoadFloat3(&lightColor);
#ifdef POINTLIGHT
	XMVECTOR l = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&lightPoint), pos));
#else
	XMVECTOR l = XMLoadFloat3(&light);
#endif
	float diff = max(XMVector3Dot(l, normal).m128_f32[0],0.0f);
	XMVECTOR diffuse = XMVectorAdd(XMVectorScale(lightc, diff), XMLoadFloat3(&aColor));
#ifdef SPECULAR
	XMVECTOR r = XMVector3Reflect(XMVector3Normalize(XMVectorSubtract(pos, XMLoadFloat3(&eyePos))), normal);
	float sp = XMVector3Dot(l, r).m128_f32[0];
	if (sp > EPS){
		XMVECTOR specular = XMVectorScale(XMLoadFloat3(&spec), powf(sp, n));
		specular = XMVectorMultiply(lightc, specular);
		XMStoreFloat4(&res, XMVectorSaturate(XMVectorAdd(specular, XMVectorMultiply(diffuse, color))));
	}
	else{
		XMStoreFloat4(&res, XMVectorSaturate(XMVectorMultiply(diffuse, color)));
	}
	
#else
	XMStoreFloat4(&res, XMVectorSaturate(XMVectorAdd(XMVectorMultiply(diffuse, color), XMVectorMultiply(XMLoadFloat3(&aColor), color))));
#endif
	return res;
}

void Renderer::VertexShader(Vertex& vert){
	XMVECTOR pos = XMLoadFloat4(&vert.position);
#ifdef ROTATE
	XMStoreFloat4(&vert.position, XMVector4Transform(pos, mw));
	XMStoreFloat4(&vert.normal, XMVector3Transform(XMLoadFloat4(&vert.normal), mw));
#endif
	XMStoreFloat4(&vert.positionH, XMVector4Transform(pos, mvp));
#ifdef VERTEXLIGHT
	vert.pcolor = lightShader(vert);
#else
	vert.pcolor = vert.color;
#endif
	
	
}

__forceinline XMFLOAT4 Renderer::PixelShader(Vertex vin, float z){
#ifdef TEXTURE
#ifdef TRILINEAR
	//vin.color = texture.MipMapNearestSampler(vin.uv, z);
	vin.color = texture.TrilinearSampler(vin.uv, z);
#else
#ifdef BILINEAR
#ifdef MIPMAP
	vin.color = texture.MipMapBilinearSampler(vin.uv, z);
#else
	vin.color = texture.BilinearSampler(vin.uv);
#endif // MIPMAP
#else
#ifdef MIPMAP
	vin.color = texture.MipMapNearestSampler(vin.uv, z);
#else
	vin.color = texture.NearestSampler(vin.uv);
#endif // MIPMAP
#endif	
#endif
#endif
#ifdef PIXELLIGHT
	return lightShader(vin);
#else
	return vin.color;
#endif	
}

__forceinline void Renderer::OutputMerge(XMFLOAT4 c, unsigned short z, int x, int y){
	int idx = y * fbWidth + x;
#ifdef ZTEST
	if (zbuffer[idx] <= z) return;
#endif
#ifdef ZWRITE
	zbuffer[idx] = z;
#endif
	unsigned char red, green, blue, alpha;
	red = c.x * 255;
	green = c.y * 255;
	blue = c.z * 255;
	fbdata[idx] = blue | (green << 8) | (red << 16);
}

void Renderer::Render(){
	count = 0;
	memset(zbuffer, 0xff, sizeof(short) * fbWidth * fbHeight);
	memset(fbdata, 0x88, sizeof(int) * fbWidth * fbHeight);
	for (int i = 0; i < bSize; i++){
		VertexShader(triangleBuffer[i].vert[0]);
		VertexShader(triangleBuffer[i].vert[1]);
		VertexShader(triangleBuffer[i].vert[2]);
		RasterizeAndOutput(triangleBuffer[i]);
	}
	count = count * count;
}

void Renderer::RasterizeAndOutput(Triangle & triangle){
	int width = fbWidth - 1, height = fbHeight - 1;
	float wScale = width * 0.5f, hScale = height * 0.5f;
	//do divide
	XMVECTOR p0 = XMLoadFloat4(&triangle.vert[0].positionH);
	XMVECTOR p1 = XMLoadFloat4(&triangle.vert[1].positionH);
	XMVECTOR p2 = XMLoadFloat4(&triangle.vert[2].positionH);
	XMVECTOR zz = { 1.0f / p0.m128_f32[3], 1.0f / p1.m128_f32[3], 1.0f / p2.m128_f32[3] };
	p0 = XMVectorScale(p0, zz.m128_f32[0]);
	p1 = XMVectorScale(p1, zz.m128_f32[1]);
	p2 = XMVectorScale(p2, zz.m128_f32[2]);
	XMVECTOR zvec = { p0.m128_f32[2] * 65535, p0.m128_f32[2] * 65535, p2.m128_f32[2] * 65535 };
#ifdef CULL_BACK
	XMVECTOR v0 = XMVectorSubtract(p1, p0);
	XMVECTOR v1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(v0, v1);
	if (cross.m128_f32[2] >= 0) return;
#endif // CULL_BACK

#ifdef CULL_FRONT
	XMVECTOR v0 = XMVectorSubtract(p1, p0);
	XMVECTOR v1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(v0, v1);
	if (cross.m128_f32[2] <= 0) return;
#endif // CULL_BACK

	XMVECTOR X = { p0.m128_f32[0], p1.m128_f32[0], p2.m128_f32[0] };
	XMVECTOR Y = { p0.m128_f32[1], p1.m128_f32[1], p2.m128_f32[1] };
	XMVECTOR wsVec = { wScale, wScale, wScale };
	XMVECTOR hsVec = { hScale, hScale, hScale };
	//to image coord
	X = XMVectorMultiplyAdd(X, wsVec, wsVec);
	Y = XMVectorMultiplyAdd(Y, hsVec, hsVec);
	XMVECTOR a, b, c;
	//coeff
	//float a0, a1, a2, b0, b1, b2, c0, c1, c2;
	float area = f12(X.m128_f32[0], Y.m128_f32[0], X.m128_f32[1], Y.m128_f32[1], X.m128_f32[2], Y.m128_f32[2]);
	if (fabs(area) < EPS) return;
	genCoe(area, X.m128_f32[1], Y.m128_f32[1], X.m128_f32[2], Y.m128_f32[2], X.m128_f32[0], Y.m128_f32[0], a.m128_f32[0], b.m128_f32[0], c.m128_f32[0]);
	genCoe(area, X.m128_f32[2], Y.m128_f32[2], X.m128_f32[0], Y.m128_f32[0], X.m128_f32[1], Y.m128_f32[1], a.m128_f32[1], b.m128_f32[1], c.m128_f32[1]);
	genCoe(area, X.m128_f32[0], Y.m128_f32[0], X.m128_f32[1], Y.m128_f32[1], X.m128_f32[2], Y.m128_f32[2], a.m128_f32[2], b.m128_f32[2], c.m128_f32[2]);
	//get bounding box
	int maxx = (int)ceilf(max(max(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	int maxy = (int)floorf(max(max(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	//int minx = (int)floorf(min(min(X.x, X.y), X.z));
	int minx = (int)ceilf(min(min(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	int miny = (int)floorf(min(min(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	int dx = maxx - minx,padding[2];
	if (maxy == miny || minx == maxx) return;
	XMVECTOR abcybase = {
		f(a.m128_f32[0], b.m128_f32[0], c.m128_f32[0], minx, maxy),
		f(a.m128_f32[1], b.m128_f32[1], c.m128_f32[1], minx, maxy),
		f(a.m128_f32[2], b.m128_f32[2], c.m128_f32[2], minx, maxy),
		0
	};
	XMVECTOR abcx = abcybase;
	float zybase = XMVector3Dot(abcybase, zvec).m128_f32[0], zx;
	float zdx = XMVector3Dot(abcybase, a).m128_f32[0], zdy = XMVector3Dot(abcybase, b).m128_f32[0];
	XMMATRIX pos, normal, uv, rgb;
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
	XMVECTOR abczBasey = XMVectorMultiply(abcybase, zz);
	XMVECTOR za = XMVectorMultiply(a, zz),zb = XMVectorMultiply(b, zz);
	XMVECTOR colorBasey = XMVector4Transform(abczBasey, rgb), colorX = colorBasey;
	XMVECTOR uvBasey = XMVector4Transform(abczBasey, uv), uvX = uvBasey;
	XMVECTOR colorZa = XMVector4Transform(za, rgb), colorZb = XMVector4Transform(zb, rgb);
	XMVECTOR uvZa = XMVector4Transform(za, uv), uvZb = XMVector4Transform(zb, uv);
#ifdef PIXELLIGHT
	XMVECTOR posBasey = XMVector4Transform(abczBasey, pos), posX = posBasey;
	XMVECTOR normalBasey = XMVector4Transform(abczBasey, normal), normalX = normalBasey;
	XMVECTOR posZa = XMVector4Transform(za, pos), posZb = XMVector4Transform(zb, pos);
	XMVECTOR normalZa = XMVector4Transform(za, normal), normalZb = XMVector4Transform(zb, normal);
#endif

	float zabcBaseY = XMVector3Dot(zz, abcybase).m128_f32[0], zabcX = zabcBaseY;
	float zaScale = XMVector3Dot(zz, a).m128_f32[0], zbScale = XMVector3Dot(zz, b).m128_f32[0];
	XMVECTOR tabcfloor, tabcCeil;
	XMVECTOR abcz;
	Vertex tvert;
#ifdef SCANLINE
	for (int i = maxy; i > miny; i--){
		int rec[3] = { 0, 0, 0 }, t, cnt = 0;
		float mint;
		for (int k = 0; k < 3; k++){
			if (fabs(a.m128_f32[k]) > EPS){
				t = ceilf(-abcybase.m128_f32[k] / a.m128_f32[k]);
				if (t >= 0 && t <= dx){
					rec[cnt++] = t;
				}
			}
		}
		int start = minx, end = minx;
		if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
		if (cnt == 3){
 			if (rec[1] > rec[2]) std::swap(rec[1], rec[2]);
 			if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
			tabcCeil = XMVectorAdd(abcybase, XMVectorScale(a, rec[0]));
			//int greater = XMVector3GreaterOrEqualR(tabcCeil, { {0.0f,0.0f,0.0f} });
			//int less = XMVector3GreaterOrEqualR({ { 1.0f, 1.0f, 1.0f } }, tabcCeil);
			if (INTRIANGLE(tabcCeil)){
			//if (XMComparisonAllTrue(greater) && XMComparisonAllTrue(less)){
				tabcfloor = XMVectorAdd(abcybase, XMVectorScale(a, rec[2] - 1));
				//greater = XMVector3GreaterOrEqualR(tabcfloor, { { 0.0f, 0.0f, 0.0f } });
				//less = XMVector3GreaterOrEqualR({ { 1.0f, 1.0f, 1.0f } }, tabcfloor);
				mint = rec[0];
				if (INTRIANGLE(tabcfloor)){
				//if (XMComparisonAllTrue(greater) && XMComparisonAllTrue(less)){
					start += rec[0];
					end += rec[2];
					abcx = tabcCeil;
				}
				else{
					start += rec[0];
					end += rec[1];
					abcx = tabcCeil;
				}
			}
			else{
				tabcfloor = XMVectorAdd(abcybase, XMVectorScale(a, rec[1]));
				//greater = XMVector3GreaterOrEqualR(tabcfloor, { { 0.0f, 0.0f, 0.0f } });
				//less = XMVector3GreaterOrEqualR({ { 1.0f, 1.0f, 1.0f } }, tabcfloor);
				mint = rec[1];
				if (INTRIANGLE(tabcfloor)){
				//if (XMComparisonAllTrue(greater) && XMComparisonAllTrue(less)){
					start += rec[1];
					end += rec[2];
					abcx = tabcfloor;
				}
			}
		}
		else {
			mint = rec[0];
			start = minx + rec[0];
			end = minx + rec[1];
			abcx = XMVectorAdd(abcybase, XMVectorScale(a, mint));
		}
		if (start != end){
			zabcX = zabcBaseY + mint * zaScale;
			zx = zybase + mint * zdx;
			colorX = XMVectorAdd(colorBasey, XMVectorScale(colorZa, mint));
			uvX = XMVectorAdd(uvBasey, XMVectorScale(uvZa, mint));
#ifdef PIXELLIGHT
			posX = XMVectorAdd(posBasey, XMVectorScale(posZa, mint));
			normalX = XMVectorAdd(normalBasey, XMVectorScale(normalZa, mint));
#endif
			for (int j = start; j < end; j++){
				float scale = 1.0f / zabcX, paddingf[3];
				XMStoreFloat2(&tvert.uv, XMVectorScale(uvX, scale));
				XMStoreFloat4(&tvert.color, XMVectorScale(colorX, scale));
#ifdef PIXELLIGHT
				XMStoreFloat4(&tvert.position, XMVectorScale(posX, scale));
				XMStoreFloat4(&tvert.normal, XMVectorScale(normalX, scale));
#endif
				//zx = XMVector3Dot(abcx, zvec).m128_f32[0];
				OutputMerge(PixelShader(tvert, scale), zx, j, i);
				//OutputMerge(texture.BilinearSampler(tvert.uv), zx, j, i);
				////////////////////////////////////////////////////////////////////
				zabcX += zaScale;
				zx += zdx;
				abcx = XMVectorAdd(abcx, a);
				uvX = XMVectorAdd(uvX, uvZa);
				colorX = XMVectorAdd(colorX, colorZa);
				
#ifdef PIXELLIGHT
				posX = XMVectorAdd(posX, posZa);
				normalX = XMVectorAdd(normalX, normalZa);
#endif
			}
		}
		zabcBaseY -= zbScale;
		zybase -= zdy;
		abcybase = XMVectorSubtract(abcybase, b);
		colorBasey = XMVectorSubtract(colorBasey, colorZb);
		uvBasey = XMVectorSubtract(uvBasey, uvZb);
#ifdef PIXELLIGHT
		posBasey = XMVectorSubtract(posBasey, posZb);
		normalBasey = XMVectorSubtract(normalBasey, normalZb);
#endif
		
	}
#else
	//brute force
	for (int i = maxy; i > miny; i--){
		for (int j = minx; j < maxx; j++){
			if (INTRIANGLE(abcx))
			{
				//XMStoreFloat4(&tvert.position, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), pos));
				//XMStoreFloat4(&tvert.normal, DirectX::XMVector4Transform(XMLoadFloat4(&abcx), normal));
				abcz = XMVectorScale(XMVectorMultiply(abcx, zz), 1.0f / zabcX);
				XMStoreFloat2(&tvert.uv, XMVector4Transform(abcx, uv));
				XMStoreFloat4(&tvert.color, XMVector4Transform(abcx, rgb));
				OutputMerge(PixelShader(tvert), XMVector3Dot(abcx, zvec).m128_f32[0], j, i);
			}
			zabcX += zaScale;
			abcx =XMVectorAdd(abcx, a);
		}
		zabcBaseY -= zbScale;
		abcybase = XMVectorSubtract(abcybase, b);
		abcx = abcybase;
	}
#endif
}