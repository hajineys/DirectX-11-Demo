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
	// ���콺 �Է��� ó���� ���� �޼����
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
	UINT m_4xMsaaQuality;	// 4X MSAA�� ǰ�� ����
	bool m_Enable4xMsaa;	// �⺻���� false

	// ������ ����(�ϵ���� ��ġ �Ǵ� ǥ�� ��ġ ��)
	D3D_DRIVER_TYPE m_D3DDriverType;

	D3D_FEATURE_LEVEL m_FeatureLevel;

	ID3D11Device* m_pD3DDevice;						// DirectX 11 ����̽�
	ID3D11DeviceContext* m_pD3DImmediateContext;	// ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain;					// ���� ü��
	ID3D11Texture2D* m_pDepthStencilBuffer;			// ���� ���Ľ� ����
	ID3D11RenderTargetView* m_pRenderTargetView;	// ���� Ÿ�� ��
	ID3D11DepthStencilView* m_pDepthStencilView;	// ���� ���Ľ� ��
	D3D11_VIEWPORT m_ScreenViewport;				// ����Ʈ

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