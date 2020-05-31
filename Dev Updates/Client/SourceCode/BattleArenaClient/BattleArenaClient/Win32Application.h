#pragma once

#include "DXSample.h"

class DXSample;

class Win32Application
{
public:
    static HWND CreateWND(DXSample* pSample, HINSTANCE hInstance, UINT width, UINT height, std::wstring name);
    static int Run(int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
};
