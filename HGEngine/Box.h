#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

/// <summary>
/// DirectX 11 Example : Box
/// 
/// 2021. 11. 19 Hamdal
/// 2021. 12. 10 Hamdal(수정) Effect 관련 부분 이동
/// </summary>
class Box : public AlignedAllocationPolicy<16>
{
public:
	Box(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Box();

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

	UINT m_BoxIndexCount;
	ID3D11Buffer* m_pBoxVB;
	ID3D11Buffer* m_pBoxIB;

	XMMATRIX m_World;		// 월드 변환 행렬 (로컬->월드)
	XMMATRIX m_View;		// 시야 변환 행렬 (카메라 뷰)
	XMMATRIX m_Proj;		// 투영 변환 행렬 (원근/직교)
};