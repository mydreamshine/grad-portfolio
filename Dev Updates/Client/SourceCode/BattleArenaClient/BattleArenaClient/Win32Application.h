#pragma once

#include "DXSample.h"
#include "EventProcessor.h"
#include "..\..\..\..\Server\Common\NWMODULE.h"

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
    static NWMODULE nwmodule;
    static EventProcessor m_eventProcessor;
    static std::queue<std::unique_ptr<packet_inheritance>> m_packetList;

};
