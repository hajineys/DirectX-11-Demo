#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

class GameTimer;

/// <summary>
/// DirectX 11 Example : Lighting
/// 
/// 2021. 12. 10 Hamdal
/// 2021. 12. 14 Hamdal(수정) Effect 관련 부분 이동
/// </summary>
class Lighting : public AlignedAllocationPolicy<16>
{
public:
	Lighting(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Lighting();

public:
	void Initialize(UINT m, UINT n, float dx, float dt, float speed, float damping);

	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer);
	void Render();

private:
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();

	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;

	void Disturb(UINT i, UINT j, float magnitude);
	void UpdateWave(float deltaTime);

	// Returns the solution normal at the ith grid point.
	const XMFLOAT3& Normal(int i)const { return m_pNormals[i]; }

private:
	ID3D11Device* m_pD3DDevice;							// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;		// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;				// Render State(렌더링을 어떻게 할 것인가)

	UINT m_NumRows;
	UINT m_NumCols;

	// Simulation constants we can precompute.
	float m_K1;
	float m_K2;
	float m_K3;

	float m_TimeStep;
	float m_SpatialStep;

	XMFLOAT3* m_pPrevSolution;
	XMFLOAT3* m_pCurrSolution;
	XMFLOAT3* m_pNormals;
	XMFLOAT3* m_pTangentX;

	UINT m_VertexCount;
	UINT m_TriangleCount;
	UINT m_LandIndexCount;

	ID3D11Buffer* m_pLandVB;
	ID3D11Buffer* m_pLandIB;

	ID3D11Buffer* m_pWavesVB;
	ID3D11Buffer* m_pWavesIB;

	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;
	Material m_LandMat;
	Material m_WavesMat;

	// Define transformations from local spaces to world space.
	XMMATRIX m_LandWorld;
	XMMATRIX m_WavesWorld;
	XMMATRIX m_View;
	XMMATRIX m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	XMFLOAT3 m_EyePosW;
};