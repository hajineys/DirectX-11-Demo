#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

/// <summary>
/// DirectX 11 Example : Hills
/// 
/// 2021. 11. 22 Hamdal
/// 2021. 12. 14 Hamdal(����) Effect ���� �κ� �̵�
/// </summary>
class Hills : public AlignedAllocationPolicy<16>
{
public:
	Hills(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Hills();

public:
	// ����� ������Ʈ �� ��ȯ��ĸ� �����Ѵ�.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	void Render();

private:
	void BuildGeometryBuffers();
	float GetHeight(float x, float z)const;

private:
	ID3D11Device* m_pD3DDevice;						// DirectX 11 ����̽�
	ID3D11DeviceContext* m_pD3DImmediateContext;	// ����̽� ���ؽ�Ʈ
	ID3D11RasterizerState* m_pRenderstate;			// Render State(�������� ��� �� ���ΰ�)

	UINT m_GridIndexCount;
	ID3D11Buffer* m_pHillsVB;
	ID3D11Buffer* m_pHillsIB;

	XMMATRIX m_World;		// ���� ��ȯ ��� (����->����)
	XMMATRIX m_View;		// �þ� ��ȯ ��� (ī�޶� ��)
	XMMATRIX m_Proj;		// ���� ��ȯ ��� (����/����)
};