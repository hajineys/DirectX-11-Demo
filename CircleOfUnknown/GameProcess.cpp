#include "GameProcess.h"
#include <windowsx.h>
#include "Resource.h"
#include "GameTimer.h"
#include "HGEngine.h"

namespace
{
	// �۷ι� â���� Windows�޽����� �����ϴ� ���� ���˴ϴ�.
	// WNDCLASS::lpfnWndProc�� ��� �Լ��� �Ҵ��� �� �����Ƿ�
	// ��� �Լ� â ���ν����� ���ν����� �����մϴ�.
	GameProcess* gameProcess = 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CreateWindow�� ���ƿ��� ���� �޽����� ���� �� �����Ƿ� ����
	// ���� mhMainWnd�� ��ȿ�ϱ� ����
	return gameProcess->WndProc(hWnd, message, wParam, lParam);
}

GameProcess::GameProcess()
	: m_MainhWnd(nullptr),
	m_AppPaused(false),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_MainWndCaption(L"DirectX 11 Graphics Engine by Hamdal"),
	m_ScreenWidth(1920),
	m_ScreenHeight(1080),
	m_pGameTimer(nullptr),
	m_pGEngine(nullptr)
{
	// ������ �� �ֵ��� ���� ���α׷� ��ü�� ���� ������ ��������
	// ��ü�� â�� ���� Windows �޽��� ���ν���
	// �۷ι� ������ ����
	gameProcess = this;
}

GameProcess::~GameProcess()
{
	
}

bool GameProcess::Initialize(HINSTANCE hInstance)
{
	// string ���� ����
	const int max_loadstring = 100;

	WCHAR szTitle[max_loadstring];			// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
	WCHAR szWindowClass[max_loadstring];	// �⺻ â Ŭ���� �̸��Դϴ�.

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, max_loadstring);
	LoadStringW(hInstance, IDC_CIRCLEOFUNKNOWN, szWindowClass, max_loadstring);

	// â Ŭ������ ����մϴ�.
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CIRCLEOFUNKNOWN));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = /*MAKEINTRESOURCEW(IDC_CIRCLEOFUNKNOWN)*/NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassExW(&wcex))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// ��û�� Ŭ���̾�Ʈ ���� ġ���� �������� â ���簢�� ġ���� ����մϴ�.
	RECT R = { 0, 0, m_ScreenWidth, m_ScreenHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	// �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
	m_MainhWnd = CreateWindowW(szWindowClass, m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		nullptr, nullptr, hInstance, NULL);

	if (!m_MainhWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_MainhWnd, SW_SHOW);
	UpdateWindow(m_MainhWnd);

	// ���� Ÿ�̸� ����
	m_pGameTimer = new GameTimer();

	// �׷��Ƚ� ���� ����
 	m_pGEngine = new HGEngine(m_MainhWnd, m_ScreenWidth, m_ScreenHeight);

	// �׷��Ƚ� ���� �ʱ�ȭ
	if (m_pGEngine->Initialize() == false)
	{
		MessageBox(0, L"DirectX 11 Graphics Engine Initialize Failed.", 0, 0);
		return false;
 	}

	return true;
}

int GameProcess::Run()
{
	MSG msg = { 0 };

	m_pGameTimer->Reset();

	// �⺻ �޽��� �����Դϴ�:
	while (msg.message != WM_QUIT)
	{
		// Windows �޼����� ������ ó���Ѵ�.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// ESC ������ �ٷ� ����
			if (GetAsyncKeyState(VK_ESCAPE))
			{
				SendMessage(m_MainhWnd, WM_DESTROY, 0, 0);
			}
		}
		// ������ �ִϸ��̼�/���� �۾��� �����Ѵ�.
		else
		{
			// �� ������ �ð��� ����Ѵ�.
			m_pGameTimer->Tick();

			if (m_AppPaused == false)
			{
				CalculateFrameStats();
				UpdateAll();
				RenderAll();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void GameProcess::Finalize()
{
	// �׷��Ƚ� ���� ���� �� �޸� ����
	m_pGEngine->Finalize();
	delete m_pGEngine;
 	m_pGEngine = nullptr;

	// ���� Ÿ�̸� �޸� ����
	delete m_pGameTimer;
	m_pGameTimer = nullptr;
}

void GameProcess::UpdateAll()
{
	// �׷��Ƚ� ���� ������Ʈ
	m_pGEngine->Update(m_pGameTimer);
}

void GameProcess::RenderAll()
{
	m_pGEngine->BeginRender();	// �׸��⸦ �غ��Ѵ�.
	m_pGEngine->Render();		// �������� �׸��⸦ �Ѵ�.
	m_pGEngine->EndRender();	// �׸��⸦ ������.
}

void GameProcess::CalculateFrameStats()
{
	// �� �޼���� ��� FPS�� ����ϸ�, �ϳ��� �������� �������ϴ� ��
	// �ɸ��� ��� �ð��� ����Ѵ�. 
	// ���� �� ���ġ���� â�� �����ٿ� �߰��Ѵ�.
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// 1�� ������ ��� ������ ���� ����Ѵ�.
	if ((m_pGameTimer->TotalTime() - timeElapsed) >= 1.0f)
	{
		float _fps = (float)frameCnt;	// fps = _frameCnt / 1
		float _mspf = 1000.0f / _fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndCaption << L"   /   "
			<< L"FPS : " << _fps << L"   /   "
			<< L"Frame Time : " << _mspf << L"(ms)";
		SetWindowText(m_MainhWnd, outs.str().c_str());

		// ���� ����� ���� �ٽ� �ʱ�ȭ�Ѵ�.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

// �� â�� �޽����� ó���մϴ�.
// WM_DESTROY - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// WM_ACTIVATE�� â�� Ȱ��ȭ�ǰų� ��Ȱ��ȭ�� �� ���۵˴ϴ�.
		// â�� ��Ȱ��ȭ�Ǹ� ������ �Ͻ� �����ϰ� Ȱ��ȭ�Ǹ� �Ͻ� ������ �����մϴ�.
	case WM_ACTIVATE:
		if (m_pGameTimer != nullptr)
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_AppPaused = true;
				m_pGameTimer->Stop();
			}
			else
			{
				m_AppPaused = false;
				m_pGameTimer->Start();
			}
		}
		return 0;

		// WM_SIZE�� ����ڰ� â ũ�⸦ ������ �� ���޵ȴ�.
	case WM_SIZE:
		// �� Ŭ���̾�Ʈ ���� ġ�� ����
		m_ScreenWidth = LOWORD(lParam);
		m_ScreenHeight = HIWORD(lParam);

		if (m_pGEngine == nullptr)
		{
			break;
		}
		else
		{
			m_pGEngine->SetClientWidth(m_ScreenWidth);
			m_pGEngine->SetClientHeight(m_ScreenHeight);

			if (wParam == SIZE_MINIMIZED)
			{
				m_AppPaused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_AppPaused = false;
				m_Minimized = false;
				m_Maximized = true;
				m_pGEngine->OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// �ּ�ȭ�� ���¿��� �����Ϸ��°�?
				if (m_Minimized)
				{
					m_AppPaused = false;
					m_Minimized = false;
					m_pGEngine->OnResize();
				}
				// �ִ�ȭ�� ���¿��� �����Ϸ��°�?
				else if (m_Maximized)
				{
					m_AppPaused = false;
					m_Maximized = false;
					m_pGEngine->OnResize();
				}
				else if (m_Resizing)
				{
					// ����ڰ� ũ�� ���� ���븦 ���� ��� ũ�⸦ �������� �ʽ��ϴ�.
					// ����ڰ� ũ�� ���� ���븦 ��� ���� ���۰� ���⿡ ǥ�õ˴ϴ�.
					// �Ϸ��� WM_SIZE �޽����� â���� ���۵˴ϴ�.
					// ũ�� ���� ���븦 ��� ���� �� WM_SIZE �޽����� ���� ũ�⸦ �����ϴ� ����
					// ���ǹ��ϸ� �����ϴ�.
					// ��� ����ڰ� â�� ũ�⸦ �����ϰ� ũ�� ���� ���븦 ������ �� �缳���մϴ�.
					// WM_EXITSIZEMOVE �޽����� �����ϴ�.
				}
				else // SetWindowPos �Ǵ� mSwapChain->SetFullscreenState�� ���� API ȣ��.
				{
					m_pGEngine->OnResize();
				}
			}
		}
		return 0;

		// WM_ENTERSIZEMOVE�� ����ڰ� ũ�� ���� �׵θ��� ������ ���޵ȴ�.
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		m_pGameTimer->Stop();
		return 0;

		// WM_EXITSIZEMOVE�� ����ڰ� ũ�� ���� �׵θ��� ������ ���޵ȴ�.
		// �׷��� â�� �� ũ�⿡ �°� ��� ���� �缳���Ѵ�.
	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_pGameTimer->Start();
		m_pGEngine->OnResize();
		return 0;

		// WM_DESTROY�� â�� �ı��Ƿ� �� �� ���޵ȴ�.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// WM_MENUCHAR �޼����� �޴��� Ȱ��ȭ�Ǿ ����ڰ� Ű�� �������� �� Ű��
		// �� � �ϸ���̳� ����Ű���� �ش����� ���� �� ���޵ȴ�.
	case WM_MENUCHAR:
		// Alt-Enter�� ������ �� �� �Ҹ��� ���� �ʰ� �Ѵ�.
		return MAKELRESULT(0, MNC_CLOSE);

		// �� �޼����� �߻��� ��� â�� �ʹ� �۾����� �ʵ��� �Ѵ�.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		m_pGEngine->OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		m_pGEngine->OnMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		m_pGEngine->OnMouseMove((int)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}