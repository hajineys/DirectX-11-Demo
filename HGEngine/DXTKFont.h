#pragma once
#include "SpriteFont.h"
#include "SpriteBatch.h"

/// <summary>
/// DXTK를 이용한 텍스트 출력
/// 
/// 추가 필요한 목록
/// 01. 자간, 장평, 줄 간격 고려한 멀티라인 그리기
///	02. WriteFactory를 이용한 창 변환에도 깔끔한 폰트 
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

	// 그냥 사용하면 뎁스 문제가 생긴다.
	ID3D11RasterizerState* m_RasterizerState;
	ID3D11DepthStencilState* m_DepthStencilState;
};