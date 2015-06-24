#ifndef _TRIANGLE
#define _TRIANGLE
#include "Vertex.hpp"

struct Triangle{
	Triangle(){};
	Triangle(Vertex &v0, Vertex &v1, Vertex &v2){
		vert[0] = v0;
		vert[1] = v1;
		vert[2] = v2;
	}
	Vertex vert[3];
};
#endif