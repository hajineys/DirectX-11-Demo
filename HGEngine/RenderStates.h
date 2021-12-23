#pragma once
#include "DirectXDefine.h"

/// <summary>
/// RenderState Create Class
/// 
/// 2021. 12. 16 Hamdal
/// </summary>
class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* SolidRS;
	static ID3D11RasterizerState* NoCullRS;

	static ID3D11DepthStencilState* NormalDSS;

	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBS;
};