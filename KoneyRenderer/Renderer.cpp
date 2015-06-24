#include "Renderer.hpp"
inline float f12(DirectX::XMFLOAT4 &p1, DirectX::XMFLOAT4 &p2, float x, float y){
	return (p1.y - p2.y) * x + (p2.x - p1.x) * y + p1.x * p2.y - p2.x * p1.y;
}

inline bool genCoe(DirectX::XMFLOAT4 &p1, DirectX::XMFLOAT4 &p2, DirectX::XMFLOAT4 &p, float &a, float &b, float &c){
	float tmp = f12(p1, p2, p.x, p.y);
	if (fabs(tmp) < EPS) return false;
	a = (p1.y - p2.y) / tmp;
	b = (p2.x - p1.x) / tmp;
	c = (p1.x * p2.y - p2.x * p1.y) / tmp;
}

inline float f(float a, float b, float c, int x, int y){
	return a * x + b * y + c;
}


inline int min3(float a, float b, float c){

}

Renderer::Renderer(){}
DirectX::XMVECTOR tmpVec;

void Renderer::setSize(int width, int height){
	fbWidth = width, fbWidth = height;
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
	//triangle.vert[0].positionH = XMVector4Transform(triangle.vert[0].position, mw);
	//triangle.vert[1].positionH = XMVector4Transform(triangle.vert[1].position, mw);
	//triangle.vert[2].positionH = XMVector4Transform(triangle.vert[2].position, mw);

	/*triangle.vert[0].normal = XMVector4Transform(triangle.vert[0].normal, mw);
	triangle.vert[1].normal = XMVector4Transform(triangle.vert[1].normal, mw);
	triangle.vert[2].normal = XMVector4Transform(triangle.vert[2].normal, mw);*/
	XMStoreFloat4(&triangle.vert[0].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[0].position), mvp));
	XMStoreFloat4(&triangle.vert[1].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[1].position), mvp));
	XMStoreFloat4(&triangle.vert[2].positionH, DirectX::XMVector4Transform(XMLoadFloat4(&triangle.vert[2].position), mvp));
}

unsigned Renderer::PixelShader(Vertex& vin, int x, int y){
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
	int idx = y * fbWidth + x;
	if (zbuffer[idx] > z){//ztest
		fbdata[idx] = c;
		zbuffer[idx] = z;
	}
}

void Renderer::Render(){
	for (int i = 0; i < bSize; i++){
		VertexShader(triangleBuffer[i]);
		RasterizeAndOutput(triangleBuffer[i]);
	}
}

void Renderer::RasterizeAndOutput(Triangle & triangle){
	DirectX::XMFLOAT4 &p0 = triangle.vert[0].positionH;
	DirectX::XMFLOAT4 &p1 = triangle.vert[1].positionH;
	DirectX::XMFLOAT4 &p2 = triangle.vert[2].positionH;
	float z0 = p0.w;
	float z1 = p1.w;
	float z2 = p2.w;
	float a0, a1, a2, b0, b1, b2, c0, c1, c2;
	XMStoreFloat4(&p0, DirectX::XMVectorScale(XMLoadFloat4(&p0), z0));
	XMStoreFloat4(&p1, DirectX::XMVectorScale(XMLoadFloat4(&p1), z1));
	XMStoreFloat4(&p2, DirectX::XMVectorScale(XMLoadFloat4(&p2), z2));
	genCoe(p1, p2, p0, a0, b0, c0);
	genCoe(p2, p0, p1, a1, b1, c1);
	genCoe(p0, p1, p2, a2, b2, c2);
	int maxx = (int)((max(p0.x, p1.x, p2.x) * 0.5f + 0.5f) * fbWidth);
	int maxy = (int)((max(p0.y, p1.y, p2.y) * 0.5f + 0.5f) * fbHeight);
	int minx = (int)((min(p0.x, p1.x, p2.x) * 0.5f + 0.5f) * fbWidth);
	int miny = (int)((min(p0.y, p1.y, p2.y) * 0.5f + 0.5f) * fbHeight);

}