#include "stdafx.h"
#include "ClientTest.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    ClientTest sample(1280, 720, L"Client Test");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}
