#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

class GameTimer;

enum class RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

/// <summary>
/// DirectX 11 Example : Blend
/// 
/// 2021. 12. 16 Hamdal
/// </summary>
class Blend : public AlignedAllocationPolicy<16>
{
public:
	Blend(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Blend();

public:
	void Initialize(size_t m, size_t n, float dx, float dt, float speed, float damping);

	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer);
	void Render();

private:
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildCrateGeometryBuffers();

	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;

	float Width()const;
	float Depth()const;

	void Disturb(UINT i, UINT j, float magnitude);
	void UpdateWave(float deltaTime);

	// Returns the solution normal at the ith grid point.
	const XMFLOAT3& Normal(int i)const { return m_Normals[i]; }

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

	XMFLOAT3* m_PrevSolution;
	XMFLOAT3* m_CurrSolution;
	XMFLOAT3* m_Normals;
	XMFLOAT3* m_TangentX;

	ID3D11ShaderResourceView* m_GrassMapSRV;
	ID3D11ShaderResourceView* m_WavesMapSRV;
	ID3D11ShaderResourceView* m_BoxMapSRV;

	UINT m_LandIndexCount;
	ID3D11Buffer* m_LandVB;
	ID3D11Buffer* m_LandIB;

	ID3D11Buffer* m_WavesVB;
	ID3D11Buffer* m_WavesIB;

	ID3D11Buffer* m_BoxVB;
	ID3D11Buffer* m_BoxIB;

	XMFLOAT2 m_WaterTexOffset;

	DirectionalLight m_DirLights[3];
	Material m_LandMat;
	Material m_WavesMat;
	Material m_BoxMat;

	XMMATRIX m_GrassTexTransform;
	XMMATRIX m_WaterTexTransform;
	XMMATRIX m_LandWorld;
	XMMATRIX m_WavesWorld;
	XMMATRIX m_BoxWorld;

	XMMATRIX m_View;
	XMMATRIX m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	XMFLOAT3 m_EyePosW;

	RenderOptions m_RenderOptions;
};