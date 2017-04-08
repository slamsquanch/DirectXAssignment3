#define WIN32_LEAN_AND_MEAN

#include "Headers.h"

/*
 WinMain is the entry point for the Win32 API. Creates a custom window class and
 an instance of it to be the game's window. Finally, it starts the game loop for
 the game.
 
 @param hInstance - The handle to the instance of the executable
 @param hPrevInstance - Has no meaning, was used in 16-bit Windows
 @param pstrCmdLine - A string containing the command line arguments
 @param iCmdShow - a flag that says whether the main application window will be
				   minimized, maximized, or shown normally
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pstrCmdLine, int iCmdShow) {
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wc;
	Game newGame;

	static TCHAR strAppName[] = TEXT("First Windows App, Zen Style");

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.cbClsExtra = sizeof(Game*);
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = Game::StaticProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_HAND);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = strAppName;

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		strAppName,
		strAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512, 512,
		NULL,
		NULL,
		hInstance,
		NULL);

	newGame = Game(hWnd);

	SetClassLongPtr(hWnd, 0, (LONG)&newGame);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	if (FAILED(newGame.GameInit())) {
		SetError(TEXT("Initialization Failed"));
		newGame.GameShutdown();
		return E_FAIL;
	}


	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			newGame.GameLoop();
		}
	}
	newGame.GameShutdown();// clean up the game
	return msg.wParam;
}