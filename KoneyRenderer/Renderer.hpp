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
	DirectX::XMVECTOR PixelShader(Vertex& vin,float z );
	void OutputMerge(DirectX::XMVECTOR c, unsigned short z, int x, int y);
	void Render();
	void RasterizeLine(int sy, int ey);
	void RasterizeMultiThread(Triangle& triangle);
	void RasterizeMultiThreadIndexed(int i);
	void setSize(int width, int height);
	void ClippNearRasterization(Triangle& triangle);
	DirectX::XMVECTOR lightShader(Vertex& vert);
	inline bool check(int x, int y){
		int idx = y * fbWidth + x;
		return fbdata[idx] != 0xAAAAAAAA;
	}
	Triangle triangleBuffer[1024];
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX mw;
	DirectX::XMVECTOR light ;
	DirectX::XMVECTOR lightPoint;
	DirectX::XMVECTOR lightColor;
	DirectX::XMVECTOR aColor;
	DirectX::XMVECTOR eyePos;
	DirectX::XMVECTOR spec;
	DirectX::XMVECTOR colorOverRide;
	DirectX::XMVECTOR gammaCorrect;
	int n = 60;
	float nf = 60.0f;
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
};
#endif