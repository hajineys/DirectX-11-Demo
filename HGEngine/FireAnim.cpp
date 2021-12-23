#include "FireAnim.h"
//#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"
#include "MathHelper.h"
#include "GameTimer.h"

FireAnim::FireAnim(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_BoxVertexOffset(0),
	m_BoxIndexOffset(0),
	m_BoxIndexCount(0),
	m_BoxVB(0), m_BoxIB(0),
	m_DiffuseMapSRV(),
	m_DirLights(),
	m_BoxMat(),
	m_TexTransform(), m_BoxWorld(), m_View(), m_Proj(),
	m_Theta(1.3f * MathHelper::Pi),
	m_Phi(0.4f * MathHelper::Pi),
	m_Radius(2.5f),
	m_EyePosW(0.0f, 0.0f, 0.0f),
	m_TextureIndex(0),
	m_AnimationTime(0.0f),
	m_Speed(1.0f)
{
	XMMATRIX identity = XMMatrixIdentity();
	m_BoxWorld = identity;
	m_TexTransform = identity;

	m_DirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_DirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	m_BoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_BoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_BoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	BuildGeometryBuffers();

	std::wstring filePath = L"../Resource/Textures/FireAnim/Fire";
	std::wstring ext = L".bmp";

	ID3D11Resource* texResource = nullptr;

	for (int i = 1; i <= 120; i++)
	{
		int index = i;
		std::wstring fileName;

		if (1 <= i && i <= 9)
		{
			std::wstring temp = L"00";
			fileName = filePath + temp + std::to_wstring(i) + ext;
		}
		else if (10 <= i && i <= 99)
		{
			std::wstring temp = L"0";
			fileName = filePath + temp + std::to_wstring(i) + ext;
		}
		else
		{
			fileName = filePath + std::to_wstring(i) + ext;
		}

		ID3D11ShaderResourceView* tempDiffuseMap = nullptr;
		HR(CreateWICTextureFromFile(m_pD3DDevice,
			fileName.c_str(), &texResource, &tempDiffuseMap));

		m_DiffuseMapSRV.push_back(tempDiffuseMap);
	}

	ReleaseCOM(texResource);	// view saves reference
}

FireAnim::~FireAnim()
{
	ReleaseCOM(m_BoxVB);
	ReleaseCOM(m_BoxIB);

	for (int i = 0; i <= m_DiffuseMapSRV.size() - 1; i++)
	{
		ReleaseCOM(m_DiffuseMapSRV[i]);
	}
}

void FireAnim::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer, float& nowSpeed)
{
	m_View = view;
	m_Proj = proj;

	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);

	// 이전 프레임 ~ 현재 프레임까지 걸린 시간
	float deltaTime = pTimer->DeltaTime();

	m_AnimationTime += deltaTime * m_Speed;

	// 약 1초가 지나면
	if (m_AnimationTime > 1.0f)
	{
		m_AnimationTime = 0.0f;
		m_TextureIndex++;
	}

	// 벡터 최대사이즈가 되면 다시 0으로 (Loop)
	if (m_TextureIndex == m_DiffuseMapSRV.size() - 1)
	{
		m_TextureIndex = 0;
	}

	// 키인풋으로 스피드 조절
	if (GetAsyncKeyState(VK_F7))
	{
		m_Speed += 0.5f;
	}

	if (GetAsyncKeyState(VK_F8))
	{
		m_Speed -= 0.5f;
	}

	// 스피드의 최소, 최대 설정
	if (m_Speed > 30.0f)
	{
		m_Speed = 30.0f;
	}
	else if (m_Speed <= 0.0f)
	{
		m_Speed = 0.1f;
	}

	nowSpeed = m_Speed;
}

void FireAnim::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosNorTexture);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosNorTexture);
	UINT offset = 0;

	XMMATRIX viewProj = m_View * m_Proj;

	// Set per frame constants.
	Effects::BasicDifMapFX->SetDirLights(m_DirLights);
	Effects::BasicDifMapFX->SetEyePosW(m_EyePosW);

	ID3DX11EffectTechnique* activeTech = Effects::BasicDifMapFX->Light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_BoxVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_BoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(m_BoxWorld);
		XMMATRIX worldViewProj = m_BoxWorld * viewProj;

		Effects::BasicDifMapFX->SetWorld(m_BoxWorld);
		Effects::BasicDifMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicDifMapFX->SetWorldViewProj(worldViewProj);
		Effects::BasicDifMapFX->SetTexTransform(m_TexTransform);
		Effects::BasicDifMapFX->SetMaterial(m_BoxMat);

		Effects::BasicDifMapFX->SetDiffuseMap(m_DiffuseMapSRV[m_TextureIndex]);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);
	}
}

void FireAnim::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	m_BoxVertexOffset = 0;

	// Cache the index count of each object.
	m_BoxIndexCount = (UINT)box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	m_BoxIndexOffset = 0;

	UINT totalVertexCount = (UINT)box.Vertices.size();

	UINT totalIndexCount = m_BoxIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNorTexture> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNorTexture) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_BoxVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_BoxIB));
}