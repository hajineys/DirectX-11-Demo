#include "Vertex.h"
#include "Effects.h"
#include "DirectXDefine.h"

#pragma region InputLayoutDesc
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosColor[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormal[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNorTexture[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
#pragma endregion

#pragma region InputLayouts
ID3D11InputLayout* InputLayouts::PosColor = 0;
ID3D11InputLayout* InputLayouts::PosNormal = 0;
ID3D11InputLayout* InputLayouts::PosNorTexture = 0;

void InputLayouts::InitAll(ID3D11Device* device)
{
	// PosColor
	D3DX11_PASS_DESC passDesc;
	Effects::ColorFX->ColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosColor, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosColor));

	// PosNormal
	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormal, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormal));

	// PosNorTexture
	Effects::BasicDifMapFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosNorTexture, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNorTexture));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(PosNorTexture);
	ReleaseCOM(PosNormal);
	ReleaseCOM(PosColor);
}
#pragma endregion