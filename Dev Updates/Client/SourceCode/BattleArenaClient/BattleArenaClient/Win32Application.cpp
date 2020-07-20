#include "stdafx.h"
#include "Win32Application.h"


HWND Win32Application::m_hwnd = nullptr;
EventProcessor Win32Application::m_eventProcessor;

std::queue<std::unique_ptr<packet_inheritance>> Win32Application::m_SendpacketList;
std::queue<std::unique_ptr<packet_inheritance>> Win32Application::m_RecvpacketList;

HWND Win32Application::CreateWND(DXSample* pSample, HINSTANCE hInstance, UINT width, UINT height, std::wstring name)
{

    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pSample->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(WND_WIDTH), static_cast<LONG>(WND_HEIGHT) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pSample->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        pSample);

    //m_nwmodule = new NWMODULE<EventProcessor>{ m_eventProcessor };
    //for (int i = 0; i < SSCS_PACKET_COUNT; ++i)
    //    m_nwmodule->enroll_callback(i, &EventProcessor::PacketToEvent);
    //m_nwmodule->connect_lobby(0);

    return m_hwnd;
}

int Win32Application::Run(int nCmdShow)
{
    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    static bool ChangeWndSize = false;

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN)
        {
            if (message == WM_LBUTTONDOWN) wParam = VK_LBUTTON;
            else if (message == WM_RBUTTONDOWN) wParam = VK_RBUTTON;
            POINT OldCursorPos;
            ::SetCapture(m_hwnd);
            ::GetCursorPos(&OldCursorPos);
            ::ScreenToClient(m_hwnd, &OldCursorPos);

            if (pSample) pSample->OnKeyDown(static_cast<UINT8>(wParam), &OldCursorPos);
        }
        else
        {
            if (pSample) pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (message == WM_LBUTTONUP || message == WM_RBUTTONUP)
        {
            if (message == WM_LBUTTONUP) wParam = VK_LBUTTON;
            else if (message == WM_RBUTTONUP) wParam = VK_RBUTTON;
            //마우스 캡쳐를 해제한다.
            ::ReleaseCapture();
        }
        if (pSample) pSample->OnKeyUp(static_cast<UINT8>(wParam));
        return 0;

    case WM_SIZE:
        ChangeWndSize = true;
        return 0;

    case WM_PAINT:
        if (pSample)
        {
            m_eventProcessor.GenerateExternalEventsFrom();
            pSample->ProcessEvents(m_eventProcessor.GetExternalEvents());

            std::queue<std::unique_ptr<EVENT>> GeneratedEvents;
            if (ChangeWndSize == true)
            {
                RECT ClientRect;
                ::GetClientRect(m_hwnd, &ClientRect);
                pSample->OnUpdate(GeneratedEvents, &ClientRect);
            }
            else pSample->OnUpdate(GeneratedEvents);
            pSample->OnRender();
            m_eventProcessor.ProcessGeneratedEvents(GeneratedEvents);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
