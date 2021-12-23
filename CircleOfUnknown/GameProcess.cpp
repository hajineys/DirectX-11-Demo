#include "GameProcess.h"
#include <windowsx.h>
#include "Resource.h"
#include "GameTimer.h"
#include "HGEngine.h"

namespace
{
	// 글로벌 창에서 Windows메시지를 전달하는 데만 사용됩니다.
	// WNDCLASS::lpfnWndProc에 멤버 함수를 할당할 수 없으므로
	// 멤버 함수 창 프로시저에 프로시저를 지정합니다.
	GameProcess* gameProcess = 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CreateWindow가 돌아오기 전에 메시지를 받을 수 있으므로 전달
	// 따라서 mhMainWnd가 유효하기 전에
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
	// 전달할 수 있도록 응용 프로그램 개체에 대한 포인터 가져오기
	// 개체의 창에 대한 Windows 메시지 프로시저
	// 글로벌 윈도우 절차
	gameProcess = this;
}

GameProcess::~GameProcess()
{
	
}

bool GameProcess::Initialize(HINSTANCE hInstance)
{
	// string 길이 설정
	const int max_loadstring = 100;

	WCHAR szTitle[max_loadstring];			// 제목 표시줄 텍스트입니다.
	WCHAR szWindowClass[max_loadstring];	// 기본 창 클래스 이름입니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, max_loadstring);
	LoadStringW(hInstance, IDC_CIRCLEOFUNKNOWN, szWindowClass, max_loadstring);

	// 창 클래스를 등록합니다.
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

	// 요청된 클라이언트 영역 치수를 기준으로 창 직사각형 치수를 계산합니다.
	RECT R = { 0, 0, m_ScreenWidth, m_ScreenHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	// 인스턴스 핸들을 저장하고 주 창을 만듭니다.
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

	// 게임 타이머 생성
	m_pGameTimer = new GameTimer();

	// 그래픽스 엔진 생성
 	m_pGEngine = new HGEngine(m_MainhWnd, m_ScreenWidth, m_ScreenHeight);

	// 그래픽스 엔진 초기화
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

	// 기본 메시지 루프입니다:
	while (msg.message != WM_QUIT)
	{
		// Windows 메세지가 있으면 처리한다.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// ESC 누르면 바로 종료
			if (GetAsyncKeyState(VK_ESCAPE))
			{
				SendMessage(m_MainhWnd, WM_DESTROY, 0, 0);
			}
		}
		// 없으면 애니메이션/게임 작업을 수행한다.
		else
		{
			// 매 프레임 시간을 계산한다.
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
	// 그래픽스 엔진 종료 후 메모리 해제
	m_pGEngine->Finalize();
	delete m_pGEngine;
 	m_pGEngine = nullptr;

	// 게임 타이머 메모리 해제
	delete m_pGameTimer;
	m_pGameTimer = nullptr;
}

void GameProcess::UpdateAll()
{
	// 그래픽스 엔진 업데이트
	m_pGEngine->Update(m_pGameTimer);
}

void GameProcess::RenderAll()
{
	m_pGEngine->BeginRender();	// 그리기를 준비한다.
	m_pGEngine->Render();		// 엔진만의 그리기를 한다.
	m_pGEngine->EndRender();	// 그리기를 끝낸다.
}

void GameProcess::CalculateFrameStats()
{
	// 이 메서드는 평균 FPS를 계산하며, 하나의 프레임을 렌더링하는 데
	// 걸리는 평균 시간도 계산한다. 
	// 또한 이 통계치들을 창의 제목줄에 추가한다.
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// 1초 동안의 평균 프레임 수를 계산한다.
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

		// 다음 평균을 위해 다시 초기화한다.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

// 주 창의 메시지를 처리합니다.
// WM_DESTROY - 종료 메시지를 게시하고 반환합니다.
LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// WM_ACTIVATE는 창이 활성화되거나 비활성화될 때 전송됩니다.
		// 창이 비활성화되면 게임을 일시 중지하고 활성화되면 일시 중지를 해제합니다.
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

		// WM_SIZE는 사용자가 창 크기를 조정할 때 전달된다.
	case WM_SIZE:
		// 새 클라이언트 영역 치수 저장
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
				// 최소화된 상태에서 복원하려는가?
				if (m_Minimized)
				{
					m_AppPaused = false;
					m_Minimized = false;
					m_pGEngine->OnResize();
				}
				// 최대화된 상태에서 복원하려는가?
				else if (m_Maximized)
				{
					m_AppPaused = false;
					m_Maximized = false;
					m_pGEngine->OnResize();
				}
				else if (m_Resizing)
				{
					// 사용자가 크기 조정 막대를 끄는 경우 크기를 조정하지 않습니다.
					// 사용자가 크기 조정 막대를 계속 끌면 버퍼가 여기에 표시됩니다.
					// 일련의 WM_SIZE 메시지가 창으로 전송됩니다.
					// 크기 조정 막대를 끌어서 받은 각 WM_SIZE 메시지에 대해 크기를 조정하는 것은
					// 무의미하며 느립니다.
					// 대신 사용자가 창의 크기를 조정하고 크기 조정 막대를 해제한 후 재설정합니다.
					// WM_EXITSIZEMOVE 메시지를 보냅니다.
				}
				else // SetWindowPos 또는 mSwapChain->SetFullscreenState와 같은 API 호출.
				{
					m_pGEngine->OnResize();
				}
			}
		}
		return 0;

		// WM_ENTERSIZEMOVE는 사용자가 크기 변경 테두리를 잡으면 전달된다.
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		m_pGameTimer->Stop();
		return 0;

		// WM_EXITSIZEMOVE는 사용자가 크기 변경 테두리를 놓으면 전달된다.
		// 그러면 창의 새 크기에 맞게 모든 것을 재설정한다.
	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_pGameTimer->Start();
		m_pGEngine->OnResize();
		return 0;

		// WM_DESTROY는 창이 파괴되려 할 때 전달된다.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// WM_MENUCHAR 메세지는 메뉴가 활성화되어서 사용자가 키를 눌렀으나 그 키가
		// 그 어떤 니모닉이나 단축키에도 해당하지 않을 때 전달된다.
	case WM_MENUCHAR:
		// Alt-Enter를 눌렀을 때 삐 소리가 나지 않게 한다.
		return MAKELRESULT(0, MNC_CLOSE);

		// 이 메세지가 발생한 경우 창이 너무 작아지지 않도록 한다.
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