#include "overlay.h"

Memory m;
Overlay Ov;
HWND hwnd;
RECT GameSize;
POINT GamePos;

bool Run = true;
bool ShowMenu = false;
bool m_esp = true;
bool SwayEditor = false;
float SwayV = 0.95f;
bool UnlockAll = false;
extern bool Alive;

bool IsKeyDown(int VK);
void CheatFunc();
void CThread();

int main(int, char**)
{
    SetConsoleTitleA("Battlefield 4 [ unknowncheats.me ]");

    m.Ghwnd = FindWindowA(NULL, "Battlefield 4");
    if (!m.Ghwnd)
    {
        printf("[-] Please open BF4...\n");

        while (!m.Ghwnd)
        {
            m.Ghwnd = FindWindowA(NULL, "Battlefield 4");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Get window info
    GetClassNameA(m.Ghwnd, m.ClassName, sizeof(m.ClassName));
    GetWindowTextA(m.Ghwnd, m.MenuName, sizeof(m.MenuName));
    GetClientRect(m.Ghwnd, &GameSize);
    ClientToScreen(m.Ghwnd, &GamePos);

    GetWindowThreadProcessId(m.Ghwnd, &m.PID);
    m.pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m.PID);

    if (!m.pHandle)
    {
        printf("[-] Can't get pHandle\n");
        system("PAUSE");
        return 0;
    }

    printf("[+] Ready!\n");
    system("cls");
    printf("MenuKey : Insert");

    // Create Overlay
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), 0, WndProc, 0, 0, NULL, NULL, NULL, NULL, "GG", "WP", NULL };
    RegisterClassExA(&wc);
    hwnd = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, wc.lpszMenuName, WS_POPUP, GamePos.x, GamePos.y, GameSize.right, GameSize.bottom, NULL, NULL, wc.hInstance, NULL); // ToDo : Set your screen size
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImStyle();

    // Our state
    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 0.f);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CThread, NULL, 0, NULL);

    // Main loop
    while (Run)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                Run = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Ov.Info();

        if (m_esp)
            Ov.ESP();

        if (ShowMenu)
            Ov.Menu();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    return 0;
}

void CheatFunc()
{
    // Recoil / Spread
    if (SwayEditor && Alive)
    {
        DWORD_PTR Weapon = m.RPM<DWORD_PTR>(OFFSET_WEAPON);

        // Sway
        DWORD_PTR weaponPtr = m.RPM<DWORD_PTR>(Weapon + 0x128);
        weaponPtr = m.RPM<DWORD_PTR>(weaponPtr + 0x30);

        if (weaponPtr && m.RPM<float>(weaponPtr + 0x430) != SwayV)
        {
            m.WPM<float>(weaponPtr + 0x430, SwayV);
            m.WPM<float>(weaponPtr + 0x438, SwayV);
            m.WPM<float>(weaponPtr + 0x434, SwayV);
            m.WPM<float>(weaponPtr + 0x43C, SwayV);
        }
    }

    // UnlockAll
    if (UnlockAll)
    {
        DWORD_PTR SyncBFSetting = m.RPM<DWORD_PTR>(OFFSET_SYNCBFSETTING);

        if (UnlockAll && m.RPM<bool>(SyncBFSetting + 0x54) == false)
            m.WPM<bool>(SyncBFSetting + 0x54, true);
    }
}

void CThread()
{
    while (Run)
    {
        // ESP
        static bool tmp = false;
        static bool change = false;
        if (IsKeyDown(VK_INSERT) && !tmp)
        {
            ShowMenu = !ShowMenu;
            tmp = true;
            change = false;
        }
        else if (!IsKeyDown(VK_INSERT) && tmp)
        {
            tmp = false;
        }

        // ウィンドウスタイルの変更
        if (ShowMenu && !change)
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
            change = true;

        }
        else if (!change)
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
            change = true;
        }

        CheatFunc();

        HWND bf4 = FindWindow(NULL, L"Battlefield 4");
        if (bf4)
        {
            RECT tRect = {};
            POINT tPos = {};
            GetClientRect(bf4, &tRect);
            ClientToScreen(bf4, &tPos);

            if (GamePos.x != tPos.x || GamePos.x != tPos.y || GameSize.left != tRect.left || GameSize.bottom != tRect.bottom)
            {
                SetWindowPos(hwnd, NULL, tPos.x, tPos.y, tRect.right, tRect.bottom, SWP_NOREDRAW);

                GameSize = tRect;
                GamePos = tPos;
            }
        }
        else  
            Run = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ExitThread(0);
}

bool IsKeyDown(int VK)
{
    return (GetAsyncKeyState(VK) & 0x8000) != 0;
}