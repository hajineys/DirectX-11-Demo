#include "Hills.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"

Hills::Hills(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_GridIndexCount(0),
	m_pHillsVB(0), m_pHillsIB(0),
	m_World(), m_View(), m_Proj()
{
	BuildGeometryBuffers();

	m_World = XMMatrixIdentity();
}

Hills::~Hills()
{
	ReleaseCOM(m_pHillsVB);
	ReleaseCOM(m_pHillsIB);
}

void Hills::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;
}

void Hills::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosColor);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// RenderState
	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;

	// World * View * Projection Transform Matrix 셋팅
	XMMATRIX worldViewProj = m_World * m_View * m_Proj;

	// 어떤 Effect Tech를 사용할지 파악.
	ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pHillsVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pHillsIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::ColorFX->SetWorldViewProj(worldViewProj);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);

		// Draw the grid.
		m_pD3DImmediateContext->DrawIndexed(m_GridIndexCount, 0, 0);
	}
}

void Hills::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_GridIndexCount = (UINT)grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex::PosColor> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * (UINT)grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pHillsVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_GridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pHillsIB));
}

float Hills::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}