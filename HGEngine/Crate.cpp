#include "Crate.h"
#include "MathHelper.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"

Crate::Crate(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_BoxVertexOffset(0),
	m_BoxIndexCount(0),
	m_BoxIndexOffset(0),
	m_BoxVB(0), m_BoxIB(0),
	m_DiffuseMapSRV(),
	m_DirLights(),
	m_BoxMat(),
	m_TexTransform(), m_BoxWorld(), m_View(), m_Proj(),
	m_Theta(1.3f * MathHelper::Pi),
	m_Phi(0.4f * MathHelper::Pi),
	m_Radius(2.5f),
	m_EyePosW(0.0f, 0.0f, 0.0f)
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

	ID3D11Resource* texResource = nullptr;
	HR(CreateDDSTextureFromFile(m_pD3DDevice,
		L"../Resource/Textures/WoodCrate01.dds", &texResource, &m_DiffuseMapSRV));
	ReleaseCOM(texResource)	// view saves reference

	BuildGeometryBuffers();
}

Crate::~Crate()
{
	ReleaseCOM(m_BoxVB);
	ReleaseCOM(m_BoxIB);
	ReleaseCOM(m_DiffuseMapSRV);
}

void Crate::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;

	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);
}

void Crate::Render()
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
		Effects::BasicDifMapFX->SetDiffuseMap(m_DiffuseMapSRV);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);
	}
}

void Crate::BuildGeometryBuffers()
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