#include "LightingSkull.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"

LightingSkull::LightingSkull(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_BoxVertexOffset(0), m_GridVertexOffset(0), m_SphereVertexOffset(0), m_CylinderVertexOffset(0),
	m_BoxIndexOffset(0), m_GridIndexOffset(0), m_SphereIndexOffset(0), m_CylinderIndexOffset(0),
	m_BoxIndexCount(0), m_GridIndexCount(0), m_SphereIndexCount(0), m_CylinderIndexCount(0),
	m_SkullIndexCount(0),
	m_pShapesVB(0), m_pShapesIB(0),
	m_pSkullVB(0), m_pSkullIB(0),
	m_DirLights(),
	m_GridMat(), m_BoxMat(), m_CylinderMat(), m_SphereMat(), m_SkullMat(),
	m_SphereWorld(), m_CylWorld(), m_BoxWorld(), m_GridWorld(), m_SkullWorld(),
	m_View(), m_Proj(),
	m_LightCount(1),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(15.0f),
	m_EyePosW(0.0f, 0.0f, 0.0f)
{
	m_GridWorld = XMMatrixIdentity();

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	m_BoxWorld = XMMatrixMultiply(boxScale, boxOffset);

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	m_SkullWorld = XMMatrixMultiply(skullScale, skullOffset);

	for (int i = 0; i < 5; ++i)
	{
		m_CylWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		m_CylWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

		m_SphereWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		m_SphereWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);
	}

	m_DirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_GridMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_GridMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_GridMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_CylinderMat.Ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_CylinderMat.Diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_CylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	m_SphereMat.Ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	m_SphereMat.Diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	m_SphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

	m_BoxMat.Ambient = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_BoxMat.Diffuse = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_BoxMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_SkullMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_SkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_SkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();
}

LightingSkull::~LightingSkull()
{
	ReleaseCOM(m_pShapesVB);
	ReleaseCOM(m_pShapesIB);
	ReleaseCOM(m_pSkullVB);
	ReleaseCOM(m_pSkullIB);
}

void LightingSkull::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	m_View = view;
	m_Proj = proj;

	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);

	//
	// Switch the number of lights based on key presses.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		m_LightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		m_LightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		m_LightCount = 3;
}

void LightingSkull::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosNormal);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// RenderState
	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosNormal);
	UINT offset = 0;

	XMMATRIX viewProj = m_View * m_Proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(m_DirLights);
	Effects::BasicFX->SetEyePosW(m_EyePosW);

	// Figure out which technique to use.
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light1Tech;
	switch (m_LightCount)
	{
	case 1:
		activeTech = Effects::BasicFX->Light1Tech;
		break;
	case 2:
		activeTech = Effects::BasicFX->Light2Tech;
		break;
	case 3:
		activeTech = Effects::BasicFX->Light3Tech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pShapesVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pShapesIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the grid.
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(m_GridWorld);
		XMMATRIX worldViewProj = m_GridWorld * viewProj;

		Effects::BasicFX->SetWorld(m_GridWorld);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(m_GridMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_GridIndexCount, m_GridIndexOffset, m_GridVertexOffset);

		// Draw the box.
		worldInvTranspose = MathHelper::InverseTranspose(m_BoxWorld);
		worldViewProj = m_BoxWorld * viewProj;

		Effects::BasicFX->SetWorld(m_BoxWorld);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(m_BoxMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_BoxIndexCount, m_BoxIndexOffset, m_BoxVertexOffset);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			worldInvTranspose = MathHelper::InverseTranspose(m_CylWorld[i]);
			worldViewProj = m_CylWorld[i] * viewProj;

			Effects::BasicFX->SetWorld(m_CylWorld[i]);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(m_CylinderMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
			m_pD3DImmediateContext->DrawIndexed(m_CylinderIndexCount, m_CylinderIndexOffset, m_CylinderVertexOffset);
		}

		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			worldInvTranspose = MathHelper::InverseTranspose(m_SphereWorld[i]);
			worldViewProj = m_SphereWorld[i] * viewProj;

			Effects::BasicFX->SetWorld(m_SphereWorld[i]);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(m_SphereMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
			m_pD3DImmediateContext->DrawIndexed(m_SphereIndexCount, m_SphereIndexOffset, m_SphereVertexOffset);
		}

		// Draw the skull.
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pSkullVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pSkullIB, DXGI_FORMAT_R32_UINT, 0);

		worldInvTranspose = MathHelper::InverseTranspose(m_SkullWorld);
		worldViewProj = m_SkullWorld * viewProj;

		Effects::BasicFX->SetWorld(m_SkullWorld);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(m_SkullMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_SkullIndexCount, 0, 0);
	}
}

void LightingSkull::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
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

	std::vector<Vertex::PosNormal> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pShapesVB));

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
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pShapesIB));
}

void LightingSkull::BuildSkullGeometryBuffers()
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

	std::vector<Vertex::PosNormal> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
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
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * vcount;
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