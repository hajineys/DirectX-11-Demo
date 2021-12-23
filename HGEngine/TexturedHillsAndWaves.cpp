#include "TexturedHillsAndWaves.h"
#include "MathHelper.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"
#include "GameTimer.h"
#include "Vertex.h"
#include "Effects.h"

TexturedHillsAndWaves::TexturedHillsAndWaves(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_NumRows(0), m_NumCols(0),
	m_VertexCount(0), m_TriangleCount(0),
	m_K1(0.0f), m_K2(0.0f), m_K3(0.0f),
	m_TimeStep(0.0f), m_SpatialStep(0.0f),
	m_PrevSolution(), m_CurrSolution(), m_Normals(), m_TangentX(),
	m_LandIndexCount(0),
	m_WaterTexOffset(),
	m_LandVB(0), m_LandIB(0),
	m_WavesVB(0), m_WavesIB(0),
	m_GrassMapSRV(), m_WavesMapSRV(),
	m_DirLights(),
	m_LandMat(), m_WavesMat(),
	m_GrassTexTransform(), m_WaterTexTransform(),
	m_LandWorld(), m_WavesWorld(), m_View(), m_Proj(),
	m_LightCount(7),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(80.0f),
	m_EyePosW(0.0f, 0.0f, 0.0f)
{
	XMMATRIX identity = XMMatrixIdentity();
	m_LandWorld = identity;
	m_WavesWorld = identity;

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	m_GrassTexTransform = grassTexScale;

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

	m_LandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_LandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_LandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_WavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_WavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_WavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
}

TexturedHillsAndWaves::~TexturedHillsAndWaves()
{
	delete[] m_PrevSolution;
	delete[] m_CurrSolution;
	delete[] m_Normals;
	delete[] m_TangentX;

	ReleaseCOM(m_LandVB);
	ReleaseCOM(m_LandIB);
	ReleaseCOM(m_WavesVB);
	ReleaseCOM(m_WavesIB);
	ReleaseCOM(m_GrassMapSRV);
	ReleaseCOM(m_WavesMapSRV);
}

void TexturedHillsAndWaves::Initialize(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	m_NumRows = m;
	m_NumCols = n;

	m_VertexCount = m * n;
	m_TriangleCount = (m - 1) * (n - 1) * 2;

	m_TimeStep = dt;
	m_SpatialStep = dx;

	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);
	m_K1 = (damping * dt - 2.0f) / d;
	m_K2 = (4.0f - 8.0f * e) / d;
	m_K3 = (2.0f * e) / d;

	// In case Init() called again.
	delete[] m_PrevSolution;
	delete[] m_CurrSolution;
	delete[] m_Normals;
	delete[] m_TangentX;

	m_PrevSolution = new XMFLOAT3[m * n];
	m_CurrSolution = new XMFLOAT3[m * n];
	m_Normals = new XMFLOAT3[m * n];
	m_TangentX = new XMFLOAT3[m * n];

	// Generate grid vertices in system memory.

	float halfWidth = (n - 1) * dx * 0.5f;
	float halfDepth = (m - 1) * dx * 0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			m_PrevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			m_CurrSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			m_Normals[i * n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_TangentX[i * n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}

	ID3D11Resource* texResource = nullptr;
	HR(CreateDDSTextureFromFile(m_pD3DDevice,
		L"../Resource/Textures/grass.dds", &texResource, &m_GrassMapSRV));

	HR(CreateDDSTextureFromFile(m_pD3DDevice,
		L"../Resource/Textures/water2.dds", &texResource, &m_WavesMapSRV));
	ReleaseCOM(texResource)	// view saves reference

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
}

void TexturedHillsAndWaves::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer)
{
	m_View = view;
	m_Proj = proj;

	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if ((pTimer->TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (m_NumRows - 10);
		DWORD j = 5 + rand() % (m_NumCols - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		Disturb(i, j, r);
	}

	UpdateWave(pTimer->DeltaTime());

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3DImmediateContext->Map(m_WavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::PosNorTexture* v = reinterpret_cast<Vertex::PosNorTexture*>(mappedData.pData);
	for (UINT i = 0; i < m_VertexCount; ++i)
	{
		v[i].Pos = m_PrevSolution[i];
		v[i].Normal = Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x = 0.5f + m_PrevSolution[i].x / Width();
		v[i].Tex.y = 0.5f - m_PrevSolution[i].z / Depth();
	}

	m_pD3DImmediateContext->Unmap(m_WavesVB, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	m_WaterTexOffset.y += 0.05f * pTimer->DeltaTime();
	m_WaterTexOffset.x += 0.1f * pTimer->DeltaTime();
	XMMATRIX wavesOffset = XMMatrixTranslation(m_WaterTexOffset.x, m_WaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	m_WaterTexTransform = wavesScale * wavesOffset;

	//
	// Switch the number of lights based on key presses.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		m_LightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		m_LightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		m_LightCount = 3;

	if (GetAsyncKeyState('4') & 0x8000)
		m_LightCount = 4;

	if (GetAsyncKeyState('5') & 0x8000)
		m_LightCount = 5;

	if (GetAsyncKeyState('6') & 0x8000)
		m_LightCount = 6;

	if (GetAsyncKeyState('7') & 0x8000)
		m_LightCount = 7;
}

void TexturedHillsAndWaves::Render()
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

	ID3DX11EffectTechnique* activeTech = Effects::BasicDifMapFX->Light1TexTech;
	switch (m_LightCount)
	{
	case 1:
		activeTech = Effects::BasicDifMapFX->Light1Tech;
		break;
	case 2:
		activeTech = Effects::BasicDifMapFX->Light2Tech;
		break;
	case 3:
		activeTech = Effects::BasicDifMapFX->Light3Tech;
		break;
	case 4:
		activeTech = Effects::BasicDifMapFX->Light0TexTech;
		break;
	case 5:
		activeTech = Effects::BasicDifMapFX->Light1TexTech;
		break;
	case 6:
		activeTech = Effects::BasicDifMapFX->Light2TexTech;
		break;
	case 7:
		activeTech = Effects::BasicDifMapFX->Light3TexTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the hills.
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_LandVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_LandIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(m_LandWorld);
		XMMATRIX worldViewProj = m_LandWorld * viewProj;

		Effects::BasicDifMapFX->SetWorld(m_LandWorld);
		Effects::BasicDifMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicDifMapFX->SetWorldViewProj(worldViewProj);
		Effects::BasicDifMapFX->SetTexTransform(m_GrassTexTransform);
		Effects::BasicDifMapFX->SetMaterial(m_LandMat);
		Effects::BasicDifMapFX->SetDiffuseMap(m_GrassMapSRV);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_LandIndexCount, 0, 0);

		// Draw the waves.
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_WavesVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_WavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		worldInvTranspose = MathHelper::InverseTranspose(m_WavesWorld);
		worldViewProj = m_WavesWorld * viewProj;

		Effects::BasicDifMapFX->SetWorld(m_WavesWorld);
		Effects::BasicDifMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicDifMapFX->SetWorldViewProj(worldViewProj);
		Effects::BasicDifMapFX->SetTexTransform(m_WaterTexTransform);
		Effects::BasicDifMapFX->SetMaterial(m_WavesMat);
		Effects::BasicDifMapFX->SetDiffuseMap(m_WavesMapSRV);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(3 * m_TriangleCount, 0, 0);
	}
}

void TexturedHillsAndWaves::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_LandIndexCount = (UINT)grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::PosNorTexture> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNorTexture) * (UINT)grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_LandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_LandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_LandIB));
}

void TexturedHillsAndWaves::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::PosNorTexture) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_pD3DDevice->CreateBuffer(&vbd, 0, &m_WavesVB));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3 * m_TriangleCount); // 3 indices per face

	// Iterate over each quad.
	UINT m = m_NumRows;
	UINT n = m_NumCols;
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_WavesIB));
}

float TexturedHillsAndWaves::GetHillHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

DirectX::XMFLOAT3 TexturedHillsAndWaves::GetHillNormal(float x, float z) const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

float TexturedHillsAndWaves::Width() const
{
	return m_NumCols * m_SpatialStep;
}

float TexturedHillsAndWaves::Depth() const
{
	return m_NumCols * m_SpatialStep;
}

void TexturedHillsAndWaves::Disturb(UINT i, UINT j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < m_NumRows - 2);
	assert(j > 1 && j < m_NumCols - 2);

	float halfMag = 0.5f * magnitude;

	// Disturb the ijth vertex height and its neighbors.
	m_CurrSolution[i * m_NumCols + j].y += magnitude;
	m_CurrSolution[i * m_NumCols + j + 1].y += halfMag;
	m_CurrSolution[i * m_NumCols + j - 1].y += halfMag;
	m_CurrSolution[(i + 1) * m_NumCols + j].y += halfMag;
	m_CurrSolution[(i - 1) * m_NumCols + j].y += halfMag;
}

void TexturedHillsAndWaves::UpdateWave(float deltaTime)
{
	static float t = 0;

	// Accumulate time.
	t += deltaTime;

	// Only update the simulation at the specified time step.
	if (t >= m_TimeStep)
	{
		// Only update interior points; we use zero boundary conditions.
		for (UINT i = 1; i < m_NumRows - 1; ++i)
		{
			for (UINT j = 1; j < m_NumCols - 1; ++j)
			{
				// After this update we will be discarding the old previous
				// buffer, so overwrite that buffer with the new update.
				// Note how we can do this inplace (read/write to same element) 
				// because we won't need prev_ij again and the assignment happens last.

				// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				// Moreover, our +z axis goes "down"; this is just to 
				// keep consistent with our row indices going down.

				m_PrevSolution[i * m_NumCols + j].y =
					m_K1 * m_PrevSolution[i * m_NumCols + j].y +
					m_K2 * m_CurrSolution[i * m_NumCols + j].y +
					m_K3 * (m_CurrSolution[(i + 1) * m_NumCols + j].y +
						m_CurrSolution[(i - 1) * m_NumCols + j].y +
						m_CurrSolution[i * m_NumCols + j + 1].y +
						m_CurrSolution[i * m_NumCols + j - 1].y);
			}
		}

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(m_PrevSolution, m_CurrSolution);

		t = 0.0f; // reset time

		//
		// Compute normals using finite difference scheme.
		//
		for (UINT i = 1; i < m_NumRows - 1; ++i)
		{
			for (UINT j = 1; j < m_NumCols - 1; ++j)
			{
				float l = m_CurrSolution[i * m_NumCols + j - 1].y;
				float r = m_CurrSolution[i * m_NumCols + j + 1].y;
				float t = m_CurrSolution[(i - 1) * m_NumCols + j].y;
				float b = m_CurrSolution[(i + 1) * m_NumCols + j].y;
				m_Normals[i * m_NumCols + j].x = -r + l;
				m_Normals[i * m_NumCols + j].y = 2.0f * m_SpatialStep;
				m_Normals[i * m_NumCols + j].z = b - t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_Normals[i * m_NumCols + j]));
				XMStoreFloat3(&m_Normals[i * m_NumCols + j], n);

				m_TangentX[i * m_NumCols + j] = XMFLOAT3(2.0f * m_SpatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_TangentX[i * m_NumCols + j]));
				XMStoreFloat3(&m_TangentX[i * m_NumCols + j], T);
			}
		}
	}
}