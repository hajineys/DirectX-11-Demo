#pragma once
//***************************************************************************************
// Vertex.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines vertex structures and input layouts.
//***************************************************************************************
#ifndef VERTEX_H
#define VERTEX_H

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

namespace Vertex
{
	struct PosColor
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	struct PosNormal
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};

	struct PosNorTexture
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC PosColor[2];
	static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
	static const D3D11_INPUT_ELEMENT_DESC PosNorTexture[3];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* PosColor;
	static ID3D11InputLayout* PosNormal;
	static ID3D11InputLayout* PosNorTexture;
};

#endif // VERTEX_H