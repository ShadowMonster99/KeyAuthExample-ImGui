#include <window/Framework.hpp>
#include <string>

#pragma region DirectX

struct OverlayWindow 
{
    WNDCLASSEX WindowClass;
    HWND Hwnd = NULL;
    LPCSTR Name = NULL;
}Overlay;

HWND WindowClass::GetOverlayHWND(){return Overlay.Hwnd;}

struct DirectX9Interface 
{
    LPDIRECT3D9 IDirect3D9 = NULL;
    LPDIRECT3DDEVICE9 pDevice = NULL;
    D3DPRESENT_PARAMETERS pParameters;
    MSG Message;
}DirectX9;

#pragma endregion

struct AppInfo
{
    std::string Title;
    uint32_t width = { 1215 };
    uint32_t height = { 850 };
}appinfo;

void WindowClass::WindowTitle(char* name)
{
    appinfo.Title = name;
}
void WindowClass::WindowDimensions(ImVec2 dimensions)
{
    appinfo.width = dimensions.x;
    appinfo.height = dimensions.y;
}

bool CreateDeviceD3D(HWND hWnd) {
    if ((DirectX9.IDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
        return false;
    }
    ZeroMemory(&DirectX9.pParameters, sizeof(DirectX9.pParameters));
    DirectX9.pParameters.Windowed = TRUE;
    DirectX9.pParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    DirectX9.pParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    DirectX9.pParameters.EnableAutoDepthStencil = TRUE;
    DirectX9.pParameters.AutoDepthStencilFormat = D3DFMT_D16;
    DirectX9.pParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (DirectX9.IDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DirectX9.pParameters, &DirectX9.pDevice) < 0) {
        return false;
    }
    return true;
}

void ClearAll() 
{ 
    if (DirectX9.pDevice) {
        DirectX9.pDevice->Release();
        DirectX9.pDevice = NULL;
    }

    if (DirectX9.IDirect3D9) {
        DirectX9.IDirect3D9->Release();
        DirectX9.IDirect3D9 = NULL;
    }

    UnregisterClass(Overlay.WindowClass.lpszClassName, Overlay.WindowClass.hInstance); 
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    if (DirectX9.pDevice->Reset(&DirectX9.pParameters) == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Center(float avail_width, float element_width, float padding)
{
    ImGui::SameLine((avail_width / 2) - (element_width / 2) + padding);
}

void Center_Text(const char* format, float spacing, ImColor color) {
    Center(ImGui::GetContentRegionAvail().x, ImGui::CalcTextSize(format).x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
    ImGui::TextColored(color.Value, format);
}

static ImVec2 ScreenRes, WindowPos;

bool Application::Create(void (*Handler)(void), void (*Initializer)(void))
{
    Overlay.Name = { appinfo.Title.c_str() };
    Overlay.WindowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, Overlay.Name, NULL };

    (ATOM)RegisterClassExA(&Overlay.WindowClass);

    Overlay.Hwnd = CreateWindow(Overlay.Name, Overlay.Name, WS_POPUP, 0, 0, appinfo.width, appinfo.height, NULL, NULL, Overlay.WindowClass.hInstance, NULL);
    if (CreateDeviceD3D(Overlay.Hwnd) == FALSE) { ClearAll(); }

    ImGui::CreateContext();

    ImGuiIO* IO = &ImGui::GetIO();
    IO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if (ImGui_ImplWin32_Init(Overlay.Hwnd) && ImGui_ImplDX9_Init(DirectX9.pDevice))
    {
        Initializer();
        memset((&DirectX9.Message), 0, (sizeof(DirectX9.Message)));

        //get direct x message and creates application loop while the device isnt exiting
        while (DirectX9.Message.message != WM_QUIT)
        {
            if (PeekMessageA(&DirectX9.Message, NULL, (UINT)0U, (UINT)0U, PM_REMOVE)) 
            { 
                TranslateMessage(&DirectX9.Message); 
                DispatchMessageA(&DirectX9.Message); 
                continue; 
            }

            ImGui_ImplDX9_NewFrame(); ImGui_ImplWin32_NewFrame();

            RECT ScreenRect;
            GetWindowRect(GetDesktopWindow(), &ScreenRect);
            ScreenRes = ImVec2(float(ScreenRect.right), float(ScreenRect.bottom));

            WindowPos.x = (ScreenRes.x - appinfo.width) * 0.5f;
            WindowPos.y = (ScreenRes.y - appinfo.height) * 0.5f;

            //create new application frame
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(WindowPos.x, WindowPos.y), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(appinfo.width, appinfo.height), ImGuiCond_Once);

            static ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

            //Create Application Context
            ImGui::Begin(appinfo.Title.c_str(), &WindowOpen, WindowFlags); Handler(); ImGui::End();

            ImGui::EndFrame();

            //cleanup, exit device and close window
            DirectX9.pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

            if (DirectX9.pDevice->BeginScene() >= 0) { ImGui::Render(); ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData()); DirectX9.pDevice->EndScene(); }
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault();
            if (DirectX9.pDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST && DirectX9.pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();

            if (!WindowOpen) exit(0);
        }
    }
    else return false;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (DirectX9.pDevice != NULL && wParam != SIZE_MINIMIZED) {
            DirectX9.pParameters.BackBufferWidth = LOWORD(lParam);
            DirectX9.pParameters.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

