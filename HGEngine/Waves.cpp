#include "Waves.h"
#include "MathHelper.h"
#include "GameTimer.h"
#include "Vertex.h"
#include "Effects.h"

Waves::Waves(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS)
	: m_pD3DDevice(pDevice),
	m_pD3DImmediateContext(pDeviceContext),
	m_pRenderstate(pRS),
	m_NumRows(0), m_NumCols(0),
	m_VertexCount(0), m_TriangleCount(0),
	m_K1(0.0f), m_K2(0.0f), m_K3(0.0f),
	m_TimeStep(0.0f), m_SpatialStep(0.0f),
	m_pPrevSolution(0), m_pCurrSolution(0),
	m_pWavesVB(0), m_pWavesIB(0),
	m_World(), m_View(), m_Proj()
{
	m_World = XMMatrixIdentity();
}

Waves::~Waves()
{
	delete[] m_pPrevSolution;
	delete[] m_pCurrSolution;

	ReleaseCOM(m_pWavesVB);
	ReleaseCOM(m_pWavesIB);
}

void Waves::Initialize(UINT m, UINT n, float dx, float dt, float speed, float damping)
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

	m_pPrevSolution = new XMFLOAT3[m * n];
	m_pCurrSolution = new XMFLOAT3[m * n];

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
		}
	}

	BuildGeometryBuffers();
}

void Waves::Update(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, GameTimer* pTimer)
{
	m_View = view;
	m_Proj = proj;

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if ((pTimer->TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		Disturb(i, j, r);
	}

	UpdateWave(pTimer->DeltaTime());

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3DImmediateContext->Map(m_pWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::PosColor* v = reinterpret_cast<Vertex::PosColor*>(mappedData.pData);
	for (UINT i = 0; i < m_VertexCount; ++i)
	{
		v[i].Pos = m_pCurrSolution[i];
		v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	m_pD3DImmediateContext->Unmap(m_pWavesVB, 0);
}

void Waves::Render()
{
	m_pD3DImmediateContext->IASetInputLayout(InputLayouts::PosColor);
	m_pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
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
		m_pD3DImmediateContext->IASetVertexBuffers(0, 1, &m_pWavesVB, &stride, &offset);
		m_pD3DImmediateContext->IASetIndexBuffer(m_pWavesIB, DXGI_FORMAT_R32_UINT, 0);

		Effects::ColorFX->SetWorldViewProj(worldViewProj);

		activeTech->GetPassByIndex(p)->Apply(0, m_pD3DImmediateContext);

		//
		// Draw the waves.
		//
		m_pD3DImmediateContext->DrawIndexed(3 * m_TriangleCount, 0, 0);
	}
}

void Waves::BuildGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::PosColor) * m_VertexCount;
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

void Waves::Disturb(UINT i, UINT j, float magnitude)
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

void Waves::UpdateWave(float deltaTime)
{
	static float t = 0;

	// Accumulate time.
	t += deltaTime;

	// Only update the simulation at the specified time step.
	if (t >= m_TimeStep)
	{
		// Only update interior points; we use zero boundary conditions.
		for (DWORD i = 1; i < m_NumRows - 1; ++i)
		{
			for (DWORD j = 1; j < m_NumCols - 1; ++j)
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
	}
}