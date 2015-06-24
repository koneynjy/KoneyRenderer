#ifndef _RENDERER
#define _RENDERER

class Renderer
{
public:
	Renderer();
	Renderer(int width, int height, void *pdata);
private:
	int fbWidth, fbHeight;
	void *fbdata;
	unsigned short *zbuffer;
};
#endif