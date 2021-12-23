#include "Lighting.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "GameTimer.h"
#include "Vertex.h"
#include "Effects.h"

Lighting::Lighting(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_NumRows(0), m_NumCols(0),
	m_K1(0.0f), m_K2(0.0f), m_K3(0.0f),
	m_TimeStep(0.0f), m_SpatialStep(0.0f),
	m_pPrevSolution(), m_pCurrSolution(), m_pNormals(), m_pTangentX(),
	m_VertexCount(0), m_TriangleCount(0), m_LandIndexCount(0),
	m_pLandVB(0), m_pLandIB(0),
	m_pWavesVB(0), m_pWavesIB(0),
	m_DirLight(), m_PointLight(), m_SpotLight(),
	m_LandMat(), m_WavesMat(),
	m_LandWorld(), m_WavesWorld(), m_View(), m_Proj(),
	m_Theta(1.5f * MathHelper::Pi),
	m_Phi(0.1f * MathHelper::Pi),
	m_Radius(80.0f),
	m_EyePosW(0.0f, 0.0f, 0.0f)
{
	m_LandWorld = XMMatrixIdentity();
	m_WavesWorld = XMMatrixIdentity();

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	m_WavesWorld = wavesOffset;

	// Directional light.
	m_DirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Point light--position is changed every frame to animate in UpdateScene function.
	m_PointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_PointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.Range = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_SpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_SpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_SpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.Spot = 96.0f;
	m_SpotLight.Range = 10000.0f;

	m_LandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_LandMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_LandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_WavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_WavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_WavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

Lighting::~Lighting()
{
	delete[] m_pPrevSolution;
	delete[] m_pCurrSolution;
	delete[] m_pNormals;
	delete[] m_pTangentX;

	ReleaseCOM(m_pLandVB);
	ReleaseCOM(m_pLandIB);
	ReleaseCOM(m_pWavesVB);
	ReleaseCOM(m_pWavesIB);
}

void Lighting::Initialize(UINT m, UINT n, float dx, float dt, float speed, float damping)
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
	delete[] m_pPrevSolution;
	delete[] m_pCurrSolution;
	delete[] m_pNormals;
	delete[] m_pTangentX;

	m_pPrevSolution = new XMFLOAT3[m * n];
	m_pCurrSolution = new XMFLOAT3[m * n];
	m_pNormals = new XMFLOAT3[m * n];
	m_pTangentX = new XMFLOAT3[m * n];

	// Generate grid vertices in system memory.

	float halfWidth = (n - 1) * dx * 0.5f;
	float halfDepth = (m - 1) * dx * 0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			m_pPrevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			m_pCurrSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			m_pNormals[i * n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_pTangentX[i * n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
}

void Lighting::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer)
{
	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_EyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_View = view;
	m_Proj = proj;

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
	HR(m_pD3DImmediateContext->Map(m_pWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::PosNormal* v = reinterpret_cast<Vertex::PosNormal*>(mappedData.pData);
	for (UINT i = 0; i < m_VertexCount; ++i)
	{
		v[i].Pos = m_pCurrSolution[i];
		v[i].Normal = Normal(i);
	}

	m_pD3DImmediateContext->Unmap(m_pWavesVB, 0);

	//
	// Animate the lights.
	//

	// Circle light over the land surface.
	m_PointLight.Position.x = 70.0f * cosf(0.2f * pTimer->TotalTime());
	m_PointLight.Position.z = 70.0f * sinf(0.2f * pTimer->TotalTime());
	m_PointLight.Position.y = MathHelper::Max(GetHillHeight(m_PointLight.Position.x,
		m_PointLight.Position.z), -3.0f) + 10.0f;

	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	m_SpotLight.Position = m_EyePosW;
	XMStoreFloat3(&m_SpotLight.Direction, XMVector3Normalize(target - pos));
}

void Lighting::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosNormal);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// RenderState
	m_pD3DImmediateContext->RSSetState(m_pRenderstate);

	UINT stride = sizeof(Vertex::PosNormal);
	UINT offset = 0;

	// Set per frame constants.
	Effects::LightingFX->SetDirLight(&m_DirLight);
	Effects::LightingFX->SetPoiLight(&m_PointLight);
	Effects::LightingFX->SetSptLight(&m_SpotLight);
	Effects::LightingFX->SetEyePosW(m_EyePosW);

	// Figure out which technique to use.
	ID3DX11EffectTechnique* activeTech = Effects::LightingFX->LightTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the hills.
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pLandVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pLandIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(m_LandWorld);
		XMMATRIX worldViewProj = m_LandWorld * m_View * m_Proj;

		Effects::LightingFX->SetWorld(m_LandWorld);
		Effects::LightingFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::LightingFX->SetWorldViewProj(worldViewProj);
		Effects::LightingFX->SetMaterial(m_LandMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(m_LandIndexCount, 0, 0);

		// Draw the waves.
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pWavesVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		worldInvTranspose = MathHelper::InverseTranspose(m_WavesWorld);
		worldViewProj = m_WavesWorld * m_View * m_Proj;

		Effects::LightingFX->SetWorld(m_WavesWorld);
		Effects::LightingFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::LightingFX->SetWorldViewProj(worldViewProj);
		Effects::LightingFX->SetMaterial(m_WavesMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);
		m_pD3DImmediateContext->DrawIndexed(3 * m_TriangleCount, 0, 0);
	}
}

void Lighting::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_LandIndexCount = (UINT)grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::PosNormal> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * (UINT)grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pD3DDevice->CreateBuffer(&vbd, &vinitData, &m_pLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)m_LandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pLandIB));
}

void Lighting::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_pD3DDevice->CreateBuffer(&vbd, 0, &m_pWavesVB));


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
	HR(m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pWavesIB));
}

float Lighting::GetHillHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

DirectX::XMFLOAT3 Lighting::GetHillNormal(float x, float z) const
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

void Lighting::Disturb(UINT i, UINT j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < m_NumRows - 2);
	assert(j > 1 && j < m_NumCols - 2);

	float halfMag = 0.5f * magnitude;

	// Disturb the ijth vertex height and its neighbors.
	m_pCurrSolution[i * m_NumCols + j].y += magnitude;
	m_pCurrSolution[i * m_NumCols + j + 1].y += halfMag;
	m_pCurrSolution[i * m_NumCols + j - 1].y += halfMag;
	m_pCurrSolution[(i + 1) * m_NumCols + j].y += halfMag;
	m_pCurrSolution[(i - 1) * m_NumCols + j].y += halfMag;
}

void Lighting::UpdateWave(float deltaTime)
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

				m_pPrevSolution[i * m_NumCols + j].y =
					m_K1 * m_pPrevSolution[i * m_NumCols + j].y +
					m_K2 * m_pCurrSolution[i * m_NumCols + j].y +
					m_K3 * (m_pCurrSolution[(i + 1) * m_NumCols + j].y +
						m_pCurrSolution[(i - 1) * m_NumCols + j].y +
						m_pCurrSolution[i * m_NumCols + j + 1].y +
						m_pCurrSolution[i * m_NumCols + j - 1].y);
			}
		}

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(m_pPrevSolution, m_pCurrSolution);

		t = 0.0f; // reset time

		//
		// Compute normals using finite difference scheme.
		//
		for (UINT i = 1; i < m_NumRows - 1; ++i)
		{
			for (UINT j = 1; j < m_NumCols - 1; ++j)
			{
				float l = m_pCurrSolution[i * m_NumCols + j - 1].y;
				float r = m_pCurrSolution[i * m_NumCols + j + 1].y;
				float t = m_pCurrSolution[(i - 1) * m_NumCols + j].y;
				float b = m_pCurrSolution[(i + 1) * m_NumCols + j].y;
				m_pNormals[i * m_NumCols + j].x = -r + l;
				m_pNormals[i * m_NumCols + j].y = 2.0f * m_SpatialStep;
				m_pNormals[i * m_NumCols + j].z = b - t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_pNormals[i * m_NumCols + j]));
				XMStoreFloat3(&m_pNormals[i * m_NumCols + j], n);

				m_pTangentX[i * m_NumCols + j] = XMFLOAT3(2.0f * m_SpatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_pTangentX[i * m_NumCols + j]));
				XMStoreFloat3(&m_pTangentX[i * m_NumCols + j], T);
			}
		}
	}
}