#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

/// <summary>
/// DirectX 11 Example : Skull
/// 
/// 2021. 12. 09 Hamdal
/// 2021. 12. 14 Hamdal(����) Effect ���� �κ� �̵�
/// </summary>
class Skull : public AlignedAllocationPolicy<16>
{
public:
	Skull(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Skull();

public:
	// ����� ������Ʈ �� ��ȯ��ĸ� �����Ѵ�.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	void Render();

private:
	void BuildGeometryBuffers();

private:
	ID3D11Device* m_pD3DDevice;						// DirectX 11 ����̽�
	ID3D11DeviceContext* m_pD3DImmediateContext;	// ����̽� ���ؽ�Ʈ
	ID3D11RasterizerState* m_pRenderstate;			// Render State(�������� ��� �� ���ΰ�)

	UINT m_SkullIndexCount;
	ID3D11Buffer* m_pSkullVB;
	ID3D11Buffer* m_pSkullIB;

	// ���� �������� ���� ���������� ��ȯ ����
	XMMATRIX m_World;		// ���� ��ȯ ��� (����->����)
	XMMATRIX m_View;		// �þ� ��ȯ ��� (ī�޶� ��)
	XMMATRIX m_Proj;		// ���� ��ȯ ��� (����/����)
};