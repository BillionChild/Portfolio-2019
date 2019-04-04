#include "../_Include/SystemH.h"
#include "../_Include/SystemDef.h"
#include "Window.h"
#include "../GameSystem/GameMain.h"


Window::Window(DWORD style) : winStyle(style)
{
	strAppName = L"DXFramework";
	gameSystem = nullptr;
}

Window::~Window()
{
	SAFE_DELETE(gameSystem);
}

int Window::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wcex.hIconSm = wcex.hIcon;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = strAppName.c_str();

	return RegisterClassExW(&wcex);
}

void Window::ResolutionAndPositionSetting(int& posX, int& posY)
{
	// FULL_SCREEN 변수 값에 따라 화면을 설정합니다.
	if (FULL_SCREEN)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= (UINT)resolution.right;
		dmScreenSettings.dmPelsHeight	= (UINT)resolution.bottom;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}
	else
	{
		resolution.right = GAME_WIDTH;
		resolution.bottom = GAME_HEIGHT;

		posX = (GetSystemMetrics(SM_CXSCREEN) - resolution.right) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - resolution.bottom) / 2;
	}

	// Device class에서 사용할 설정 내용 
	{
		DeviceDesc desc;
		desc.appName = strAppName;
		desc.isFullScreen = FULL_SCREEN;
		desc.isVSync = VSYNC_ENABLED;
		desc.width = static_cast<float>(resolution.right);
		desc.height = static_cast<float>(resolution.bottom);

		Device::SetDesc(desc);
	}
}

bool Window::Initialize()
{
	// Window Pointer Get
	appInstanceHandle = this;

	// Instance Handle Get
	hInstance = GetModuleHandle(NULL);

	// Window Class Registration
	if (!MyRegisterClass(hInstance))
		return false;

	int posX = 0;
	int posY = 0;

	ResolutionAndPositionSetting(posX, posY);

	// Window Create
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, strAppName.c_str(), strAppName.c_str(), winStyle,
		posX, posY, resolution.right, resolution.bottom, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
		return false;

	//Window Size Fix
	RECT rect = { 0, 0, resolution.right, resolution.bottom };


	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	MoveWindow(hWnd, posX, posY, rect.right - rect.left,
		rect.bottom - rect.top, TRUE);

	// GameMain Initialize
	{
		DeviceDesc desc;
		Device::GetDesc(&desc);
		desc.hWnd = hWnd;
		desc.hInst = hInstance;
		Device::SetDesc(desc);
	}

	gameSystem = new GameMain;
	assert(gameSystem != nullptr);

	gameSystem->Init(hWnd);

	// Show Window
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	return true;
}

void Window::Release()
{
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// 
	gameSystem->Release();
	SAFE_DELETE(gameSystem);
	
	// 창을 제거합니다
	DestroyWindow(hWnd);
	hWnd = NULL;

	// 프로그램 인스턴스를 제거합니다
	UnregisterClass(strAppName.c_str(), hInstance);
	hInstance = NULL;

	// 외부포인터 참조를 초기화합니다
	appInstanceHandle = nullptr;
}

void Window::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			gameSystem->Run(hWnd);
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (appInstanceHandle && appInstanceHandle->GetGameSystem())
		appInstanceHandle->GetGameSystem()->MsgHandler(hWnd, message, wParam, lParam);

	switch (message)
	{
		PAINTSTRUCT ps;
		HDC			hdc;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}