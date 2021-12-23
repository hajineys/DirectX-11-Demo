#pragma once
#include <windows.h>
#include <sstream>

class GameTimer;
class HGEngine;

/// <summary>
/// Game Process Class
/// 
/// 2021. 11. 16 Hamdal
/// </summary>
class GameProcess
{
public:
	GameProcess();
	~GameProcess();

public:
	bool Initialize(HINSTANCE hInstance);
	int Run();
	void Finalize();

private:
	void UpdateAll();
	void RenderAll();
	void CalculateFrameStats();		// 평균 FPS 계산

public:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_MainhWnd;	// 주 창 핸들
	bool m_AppPaused;	// 응용 프로그램이 일시 정지된 상태인가?
	bool m_Minimized;	// 응용 프로그램이 최소화된 상태인가?
	bool m_Maximized;	// 응용 프로그램이 최대화된 상태인가?
	bool m_Resizing;	// 사용자가 크기 조정용 테두리를 끌고 있는 상태인가?

	std::wstring m_MainWndCaption;	// 창의 제목

	int m_ScreenWidth;
	int m_ScreenHeight;

private:
	// 경과 시간과 게임 전체 시간을 측정하는 데 쓰인다.
	GameTimer* m_pGameTimer;

	// DirectX 11 Graphics Engine
	HGEngine* m_pGEngine;
};