#pragma once
#pragma once
#include "AlignedAllocationPolicy.h"
#include "DirectXDefine.h"
#include "LightHelper.h"

class GameTimer;

/// <summary>
/// DirectX 11 Question : FireAnim
/// 
/// 2021. 12. 16 Hamdal
/// </summary>
class FireAnim : public AlignedAllocationPolicy<16>
{
public:
	FireAnim(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);
	~FireAnim();

public:
	// 현재는 업데이트 시 변환행렬만 갱신한다.
	void Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer, float& nowSpeed);
	void Render();

private:
	void BuildGeometryBuffers();

private:
	ID3D11Device* m_pD3DDevice;						// DirectX 11 디바이스
	ID3D11DeviceContext* m_pD3DImmediateContext;	// 디바이스 컨텍스트
	ID3D11RasterizerState* m_pRenderstate;			// Render State(렌더링을 어떻게 할 것인가)

	int m_BoxVertexOffset;
	UINT m_BoxIndexOffset;
	UINT m_BoxIndexCount;

	ID3D11Buffer* m_BoxVB;
	ID3D11Buffer* m_BoxIB;

	std::vector<ID3D11ShaderResourceView*> m_DiffuseMapSRV;

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

	int m_TextureIndex;
	float m_AnimationTime;

	float m_Speed;
};