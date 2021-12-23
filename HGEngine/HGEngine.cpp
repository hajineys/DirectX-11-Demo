#include "HGEngine.h"
#include "DXCamera.h"
#include "DXTKFont.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "GameTimer.h"
#include "MathHelper.h"
#include "SimpleMath.h"

// 3D Object
#include "Box.h"
#include "Hills.h"
#include "Shapes.h"
#include "Skull.h"
#include "Waves.h"
#include "Lighting.h"
#include "LightingSkull.h"
#include "TexturedHillsAndWaves.h"
#include "Crate.h"
#include "Blend.h"
#include "FireAnim.h"

HGEngine::HGEngine(HWND hWnd, int clientWidth, int clientHeight)
	: m_EnginehWnd(hWnd),
	m_ClientWidth(clientWidth), m_ClientHeight(clientHeight),
	m_4xMsaaQuality(0), m_Enable4xMsaa(true),
	m_D3DDriverType(D3D_DRIVER_TYPE_HARDWARE),	// 3차원 그래픽 가속이 적용되게 하기 위해 지정
	m_FeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_pD3DDevice(nullptr),
	m_pD3DImmediateContext(nullptr),
	m_pSwapChain(nullptr),
	m_pDepthStencilBuffer(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr),
	m_AdapterDesc1(),
	m_LastMousePos{ 0,0 },
	m_FireAnimSpeed(0.0f)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));

	// 카메라 생성
	m_pCamera = new DXCamera();
	m_pCamera->LookAt(XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 1.0f, 0));

	// DXTK Font 생성
	m_pFont = new DXTKFont();
}

HGEngine::~HGEngine()
{
	// 릴리즈/삭제는 할당/생성의 반대 순서로 한다
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilView);
	ReleaseCOM(m_pSwapChain);
	ReleaseCOM(m_pDepthStencilBuffer);

	// 모든 기본 설정을 복구한다.
	if (m_pD3DImmediateContext)
		m_pD3DImmediateContext->ClearState();

	ReleaseCOM(m_pD3DImmediateContext);
	ReleaseCOM(m_pD3DDevice);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool HGEngine::Initialize()
{
	UINT createDeviceFlags = 0;

	// 1. D3D11CreateDevice 함수를 이용해서 장치, 즉 ID3D11Device 인터페이스와 장치 문맥,
	// 즉 ID3D11DeviceContext 인터페이스를 생성한다.
	HRESULT hr = D3D11CreateDevice(
		0,					// 기본 어댑터
		m_D3DDriverType,
		0,					// 소프트웨어 장치를 사용하지 않음
		createDeviceFlags,
		NULL,				// 지원되는 최고 기능 수준 선택
		0,					// 기본 기능 수준 배열
		D3D11_SDK_VERSION,
		&m_pD3DDevice,
		&m_FeatureLevel,
		&m_pD3DImmediateContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (m_FeatureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

#if defined(DEBUG) || defined(_DEBUG)  
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	ID3D11Debug* d3dDebug;
	HRESULT hrQuery = m_pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hrQuery))
	{
		hrQuery = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}

	if (d3dDebug != nullptr) d3dDebug->Release();
#endif
	if (m_pD3DDevice != nullptr) m_pD3DDevice->Release();

	// 2. ID3D11Device::CheckMultisampleQualityLevels 메서드를 이용해서
	// 4X MSAA 품질 수준 지원 여부를 점검한다.
	HR(m_pD3DDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));

	// 4X MSAA가 항상 지원되므로, 반환된 품질 수준 값은 반드시 0보다 커야 한다.
	// 이를 강제하기 위해 assert를 사용했다.
	assert(m_4xMsaaQuality > 0);

	// 3. 생성할 교환 사슬의 특성을 서술하는 DXGI_SWAP_CHAIN_DESC 구조체를 채운다.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_ClientWidth;		// 창의 클라이언트 영역 크기를 사용한다.
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;	// 디스플레이 모드 갱신율
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// 후면 버퍼 픽셀 형식
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// 디스플레이 스캔라인 모드
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// 디스플레이 비례 모드

	// 4X MSAA를 사용하는가? 
	if (m_Enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;

		// m_4xMsaaQuality는 CheckMultisampleQualityLevels()로 얻은 것이다.
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// MSAA를 사용하지 않음.
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_EnginehWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// 4. 장치를 생성하는 데 사용했던 IDXFactory 인터페이스를 질의해서 IDXGISwapChain
	// 인스턴스를 생성한다.

	// D3디바이스로부터 dxgi디바이스를 얻어온다.
	IDXGIDevice* dxgiDevice = 0;
	HR(m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice),
		(void**)&dxgiDevice));

	// dxgi디바이스로부터 dxgi어댑터를 얻어온다.
	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter),
		(void**)&dxgiAdapter));

	// dxgi어댑터로부터 dxgi팩토리를 얻어온다.
	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
		(void**)&dxgiFactory));

	// 이제 교환 사슬을 생성한다.
	HR(dxgiFactory->CreateSwapChain(m_pD3DDevice, &sd, &m_pSwapChain));

	// 획득한 COM 인터페이스들을 해제한다(다 사용했으므로).
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// 창 크기가 조정될 때마다 실행되어야 한다.
	// 코드 중복을 방지하려면 여기서 OnResize 메서드를 호출해야한다.
	// 5. 교환 사슬의 후면 버퍼에 대한 렌더 대상 뷰를 생성한다.
	OnResize();

	// 어댑터 정보를 얻는다.
	GetAdapterInfo();

	m_pFont->Create(m_pD3DDevice, RenderStates::SolidRS, RenderStates::NormalDSS);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(m_pD3DDevice);
	InputLayouts::InitAll(m_pD3DDevice);
	RenderStates::InitAll(m_pD3DDevice);

	m_pBox = new Box(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);

	m_pHills = new Hills(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);

	m_pShapes = new Shapes(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::WireframeRS);

	m_pSkull = new Skull(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::WireframeRS);

	m_pWaves = new Waves(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::WireframeRS);
	m_pWaves->Initialize(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	m_pLighting = new Lighting(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);
	m_pLighting->Initialize(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	m_pLightingSkull = new LightingSkull(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);

	m_pTexturedHillsAndWaves = new TexturedHillsAndWaves(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);
	m_pTexturedHillsAndWaves->Initialize(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	m_pCrate = new Crate(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);

	m_pBlend = new Blend(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);
	m_pBlend->Initialize(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	m_pFireAnim = new FireAnim(m_pD3DDevice, m_pD3DImmediateContext, RenderStates::SolidRS);

	return true;
}

/// <summary>
/// 창이 리사이즈 됐을 때 처리해야 할 것들
/// </summary>
void HGEngine::OnResize()
{
	assert(m_pD3DImmediateContext);
	assert(m_pD3DDevice);
	assert(m_pSwapChain);

	// 버퍼에 대한 참조가 포함된 이전 뷰를 해제한다.
	// 이전 깊이/스텐실 버퍼도 해제한다.
	ReleaseCOM(m_pRenderTargetView);
	ReleaseCOM(m_pDepthStencilView);
	ReleaseCOM(m_pDepthStencilBuffer);

	// 스왑 체인의 크기를 조정하고 렌더 대상 뷰를 재생성 한다.
	HR(m_pSwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_pD3DDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView));
	ReleaseCOM(backBuffer);

	// 깊이/스텐실 버퍼와 뷰를 생성한다.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 4X MSAA를 사용하는가? 반드시 교환 사슬의 MSAA 설정과 일치해야 함.
	if (m_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// MSAA를 사용하지 않음
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pD3DDevice->CreateTexture2D(
		&depthStencilDesc,			// 생성할 텍스처를 서술하는 구조체
		0,
		&m_pDepthStencilBuffer));	// 깊이/스텐실 버퍼를 가리키는 포인터를 돌려준다,

	HR(m_pD3DDevice->CreateDepthStencilView(
		m_pDepthStencilBuffer,		// 뷰를 생성하고자 하는 자원
		0,
		&m_pDepthStencilView));		// 깊이/스텐실 뷰를 돌려준다.

	// 렌더 타겟 뷰, 깊이/스탠실 뷰를 파이프라인에 바인딩한다.
	m_pD3DImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 뷰포트 변환을 셋팅한다.
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_pD3DImmediateContext->RSSetViewports(1, &m_ScreenViewport);

	// 창의 크기가 변했으므로, 종횡비를 업데이트하고 투영 행렬을 재계산한다.
	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	m_pCamera->SetProj(p);

	m_pCamera->SetLens(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
}

void HGEngine::Update(GameTimer* pTimer)
{
	UpdateCamera(pTimer->DeltaTime());

	m_pBox->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pHills->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pShapes->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pSkull->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pWaves->Update(m_pCamera->View(), m_pCamera->Proj(), pTimer);
	m_pLighting->Update(m_pCamera->View(), m_pCamera->Proj(), pTimer);
	m_pLightingSkull->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pTexturedHillsAndWaves->Update(m_pCamera->View(), m_pCamera->Proj(), pTimer);
	m_pCrate->Update(m_pCamera->View(), m_pCamera->Proj());
	m_pBlend->Update(m_pCamera->View(), m_pCamera->Proj(), pTimer);
	m_pFireAnim->Update(m_pCamera->View(), m_pCamera->Proj(), pTimer, m_FireAnimSpeed);
}

void HGEngine::BeginRender()
{
	assert(m_pD3DImmediateContext);

	// 후면 버퍼를 Custom Grey로 지운다.
	m_pD3DImmediateContext->ClearRenderTargetView(m_pRenderTargetView,
		reinterpret_cast<const float*>(&DirectXColors::CustomGrey));

	// 깊이 버퍼를 1.0f로, 스텐실 버퍼를 0으로 지운다.
	m_pD3DImmediateContext->ClearDepthStencilView(m_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void HGEngine::Render()
{
	m_pD3DImmediateContext->OMSetDepthStencilState(RenderStates::NormalDSS, 0);

	//m_pBox->Render();
	//m_pHills->Render();
	//m_pShapes->Render();
	//m_pSkull->Render();
	//m_pWaves->Render();
	//m_pLighting->Render();
	//m_pLightingSkull->Render();
	//m_pTexturedHillsAndWaves->Render();
	//m_pCrate->Render();
	m_pBlend->Render();
	//m_pFireAnim->Render();

	DrawInfo();

	// 기본값 복원
	m_pD3DImmediateContext->RSSetState(0);
}

void HGEngine::EndRender()
{
	assert(m_pSwapChain);

	// 후면 버퍼를 화면에 제시한다. 
	HR(m_pSwapChain->Present(0, 0));
}

/// <summary>
/// 랜더러를 종료한다.
/// </summary>
void HGEngine::Finalize()
{
	// DirectX의 자원 해제는 소멸자라는 명시적인 타이밍이 있다.
	// 여기는 그 외 자원들 (스크립트, 객체간의 레퍼런스 해제등)을 처리한다.
 	SafeDelete(m_pFireAnim);
 	SafeDelete(m_pBlend);
	SafeDelete(m_pCrate);
	SafeDelete(m_pTexturedHillsAndWaves);
	SafeDelete(m_pLightingSkull);
	SafeDelete(m_pLighting);
	SafeDelete(m_pWaves);
	SafeDelete(m_pSkull);
	SafeDelete(m_pShapes);
	SafeDelete(m_pHills);
	SafeDelete(m_pBox);
	SafeDelete(m_pFont);
	SafeDelete(m_pCamera);
}

void HGEngine::OnMouseDown(int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_EnginehWnd);
}

void HGEngine::OnMouseUp(int x, int y)
{
	ReleaseCapture();
}

void HGEngine::OnMouseMove(int btnState, int x, int y)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		m_pCamera->Pitch(dy);
		m_pCamera->RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

float HGEngine::GetAspectRatio() const
{
	return static_cast<float>(m_ClientWidth) / static_cast<float>(m_ClientHeight);
}

// 디바이스/GPU 정보 획득 <방법 #1> 
// DXGI 1.1 / DXGI Factory 사용 / DX11 이상시 권장, DXGI 1.0 과 1.1 혼용금지. DXGI 1.3 은 Windows 8.1 이상필수.
// DXGI 1.1 대응
HRESULT HGEngine::GetAdapterInfo()
{
	// DXGI버전별로 다름
	IDXGIAdapter1* pAdapter;
	IDXGIFactory1* pFactory = NULL;

	HRESULT hr = S_OK;

	// DXGIFactory 개체 생성(DXGI.lib 필요)
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
	{
		return E_FAIL;
	}

	hr = pFactory->EnumAdapters1(0, &pAdapter);

	if (hr != DXGI_ERROR_NOT_FOUND)
	{
		pAdapter->GetDesc1(&m_AdapterDesc1);
	}

	SAFE_RELEASE(pAdapter);
	SAFE_RELEASE(pFactory);

	return S_OK;
}

void HGEngine::UpdateCamera(float deltaTime)
{
	// view TM을 만든다.
	m_pCamera->UpdateViewMatrix();

	if (GetAsyncKeyState('W') & 0x8000)
		m_pCamera->Walk(10.0f * deltaTime);

	if (GetAsyncKeyState('S') & 0x8000)
		m_pCamera->Walk(-10.0f * deltaTime);

	if (GetAsyncKeyState('A') & 0x8000)
		m_pCamera->Strafe(-10.0f * deltaTime);

	if (GetAsyncKeyState('D') & 0x8000)
		m_pCamera->Strafe(10.0f * deltaTime);

	if (GetAsyncKeyState('Q') & 0x8000)
		m_pCamera->WorldUpDown(-10.0f * deltaTime);

	if (GetAsyncKeyState('E') & 0x8000)
		m_pCamera->WorldUpDown(10.0f * deltaTime);
}

void HGEngine::DrawInfo()
{
	// 폰트 라이브러리 테스트
	m_pFont->DrawTest();

	DirectX::SimpleMath::Vector4 yellow(1.f, 1.f, 0.f, 1.f);	// 드럽네..
	DirectX::SimpleMath::Vector4 white(1.f, 1.f, 1.f, 1.f);		// white

	m_pFont->DrawTextColor(200, 100, yellow, (TCHAR*)L"한글과 버퍼가 잘 작동하는가? %d / %f / %s", 79, 3.14f, L"노란색글씨 똠방각하 펲시 콜라 뾸뾸");

	// 피쳐레벨
	int yPos = 20;
	m_pFont->DrawTextColor(20, yPos, white, (TCHAR*)L"Feature Level : %x", m_FeatureLevel);

	// 어댑터 정보
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"Description : %s", m_AdapterDesc1.Description);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"VendorID : %u", m_AdapterDesc1.VendorId);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"DeviceID : %u", m_AdapterDesc1.DeviceId);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"SubSysID : %u", m_AdapterDesc1.SubSysId);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"Revision : %u", m_AdapterDesc1.Revision);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"VideoMemory : %lu MB", m_AdapterDesc1.DedicatedVideoMemory / 1024 / 1024);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"SystemMemory : %lu MB", m_AdapterDesc1.DedicatedSystemMemory / 1024 / 1024);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"SharedSysMemory : %lu MB", m_AdapterDesc1.SharedSystemMemory / 1024 / 1024);
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"AdpaterLuid : %u.%d", m_AdapterDesc1.AdapterLuid.HighPart, m_AdapterDesc1.AdapterLuid.LowPart);

	// Client Width / Height
	m_pFont->DrawTextColor(20, yPos += 35, yellow, (TCHAR*)L"ClientWidth : %d / ClientHeight : %d", m_ClientWidth, m_ClientHeight);

	// 카메라 정보
	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"Camera Pos : %.2f / %.2f / %.2f", m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z);

	m_pFont->DrawTextColor(20, yPos += 20, white, (TCHAR*)L"Fire Animation Speed : %.2f", m_FireAnimSpeed);
}