#include "DXTKFont.h"
#include <tchar.h>
#include "SimpleMath.h"

#define SafeDelete(x) { delete x; x = 0; }
#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

DXTKFont::DXTKFont()
	: m_pSpriteBatch(nullptr),
	m_pSpriteFont(nullptr),
	m_RasterizerState(nullptr),
	m_DepthStencilState(nullptr)
{
	
}

DXTKFont::~DXTKFont()
{
	m_RasterizerState = nullptr;
	m_DepthStencilState = nullptr;

	SafeDelete(m_pSpriteFont);
	SafeDelete(m_pSpriteBatch);
}

void DXTKFont::Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds)
{
	ID3D11DeviceContext* pDC = nullptr;
	pDevice->GetImmediateContext(&pDC);

	m_pSpriteBatch = new DirectX::SpriteBatch(pDC);

	TCHAR* fileName = (TCHAR*)L"../Font/gulim9k.spritefont";

	m_pSpriteFont = new DirectX::SpriteFont(pDevice, fileName);
	m_pSpriteFont->SetLineSpacing(14.0f);
	m_pSpriteFont->SetDefaultCharacter(' ');

	ReleaseCOM(pDC);

	m_RasterizerState = rs;
	m_DepthStencilState = ds;
}

void DXTKFont::DrawTest()
{
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, L"Hello, world! 한글 테스트", DirectX::XMFLOAT2(300.0f, 10.0f));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text, ...)
{
	TCHAR buffer[1024] = L"";
	va_list vl;
	va_start(vl, text);
	_vstprintf(buffer, 1024, text, vl);
	va_end(vl);

	// SpriteBatch에 여러 렌더 스테이트를 지정할 수 있다.
	// 문제는, 그냥 Begin만 하면 뎁스스탠실버퍼 옵션이 D3D11_DEPTH_WRITE_MASK_ZERO가 되는 듯. DSS를 다루지 않는 포반에는 문제가 될 수 있다.
	// 아래처럼 여기에 옵션을 ALL로 넣어줘서 ZERO가 되는 것을 막을 수도 있고, 다른 쪽 오브젝트를 그릴 때 렌더스테이트를 지정 해 줄 수도 있다.
	// DX12에서 렌더스테이트등을 그렇게 가져가는 것도 이해는 간다. 별로 맘에 안들었겠지..
	///m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, buffer, DirectX::XMFLOAT2((float)posX, (float)posY), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text)
{
	// SpriteBatch에 여러 렌더 스테이트를 지정할 수 있다.
	// 문제는, 그냥 Begin만 하면 뎁스스탠실버퍼 옵션이 D3D11_DEPTH_WRITE_MASK_ZERO가 되는 듯. DSS를 다루지 않는 포반에는 문제가 될 수 있다.
	// 아래처럼 여기에 옵션을 ALL로 넣어줘서 ZERO가 되는 것을 막을 수도 있고, 다른 쪽 오브젝트를 그릴 때 렌더스테이트를 지정 해 줄 수도 있다.
	// DX12에서 렌더스테이트등을 그렇게 가져가는 것도 이해는 간다. 별로 맘에 안들었겠지..
	///m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, text, DirectX::XMFLOAT2((float)posX, (float)posY), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}