#include "stdafx.h"
#include "Framework.h"
#include "ResourceManager.h"
#include <crtdbg.h>

#ifdef _DEBUG
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    std::unique_ptr<ResourceManager> resoruceManager = std::make_unique<ResourceManager>();
    resoruceManager->OnInit();
    resoruceManager->LoadAsset();

    std::unique_ptr <Framework> sample = std::make_unique<Framework>();
    HWND hWnd = Win32Application::CreateWND(sample.get(), hInstance, WND_WIDTH, WND_HEIGHT, L"Client Test");
    sample->OnInit(hWnd, WND_WIDTH, WND_HEIGHT, L"Client Test", resoruceManager.get());

    Framework dummySamples[4];
    for (int i = 0; i < 4; ++i)
        dummySamples[i].OnInit(0, WND_WIDTH, WND_HEIGHT, L"Dummy Framework" + std::to_wstring(i), resoruceManager.get());

    int ret_num = Win32Application::Run(nCmdShow);

    sample->OnDestroy();
    resoruceManager->OnDestroy();

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    return ret_num;
}
