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
	void CalculateFrameStats();		// ��� FPS ���

public:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_MainhWnd;	// �� â �ڵ�
	bool m_AppPaused;	// ���� ���α׷��� �Ͻ� ������ �����ΰ�?
	bool m_Minimized;	// ���� ���α׷��� �ּ�ȭ�� �����ΰ�?
	bool m_Maximized;	// ���� ���α׷��� �ִ�ȭ�� �����ΰ�?
	bool m_Resizing;	// ����ڰ� ũ�� ������ �׵θ��� ���� �ִ� �����ΰ�?

	std::wstring m_MainWndCaption;	// â�� ����

	int m_ScreenWidth;
	int m_ScreenHeight;

private:
	// ��� �ð��� ���� ��ü �ð��� �����ϴ� �� ���δ�.
	GameTimer* m_pGameTimer;

	// DirectX 11 Graphics Engine
	HGEngine* m_pGEngine;
};