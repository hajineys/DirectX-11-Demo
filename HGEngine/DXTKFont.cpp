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
	m_pSpriteFont->DrawString(m_pSpriteBatch, L"Hello, world! �ѱ� �׽�Ʈ", DirectX::XMFLOAT2(300.0f, 10.0f));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text, ...)
{
	TCHAR buffer[1024] = L"";
	va_list vl;
	va_start(vl, text);
	_vstprintf(buffer, 1024, text, vl);
	va_end(vl);

	// SpriteBatch�� ���� ���� ������Ʈ�� ������ �� �ִ�.
	// ������, �׳� Begin�� �ϸ� �������Ľǹ��� �ɼ��� D3D11_DEPTH_WRITE_MASK_ZERO�� �Ǵ� ��. DSS�� �ٷ��� �ʴ� ���ݿ��� ������ �� �� �ִ�.
	// �Ʒ�ó�� ���⿡ �ɼ��� ALL�� �־��༭ ZERO�� �Ǵ� ���� ���� ���� �ְ�, �ٸ� �� ������Ʈ�� �׸� �� ����������Ʈ�� ���� �� �� ���� �ִ�.
	// DX12���� ����������Ʈ���� �׷��� �������� �͵� ���ش� ����. ���� ���� �ȵ������..
	///m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, buffer, DirectX::XMFLOAT2((float)posX, (float)posY), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text)
{
	// SpriteBatch�� ���� ���� ������Ʈ�� ������ �� �ִ�.
	// ������, �׳� Begin�� �ϸ� �������Ľǹ��� �ɼ��� D3D11_DEPTH_WRITE_MASK_ZERO�� �Ǵ� ��. DSS�� �ٷ��� �ʴ� ���ݿ��� ������ �� �� �ִ�.
	// �Ʒ�ó�� ���⿡ �ɼ��� ALL�� �־��༭ ZERO�� �Ǵ� ���� ���� ���� �ְ�, �ٸ� �� ������Ʈ�� �׸� �� ����������Ʈ�� ���� �� �� ���� �ִ�.
	// DX12���� ����������Ʈ���� �׷��� �������� �͵� ���ش� ����. ���� ���� �ȵ������..
	///m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, text, DirectX::XMFLOAT2((float)posX, (float)posY), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}