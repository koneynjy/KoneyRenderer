#ifndef _RENDERER
#define _RENDERER
#include "Vertex.hpp"
#include "Triangle.hpp"
const int maxsize = 1024;
class Renderer
{
public:
	Renderer();
	~Renderer();
	void SetTriangleBuffer(Triangle * triBuffer, int size);
	void VertexShader(Triangle & triangle);
	void RasterizeAndOutput(Triangle & triangle);
	unsigned PixelShader(Vertex vin, int x, int y);
	void OutputMerge(unsigned c, unsigned short z, int x, int y);
	void BarycentricRaster(Triangle & triangle);
	void ScanLineRaster(Triangle & triangle);
	void UpRaster(float xl, float yl, float xr, float yr, float xu, float yu, Vertex&l, Vertex&r, Vertex& u);
	void DowneRaster(float xl, float yl, float xr, float yr, float xd, float yd, Vertex&l, Vertex&r, Vertex& d);
	void Render();
	void setSize(int width, int height);
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX mw;
	Triangle triangleBuffer[1024];
	int bSize,padding;
	int fbWidth, fbHeight;
	unsigned *fbdata;
	unsigned short *zbuffer;
	unsigned int count;
};
#endif