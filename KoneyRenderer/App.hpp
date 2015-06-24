#ifndef _APP
#define _APP


#include "Globel.hpp"
#include "Timer.hpp"
#include <string>
class App
{
public:
	App(HINSTANCE hInstance);
	virtual ~App();

	HINSTANCE getAppInst();
	HWND      getMainWnd();

	int run();

	virtual void initApp();
	virtual void onResize();// reset projection/etc
	virtual void updateScene(float dt);
	virtual void drawScene();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void initMainWindow();

protected:

	HINSTANCE mhAppInst;
	HWND      mhMainWnd;
	HDC		  hdc;
	bool      mAppPaused;
	bool      mMinimized;
	bool      mMaximized;
	bool      mResizing;

	Timer mTimer;

	std::wstring mFrameStats;
	std::wstring mMainWndCaption;

	int mClientWidth;
	int mClientHeight;
};




#endif // D3DAPP_H