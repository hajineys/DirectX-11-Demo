#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

class GameTimer;

/// <summary>
/// DirectX 11 Example : LightingSkull
/// 
/// 2021. 12. 10 Hamdal
/// 2021. 12. 14 Hamdal(����) Effect ���� �κ� �̵�
/// </summary>
class LightingSkull : public AlignedAllocationPolicy<16>
{
public:
	LightingSkull(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~LightingSkull();

public:
	// ����� ������Ʈ �� ��ȯ��ĸ� �����Ѵ�.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	void Render();

private:
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();

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

	UINT m_SkullIndexCount;
	ID3D11Buffer* m_pShapesVB;
	ID3D11Buffer* m_pShapesIB;

	ID3D11Buffer* m_pSkullVB;
	ID3D11Buffer* m_pSkullIB;

	DirectionalLight m_DirLights[3];
	Material m_GridMat;
	Material m_BoxMat;
	Material m_CylinderMat;
	Material m_SphereMat;
	Material m_SkullMat;

	// Define transformations from local spaces to world space.
	XMMATRIX m_SphereWorld[10];
	XMMATRIX m_CylWorld[10];
	XMMATRIX m_BoxWorld;
	XMMATRIX m_GridWorld;
	XMMATRIX m_SkullWorld;

	XMMATRIX m_View;
	XMMATRIX m_Proj;

	UINT m_LightCount;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	XMFLOAT3 m_EyePosW;
};