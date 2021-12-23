#include "Box.h"
#include "Vertex.h"
#include "Effects.h"

Box::Box(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_BoxIndexCount(0),
	m_pBoxVB(0), m_pBoxIB(0),
	m_World(), m_View(), m_Proj()
{
	BuildGeometryBuffers();

	m_World = XMMatrixIdentity();
}

Box::~Box()
{
	ReleaseCOM(m_pBoxVB);
	ReleaseCOM(m_pBoxIB);
}

void Box::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;
}

void Box::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosColor);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// RenderState
	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;

	// World * View * Projection Transform Matrix 셋팅
	XMMATRIX worldViewProj = m_World * m_View * m_Proj;

	// 어떤 Effect Tech를 사용할지 파악
	ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pBoxVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pBoxIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::ColorFX->SetWorldViewProj(worldViewProj);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);

		// 36 indices for the box.
		m_pD3DImmediateContext->DrawIndexed(m_BoxIndexCount, 0, 0);
	}
}

void Box::BuildGeometryBuffers()
{
	// Create vertex buffer
	std::vector<Vertex::PosColor> vertices =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&DirectXColors::White)   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&DirectXColors::Black)   },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&DirectXColors::Red)     },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&DirectXColors::Green)   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&DirectXColors::Blue)    },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&DirectXColors::Yellow)  },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&DirectXColors::Cyan)    },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&DirectXColors::Magenta) }
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * (UINT)vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pBoxVB));

	// Create the index buffer
	std::vector<UINT> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	m_BoxIndexCount = (UINT)indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_BoxIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pBoxIB));
}