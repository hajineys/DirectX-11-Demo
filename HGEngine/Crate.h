#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

/// <summary>
/// DirectX 11 Example : Crate
/// 
/// 2021. 12. 16 Hamdal
/// </summary>
class Crate : public AlignedAllocationPolicy<16>
{
public:
	Crate(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~Crate();

public:
	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	void Render();

private:
	void BuildGeometryBuffers();

	ID3D11Device* m_pD3DDevice;						// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;	// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;			// Render State(렌더링을 어떻게 할 것인가)

	int m_BoxVertexOffset;
	UINT m_BoxIndexCount;
	UINT m_BoxIndexOffset;
	ID3D11Buffer* m_BoxVB;
	ID3D11Buffer* m_BoxIB;

	ID3D11ShaderResourceView* m_DiffuseMapSRV;

	DirectionalLight m_DirLights[3];
	Material m_BoxMat;

	XMMATRIX m_TexTransform;
	XMMATRIX m_BoxWorld;

	XMMATRIX m_View;
	XMMATRIX m_Proj;
	
	float m_Theta;
	float m_Phi;
	float m_Radius;

	XMFLOAT3 m_EyePosW;
};