#ifndef GAME_H
#define GAME_H

#include "Headers.h"
#include "Util.h"
#include "FrameTracker.h"
#include "Object.h"
#include "Vertex.h"
#include "Camera.h"
#include "ParticleSystem.h"




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
		ParticleSystem* snow = 0;

	public:
		Game();
		Game(HWND);
		void setHWND(HWND);
		HWND getHWND();
		int loadBitmapToSurface(LPCTSTR, LPDIRECT3DSURFACE9*, LPDIRECT3DDEVICE9);
		int initDirect3DDevice(HWND, BOOL, D3DFORMAT, LPDIRECT3D9, LPDIRECT3DDEVICE9*);
		static long CALLBACK staticProc(HWND, UINT, WPARAM, LPARAM);
		long CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
		int gameInit();
		int gameShutdown();
		int render();
		int gameLoop();
		void createLights();
		void updateCam(float timeDelta);
		Ray calcPickingRay(int x, int y);  //Compute a picking ray in "View Space"
		void transformRay(Ray* ray, D3DXMATRIX* T); //Transform computed ray into "World space" / object's local space.
		bool raySphereIntersectionTest(Ray* ray, Object* sphere);
		void renderMirror();
		void renderLeftMirror();
		void renderRightMirror();
		void renderBackMirror();
		void setupMirror();
		void setupParticles();
		void startSnow(float);

		//
		// Materials
		//
		D3DMATERIAL9 initMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
		//
		// Materials
		//
		const D3DMATERIAL9 WHITE_MTRL = initMtrl(WHITE, WHITE, WHITE, BLACK, 2.0f);
		const D3DMATERIAL9 RED_MTRL = initMtrl(RED, RED, RED, BLACK, 2.0f);
		const D3DMATERIAL9 GREEN_MTRL = initMtrl(GREEN, GREEN, GREEN, BLACK, 2.0f);
		const D3DMATERIAL9 BLUE_MTRL = initMtrl(BLUE, BLUE, BLUE, BLACK, 2.0f);
		const D3DMATERIAL9 YELLOW_MTRL = initMtrl(YELLOW, YELLOW, YELLOW, BLACK, 2.0f);

		IDirect3DVertexBuffer9* VB = 0;
		IDirect3DTexture9* MirrorTex = 0;
		D3DMATERIAL9 MirrorMtrl = WHITE_MTRL;
		D3DMATERIAL9 mirrorReflectMtrl;


};

#endif // !GAME_H
