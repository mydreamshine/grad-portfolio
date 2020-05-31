#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <vector>
#include <array>
#include <wrl.h>
#include <shellapi.h>
#include <codecvt>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG
#pragma comment(lib, "Common/Util/assimp/build/x64/Debug/assimp-vc142-mtd.lib")
#pragma comment(lib, "Common/Util/libpng/lib/x64/Debug/libpng16.lib")
#else
#pragma comment(lib, "Common/Util/assimp/build/x64/Release/assimp-vc142-mt.lib")
#pragma comment(lib, "Common/Util/libpng/lib/x64/Release/libpng16.lib")
#endif

#define WND_WIDTH 640
#define WND_HEIGHT 480
#define DXGI_FORMAT_BACKBUFFER DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM
#define DXGI_FORMAT_DEPTHSTENCIL DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT

inline void getFileName(const char* str, std::string& pFileName)
{
    char _Drive[_MAX_DRIVE];
    char _Dir[_MAX_DIR];
    char _Filename[_MAX_FNAME];
    char _Ext[_MAX_EXT];
    _splitpath_s(str, _Drive, _Dir, _Filename, _Ext);
    pFileName = _Filename;
}

inline bool compareFloat(float A, float B, float epsilon = 0.000001f)
{
    return (fabs(A - B) < epsilon);
}