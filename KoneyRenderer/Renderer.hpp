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
	unsigned PixelShader(Vertex vin);
	void OutputMerge(unsigned c, unsigned short z, int x, int y);
	void Render();
	void setSize(int width, int height);
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX mw;
	DirectX::XMFLOAT3 light = { 0.57735f, 0.57735f, -0.57735f };
	DirectX::XMFLOAT3 lightColor = {0.5f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 aColor = { 0.1f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 kdiff = {0.5f, 0.5f, 0.5f};
	DirectX::XMFLOAT3 eyePos;
	DirectX::XMFLOAT3 spec = {0.5f,0.5f,0.5f};
	DirectX::XMFLOAT3 n = { 30, 30, 30 };
	Triangle triangleBuffer[1024];
	int bSize,padding;
	int fbWidth, fbHeight;
	unsigned *fbdata;
	unsigned short *zbuffer;
	unsigned int count;
};
#endif