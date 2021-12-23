#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

/// <summary>
/// DirectX 11 Example : Shapes
/// 
/// 2021. 11. 22 Hamdal
/// 2021. 12. 14 Hamdal(수정) Effect 관련 부분 이동
/// </summary>
class Shapes : public AlignedAllocationPolicy<16>
{
public:
	Shapes(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Shapes();

public:
	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	void Render();

private:
	void BuildGeometryBuffers();

private:
	ID3D11Device* m_pD3DDevice;						// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;	// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;			// Render State(렌더링을 어떻게 할 것인가)

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