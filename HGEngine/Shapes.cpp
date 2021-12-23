#include "Shapes.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"

Shapes::Shapes(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_BoxVertexOffset(0), m_GridVertexOffset(0), m_SphereVertexOffset(0), m_CylinderVertexOffset(0),
	m_BoxIndexOffset(0), m_GridIndexOffset(0), m_SphereIndexOffset(0), m_CylinderIndexOffset(0),
	m_BoxIndexCount(0), m_GridIndexCount(0), m_SphereIndexCount(0), m_CylinderIndexCount(0),
	m_pShapeVB(0), m_pShapeIB(0),
	m_SphereWorld(), m_CylWorld(), m_BoxWorld(), m_GridWorld(),
	m_CenterSphere(),
	m_View(), m_Proj()
{
	m_GridWorld = XMMatrixIdentity();

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	m_BoxWorld = XMMatrixMultiply(boxScale, boxOffset);

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	m_CenterSphere = XMMatrixMultiply(centerSphereScale, centerSphereOffset);

	for (int i = 0; i < 5; ++i)
	{
		m_CylWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		m_CylWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

		m_SphereWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		m_SphereWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);
	}

	BuildGeometryBuffers();
}

Shapes::~Shapes()
{
	ReleaseCOM(m_pShapeVB);
	ReleaseCOM(m_pShapeIB);
}

void Shapes::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;
}

void Shapes::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosColor);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;
	
	XMMATRIX viewProj = m_View * m_Proj;

	// 어떤 Effect Tech를 사용할지 파악.
	ID3DX11EffectTechnique* activeTech = Effects::ColorFX->ColorTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pShapeVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pShapeIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the grid.
		XMMATRIX worldViewProj = m_GridWorld * viewProj;
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_GridIndexCount, m_GridIndexOffset, m_GridVertexOffset);

		// Draw the box.
		worldViewProj = m_BoxWorld * viewProj;
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);

		// Draw center sphere.
		worldViewProj = m_CenterSphere * viewProj;
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_SphereIndexCount, m_SphereIndexOffset, m_SphereVertexOffset);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			worldViewProj = m_CylWorld[i] * viewProj;
			Effects::ColorFX->SetWorldViewProj(worldViewProj);
			activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
			m_pD3DImmediateContext->DrawIndexed(m_CylinderIndexCount, m_CylinderIndexOffset, m_CylinderVertexOffset);
		}

		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			worldViewProj = m_SphereWorld[i] * viewProj;
			Effects::ColorFX->SetWorldViewProj(worldViewProj);
			activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
			m_pD3DImmediateContext->DrawIndexed(m_SphereIndexCount, m_SphereIndexOffset, m_SphereVertexOffset);
		}
	}
}

void Shapes::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	//geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	m_BoxVertexOffset = 0;
	m_GridVertexOffset = (UINT)box.Vertices.size();
	m_SphereVertexOffset = m_GridVertexOffset + (UINT)grid.Vertices.size();
	m_CylinderVertexOffset = m_SphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the index count of each object.
	m_BoxIndexCount = (UINT)box.Indices.size();
	m_GridIndexCount = (UINT)grid.Indices.size();
	m_SphereIndexCount = (UINT)sphere.Indices.size();
	m_CylinderIndexCount = (UINT)cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	m_BoxIndexOffset = 0;
	m_GridIndexOffset = m_BoxIndexCount;
	m_SphereIndexOffset = m_GridIndexOffset + m_GridIndexCount;
	m_CylinderIndexOffset = m_SphereIndexOffset + m_SphereIndexCount;

	UINT totalVertexCount =
		(UINT)box.Vertices.size() +
		(UINT)grid.Vertices.size() +
		(UINT)sphere.Vertices.size() +
		(UINT)cylinder.Vertices.size();

	UINT totalIndexCount =
		m_BoxIndexCount +
		m_GridIndexCount +
		m_SphereIndexCount +
		m_CylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosColor> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pShapeVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pShapeIB));
}