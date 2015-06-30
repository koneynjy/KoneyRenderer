#ifndef _RENDERER
#define _RENDERER
#include "Vertex.hpp"
#include "Triangle.hpp"
#include "Texture.hpp"
#include <thread>
const int maxsize = 1024;
class Renderer
{
public:
	Renderer();
	~Renderer();
	void SetTriangleBuffer(Triangle * triBuffer, int size);
	void VertexShader(Vertex & vert);
	void RasterizeAndOutput(Triangle & triangle);
	void RasterizeAndOutputIndexed(int i);
	DirectX::XMFLOAT4 PixelShader(Vertex vin,float z );
	void OutputMerge(DirectX::XMFLOAT4 c, unsigned short z, int x, int y);
	void Render();
	void RasterizeLine(int sy, int ey);
	void RasterizeMultiThread(Triangle& triangle);
	void RasterizeMultiThreadIndexed(int i);
	void setSize(int width, int height);
	DirectX::XMFLOAT4 lightShader(Vertex& vert);
	inline bool check(int x, int y){
		int idx = y * fbWidth + x;
		return fbdata[idx] != 0xAAAAAAAA;
	}
	Triangle triangleBuffer[1024];
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX mw;
	DirectX::XMFLOAT3 light = { -0.57735f, 0.57735f, 0.57735f };
	DirectX::XMFLOAT3 lightPoint = {2.0f,2.0f,2.0f};
	DirectX::XMFLOAT3 lightColor = {1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 aColor = { 0.2f, 0.2f, 0.2f };
	DirectX::XMFLOAT3 eyePos;
	DirectX::XMFLOAT3 spec = {1.0f,1.0f,1.0f};
	int n = 60;
	Texture texture;
	int bSize,padding;
	int fbWidth, fbHeight;
	int fbW, fbH;
	int fbWf, fbHf;
	float wScale, hScale;
	float wCor, hCor;
	unsigned *fbdata;
	unsigned short *zbuffer;
	unsigned int count;
	std::thread *t;
};
#endif