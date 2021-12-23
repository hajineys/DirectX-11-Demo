#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

class GameTimer;

/// <summary>
/// DirectX 11 Example : Textured Hills And Waves
/// 
/// 2021. 12. 16 Hamdal
/// </summary>
class TexturedHillsAndWaves : public AlignedAllocationPolicy<16>
{
public:
	TexturedHillsAndWaves(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~TexturedHillsAndWaves();

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

	float Width()const;
	float Depth()const;

	void Disturb(UINT i, UINT j, float magnitude);
	void UpdateWave(float deltaTime);

	// Returns the solution normal at the ith grid point.
	const XMFLOAT3& Normal(int i)const { return m_Normals[i]; }

private:
	ID3D11Device* m_pD3DDevice;							// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;		// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;				// Render State(렌더링을 어떻게 할 것인가)

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

	XMFLOAT3* m_PrevSolution;
	XMFLOAT3* m_CurrSolution;
	XMFLOAT3* m_Normals;
	XMFLOAT3* m_TangentX;

	UINT m_LandIndexCount;

	XMFLOAT2 m_WaterTexOffset;

	ID3D11Buffer* m_LandVB;
	ID3D11Buffer* m_LandIB;

	ID3D11Buffer* m_WavesVB;
	ID3D11Buffer* m_WavesIB;

	ID3D11ShaderResourceView* m_GrassMapSRV;
	ID3D11ShaderResourceView* m_WavesMapSRV;

	DirectionalLight m_DirLights[3];
	Material m_LandMat;
	Material m_WavesMat;

	XMMATRIX m_GrassTexTransform;
	XMMATRIX m_WaterTexTransform;
	XMMATRIX m_LandWorld;
	XMMATRIX m_WavesWorld;

	XMMATRIX m_View;
	XMMATRIX m_Proj;

	UINT m_LightCount;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	XMFLOAT3 m_EyePosW;
};