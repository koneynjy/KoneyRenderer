#ifndef _VERTEX
#define _VERTEX
#include "Globel.hpp"
struct Vertex{
public:
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 positionH;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT4 color;
	unsigned c;
	float z;
	DirectX::XMFLOAT4 pcolor;
};
#endif