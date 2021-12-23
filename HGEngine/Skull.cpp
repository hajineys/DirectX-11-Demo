#include "Skull.h"
#include "Vertex.h"
#include "Effects.h"

Skull::Skull(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_SkullIndexCount(0),
	m_pSkullVB(0), m_pSkullIB(0),
	m_World(), m_View(), m_Proj()
{
	m_World = XMMatrixTranslation(0.0f, -2.0f, 0.0f);

	BuildGeometryBuffers();
}

Skull::~Skull()
{
	ReleaseCOM(m_pSkullVB);
	ReleaseCOM(m_pSkullIB);
}

void Skull::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;
}

void Skull::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosColor);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;

	XMMATRIX worldViewProj = m_World * m_View * m_Proj;

	ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pSkullVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pSkullIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::ColorFX->SetWorldViewProj(worldViewProj);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);

		m_pD3DImmediateContext->DrawIndexed(m_SkullIndexCount, 0, 0);
	}
}

void Skull::BuildGeometryBuffers()
{
	std::ifstream fin("../Resource/Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<Vertex::PosColor> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;

		vertices[i].Color = black;

		// Normal not used in this demo.
		fin >> nx >> ny >> nz;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_SkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(m_SkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_SkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pSkullIB));
}