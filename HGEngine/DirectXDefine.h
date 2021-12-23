#pragma once
#include <windows.h>
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <cassert>
#include <fstream>
#include <string>
#include <vector>


/// DirectX 11
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <d3dcompiler.h>			// fx compile
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")	// adapter info


/// Effects(FX)
#ifdef _DEBUG
	#ifdef _WIN64
		#pragma comment(lib, "Effects11_x64_Debug.lib")
	#else
		#pragma comment(lib, "Effects11_Win32_Debug.lib")
	#endif
#else	// Release
	#ifdef _WIN64
		#pragma comment(lib, "Effects11_x64_Release.lib")
	#else
		#pragma comment(lib, "Effects11_Win32_Release.lib")
	#endif
#endif

/// DXTK
#ifdef _DEBUG
	#ifdef _WIN64
		#pragma comment(lib, "DirectXTK_x64_Debug.lib")
	#else
		#pragma comment(lib, "DirectXTK_Win32_Debug.lib")
	#endif
#else	// Release
	#ifdef _WIN64
		#pragma comment(lib, "DirectXTK_x64_Release.lib")
	#else
		#pragma comment(lib, "DirectXTK_Win32_Release.lib")
	#endif
#endif


/// Simple d3d error checker by LEHIDE
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
#define HR(x)													\
		{														\
			HRESULT hr = (x);									\
			if(FAILED(hr))										\
			{													\
				LPWSTR output;									\
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
					FORMAT_MESSAGE_IGNORE_INSERTS 	 |			\
					FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
					NULL,										\
					hr,											\
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
					(LPTSTR) &output,							\
					0,											\
					NULL);										\
				MessageBox(NULL, output, L"Error", MB_OK);		\
			}													\
		}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif


/// Convenience macro for releasing COM objects.
#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }
#define SAFE_RELEASE(x) { if(x){ x->Release(); x = 0; } }	// 예전 스타일


/// Convenience macro for deleting objects.
#define SafeDelete(x) { delete x; x = 0; }


// #define XMGLOBALCONST extern CONST __declspec(selectany)
//   1. extern so there is only one copy of the variable, and not a separate
//      private copy in each .obj.
//   2. __declspec(selectany) so that the compiler does not complain about
//      multiple definitions in a .cpp file (it can pick anyone and discard 
//      the rest because they are constant--all the same).
namespace DirectXColors
{
	XMGLOBALCONST DirectX::XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST DirectX::XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

	XMGLOBALCONST DirectX::XMVECTORF32 CustomGrey = { 0.2f, 0.2f, 0.2f, 1.0f };
}