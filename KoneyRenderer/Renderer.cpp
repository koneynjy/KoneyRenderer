#include "Renderer.hpp"
#include "Config.hpp"
#include "Util.hpp"
#include <assert.h>
using namespace DirectX;

#define INTRIANGLE(x)				\
	( x.m128_f32[0] >= -IMGEPS &&	\
	  x.m128_f32[1] >= -IMGEPS &&	\
	  x.m128_f32[2] >= -IMGEPS )

__forceinline void lerpVert(Vertex& v, Vertex & v0, Vertex& v1, float t, float mt){
	v.positionH = XMVectorAdd(XMVectorScale(v0.positionH, mt), XMVectorScale(v1.positionH, t));
	v.position = XMVectorAdd(XMVectorScale(v0.position, mt), XMVectorScale(v1.position, t));
	v.normal = XMVectorAdd(XMVectorScale(v0.normal, mt), XMVectorScale(v1.normal, t));
	v.pcolor = XMVectorAdd(XMVectorScale(v0.pcolor, mt), XMVectorScale(v1.pcolor, t));
	v.uv = XMVectorAdd(XMVectorScale(v0.uv, mt), XMVectorScale(v1.uv, t));
}

Renderer::Renderer() :fbdata(NULL), zbuffer(NULL){
	light = { { -0.57735f, 0.57735f, 0.57735f } };
	lightPoint = { { 2.0f, 2.0f, 2.0f } };
	lightColor = { { 1.0f, 1.0f, 1.0f } };
	aColor = { { 0.05f, 0.05f, 0.05f } };
	eyePos;
	spec = { { 1.0f, 1.0f, 1.0f } };
	colorOverRide = { { 0.8f, 0.0f, 0.0f, 0.0f } };
#ifdef GAMMACORRECT
	colorOverRide = XMVectorPow(colorOverRide, { {2.2f,2.2f,2.2f,2.2f} });
#endif
	gammaCorrect = { {0.45f,0.45f,0.45f,0.45f} };

	nearP = 1.0f, farP = 1000.0f;
}
void Renderer::setSize(int width, int height){
	fbWidth = width, fbHeight = height;
	fbdata = new unsigned[width * height];
	zbuffer = new unsigned short[width * height];
	fbW = fbWidth - 1, fbH = fbHeight - 1;
	fbWf = fbW, fbHf = fbH;
	wScale = fbWidth * 0.5f, hScale = fbHeight * 0.5f;
	wCor = wScale - 0.5f, hCor = hScale - 0.5f;
	texture.Load("Texture\\wood.bmp");
	//t = new std::thread[bSize];
	initLUT(n);
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

__forceinline XMVECTOR Renderer::lightShader(Vertex& vert){
	XMFLOAT4 res;
	XMVECTOR pos = vert.position;
	XMVECTOR normal = vert.normal;
	XMVECTOR color = vert.color;
	XMVECTOR lightc = lightColor;
#ifdef POINTLIGHT
#ifdef BLINNPHONG
	XMVECTOR l = XMVector3NormalizeEst(XMVectorSubtract(lightPoint, pos));
#else
	XMVECTOR l = XMVector3Normalize(XMVectorSubtract(lightPoint, pos));
#endif
#else
	XMVECTOR l = XMLoadFloat3(&light);
#endif
	float diff = max(XMVector3Dot(l, normal).m128_f32[0],0.0f);
	XMVECTOR diffuse;
	diffuse = XMVectorAdd(XMVectorScale(lightc, diff), aColor);

#ifdef SPECULAR
#ifdef BLINNPHONG
	XMVECTOR h = XMVector3Normalize(XMVectorAdd(XMVector3NormalizeEst(XMVectorSubtract(eyePos, pos)), l));
	float sp = XMVector3Dot(h, normal).m128_f32[0];
#else
	XMVECTOR r = XMVector3Reflect(XMVector3Normalize(XMVectorSubtract(pos, eyePos)), normal);
	float sp = XMVector3Dot(l, r).m128_f32[0];
#endif // BLINNPHONG
	if (sp > EPS){
		XMVECTOR specular = XMVectorScale(spec, lutPow(sp, n));
		specular = XMVectorMultiply(lightc, specular);
		XMVectorSaturate(XMVectorAdd(specular, XMVectorMultiply(diffuse, color)));
		return XMVectorSaturate(XMVectorAdd(specular, XMVectorMultiply(diffuse, color)));
	}
	else{
		return XMVectorSaturate(XMVectorMultiply(diffuse, color));
	}
#else
	return XMVectorSaturate(XMVectorMultiply(diffuse, color));
#endif
}

void Renderer::VertexShader(Vertex& vert){
#ifdef ROTATE
	XMStoreFloat4(&vert.position, XMVector4Transform(pos, mw));
	XMStoreFloat4(&vert.normal, XMVector3Transform(XMLoadFloat4(&vert.normal), mw));
#endif
	vert.positionH = XMVector4Transform(vert.position, mvp);
#ifdef VERTEXLIGHT
	vert.pcolor = lightShader(vert);
#else
	vert.pcolor = colorOverRide;
#endif
	
	
}

__forceinline XMVECTOR Renderer::PixelShader(Vertex &vin, float z){
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

__forceinline void Renderer::OutputMerge(XMVECTOR c, unsigned short z, int x, int y){
	int idx = y * fbWidth + x;
#ifdef ZTEST
	if (zbuffer[idx] <= z) return;
#endif
#ifdef ZWRITE
	zbuffer[idx] = z;
#endif
#ifdef GAMMACORRECT
	fbdata[idx] = GammaCorrect(c);
#else
	unsigned char red, green, blue, alpha;
	XMVECTOR cf = XMVectorScale(c, 255.0f);
	red = cf.m128_f32[0];
	green = cf.m128_f32[1];
	blue = cf.m128_f32[2];
	fbdata[idx] = blue | (green << 8) | (red << 16);
#endif
}

void Renderer::Render(){
	count = 0;
	memset(zbuffer, 0xff, sizeof(unsigned short) * fbWidth * fbHeight);
	memset(fbdata, 0xAA, sizeof(unsigned)* fbWidth * fbHeight);
	for (int i = 0; i < bSize; i++){
		Triangle &triangle = triangleBuffer[i];
		VertexShader(triangle.vert[0]);
		VertexShader(triangle.vert[1]);
		VertexShader(triangle.vert[2]);
#ifdef CLIPNEAR
		ClippNearRasterization(triangle.vert[0], triangle.vert[1], triangle.vert[2]);
#else
#ifdef MULTITHREAD
		RasterizeMultiThread(triangle.vert[0], triangle.vert[1], triangle.vert[2]);
#else
		RasterizeAndOutput(triangle.vert[0], triangle.vert[1], triangle.vert[2]);
#endif
#endif
	}
}
///////////////////////////3D clipping only for near plane/////////////////////////////////////
void Renderer::ClippNearRasterization(Vertex& v0, Vertex& v1, Vertex& v2){
	bool inout0, inout1, inout2;
	XMVECTOR p0 = v0.positionH;
	XMVECTOR p1 = v1.positionH;
	XMVECTOR p2 = v2.positionH;
	inout0 = p0.m128_f32[2] > EPS;
	inout1 = p1.m128_f32[2] > EPS;
	inout2 = p2.m128_f32[2] > EPS;
	Vertex vertex[4];
	int vcnt = 0;
	if (inout0) vertex[vcnt++] = v0;

	if (inout0 ^ inout1){
		float t = p0.m128_f32[2] / (p0.m128_f32[2] - p1.m128_f32[2]);
		lerpVert(vertex[vcnt++], v0, v1, t, 1.0f - t);
	}

	if (inout1) vertex[vcnt++] = v1;

	if (inout1 ^ inout2){
		float t = p1.m128_f32[2] / (p1.m128_f32[2] - p2.m128_f32[2]);
		lerpVert(vertex[vcnt++], v1, v2, t, 1.0f - t);
	}

	if (inout2) vertex[vcnt++] = v2;

	if (inout2 ^ inout0){
		float t = p2.m128_f32[2] / (p2.m128_f32[2] - p0.m128_f32[2]);
		lerpVert(vertex[vcnt++], v2, v0, t, 1.0f - t);
	}
	if (!vcnt) return;
#ifdef MULTITHREAD
	RasterizeMultiThread(vertex[0], vertex[1], vertex[2]);
	if (vcnt == 4) RasterizeMultiThread(vertex[0], vertex[2], vertex[3]);
#else
	RasterizeAndOutput(vertex[0], vertex[1], vertex[2]);
	if (vcnt == 4) RasterizeAndOutput(vertex[0], vertex[2], vertex[3]);
#endif
}

void Renderer::RasterizeAndOutputIndexed(int i){
	RasterizeAndOutput(
		triangleBuffer[i].vert[0],
		triangleBuffer[i].vert[0],
		triangleBuffer[i].vert[0]);
}

void Renderer::RasterizeAndOutput(Vertex& v0, Vertex& v1, Vertex& v2){
	//do divide
	XMVECTOR p0 = v0.positionH;
	XMVECTOR p1 = v1.positionH;
	XMVECTOR p2 = v2.positionH;
	XMVECTOR zz = { 1.0f / p0.m128_f32[3], 1.0f / p1.m128_f32[3], 1.0f / p2.m128_f32[3] };
	p0 = XMVectorScale(p0, zz.m128_f32[0]);
	p1 = XMVectorScale(p1, zz.m128_f32[1]);
	p2 = XMVectorScale(p2, zz.m128_f32[2]);
	
#ifdef CULL_BACK
	XMVECTOR vec0 = XMVectorSubtract(p1, p0);
	XMVECTOR vec1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(vec0, vec1);
	if (cross.m128_f32[2] >= 0) return;
#endif // CULL_BACK

#ifdef CULL_FRONT
	XMVECTOR vec0 = XMVectorSubtract(p1, p0);
	XMVECTOR vec1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(vec0, vec1);
	if (cross.m128_f32[2] <= 0) return;
#endif // CULL_BACK

	XMVECTOR zvec = { p0.m128_f32[2] * 65535, p0.m128_f32[2] * 65535, p2.m128_f32[2] * 65535 };
	XMVECTOR X = { p0.m128_f32[0], p1.m128_f32[0], p2.m128_f32[0] };
	XMVECTOR Y = { p0.m128_f32[1], p1.m128_f32[1], p2.m128_f32[1] };
	XMVECTOR wsVec = { wScale, wScale, wScale };
	XMVECTOR hsVec = { hScale, hScale, hScale };
	XMVECTOR wcVec = { wCor, wCor, wCor };
	XMVECTOR hcVec = { hCor, hCor, hCor };
	//to image coord
	X = XMVectorMultiplyAdd(X, wsVec, wcVec);
	Y = XMVectorMultiplyAdd(Y, hsVec, hcVec);
////////////////////////////////////for raterization////////////////////////////
 	double dbX[3] = { X.m128_f32[0], X.m128_f32[1], X.m128_f32[2] };
 	double dbY[3] = { Y.m128_f32[0], Y.m128_f32[1], Y.m128_f32[2] };
	double ddX[3] = { dbX[1] - dbX[2], dbX[2] - dbX[0], dbX[0] - dbX[1] };
	double ddY[3] = { dbY[1] - dbY[2], dbY[2] - dbY[0], dbY[0] - dbY[1] };
	double ddXY[3] = {
		dbX[1] * dbY[2] - dbX[2] * dbY[1],
		dbX[2] * dbY[0] - dbX[0] * dbY[2],
		dbX[0] * dbY[1] - dbX[1] * dbY[0]
	};
	
//	double a0, a1, a2, b0, b1, b2, c0, c1, c2;
/////////////////////////////////////for barycentric/////////////////////
	XMVECTOR a, b, c;
	double area = f12(ddX[0], ddY[0], ddXY[0], dbX[0], dbY[0]);
	if (abs(area) < EPS) return;
	genCoe(area, ddX[0], ddY[0], ddXY[0], X.m128_f32[0], Y.m128_f32[0], a.m128_f32[0], b.m128_f32[0], c.m128_f32[0]);
	genCoe(area, ddX[1], ddY[1], ddXY[1], X.m128_f32[1], Y.m128_f32[1], a.m128_f32[1], b.m128_f32[1], c.m128_f32[1]);
	genCoe(area, ddX[2], ddY[2], ddXY[2], X.m128_f32[2], Y.m128_f32[2], a.m128_f32[2], b.m128_f32[2], c.m128_f32[2]);

	//get bounding box
	int maxx = (int)ceilf(max(max(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	int maxy = (int)floorf(max(max(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	//int minx = (int)floorf(min(min(X.x, X.y), X.z));
	int minx = (int)ceilf(min(min(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	int miny = (int)floorf(min(min(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	int dx = maxx - minx;
	maxy = min(maxy, fbH);
	miny = max(miny, 0);
	if (maxy <= miny || maxx <= minx) return;
	XMVECTOR abcybase = {
		f(a.m128_f32[0], b.m128_f32[0], c.m128_f32[0], minx, maxy),
		f(a.m128_f32[1], b.m128_f32[1], c.m128_f32[1], minx, maxy),
		f(a.m128_f32[2], b.m128_f32[2], c.m128_f32[2], minx, maxy),
		0
	};
	XMVECTOR abcx = abcybase;
	float zybase = XMVector3Dot(abcybase, zvec).m128_f32[0], zx = zybase;
	float zdx = XMVector3Dot(zvec, a).m128_f32[0], zdy = XMVector3Dot(zvec, b).m128_f32[0];
	XMMATRIX pos, normal, uv, rgb;
	pos.r[0] = v0.position;
	pos.r[1] = v1.position;
	pos.r[2] = v2.position;
	pos.r[3] = { { 0, 0, 0, 0 } };

	normal.r[0] = v0.normal;
	normal.r[1] = v1.normal;
	normal.r[2] = v2.normal;
	normal.r[3] = { { 0, 0, 0, 0 } };

	uv.r[0] = v0.uv;
	uv.r[1] = v1.uv;
	uv.r[2] = v2.uv;
	uv.r[3] = { { 0, 0, 0, 0 } };

	rgb.r[0] = v0.pcolor;
	rgb.r[1] = v1.pcolor;
	rgb.r[2] = v2.pcolor;
	rgb.r[3] = { { 0, 0, 0, 0 } };
	XMVECTOR abczBasey =	XMVectorMultiply(abcybase, zz);
	XMVECTOR za =			XMVectorMultiply(a, zz),zb = XMVectorMultiply(b, zz);
	XMVECTOR colorBasey =	XMVector4Transform(abczBasey, rgb), colorX = colorBasey;
	XMVECTOR uvBasey =		XMVector4Transform(abczBasey, uv), uvX = uvBasey;
	XMVECTOR colorZa =		XMVector4Transform(za, rgb), colorZb = XMVector4Transform(zb, rgb);
	XMVECTOR uvZa =			XMVector4Transform(za, uv), uvZb = XMVector4Transform(zb, uv);
#ifdef PIXELLIGHT
	XMVECTOR posBasey =		XMVector4Transform(abczBasey, pos), posX = posBasey;
	XMVECTOR normalBasey =	XMVector4Transform(abczBasey, normal), normalX = normalBasey;
	XMVECTOR posZa =		XMVector4Transform(za, pos), posZb = XMVector4Transform(zb, pos);
	XMVECTOR normalZa =		XMVector4Transform(za, normal), normalZb = XMVector4Transform(zb, normal);
#endif

	float zabcBaseY =		XMVector3Dot(zz, abcybase).m128_f32[0], zabcX = zabcBaseY;
	float zaScale =			XMVector3Dot(zz, a).m128_f32[0], zbScale = XMVector3Dot(zz, b).m128_f32[0];
//////////////////////////////////////////// end barycentric//////////////////////////////////////////	
	XMVECTOR tabcfloor, tabcCeil;
	XMVECTOR abcz;
	Vertex tvert;
#ifdef SCANLINE
	double interBase[3], interdy[3];
	int icnt = 0;
	for (int k = 0; k < 3; k++){
		if (abs(ddY[k]) > EPS){
			interdy[icnt] = ddX[k] / ddY[k];
			interBase[icnt] = interdy[icnt] * maxy - ddXY[k] / ddY[k];
			icnt++;
		}
	}
	for (int i = maxy; i > miny; i--){
		int rec[3] = { 0, 0, 0 }, t, cnt = 0;
		float mint;
		for (int k = 0; k < icnt; k++){
			t = ceil(interBase[k]);
			if (t >= minx && t <= maxx) rec[cnt++] = t - minx;
		}
		int start = minx, end = minx;
		if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
		if (cnt == 3){
 			if (rec[1] > rec[2]) std::swap(rec[1], rec[2]);
 			if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);

			tabcCeil = XMVectorAdd(abcybase, XMVectorScale(a, rec[0]));
			if (INTRIANGLE(tabcCeil)){
				tabcfloor = XMVectorAdd(abcybase, XMVectorScale(a, rec[2] - 1));
				mint = rec[0];
				if (INTRIANGLE(tabcfloor)){
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
				mint = rec[1];
				if (INTRIANGLE(tabcfloor)){
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
#ifdef XCULL
			if (start < 0){
				mint = -minx;
				abcx = XMVectorAdd(abcybase, XMVectorScale(a, mint));
				start = 0;
			}
			end = min(end, fbW);
#endif
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
				//if (zx > -EPS && zx < 65535.0f){
					tvert.uv = XMVectorScale(uvX, scale);
					tvert.color = XMVectorScale(colorX, scale);
#ifdef DEBUG_MODE
					if (j == start) tvert.color = { 1.0f, 0.0f, 0.0f, 0.0f };
					else if (j == end - 1) tvert.color = { 0.0f, 0.0f, 0.0f, 0.0f };
					else if (cnt == 3) tvert.color = { 0.0f, 1.0f, 0.0f,0.0f };
					else tvert.color = { 0.0f, 0.0f, 1.0f, 0.0f };
#endif
#ifdef PIXELLIGHT
					tvert.position = XMVectorScale(posX, scale);
					tvert.normal = XMVectorScale(normalX, scale);
#endif
					//zx = XMVector3Dot(abcx, zvec).m128_f32[0];
					OutputMerge(PixelShader(tvert, scale), zx, j, i);
					//OutputMerge(texture.BilinearSampler(tvert.uv), zx, j, i);
					////////////////////////////////////////////////////////////////////
				//}
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
		for (int k = 0; k < icnt; k++){
			interBase[k] -= interdy[k];
		}
	}
#else
	//brute force
#ifdef XCULL
	minx = max(minx, 0);
	maxx = min(maxx, fbW);
#endif
	for (int i = maxy; i > miny; i--){
		zabcX = zabcBaseY;
		zx = zybase;
		abcx = abcybase;
		uvX = uvBasey;
		colorX = colorBasey;
#ifdef PIXELLIGHT
		posX = posBasey;
		normalX = normalBasey;
#endif
		bool in = false;
		for (int j = minx; j < maxx; j++){
			if (INTRIANGLE(abcx))
			{
				in = true;
				float  scale = 1.0f / zabcX;
				tvert.uv =XMVectorScale(uvX, scale);
				tvert.color = XMVectorScale(colorX, scale);
#ifdef PIXELLIGHT
				tvert.position=XMVectorScale(posX, scale);
				tvert.normal=XMVectorScale(normalX, scale);
#endif
				OutputMerge(PixelShader(tvert, scale), zx, j, i);
			}
			else if(in) break;
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
#endif

}

///muti thread data////

XMVECTOR abcybase;
XMVECTOR abczBasey;
XMVECTOR za, zb;
XMVECTOR colorBasey;
XMVECTOR uvBasey;
XMVECTOR colorZa, colorZb;
XMVECTOR uvZa,uvZb;
XMVECTOR a, b, c;
#ifdef PIXELLIGHT
XMVECTOR posBasey;
XMVECTOR normalBasey;
XMVECTOR posZa, posZb;
XMVECTOR normalZa, normalZb;
#endif
float zabcBaseY;
float zaScale, zbScale; 
float zybase;
float zdx , zdy;
int maxx, maxy, minx, miny;
double dbX[3], dbY[3], ddX[3], ddY[3], ddXY[3];
double interBase[3], interdy[3];
int icnt = 0;
///////////////////////////////////muti thread data/////////////////////////////////
void Renderer::RasterizeLine(int sy, int ey){
	float fsy = sy;
	XMVECTOR tabcybase = XMVectorSubtract(abcybase, XMVectorScale(b, fsy)), abcx;
	XMVECTOR tcolorBasey = XMVectorSubtract(colorBasey, XMVectorScale(colorZb, fsy)), colorX = tcolorBasey;
	XMVECTOR tuvBasey = XMVectorSubtract(uvBasey, XMVectorScale(uvZb, fsy)), uvX = tuvBasey;
#ifdef PIXELLIGHT
	XMVECTOR tposBasey = XMVectorSubtract(posBasey, XMVectorScale(posZb, fsy)), posX = tposBasey;
	XMVECTOR tnormalBasey = XMVectorSubtract(normalBasey, XMVectorScale(normalZb, fsy)), normalX = tnormalBasey;
#endif
//////////////////////////init /////////////////////////////
	float tzabcBaseY = zabcBaseY - zbScale * fsy, zabcX = tzabcBaseY;
	float tzybase = zybase - zdy * fsy, zx = tzybase;
	double tinterBase[3];
	XMVECTOR tabcfloor, tabcCeil;
	Vertex tvert;
	for (int k = 0; k < icnt; k++)
		tinterBase[k] = interBase[k] - interdy[k] * sy;
	int starty = maxy - sy, endy = maxy - ey;
	assert(starty >= 0 && endy >= 0);
	for (int i = starty; i > endy; i--){
		//////check for intersection/////////
		int rec[3] = { 0, 0, 0 }, t, cnt = 0;
		float mint;
		for (int k = 0; k < icnt; k++){
			t = ceil(tinterBase[k]);
			if (t >= minx && t <= maxx) rec[cnt++] = t - minx;
		}
		int start = minx, end = minx;
		if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);
		if (cnt == 3){
			if (rec[1] > rec[2]) std::swap(rec[1], rec[2]);
			if (rec[0] > rec[1]) std::swap(rec[0], rec[1]);

			tabcCeil = XMVectorAdd(tabcybase, XMVectorScale(a, rec[0]));
			if (INTRIANGLE(tabcCeil)){
				tabcfloor = XMVectorAdd(tabcybase, XMVectorScale(a, rec[2] - 1));
				mint = rec[0];
				if (INTRIANGLE(tabcfloor)){
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
				tabcfloor = XMVectorAdd(tabcybase, XMVectorScale(a, rec[1]));
				mint = rec[1];
				if (INTRIANGLE(tabcfloor)){
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
			abcx = XMVectorAdd(tabcybase, XMVectorScale(a, mint));
		}
		if (start != end){
#ifdef XCULL
			if (start < 0){
				mint = -minx;
				abcx = XMVectorAdd(tabcybase, XMVectorScale(a, mint));
				start = 0;
			}
			end = min(end, fbW);
#endif
			zabcX = tzabcBaseY + mint * zaScale;
			zx = tzybase + mint * zdx;
			colorX = XMVectorAdd(tcolorBasey, XMVectorScale(colorZa, mint));
			uvX = XMVectorAdd(tuvBasey, XMVectorScale(uvZa, mint));
#ifdef PIXELLIGHT
			posX = XMVectorAdd(tposBasey, XMVectorScale(posZa, mint));
			normalX = XMVectorAdd(tnormalBasey, XMVectorScale(normalZa, mint));
#endif
			for (int j = start; j < end; j++){
				float scale = 1.0f / zabcX, paddingf[3];
				//if (scale > -EPS){
					tvert.uv = XMVectorScale(uvX, scale);
					tvert.color = XMVectorScale(colorX, scale);
#ifdef DEBUG_MODE
					if (j == start) tvert.color = { 1.0f, 0.0f, 0.0f, 0.0f };
					else if (j == end - 1) tvert.color = { 0.0f, 0.0f, 0.0f, 0.0f };
					else if (cnt == 3) tvert.color = { 0.0f, 1.0f, 0.0f, 0.0f };
					else tvert.color = { 0.0f, 0.0f, 1.0f, 0.0f };
#endif
#ifdef PIXELLIGHT
					tvert.position = XMVectorScale(posX, scale);
					tvert.normal = XMVectorScale(normalX, scale);
#endif
					OutputMerge(PixelShader(tvert, scale), zx, j, i);
				//}
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

		tzabcBaseY -= zbScale;
		tzybase -= zdy;
		tabcybase = XMVectorSubtract(tabcybase, b);
		tcolorBasey = XMVectorSubtract(tcolorBasey, colorZb);
		tuvBasey = XMVectorSubtract(tuvBasey, uvZb);
#ifdef PIXELLIGHT
		tposBasey = XMVectorSubtract(tposBasey, posZb);
		tnormalBasey = XMVectorSubtract(tnormalBasey, normalZb);
#endif
		for (int k = 0; k < icnt; k++){
			tinterBase[k] -= interdy[k];
		}
	}
	
}

void Renderer::RasterizeMultiThread(Vertex& v0, Vertex& v1, Vertex& v2){
	//////////////////////init mutl thread data///////////////////////////////////
	XMVECTOR p0 = v0.positionH;
	XMVECTOR p1 = v1.positionH;
	XMVECTOR p2 = v2.positionH;
	XMVECTOR zz = { 1.0f / p0.m128_f32[3], 1.0f / p1.m128_f32[3], 1.0f / p2.m128_f32[3] };
	p0 = XMVectorScale(p0, zz.m128_f32[0]);
	p1 = XMVectorScale(p1, zz.m128_f32[1]);
	p2 = XMVectorScale(p2, zz.m128_f32[2]);
	XMVECTOR zvec = { p0.m128_f32[2] * 65535, p0.m128_f32[2] * 65535, p2.m128_f32[2] * 65535 };
#ifdef CULL_BACK
	XMVECTOR vec0 = XMVectorSubtract(p1, p0);
	XMVECTOR vec1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(vec0, vec1);
	if (cross.m128_f32[2] >= 0) return;
#endif // CULL_BACK
#ifdef CULL_FRONT
	XMVECTOR vec0 = XMVectorSubtract(p1, p0);
	XMVECTOR vec1 = XMVectorSubtract(p2, p1);
	XMVECTOR cross = XMVector3Cross(vec0, vec1);
	if (cross.m128_f32[2] <= 0) return;
#endif // CULL_BACK

	XMVECTOR X = { p0.m128_f32[0], p1.m128_f32[0], p2.m128_f32[0] };
	XMVECTOR Y = { p0.m128_f32[1], p1.m128_f32[1], p2.m128_f32[1] };
	XMVECTOR wsVec = { wScale, wScale, wScale };
	XMVECTOR hsVec = { hScale, hScale, hScale };
	XMVECTOR wcVec = { wCor, wCor, wCor };
	XMVECTOR hcVec = { hCor, hCor, hCor };
	//to image coord
	X = XMVectorMultiplyAdd(X, wsVec, wcVec);
	Y = XMVectorMultiplyAdd(Y, hsVec, hcVec);

	for (int i = 0; i < 3; i++){
		dbX[i] = X.m128_f32[i];
		dbY[i] = Y.m128_f32[i];
	}
	for (int i = 0; i < 3; i++){
		int j = (i + 1) % 3, k = (i + 2) % 3;
		ddX[i] = dbX[j] - dbX[k];
		ddY[i] = dbY[j] - dbY[k];
		ddXY[i] = dbX[j] * dbY[k] - dbX[k] * dbY[j];
	}

	double area = f12(ddX[0], ddY[0], ddXY[0], dbX[0], dbY[0]);
	if (abs(area) < EPS) return;
	genCoe(area, ddX[0], ddY[0], ddXY[0], X.m128_f32[0], Y.m128_f32[0], a.m128_f32[0], b.m128_f32[0], c.m128_f32[0]);
	genCoe(area, ddX[1], ddY[1], ddXY[1], X.m128_f32[1], Y.m128_f32[1], a.m128_f32[1], b.m128_f32[1], c.m128_f32[1]);
	genCoe(area, ddX[2], ddY[2], ddXY[2], X.m128_f32[2], Y.m128_f32[2], a.m128_f32[2], b.m128_f32[2], c.m128_f32[2]);

	//get bounding box
	maxx = (int)ceilf(max(max(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	maxy = (int)floorf(max(max(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	minx = (int)ceilf(min(min(X.m128_f32[0], X.m128_f32[1]), X.m128_f32[2]));
	miny = (int)floorf(min(min(Y.m128_f32[0], Y.m128_f32[1]), Y.m128_f32[2]));
	maxy = min(maxy, fbH);
	miny = max(miny, 0);
	int dx = maxx - minx, dy = maxy - miny;
	if (dy <= 0 || dx <= 0) return;
	XMMATRIX pos, normal, uv, rgb;
	pos.r[0] = v0.position;
	pos.r[1] = v1.position;
	pos.r[2] = v2.position;
	pos.r[3] = { { 0, 0, 0, 0 } };

	normal.r[0] = v0.normal;
	normal.r[1] = v1.normal;
	normal.r[2] = v2.normal;
	normal.r[3] = { { 0, 0, 0, 0 } };

	uv.r[0] = v0.uv;
	uv.r[1] = v1.uv;
	uv.r[2] = v2.uv;
	uv.r[3] = { { 0, 0, 0, 0 } };

	rgb.r[0] = v0.pcolor;
	rgb.r[1] = v1.pcolor;
	rgb.r[2] = v2.pcolor;
	rgb.r[3] = { { 0, 0, 0, 0 } };

	abcybase = { {
		f(a.m128_f32[0], b.m128_f32[0], c.m128_f32[0], minx, maxy),
		f(a.m128_f32[1], b.m128_f32[1], c.m128_f32[1], minx, maxy),
		f(a.m128_f32[2], b.m128_f32[2], c.m128_f32[2], minx, maxy),
		0
	}};
	zybase =		XMVector3Dot(abcybase, zvec).m128_f32[0];
	zdx =			XMVector3Dot(zvec, a).m128_f32[0], zdy = XMVector3Dot(zvec, b).m128_f32[0];
	abczBasey =		XMVectorMultiply(abcybase, zz);
	za =			XMVectorMultiply(a, zz), zb = XMVectorMultiply(b, zz);
	colorBasey =	XMVector4Transform(abczBasey, rgb);
	uvBasey =		XMVector4Transform(abczBasey, uv);
	colorZa =		XMVector4Transform(za, rgb), colorZb = XMVector4Transform(zb, rgb);
	uvZa =			XMVector4Transform(za, uv), uvZb = XMVector4Transform(zb, uv);
#ifdef PIXELLIGHT
	posBasey =		XMVector4Transform(abczBasey, pos);
	normalBasey =	XMVector4Transform(abczBasey, normal);
	posZa =			XMVector4Transform(za, pos), posZb = XMVector4Transform(zb, pos);
	normalZa =		XMVector4Transform(za, normal), normalZb = XMVector4Transform(zb, normal);
#endif

	zabcBaseY =		XMVector3Dot(zz, abcybase).m128_f32[0];
	zaScale =		XMVector3Dot(zz, a).m128_f32[0], zbScale = XMVector3Dot(zz, b).m128_f32[0];
	icnt = 0;
	for (int k = 0; k < 3; k++){
		if (abs(ddY[k]) > EPS){
			interdy[icnt] = ddX[k] / ddY[k];
			interBase[icnt] = interdy[icnt] * maxy - ddXY[k] / ddY[k];
			icnt++;
		}
	}
	//////////////////////////////////////////// end barycentric//////////////////////////////////////////	
	//////////////////multiThread/////////////////////////////////
	std::thread t[THREAD_COUNT];
	int perTh = (dy + THREAD_COUNT - 1) / THREAD_COUNT;
	int th = 0, sy = 0, ey;
	for (int i = 0; i < THREAD_COUNT; i++){
		ey = sy + perTh;
		if (ey <= dy) {
			t[th++] = std::thread(&Renderer::RasterizeLine, this, sy, ey);
			sy = ey;
		}
		else{
			t[th++] = std::thread(&Renderer::RasterizeLine, this, sy, dy);
			break;
		}
	}

	for (int j = 0; j < th; j++)
		t[j].join();
	//RasterizeLine(0);

}

void Renderer::RasterizeMultiThreadIndexed(int i){
	RasterizeMultiThread(
		triangleBuffer[i].vert[0],
		triangleBuffer[i].vert[0],
		triangleBuffer[i].vert[0]);
}