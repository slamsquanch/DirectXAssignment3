#include "Headers.h"

/*
 Sets up and creates the directX render device that will display the game.
 
 @param hWndTarget - a handle to the main game window
 @param bWindowed - true for windowed, false for fullscreen
 @param FullScreenFormat - The specific format to use when using fullscreen
						   mode
 @param pD3D - The directX COM object to use for accessing methods of that
			   interface
 @param ppDevice - A pointer to the graphics device that will store the
 				   created device
 
 @return - Returns an int to be used as an HRESULT in the FAILED() macro.
		   Fails if:
			- It can not get the display adapter information
			- It can not create the render device
 */
int Game::InitDirect3DDevice(HWND hWndTarget, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice) {
	D3DPRESENT_PARAMETERS d3dpp;//rendering info
	D3DDISPLAYMODE d3ddm;//current display mode info
	HRESULT r = 0;

	if (*ppDevice)
		(*ppDevice)->Release();

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	r = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(r)) {
		SetError(TEXT("Could not get display adapter information"));
		return E_FAIL;
	}

	//bWindowed = !bWindowed;

	width = d3ddm.Width;
	height = d3ddm.Height;

	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = bWindowed ? d3ddm.Format : FullScreenFormat;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = hWndTarget;
	d3dpp.Windowed = bWindowed;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz = 0;//default refresh rate
	d3dpp.PresentationInterval = bWindowed ? 0 : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	r = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWndTarget, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, ppDevice);
	if (FAILED(r)) {
		SetError(TEXT("Could not create the render device"));
		return E_FAIL;
	}

	// Turn on the zbuffer
	(*ppDevice)->SetRenderState(D3DRS_ZENABLE, TRUE);

	// Turn on ambient lighting 
	(*ppDevice)->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	return S_OK;
}

/*
 The default constructor for a Game object, initializes its member variables.
 */
Game::Game() :pD3D(0), pDevice(0), backSurface(0), bmpSurface(0), frame(FrameTracker()), fps(0) {}

/*
A constructor for a Game object that stores the hWnd, initializes its member variables.

@param newHwnd - The handle to the window that created the game object.
*/
Game::Game(HWND newHwnd) :hWnd(newHwnd), pD3D(0), pDevice(0), backSurface(0), bmpSurface(0), frame(FrameTracker()), fps(0) {}

/*
 A setter for the hWnd field of the Game class.
 
 @param newHwnd - The new hWnd value to be set.
 */
void Game::SetHWND(HWND newHwnd) {
	hWnd = newHwnd;
}

/*
A getter for the hWnd field of the Game class.

@return - Returns the hWnd being used by the Game object
*/
HWND Game::GetHWND() {
	return hWnd;
}

/*
 A dummy function that is used to allow the Window Class to use an non-static method
 as it's WndProc. It does so by getting the class long which points to a game instance,
 and then calls its WndProc, passing in the parameters it received.

 @param paramHWND - A handle to the window this is the WndProc of
 @param uMessage - The message that needs to be processed
 @param wParam - Additional message information
 @param lParam - Additional message information
 
 @return Returns the result of the message processing and depends on the message sent
 */
long CALLBACK Game::StaticProc(HWND paramHWND, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	Game* gPtr;

	gPtr = (Game*)GetClassLongPtr(paramHWND, 0);

	if (gPtr) {
		return gPtr->WndProc(paramHWND, uMessage, wParam, lParam);
	}
	else 
	{
		return DefWindowProc(paramHWND, uMessage, wParam, lParam);
	}
}

/*
The WndProc processes messages sent to it by the OS.

@param paramHWND - A handle to the window this is the WndProc of
@param uMessage - The message that needs to be processed
@param wParam - Additional message information
@param lParam - Additional message information

@return Returns the result of the message processing and depends on the message sent
*/
long CALLBACK Game::WndProc(HWND paramHWND, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	POINT curPos;

	switch (uMessage) {
		case WM_CREATE:
		{
			return 0;
		}
		case WM_PAINT:
		{
			ValidateRect(hWnd, NULL);//basically saying - yeah we took care of any paint msg without any overhead
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			GetCursorPos(&startPos);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			GetCursorPos(&startPos);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			GetCursorPos(&startPos);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			if (wParam == MK_LBUTTON) {
				GetCursorPos(&curPos);
				models[selectedModel].translate((curPos.x - startPos.x) / 200.0f, -(curPos.y - startPos.y) / 200.0f, 0);
				startPos = curPos;
			}
			if (wParam == MK_RBUTTON) {
				GetCursorPos(&curPos);
				models[selectedModel].rotateAboutX(-(curPos.y - startPos.y) / 100.0f);
				models[selectedModel].rotateAboutY(-(curPos.x - startPos.x) / 100.0f);
				startPos = curPos;
			}
			if (wParam == MK_MBUTTON) {
				GetCursorPos(&curPos);
				models[selectedModel].rotateAboutZ(-(curPos.x - startPos.x) / 1000.0f);
				models[selectedModel].translate(0.0f, 0.0f, (curPos.y - startPos.y) / 200.0f);

			}
			return 0;
		}
		case WM_KEYDOWN:
			if (wParam == 0x31) {
				selectedModel = 0;
			}
			if (wParam == 0x32) {
				selectedModel = 1;
			}
			if (wParam == 0x33) {
				lightsOn[0] = !lightsOn[0];
				if (lightsOn[0])
				{
					// Turn on ambient lighting 
					pDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);
				}
				else 
				{
					// Turn off ambient lighting 
					pDevice->SetRenderState(D3DRS_AMBIENT, 0x00000000);
				}
			}
			if (wParam == 0x34) {
				lightsOn[1] = !lightsOn[1];

				// Turn on/off directional light
				pDevice->LightEnable(0, lightsOn[1]);
			}
			if (wParam == 0x35) {
				lightsOn[2] = !lightsOn[2];

				// Turn on/off directional light
				pDevice->LightEnable(1, lightsOn[2]);
			}
			if (wParam == 0x36) {
				lightsOn[3] = !lightsOn[3];

				// Turn on/off directional light
				pDevice->LightEnable(2, lightsOn[3]);
			}
			return 0;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}
}

/*
 Initializes the directX surfaces, device, and various components used to
 display the game.

 @return - Returns an int to be used as an HRESULT in the FAILED() macro.
 Fails if:
 - The COM object failed creation
 - The directX device could not be initialized
 - Getting the back buffer or creating a surface of its size fails
 - Loading the bitmap or scaling it into the stored surface fails
*/
int Game::GameInit() {
	HRESULT r = 0;//return values
	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 pSurface = 0;

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);//COM object
	if (pD3D == NULL) {
		SetError(TEXT("Could not create IDirect3D9 object"));
		return E_FAIL;
	}

	r = InitDirect3DDevice(hWnd, FALSE, D3DFMT_X8R8G8B8, pD3D, &pDevice);
	if (FAILED(r)) {//FAILED is a macro that returns false if return value is a failure - safer than using value itself
		SetError(TEXT("Initialization of the device failed"));
		return E_FAIL;
	}

	D3DXCreateFont(pDevice, 50, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Ariel"), &font);

	r = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
	if (FAILED(r)) {
		SetError(TEXT("Could not get back buffer"));
	}

	pSurface->GetDesc(&desc);

	r = pDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &bmpSurface, NULL);
	if (FAILED(r)) {
		SetError(TEXT("Could not create bmpSurface"));
	}

	r = LoadBitmapToSurface(TEXT(BMP_PATH), &pSurface, pDevice);
	if (FAILED(r)) {
		SetError(TEXT("Could not load baboon.bmp"));
	}

	r = D3DXLoadSurfaceFromSurface(bmpSurface, NULL, NULL, pSurface, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
	if (FAILED(r)) {
		SetError(TEXT("Could not create surface"));
	}

	frame.initTracker();
	frame.startReset();

	cam = Camera(Camera::CameraType::AIRCRAFT);

	models[0] = Object(&pDevice, TEXT("Dwarf.x"));
	models[0].InitGeometry();

	models[1] = Object(&pDevice, TEXT("tiger.x"));
	models[1].InitGeometry();

	selectedModel = 0;

	createLights();

	lightsOn[1] = false;
	lightsOn[2] = false;
	lightsOn[3] = false;

	return S_OK;
}

/*
Releases the resources used by the game, first the display adapter, and then the COM object.

@return - Returns an int to be used as an HRESULT in the FAILED() macro. Should never fail.
*/
int Game::GameShutdown() {
	for (int i = 0; i < 2; i++) {
		models[i].cleanup();
	}

	if (pDevice)
		pDevice->Release();

	if (pD3D)
		pD3D->Release();

	return S_OK;
}

/*
Renders the next frame on a the back buffer surface with the background, fps counter,
and user-drawn line. After rendering it uses the present method to show the frame.

@return - Returns an int to be used as an HRESULT in the FAILED() macro.
		  Fails if:
			- A directX device has not yet been created
*/
int Game::Render() {
	HRESULT r;
	D3DLOCKED_RECT LockedRect;//locked area of display memory(buffer really) we are drawing to
	LPDIRECT3DSURFACE9 pBackSurf = 0;
	TCHAR text[200];
	RECT rect;
	TEXTMETRIC fontMetrics;

	//models[0].translate(0.01f, 0.0f, 0.0f);
	//models[0].rotateAboutZ(1.0f);

	font->GetTextMetrics(&fontMetrics);
	UINT charWidth = fontMetrics.tmAveCharWidth;
	_stprintf_s(text, 200, TEXT("FPS: %d"), fps);

	rect.top = 0;
	rect.left = width - charWidth * (wcslen(text) + 2);
	rect.bottom = 25;
	rect.right = width;

	if (!pDevice) {
		SetError(TEXT("Cannot render because there is no device"));
		return E_FAIL;
	}

	//clear the display arera with colour black, ignore stencil buffer
	pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 25), 1.0f, 0);

	//get pointer to backbuffer
	r = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurf);
	if (FAILED(r)) {
		SetError(TEXT("Couldn't get backbuffer"));
	}

	r = pDevice->UpdateSurface(bmpSurface, NULL, pBackSurf, NULL);
	if (FAILED(r)) {
		SetError(TEXT("Error updating back surface with bmp"));
	}

	//get a lock on the surface
	r = pBackSurf->LockRect(&LockedRect, NULL, 0);
	if (FAILED(r)) {
		SetError(TEXT("Could not lock the back buffer"));
	}


	pDevice->BeginScene();

	float curTime = timeGetTime();

	updateCam((curTime - lastTime) / 1000);

	lastTime = curTime;

	D3DXMATRIX camView;

	cam.getViewMatrix(&camView);

	for (int i = 0; i < 2; i++) {
		models[i].setupMatrices(camView);
		models[i].drawObject();
	}

	DWORD* pData = (DWORD*)(LockedRect.pBits);
	//DRAW CODE GOES HERE - use pData
	
	r = font->DrawText(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, 0xFF000000);
	if (FAILED(r)) {
		SetError(TEXT("Could not draw fps counter"));
	}

	pDevice->EndScene();

	pBackSurf->UnlockRect();
	pData = 0;

	pBackSurf->Release();//release lock

	pBackSurf = 0;

	pDevice->Present(NULL, NULL, NULL, NULL);//swap over buffer to primary surface
	return S_OK;
}

/*
The game loop updates the frame counter, and the fps if a second has passed, then calls
render to update the screen.

@return - Returns an int to be used as an HRESULT in the FAILED() macro.
*/
int Game::GameLoop() {
	frame.incCount();

	if (frame.secondPassed()) {
		fps = frame.getFPS();
		frame.startReset();
	}

	this->Render();

	if (GetAsyncKeyState(VK_ESCAPE))
		PostQuitMessage(0);

	return S_OK;
}

/*
Loads the bitmap specified by bmpPath to the target surface. The bitmap is loaded based on
its own dimensions. (i.e. not yet scaled to fit the screen)

@param bmpPath - The file path of the bitmap to be loaded
@param target - A pointer to the surface that will the bitmap will be loaded to
@param pDevice - The directX device being used to create surfaces

@return - Returns an int to be used as an HRESULT in the FAILED() macro.
		  Fails if:
			- The bitmap could not be loaded
			- A surface could not be created based on the bitmap
			- The surface could not hace the bitmap loaded onto it
*/
int Game::LoadBitmapToSurface(LPCTSTR bmpPath, LPDIRECT3DSURFACE9* target, LPDIRECT3DDEVICE9 pDevice) {
	HRESULT r = 0;//return results
	HBITMAP hBitmap;//handle to bmp
	BITMAP bitmap;//Actual bmp

	hBitmap = (HBITMAP)LoadImage(NULL, bmpPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hBitmap) {
		SetError(TEXT("Could not load BMP."));
		return E_FAIL;
	}

	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	DeleteObject(hBitmap);

	r = pDevice->CreateOffscreenPlainSurface(bitmap.bmWidth, bitmap.bmHeight, D3DFMT_X8R8G8B8, D3DPOOL_SCRATCH, target, NULL);
	if (FAILED(r)) {
		SetError(TEXT("Could not create surface with bitmaps dimensions."));
		return E_FAIL;
	}

	r = D3DXLoadSurfaceFromFile(*target, NULL, NULL, bmpPath, NULL, D3DX_DEFAULT, 0, NULL);
	if (FAILED(r)) {
		SetError(TEXT("Could not load BMP to surface."));
		return E_FAIL;
	}

	return S_OK;
}

void Game::createLights() {
	ZeroMemory(&lights[0], sizeof(D3DLIGHT9));
	lights[0].Type = D3DLIGHT_DIRECTIONAL;
	lights[0].Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].Direction = D3DXVECTOR3(-100.0f, -100.0f, 0.0f);
	lights[0].Position = D3DXVECTOR3(100.0f, 100.0f, 0.0f);
	lights[0].Range = 100.0f;
	pDevice->SetLight(0, &(lights[0]));
	pDevice->LightEnable(0, false);

	ZeroMemory(&lights[1], sizeof(D3DLIGHT9));
	lights[1].Type = D3DLIGHT_POINT;
	lights[1].Diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	lights[1].Position = D3DXVECTOR3(-5.0f, -5.0f, -5.0f);
	lights[1].Range = 100.0f;
	lights[1].Attenuation0 = 0.0f;
	lights[1].Attenuation1 = 0.125f;
	lights[1].Attenuation2 = 0.0f;
	lights[1].Falloff = 1.0f;
	pDevice->SetLight(1, &(lights[1]));
	pDevice->LightEnable(1, false);

	ZeroMemory(&lights[2], sizeof(D3DLIGHT9));
	lights[2].Type = D3DLIGHT_POINT;
	lights[2].Diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	lights[2].Direction = D3DXVECTOR3(-12.0f, 0.0f, 30.0f);
	lights[2].Position = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	lights[2].Range = 100.0f;
	lights[2].Attenuation0 = 0.0f;
	lights[2].Attenuation1 = 0.125f;
	lights[2].Attenuation2 = 0.0f;
	lights[2].Falloff = 1.0f;
	lights[2].Phi = D3DXToRadian(40.0f);    // set the outer cone to 30 degrees
	lights[2].Theta = D3DXToRadian(20.0f);    // set the inner cone to 10 degrees
	pDevice->SetLight(2, &(lights[2]));
	pDevice->LightEnable(2, false);

	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
}

void Game::updateCam(float timeDelta) {
	if (::GetAsyncKeyState('W') & 0x8000f)
		cam.walk(1.0f * timeDelta);
	if (::GetAsyncKeyState('S') & 0x8000f)
		cam.walk(-1.0f * timeDelta);
	if (::GetAsyncKeyState('A') & 0x8000f)
		cam.strafe(-1.0f * timeDelta);
	if (::GetAsyncKeyState('D') & 0x8000f)
		cam.strafe(1.0f * timeDelta);
	if (::GetAsyncKeyState('R') & 0x8000f)
		cam.fly(1.0f * timeDelta);
	if (::GetAsyncKeyState('F') & 0x8000f)
		cam.fly(-1.0f * timeDelta);
	if (::GetAsyncKeyState(VK_UP) & 0x8000f)
		cam.pitch(0.5f * timeDelta);
	if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
		cam.pitch(-0.5f * timeDelta);
	if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
		cam.yaw(-0.5f * timeDelta);
	if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
		cam.yaw(0.5f * timeDelta);
	if (::GetAsyncKeyState('N') & 0x8000f)
		cam.roll(0.5f * timeDelta);
	if (::GetAsyncKeyState('M') & 0x8000f)
		cam.roll(-0.5f * timeDelta);
}