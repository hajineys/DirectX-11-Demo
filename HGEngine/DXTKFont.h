#pragma once
#include "SpriteFont.h"
#include "SpriteBatch.h"

/// <summary>
/// DXTK�� �̿��� �ؽ�Ʈ ���
/// 
/// �߰� �ʿ��� ���
/// 01. �ڰ�, ����, �� ���� ����� ��Ƽ���� �׸���
///	02. WriteFactory�� �̿��� â ��ȯ���� ����� ��Ʈ 
/// 2021. 11. 22 Hamdal
/// </summary>
class DXTKFont
{
public:
	DXTKFont();
	~DXTKFont();

	void Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds);
	void DrawTest();
	void DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text, ...);
	void DrawTextColor(int posX, int posY, DirectX::XMFLOAT4 color, TCHAR* text);

private:
	DirectX::SpriteBatch* m_pSpriteBatch;
	DirectX::SpriteFont* m_pSpriteFont;

	// �׳� ����ϸ� ���� ������ �����.
	ID3D11RasterizerState* m_RasterizerState;
	ID3D11DepthStencilState* m_DepthStencilState;
};