#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

/// <summary>
/// DirectX 11 Example : Shapes
/// 
/// 2021. 11. 22 Hamdal
/// 2021. 12. 14 Hamdal(����) Effect ���� �κ� �̵�
/// </summary>
class Shapes : public AlignedAllocationPolicy<16>
{
public:
	Shapes(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Shapes();

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

	int m_BoxVertexOffset;
	int m_GridVertexOffset;
	int m_SphereVertexOffset;
	int m_CylinderVertexOffset;

	UINT m_BoxIndexOffset;
	UINT m_GridIndexOffset;
	UINT m_SphereIndexOffset;
	UINT m_CylinderIndexOffset;

	UINT m_BoxIndexCount;
	UINT m_GridIndexCount;
	UINT m_SphereIndexCount;
	UINT m_CylinderIndexCount;

	ID3D11Buffer* m_pShapeVB;
	ID3D11Buffer* m_pShapeIB;

	// Define transformations from local spaces to world space.
	XMMATRIX m_SphereWorld[10];
	XMMATRIX m_CylWorld[10];
	XMMATRIX m_BoxWorld;
	XMMATRIX m_GridWorld;
	XMMATRIX m_CenterSphere;

	XMMATRIX m_View;
	XMMATRIX m_Proj;
};