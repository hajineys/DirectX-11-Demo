#pragma once
#include "DirectXDefine.h"

class GameTimer;
class DXCamera;
class DXTKFont;

class Box;
class Hills;
class Shapes;
class Skull;
class Waves;
class Lighting;
class LightingSkull;
class TexturedHillsAndWaves;
class Crate;
class Blend;
class FireAnim;

/// <summary>
/// DirectX 11 Graphics Engine
/// 
/// 2021. 11. 17 Hamdal
/// </summary>
class HGEngine
{
public:
	HGEngine(HWND hWnd, int clientWidth, int clientHeight);
	~HGEngine();

public:
	bool Initialize();
	void OnResize();
	void Update(GameTimer* pTimer);
	void BeginRender();
	void Render();
	void EndRender();
	void Finalize();

public:
	// 마우스 입력의 처리를 위한 메서드들
	void OnMouseDown(int x, int y);
	void OnMouseUp(int x, int y);
	void OnMouseMove(int btnState, int x, int y);

private:
	float GetAspectRatio() const;
	HRESULT GetAdapterInfo();
	void UpdateCamera(float deltaTime);
	void DrawInfo();

	// Getter / Setter
public:
	ID3D11Device* GetD3DDevice() const { return m_pD3DDevice; }
	void SetClientWidth(int width) { m_ClientWidth = width; }
	void SetClientHeight(int height) { m_ClientHeight = height; }

private:
	HWND m_EnginehWnd;
	int m_ClientWidth;
	int m_ClientHeight;
	UINT m_4xMsaaQuality;	// 4X MSAA의 품질 수준
	bool m_Enable4xMsaa;	// 기본값은 false

	// 구동기 종류(하드웨어 장치 또는 표준 장치 등)
	D3D_DRIVER_TYPE m_D3DDriverType;

	D3D_FEATURE_LEVEL m_FeatureLevel;

	ID3D11Device* m_pD3DDevice;						// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;	// 디바이스 컨텍스트
	IDXGISwapChain* m_pSwapChain;					// 스왑 체인
	ID3D11Texture2D* m_pDepthStencilBuffer;			// 뎁스 스탠실 버퍼
	ID3D11RenderTargetView* m_pRenderTargetView;	// 랜더 타겟 뷰
	ID3D11DepthStencilView* m_pDepthStencilView;	// 뎁스 스탠실 뷰
	D3D11_VIEWPORT m_ScreenViewport;				// 뷰포트

	DXGI_ADAPTER_DESC1 m_AdapterDesc1;

	POINT m_LastMousePos;

private:
	DXCamera* m_pCamera;
	DXTKFont* m_pFont;

	Box* m_pBox;
	Hills* m_pHills;
	Shapes* m_pShapes;
	Skull* m_pSkull;
	Waves* m_pWaves;
	Lighting* m_pLighting;
	LightingSkull* m_pLightingSkull;
	TexturedHillsAndWaves* m_pTexturedHillsAndWaves;
	Crate* m_pCrate;
	Blend* m_pBlend;

	FireAnim* m_pFireAnim;
	float m_FireAnimSpeed;
};