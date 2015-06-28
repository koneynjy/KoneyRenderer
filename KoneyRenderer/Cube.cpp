#include "App.hpp"
using namespace std;
using namespace DirectX;

class CubeApp : public App
{
public:
	CubeApp(HINSTANCE hInstance);
	~CubeApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene();
	void initVertex();
	void initTriangle();
private:
	XMMATRIX mView;
	XMMATRIX mProj;
	XMMATRIX mWVP;
	Triangle triangle[12];
	XMFLOAT4 point[8];
	XMFLOAT4 normal[6];
	float mTheta;
	float mPhi;
	float mRotate;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	CubeApp theApp(hInstance);

	theApp.initApp();

	return theApp.run();
}

CubeApp::CubeApp(HINSTANCE hInstance)
	: App(hInstance), mTheta(0.0f), mPhi(PI*0.25f), mRotate(0.0f){
	mView = XMMatrixIdentity();
	mProj = XMMatrixIdentity();
	mWVP = XMMatrixIdentity();
	initVertex();
	initTriangle();
	renderer.SetTriangleBuffer(triangle, 12);
}

CubeApp::~CubeApp()
{
}

void CubeApp::initApp()
{
	App::initApp();
}

void CubeApp::onResize()
{
	App::onResize();

	float aspect = (float)mClientWidth / mClientHeight;
	mProj = XMMatrixPerspectiveFovLH(0.25f*PI, aspect, 1.0f, 1000.0f);
}

void CubeApp::updateScene(float dt)
{
	App::updateScene(dt);

	// Update angles based on input to orbit camera around box.
	if (GetAsyncKeyState('A') & 0x8000)	mTheta -= 2.0f*dt;
	if (GetAsyncKeyState('D') & 0x8000)	mTheta += 2.0f*dt;
	if (GetAsyncKeyState('W') & 0x8000)	mPhi -= 2.0f*dt;
	if (GetAsyncKeyState('S') & 0x8000)	mPhi += 2.0f*dt;

	// Restrict the angle mPhi.
	if (mPhi < 0.1f)	mPhi = 0.1f;
	if (mPhi > PI - 0.1f)	mPhi = PI - 0.1f;

	// Convert Spherical to Cartesian coordinates: mPhi measured from +y
	// and mTheta measured counterclockwise from -z.
	float x = 6.0f*sinf(mPhi)*sinf(mTheta);
	float z = -6.0f*sinf(mPhi)*cosf(mTheta);
	float y = 6.0f*cosf(mPhi);
	// Build the view matrix.
	XMVECTOR pos = { x, y, z, 1.0f };
	XMVECTOR target = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 1.0f };
	mView = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat3(&renderer.eyePos, pos);
#ifdef ROTATE
	renderer.mw = XMMatrixRotationAxis({ { 0.0f, 1.0f, 0.0f, 1.0f } }, dt * 1.0f);
	renderer.mvp = renderer.mw * mView*mProj;
#else
	renderer.mvp = mView*mProj;
#endif
	//XMVector4Transform(pos, mView);
}

void CubeApp::drawScene()
{
	App::drawScene();
	renderer.Render();
	SetDIBitsToDevice(hdc, 0, 0, mClientWidth, mClientHeight,
		0, 0, 0, mClientHeight, 
		renderer.fbdata, (LPBITMAPINFO)&bitmapInfo, DIB_RGB_COLORS);
}

void CubeApp::initVertex(){
	point[0] = { -1.0f, 1.0f, -1.0f, 1.0f };//-z
	point[1] = {  1.0f, 1.0f, -1.0f, 1.0f };
	point[2] = { -1.0f,-1.0f, -1.0f, 1.0f };
	point[3] = {  1.0f,-1.0f, -1.0f, 1.0f };
	point[4] = { -1.0f, 1.0f, 1.0f, 1.0f  };//z
	point[5] = {  1.0f, 1.0f, 1.0f, 1.0f  };
	point[6] = { -1.0f,-1.0f, 1.0f, 1.0f  };
	point[7] = {  1.0f,-1.0f, 1.0f, 1.0f  };

	normal[0] = { 0.0f, 0.0f, -1.0f, 0.0f };//-z
	normal[1] = { 0.0f, 0.0f, 1.0f, 0.0f  };//z
	normal[2] = { -1.0f, 0.0f, 0.0f, 0.0f };//-x
	normal[3] = { 1.0f, 0.0f, 0.0f, 0.0f  };//x
	normal[4] = { 0.0f,-1.0f,  0.0f, 0.0f };//-y
	normal[5] = { 0.0f, 1.0f,  0.0f, 0.0f };//y
	
}

void CubeApp::initTriangle()
{
	triangle[0].vert[0] = { point[0], {}, normal[0], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[0].vert[1] = { point[1], {}, normal[0], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[0].vert[2] = { point[2], {}, normal[0], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[1].vert[0] = { point[2], {}, normal[0], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[1].vert[1] = { point[1], {}, normal[0], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[1].vert[2] = { point[3], {}, normal[0], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[2].vert[0] = { point[1], {}, normal[3], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[2].vert[1] = { point[5], {}, normal[3], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[2].vert[2] = { point[3], {}, normal[3], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[3].vert[0] = { point[3], {}, normal[3], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[3].vert[1] = { point[5], {}, normal[3], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[3].vert[2] = { point[7], {}, normal[3], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[4].vert[0] = { point[5], {}, normal[1], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[4].vert[1] = { point[4], {}, normal[1], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[4].vert[2] = { point[7], {}, normal[1], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[5].vert[0] = { point[7], {}, normal[1], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[5].vert[1] = { point[4], {}, normal[1], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[5].vert[2] = { point[6], {}, normal[1], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[6].vert[0] = { point[4], {}, normal[2], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[6].vert[1] = { point[0], {}, normal[2], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[6].vert[2] = { point[6], {}, normal[2], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[7].vert[0] = { point[6], {}, normal[2], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[7].vert[1] = { point[0], {}, normal[2], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[7].vert[2] = { point[2], {}, normal[2], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[8].vert[0] = { point[3], {}, normal[4], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[8].vert[1] = { point[7], {}, normal[4], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[8].vert[2] = { point[2], {}, normal[4], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[9].vert[0] = { point[2], {}, normal[4], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[9].vert[1] = { point[7], {}, normal[4], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[9].vert[2] = { point[6], {}, normal[4], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[10].vert[0] = { point[0], {}, normal[5], { 0.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[10].vert[1] = { point[4], {}, normal[5], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[10].vert[2] = { point[1], {}, normal[5], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

	triangle[11].vert[0] = { point[1], {}, normal[5], { 0.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[11].vert[1] = { point[4], {}, normal[5], { 1.0f, 0.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };
	triangle[11].vert[2] = { point[5], {}, normal[5], { 1.0f, 1.0f }, {}, { 1.0f, 0.0f, 0.0f, 0.0f } };

}

