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
int Game::initDirect3DDevice(HWND hWndTarget, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice) {
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

	bWindowed = !bWindowed;

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
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
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
void Game::setHWND(HWND newHwnd) {
	hWnd = newHwnd;
}

/*
A getter for the hWnd field of the Game class.

@return - Returns the hWnd being used by the Game object
*/
HWND Game::getHWND() {
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
long CALLBACK Game::staticProc(HWND paramHWND, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	Game* gPtr;

	gPtr = (Game*)GetClassLongPtr(paramHWND, 0);

	if (gPtr) {
		return gPtr->wndProc(paramHWND, uMessage, wParam, lParam);
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
long CALLBACK Game::wndProc(HWND paramHWND, UINT uMessage, WPARAM wParam, LPARAM lParam) {
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
		case WM_LBUTTONDOWN:  // *** PICKING ***
		{
			GetCursorPos(&startPos);

			// compute the ray in view space given the clicked screen point
			Ray ray = calcPickingRay(LOWORD(lParam), HIWORD(lParam));

			// transform the ray to world space
			D3DXMATRIX view;
			pDevice->GetTransform(D3DTS_VIEW, &view);

			D3DXMATRIX viewInverse;
			D3DXMatrixInverse(&viewInverse, 0, &view);

			transformRay(&ray, &viewInverse);
			//selectedModel = 0;
			
			int i = 0;
			for (Object obj: models) {
				models[i]._radius = 1.0;
				models[i]._center.x = models[i].worldMatrix._41; //x 
				models[i]._center.y = models[i].worldMatrix._42 +1; //y
				models[i]._center.z = models[i].worldMatrix._43; //z
				float p41 = models[i].worldMatrix._41;
				float p42 = models[i].worldMatrix._42;
				float p43 = models[i].worldMatrix._43;
				float p44 = models[i].worldMatrix._44;
				// test for a hit
				if (raySphereIntersectionTest(&ray, &models[i])) {
					
					/*if (i == 0) {
						::MessageBox(0, TEXT("Hit Dwarf!"), TEXT("HIT"), 0);
						ss << "HIT Dwarf obj:  " << i << endl;
					}
					else {
						::MessageBox(0, TEXT("Hit Tiger!"), TEXT("HIT"), 0);
						ss << "HIT Tiger obj:  " << i << endl;
					}
					ss << "test matrix obj: "  << i << "\n" << p41 << " , " << p42 << " , " << p43 << " , " << p44 << endl;
					OutputDebugStringW(ss.str().c_str()); */
					selectedModel = i;
				}
				i++;
			}

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
				models[selectedModel].setXrotate((-(curPos.y - startPos.y) / 100.0f));  //Set x theta
				//models[selectedModel].setXrotate(-(curPos.y - startPos.y) / 100.0f);
				models[selectedModel].rotateAboutY(-(curPos.x - startPos.x) / 100.0f);  
				models[selectedModel].setYrotate((-(curPos.x - startPos.x) / 100.0f));  //set Y theta.
				//models[selectedModel].setYrotate(-(curPos.x - startPos.x) / 100.0f);
				startPos = curPos;
			}
			if (wParam == MK_SHIFT) {
				GetCursorPos(&curPos);
				models[selectedModel].rotateAboutZ(-(curPos.x - startPos.x) / 10000.0f);
				models[selectedModel].zTheta = (-(curPos.x - startPos.x) / 10000.0f);
				models[selectedModel].setZrotate((-(curPos.x - startPos.x) / 10000.0f));  // set z theta.
				//models[selectedModel].setZ(-(curPos.x - startPos.x) / 1000.0f);
				models[selectedModel].translate(0.0f, 0.0f, (curPos.y - startPos.y) / 2000.0f);

			}
			return 0;
		}
		case WM_KEYDOWN:

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
int Game::gameInit() {
	HRESULT r = 0;//return values
	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 pSurface = 0;

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);//COM object
	if (pD3D == NULL) {
		SetError(TEXT("Could not create IDirect3D9 object"));
		return E_FAIL;
	}

	r = initDirect3DDevice(hWnd, FALSE, D3DFMT_X8R8G8B8, pD3D, &pDevice);
	if (FAILED(r)) {//FAILED is a macro that returns false if return value is a failure - safer than using value itself
		SetError(TEXT("Initialization of the device failed"));
		return E_FAIL;
	}

	setupMirror();
	setupParticles();

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

	r = loadBitmapToSurface(TEXT(BMP_PATH), &pSurface, pDevice);
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
	
	models[0] = Object(&pDevice, TEXT("airplane 2.x"));
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
int Game::gameShutdown() {
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
int Game::render() {
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

	//snow->update(0.001);

	//clear the display arera with colour black, ignore stencil buffer
	pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0, 0, 25), 1.0f, 0);

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


	//RESET BUTTON IS Q.  RESETS MODELS TO CENTER.
	if (::GetAsyncKeyState('Q') & 0x8000f) 
	{
		models[0] = Object(&pDevice, TEXT("airplane 2.x"));
		models[0].InitGeometry();
		models[1] = Object(&pDevice, TEXT("tiger.x"));
		models[1].InitGeometry();
	}


	pDevice->BeginScene();

	float curTime = timeGetTime();

	updateCam((curTime - lastTime) / 1000);

	lastTime = curTime;

	D3DXMATRIX camView;

	cam.getViewMatrix(&camView);

	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	pDevice->SetTransform(D3DTS_WORLD, &I);

	pDevice->SetStreamSource(0, VB, 0, sizeof(Vertex));
	pDevice->SetFVF(Vertex::FVF);


	//DRAW THE MIRROR
	pDevice->SetMaterial(&MirrorMtrl);
	pDevice->SetTexture(0, MirrorTex);
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);

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


	//RENDER THE MIRROR
	renderMirror();
	renderLeftMirror();
	renderRightMirror();
	renderBackMirror();

	if (::GetAsyncKeyState('P') & 0x8000f)
		snow->render();


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
int Game::gameLoop() {
	frame.incCount();

	if (frame.secondPassed()) {
		fps = frame.getFPS();
		frame.startReset();
	}

	this->render();

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
int Game::loadBitmapToSurface(LPCTSTR bmpPath, LPDIRECT3DSURFACE9* target, LPDIRECT3DDEVICE9 pDevice) {
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
		cam.pitch(1.0f * timeDelta);
	if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
		cam.pitch(-1.0f * timeDelta);
	if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
		cam.yaw(-1.0f * timeDelta);
	if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
		cam.yaw(1.0f * timeDelta);
	if (::GetAsyncKeyState('N') & 0x8000f)
		cam.roll(0.5f * timeDelta);
	if (::GetAsyncKeyState('M') & 0x8000f)
		cam.roll(-0.5f * timeDelta);
	//MAKE HER SNOW BY HOLDING P! :D
	if (::GetAsyncKeyState('P') & 0x8000f)
		startSnow(timeDelta);

}



//Compute a picking ray in "View Space".
Ray Game::calcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;
	D3DVIEWPORT9 vp;
	pDevice->GetViewport(&vp);
	D3DXMATRIX proj;
	pDevice->GetTransform(D3DTS_PROJECTION, &proj);
	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);
	Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);
	return ray;
}



//Transform our picking ray into "World Space" where the objects are.
void Game::transformRay(Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.  Transforms points.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);
	// transform the ray's direction, w = 0.  Transforms vectors.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);
	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}



//Returns true if the ray passed in intersects the sphere passed in.  Returns false if ray misses.
bool Game::raySphereIntersectionTest(Ray* ray, Object* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;
	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);
	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);
	// test for imaginary number
	if (discriminant < 0.0f)
		return false;
	discriminant = sqrtf(discriminant);
	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;
	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;
	return false;
}


void Game::renderMirror()
{

	if (cam.getPosition()->z > 0.0)
		return;

	//PART I -Enabling the stencil buffer. Set related render states.
	pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	//PART II -Render mirror to the stencil buffer.
	// disable writes to the depth and back buffers
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);

	// draw the mirror to the stencil buffer
	pDevice->SetStreamSource(0, VB, 0, sizeof(Vertex));
	pDevice->SetFVF(Vertex::FVF);
	pDevice->SetMaterial(&MirrorMtrl);
	pDevice->SetTexture(0, MirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	pDevice->SetTransform(D3DTS_WORLD, &I);
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);
	// re-enable depth writes
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);


	//PART III -Render only the pixels that should appear in the mirror.
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	wostringstream ss;

	ss << "Airplane z:  " << (int)models[0].getZ() << endl;

	if (models[0].getZ() > 2.5f)  //airplane
	{
		ss << "Airplane z:  " << (int)models[0].getZ() << endl;


		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 2.5f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[0].getX(),
			models[0].getY(),
			models[0].getZ()
		);

		D3DXMatrixRotationX(&rotMatrixX, models[0].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[0].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[0].zTheta);
		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[0].getX,
			models[0].getY,
			models[0].getZ
		)*/

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[0].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[0].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[0].pMeshTextures[i]);

			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[0].pMesh->DrawSubset(i);
		}
		//pDevice->SetMaterial(&mirrorReflectMtrl);

	}


	ss << "Tiger z:  " << (int)models[1].getZ() << endl;

	if (models[1].getZ() > 2.5f)  //tiger
	{
		ss << "Tiger z:  " << (int)models[1].getZ() << endl;
		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 2.5f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//mirrorReflectMtrl = models[1].pMeshMaterials;
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		); */


		D3DXMatrixRotationX(&rotMatrixX, models[1].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[1].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[1].zTheta);

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[1].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[1].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[1].pMeshTextures[i]);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[1].pMesh->DrawSubset(i);
		}
	}

	/*else {
		//PART IV -Computes the matrix that postitions the reflection in the scene
	    // position reflection
		D3DXMATRIX W, T, R, rotationMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
		//D3DXMatrixReflect(&R, &plane);
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		//D3DXMatrixRotationY(&rotationMatrix, models[1].getYrotate());

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		W = T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[1].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[1].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[1].pMeshTextures[i]);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[1].pMesh->DrawSubset(i);
		}

	} */


	// RESTORE RENDER STATES.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
} // end RenderMirror() 



void Game::renderLeftMirror()
{
	if (cam.getPosition()->z > 2.5)
		return;

	//PART I -Enabling the stencil buffer. Set related render states.
	pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	//PART II -Render mirror to the stencil buffer.
	// disable writes to the depth and back buffers
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);

	// draw the mirror to the stencil buffer
	pDevice->SetStreamSource(0, VB, 0, sizeof(Vertex));
	pDevice->SetFVF(Vertex::FVF);
	pDevice->SetMaterial(&MirrorMtrl);
	pDevice->SetTexture(0, MirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	pDevice->SetTransform(D3DTS_WORLD, &I);
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);
	// re-enable depth writes
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);


	//PART III -Render only the pixels that should appear in the mirror.
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	wostringstream ss;

	ss << "Airplane z:  " << (int)models[0].getZ() << endl;

	if (models[0].getX() > -2.5f)  //airplane
	{
		ss << "Airplane z:  " << (int)models[0].getZ() << endl;


		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 2.5f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[0].getX(),
			models[0].getY(),
			models[0].getZ()
		);

		D3DXMatrixRotationX(&rotMatrixX, models[0].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[0].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[0].zTheta);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[0].getX,
			models[0].getY,
			models[0].getZ
		);*/

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[0].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[0].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[0].pMeshTextures[i]);

			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[0].pMesh->DrawSubset(i);
		}
		//pDevice->SetMaterial(&mirrorReflectMtrl);

	}


	ss << "Tiger z:  " << (int)models[1].getZ() << endl;

	if (models[1].getX() > -2.5f)  //tiger
	{
		ss << "Tiger z:  " << (int)models[1].getZ() << endl;
		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//mirrorReflectMtrl = models[1].pMeshMaterials;
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);*/


		D3DXMatrixRotationX(&rotMatrixX, models[1].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[1].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[1].zTheta);

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[1].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[1].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[1].pMeshTextures[i]);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[1].pMesh->DrawSubset(i);
		}
	}


	// RESTORE RENDER STATES.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}  // end RenderLeftMirror() 







void Game::renderRightMirror()
{

	if (cam.getPosition()->x < 2.5)
		return;

	//PART I -Enabling the stencil buffer. Set related render states.
	pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	//PART II -Render mirror to the stencil buffer.
	// disable writes to the depth and back buffers
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);

	// draw the mirror to the stencil buffer
	pDevice->SetStreamSource(0, VB, 0, sizeof(Vertex));
	pDevice->SetFVF(Vertex::FVF);
	pDevice->SetMaterial(&MirrorMtrl);
	pDevice->SetTexture(0, MirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	pDevice->SetTransform(D3DTS_WORLD, &I);
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 2);
	// re-enable depth writes
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);


	//PART III -Render only the pixels that should appear in the mirror.
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	wostringstream ss;

	ss << "Airplane z:  " << (int)models[0].getZ() << endl;

	if (models[0].getX() < 2.5f)  //airplane
	{
		ss << "Airplane z:  " << (int)models[0].getZ() << endl;


		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(2.5f, 0.0f, 0.0f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[0].getX(),
			models[0].getY(),
			models[0].getZ()
		);

		D3DXMatrixRotationX(&rotMatrixX, models[0].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[0].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[0].zTheta);
		
		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[0].getX,
			models[0].getY,
			models[0].getZ
		);*/

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[0].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[0].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[0].pMeshTextures[i]);

			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[0].pMesh->DrawSubset(i);
		}
		//pDevice->SetMaterial(&mirrorReflectMtrl);

	}


	ss << "Tiger z:  " << (int)models[1].getZ() << endl;

	if (models[1].getX() < 2.5f)  //tiger
	{
		ss << "Tiger z:  " << (int)models[1].getZ() << endl;
		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(2.5f, 0.0f, 0.0f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//mirrorReflectMtrl = models[1].pMeshMaterials;
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);*/


		D3DXMatrixRotationX(&rotMatrixX, models[1].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[1].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[1].zTheta);

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[1].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[1].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[1].pMeshTextures[i]);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[1].pMesh->DrawSubset(i);
		}
	}


	// RESTORE RENDER STATES.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

}  // end RenderRightMirror() 







void Game::renderBackMirror()
{

	if (cam.getPosition()->z > 0)
		return;

	//PART I -Enabling the stencil buffer. Set related render states.
	pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_STENCILREF, 0x1);
	pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	//PART II -Render mirror to the stencil buffer.
	// disable writes to the depth and back buffers
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);

	// draw the mirror to the stencil buffer
	pDevice->SetStreamSource(0, VB, 0, sizeof(Vertex));
	pDevice->SetFVF(Vertex::FVF);
	pDevice->SetMaterial(&MirrorMtrl);
	pDevice->SetTexture(0, MirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	pDevice->SetTransform(D3DTS_WORLD, &I);
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	// re-enable depth writes
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);


	//PART III -Render only the pixels that should appear in the mirror.
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	wostringstream ss;

	ss << "Airplane z:  " << (int)models[0].getZ() << endl;

	if (models[0].getZ() < 2.5f)  //airplane
	{
		ss << "Airplane z:  " << (int)models[0].getZ() << endl;


		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, -2.5f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[0].getX(),
			models[0].getY(),
			models[0].getZ()
		);

		D3DXMatrixRotationX(&rotMatrixX, models[0].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[0].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[0].zTheta);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[0].getX,
			models[0].getY,
			models[0].getZ
		);*/

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[0].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[0].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[0].pMeshTextures[i]);

			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[0].pMesh->DrawSubset(i);
		}
		//pDevice->SetMaterial(&mirrorReflectMtrl);

	}


	ss << "Tiger z:  " << (int)models[1].getZ() << endl;

	if (models[1].getZ() < -2.5f)  //tiger
	{
		ss << "Tiger z:  " << (int)models[1].getZ() << endl;
		//PART IV -Computes the matrix that postitions the reflection in the scene
		// position reflection
		D3DXMATRIX W, T, R, rotationMatrix, rotMatrixX, rotMatrixY, rotMatrixZ, scaleMatrix;
		D3DXPLANE plane(0.0f, 0.0f, -1.0f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);
		//mirrorReflectMtrl = models[1].pMeshMaterials;
		//for (int i = 0; i < sizeof(models) / sizeof(int); i++) {
		D3DXMatrixTranslation(&T,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		);

		/*D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			models[1].getX(),
			models[1].getY(),
			models[1].getZ()
		); */


		D3DXMatrixRotationX(&rotMatrixX, models[1].xTheta);
		D3DXMatrixRotationY(&rotMatrixY, models[1].yTheta);
		D3DXMatrixRotationZ(&rotMatrixZ, models[1].zTheta);

		W = rotMatrixX  *rotMatrixY * rotMatrixZ * T * R;
		//W = rotationMatrix * T * R;

		//PART V - clear the depth buffer to make reflection visible.
		pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		//Blend reflected objects with the mirror.
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		//Draw the reflected image.
		pDevice->SetTransform(D3DTS_WORLD, &W);
		for (DWORD i = 0; i < models[1].dwNumMaterials; i++)
		{
			pDevice->SetMaterial(&models[1].pMeshMaterials[i]);
			pDevice->SetTexture(0, models[1].pMeshTextures[i]);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			models[1].pMesh->DrawSubset(i);
		}
	}


	// RESTORE RENDER STATES.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

}  // end RenderBackMirror() 





//Make the game mirror.
void Game::setupMirror() {
	pDevice->CreateVertexBuffer(
		24 * sizeof(Vertex),
		0, // usage
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Vertex* v = 0;

	VB->Lock(0, 0, (void**)&v, 0);


	// mirror back face
	v[5] = Vertex(-2.5f, 0.0f, -2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[4] = Vertex(-2.5f, 5.0f, -2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[3] = Vertex(2.5f, 5.0f, -2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	v[2] = Vertex(-2.5f, 0.0f, -2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[1] = Vertex(2.5f, 5.0f, -2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[0] = Vertex(2.5f, 0.0f, -2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	// mirror right face
	v[11] = Vertex(2.5f, 5.0f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[10] = Vertex(2.5f, 5.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[9] = Vertex(2.5f, 0.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	v[8] = Vertex(2.5f, 0.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[7] = Vertex(2.5f, 0.0f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[6] = Vertex(2.5f, 5.0f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// mirror left face
	v[12] = Vertex(-2.5f, 5.0f, 2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[13] = Vertex(-2.5f, 5.0f, -2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[14] = Vertex(-2.5f, 0.0f, -2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	v[15] = Vertex(-2.5f, 0.0f, 2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[16] = Vertex(-2.5f, 0.0f, -2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[17] = Vertex(-2.5f, 5.0f, -2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	
	// mirror front face 
	v[18] = Vertex(-2.5f, 0.0f, 2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex(-2.5f, 5.0f, 2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex(2.5f, 5.0f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[21] = Vertex(-2.5f, 0.0f, 2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex(2.5f, 5.0f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[23] = Vertex(2.5f, 0.0f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	VB->Unlock();

	D3DXCreateTextureFromFile(pDevice, TEXT("ice.bmp"), &MirrorTex);

	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}





D3DMATERIAL9 Game::initMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;
	mtrl.Diffuse = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power = p;
	return mtrl;
}


void Game::startSnow(float timeDelta) {
	snow->update(timeDelta);
}



void Game::setupParticles()
{
	srand((unsigned int)123);

	ParticleSystem::BoundingBox boundingBox;
	boundingBox._min = D3DXVECTOR3(-10.0f, -10.0f, -10.0f);
	boundingBox._max = D3DXVECTOR3(10.0f, 10.0f, 10.0f);

	snow = new Snow(&boundingBox, 5000);
	snow->init(pDevice, "snowflake.dds");
}
