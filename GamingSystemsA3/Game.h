#ifndef GAME_H
#define GAME_H

#include "Headers.h"
#include "FrameTracker.h"
#include "Object.h"
#include "Camera.h"

/*
 The game class uses directX to display the "game".
*/
class Game {
private:
	HWND hWnd;
	LPDIRECT3D9 pD3D;//COM object
	LPDIRECT3DDEVICE9 pDevice;//graphics device
	LPDIRECT3DSURFACE9 backSurface;
	LPDIRECT3DSURFACE9 bmpSurface;
	LPD3DXFONT font;
	FrameTracker frame;
	Camera cam;
	Object models[2];
	D3DLIGHT9 lights[3];
	bool lightsOn[4];
	int width, height, fps, selectedModel;
	float lastTime;
	POINT startPos;

public:
	Game();
	Game(HWND);
	void SetHWND(HWND);
	HWND GetHWND();
	int LoadBitmapToSurface(LPCTSTR, LPDIRECT3DSURFACE9*, LPDIRECT3DDEVICE9);
	int InitDirect3DDevice(HWND, BOOL, D3DFORMAT, LPDIRECT3D9, LPDIRECT3DDEVICE9*);
	static long CALLBACK StaticProc(HWND, UINT, WPARAM, LPARAM);
	long CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	int GameInit();
	int GameShutdown();
	int Render();
	int GameLoop();
	void createLights();
	void updateCam(float timeDelta);
	Ray CalcPickingRay(int x, int y);  //Compute a picking ray in "View Space"
	void TransformRay(Ray* ray, D3DXMATRIX* T); //Transform computed ray into "World space" / object's local space.
	bool raySphereIntersectionTest(Ray* ray, Object* sphere);
};

#endif // !GAME_H
