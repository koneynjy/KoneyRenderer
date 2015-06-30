#ifndef _VERTEX
#define _VERTEX
#include "Globel.hpp"
struct Vertex{
public:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR positionH;
	DirectX::XMVECTOR normal;
	DirectX::XMVECTOR uv;
	int padding[2];
	DirectX::XMVECTOR color;
	DirectX::XMVECTOR pcolor;
};
#endif