//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************
#include "Effects.h"
#include "DirectXDefine.h"

Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: m_FX(0)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, device, &m_FX));
}

Effect::~Effect()
{
	ReleaseCOM(m_FX);
}

#pragma region ColorEffect
ColorEffect::ColorEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	ColorTech = m_FX->GetTechniqueByName("ColorTech");

	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

ColorEffect::~ColorEffect()
{

}
#pragma endregion

#pragma region LightingEffect
LightingEffect::LightingEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	LightTech = m_FX->GetTechniqueByName("LightTech");

	WorldViewProj		= m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World				= m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose	= m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	EyePosW				= m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLight			= m_FX->GetVariableByName("gDirLight");
	PoiLight			= m_FX->GetVariableByName("gPointLight");
	SptLight			= m_FX->GetVariableByName("gSpotLight");

	Mat = m_FX->GetVariableByName("gMaterial");
}

LightingEffect::~LightingEffect()
{

}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = m_FX->GetTechniqueByName("Light1");
	Light2Tech = m_FX->GetTechniqueByName("Light2");
	Light3Tech = m_FX->GetTechniqueByName("Light3");

	WorldViewProj		= m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World				= m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose	= m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	EyePosW				= m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLights			= m_FX->GetVariableByName("gDirLights");

	Mat = m_FX->GetVariableByName("gMaterial");
}

BasicEffect::~BasicEffect()
{

}
#pragma endregion

#pragma region BasicDifMapEffect
BasicDifMapEffect::BasicDifMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = m_FX->GetTechniqueByName("Light1");
	Light2Tech = m_FX->GetTechniqueByName("Light2");
	Light3Tech = m_FX->GetTechniqueByName("Light3");

	Light0TexTech = m_FX->GetTechniqueByName("Light0Tex");
	Light1TexTech = m_FX->GetTechniqueByName("Light1Tex");
	Light2TexTech = m_FX->GetTechniqueByName("Light2Tex");
	Light3TexTech = m_FX->GetTechniqueByName("Light3Tex");

	WorldViewProj		= m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World				= m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose	= m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform		= m_FX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW				= m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLights			= m_FX->GetVariableByName("gDirLights");

	Mat = m_FX->GetVariableByName("gMaterial");

	DiffuseMap = m_FX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicDifMapEffect::~BasicDifMapEffect()
{

}
#pragma endregion

#pragma region BasicDifMapEffect
BasicTexAlphaEffect::BasicTexAlphaEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = m_FX->GetTechniqueByName("Light1");
	Light2Tech = m_FX->GetTechniqueByName("Light2");
	Light3Tech = m_FX->GetTechniqueByName("Light3");

	Light0TexTech = m_FX->GetTechniqueByName("Light0Tex");
	Light1TexTech = m_FX->GetTechniqueByName("Light1Tex");
	Light2TexTech = m_FX->GetTechniqueByName("Light2Tex");
	Light3TexTech = m_FX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = m_FX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = m_FX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = m_FX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = m_FX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech = m_FX->GetTechniqueByName("Light1Fog");
	Light2FogTech = m_FX->GetTechniqueByName("Light2Fog");
	Light3FogTech = m_FX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = m_FX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = m_FX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = m_FX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = m_FX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light3TexAlphaClipFog");

	WorldViewProj		= m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World				= m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose	= m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform		= m_FX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW				= m_FX->GetVariableByName("gEyePosW")->AsVector();
	FogColor			= m_FX->GetVariableByName("gFogColor")->AsVector();
	FogStart			= m_FX->GetVariableByName("gFogStart")->AsScalar();
	FogRange			= m_FX->GetVariableByName("gFogRange")->AsScalar();
	DirLights			= m_FX->GetVariableByName("gDirLights");

	Mat = m_FX->GetVariableByName("gMaterial");

	DiffuseMap = m_FX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicTexAlphaEffect::~BasicTexAlphaEffect()
{

}
#pragma endregion

#pragma region Effects
ColorEffect* Effects::ColorFX = 0;
LightingEffect* Effects::LightingFX = 0;
BasicEffect* Effects::BasicFX = 0;
BasicDifMapEffect* Effects::BasicDifMapFX = 0;
BasicTexAlphaEffect* Effects::BasicTexAlphaFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	ColorFX = new ColorEffect(device, L"../FX/Color.cso");
	LightingFX = new LightingEffect(device, L"../FX/Lighting.cso");
	BasicFX = new BasicEffect(device, L"../FX/Basic.cso");
	BasicDifMapFX = new BasicDifMapEffect(device, L"../FX/BasicDifMap.cso");
	BasicTexAlphaFX = new BasicTexAlphaEffect(device, L"../FX/BasicTexAlpha.cso");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicTexAlphaFX);
	SafeDelete(BasicDifMapFX);
	SafeDelete(BasicFX);
	SafeDelete(LightingFX);
	SafeDelete(ColorFX);
}
#pragma endregion