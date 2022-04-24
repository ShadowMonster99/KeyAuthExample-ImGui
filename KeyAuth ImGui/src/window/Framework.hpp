#include <d3d9.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <D3dx9tex.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

static int CurrentPanel = 0;
static bool WindowOpen = true;

class WindowClass
{
public:
    HWND GetOverlayHWND();

    static void WindowTitle(char* name);
    static void WindowDimensions(ImVec2 dimensions);
};

//GUI helpers
void Center(float avail_width, float element_width, float padding = 15);
void Center_Text(const char* format, float spacing = 15, ImColor color = ImColor(255, 255, 255));

//Process handler, ie window destroying, changing size and other commands
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Application
{
public:
    static bool Create(void (*Handler)(void), void (*Initializer)(void));
    ~Application(); // TODO
};

