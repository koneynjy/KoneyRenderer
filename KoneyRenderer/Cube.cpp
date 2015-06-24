#include "App.hpp"
#include "Util.hpp"
using namespace std;
using namespace DirectX;

class ColoredCubeApp : public App
{
public:
	ColoredCubeApp(HINSTANCE hInstance);
	~ColoredCubeApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene();

private:
	XMMATRIX mView;
	XMMATRIX mProj;
	XMMATRIX mWVP;

	float mTheta;
	float mPhi;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	ColoredCubeApp theApp(hInstance);

	theApp.initApp();

	return theApp.run();
}

ColoredCubeApp::ColoredCubeApp(HINSTANCE hInstance)
	: App(hInstance), mTheta(0.0f), mPhi(PI*0.25f){
	mView = XMMatrixIdentity();
	mProj = XMMatrixIdentity();
	mWVP = XMMatrixIdentity();
}

ColoredCubeApp::~ColoredCubeApp()
{
}

void ColoredCubeApp::initApp()
{
	App::initApp();
}

void ColoredCubeApp::onResize()
{
	App::onResize();

	//float aspect = (float)mClientWidth / mClientHeight;
	//mProj = XMMatrixPerspectiveFovLH(0.25f*PI, aspect, 1.0f, 1000.0f);
}

void ColoredCubeApp::updateScene(float dt)
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
	float x = 5.0f*sinf(mPhi)*sinf(mTheta);
	float z = -5.0f*sinf(mPhi)*cosf(mTheta);
	float y = 5.0f*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = { x, y, z, 1 };
	XMVECTOR target = { 0.0f, 0.0f, 0.0f, 1.0 };
	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 1.0f };
	mView = XMMatrixLookAtLH(pos, target, up);
}

void ColoredCubeApp::drawScene()
{
	App::drawScene();

	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.

	// set constants
	for (int i = 0; i < mClientWidth ; i++)
		for (int j = 0; j < mClientHeight ; j++)
			SetPixel(hdc, i, j, 0x000000ff);
	//mWVP = mView*mProj;
}

