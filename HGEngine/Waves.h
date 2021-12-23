#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"

class GameTimer;

/// <summary>
/// DirectX 11 Example : Waves
/// 
/// 2021. 12. 09 Hamdal
/// 2021. 12. 14 Hamdal(수정) Effect 관련 부분 이동
/// </summary>
class Waves : public AlignedAllocationPolicy<16>
{
public:
	Waves(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Waves();

public:
	void Initialize(UINT m, UINT n, float dx, float dt, float speed, float damping);

	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer);
	void Render();

private:
	void BuildGeometryBuffers();
	void Disturb(UINT i, UINT j, float magnitude);
	void UpdateWave(float deltaTime);

private:
	ID3D11Device* m_pD3DDevice;						// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;	// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;			// Render State(렌더링을 어떻게 할 것인가)

	UINT m_NumRows;
	UINT m_NumCols;

	UINT m_VertexCount;
	UINT m_TriangleCount;

	// Simulation constants we can precompute.
	float m_K1;
	float m_K2;
	float m_K3;

	float m_TimeStep;
	float m_SpatialStep;

	XMFLOAT3* m_pPrevSolution;
	XMFLOAT3* m_pCurrSolution;

	ID3D11Buffer* m_pWavesVB;
	ID3D11Buffer* m_pWavesIB;

	XMMATRIX m_World;		// 월드 변환 행렬 (로컬->월드)
	XMMATRIX m_View;		// 시야 변환 행렬 (카메라 뷰)
	XMMATRIX m_Proj;		// 투영 변환 행렬 (원근/직교)
};