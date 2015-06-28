#ifndef _RENDERER
#define _RENDERER
#include "Vertex.hpp"
#include "Triangle.hpp"
#include "Texture.hpp"
const int maxsize = 1024;
class Renderer
{
public:
	Renderer();
	~Renderer();
	void SetTriangleBuffer(Triangle * triBuffer, int size);
	void VertexShader(Vertex & vert);
	void RasterizeAndOutput(Triangle & triangle);
	DirectX::XMFLOAT4 PixelShader(Vertex vin,float z );
	void OutputMerge(DirectX::XMFLOAT4 c, unsigned short z, int x, int y);
	void Render();
	void setSize(int width, int height);
	DirectX::XMFLOAT4 lightShader(Vertex& vert);
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX mw;
	Triangle triangleBuffer[1024];
	DirectX::XMFLOAT3 light = { -0.57735f, 0.57735f, 0.57735f };
	DirectX::XMFLOAT3 lightPoint = {5.0f,5.0f,4.0f};
	DirectX::XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 aColor = { 0.3f, 0.3f, 0.3f };
	DirectX::XMFLOAT3 eyePos;
	DirectX::XMFLOAT3 spec = {1.0f,1.0f,1.0f};
	float n = 30.0f;
	Texture texture;
	int bSize,padding;
	int fbWidth, fbHeight;
	unsigned *fbdata;
	unsigned short *zbuffer;
	unsigned int count;
};
#endif