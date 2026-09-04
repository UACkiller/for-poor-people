#include "globals.h"
#include "helpers.h"
#include "resolve.h"
#include "entity.h"
#include "hooks.h"
#include "../kiero/kiero.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include <d3d11.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IL2CPP_API       il2cpp;
HMODULE          g_hModule   = nullptr;
volatile bool    g_Running   = true;

std::vector<Entity> g_Entities;

Resolved g_R;

fn_get_position   g_fp_getPos  = nullptr;
fn_get_transform  g_fp_getTr   = nullptr;
fn_camera_main    g_fp_camMain = nullptr;
fn_w2s            g_fp_w2s     = nullptr;
fn_screen_int     g_fp_scrW    = nullptr;
fn_screen_int     g_fp_scrH    = nullptr;
bool g_directCalls = false;

const MethodInfo* g_fot = nullptr;
int g_fotArgs = 0;

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static ID3D11RenderTargetView* g_mainRT = nullptr;
static HWND g_hWnd = nullptr;
static WNDPROC g_oWndProc = nullptr;
typedef HRESULT(__stdcall* fnPresent)(IDXGISwapChain*, UINT, UINT);
static fnPresent g_oPresent = nullptr;

static bool g_ShowMenu = false;
static bool g_Tracers = true, g_Chests = true, g_Stones = true, g_Circles = true;
static float g_ColorP[3] = {1,0,0}, g_ColorC[3] = {0,1,0}, g_ColorS[3] = {0,0,1}, g_ColorCir[3] = {1,0,1};
static float g_MaxPlayerDist = 60.0f;

extern volatile float g_ZoomOverride;
extern float g_ZoomDefault;
extern bool  g_ZoomHooked;

static bool IsChestType(int entityType) {
    return entityType == 59;
}

static bool IsSoulStoneType(int entityType) {
    return entityType == 25;
}

static bool IsCircleType(int entityType) {
    return entityType == 196;
}

struct Hotkey {
    int key; bool prev;
    bool pressed() { bool cur = GetAsyncKeyState(key) & 0x8000; bool r = cur && !prev; prev = cur; return r; }
};
static Hotkey hkMenu = { VK_RSHIFT };

LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_ShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return 1;
    return CallWindowProcW(g_oWndProc, hWnd, msg, wParam, lParam);
}

static HRESULT __stdcall hkPresent(IDXGISwapChain* sc, UINT sync, UINT flags) {
    __try {
        static bool once = false;
        if (!once) {
            if (SUCCEEDED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice))) {
                g_pDevice->GetImmediateContext(&g_pContext);
                DXGI_SWAP_CHAIN_DESC d; sc->GetDesc(&d);
                g_hWnd = d.OutputWindow;
                ID3D11Texture2D* bb = nullptr;
                sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb);
                g_pDevice->CreateRenderTargetView(bb, nullptr, &g_mainRT);
                bb->Release();
                g_oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
                ImGui::CreateContext();
                ImGui_ImplWin32_Init(g_hWnd);
                ImGui_ImplDX11_Init(g_pDevice, g_pContext);
                ImGui::StyleColorsDark();
                once = true;
            } else return g_oPresent(sc, sync, flags);
        }

        if (hkMenu.pressed()) g_ShowMenu = !g_ShowMenu;

        if (g_ZoomHooked) {
            if (GetAsyncKeyState(VK_F1) & 1) { g_ZoomOverride -= 1.0f; if (g_ZoomOverride < 1.0f) g_ZoomOverride = 1.0f; }
            if (GetAsyncKeyState(VK_F2) & 1) { g_ZoomOverride += 1.0f; if (g_ZoomOverride > 100.0f) g_ZoomOverride = 100.0f; }
        }

        ImVec2 org(0, 0);
        for (auto& e : g_Entities) {
            if (e.isLocal) {
                org = ImVec2(e.screenPos.x, e.screenPos.y);
                break;
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (org.x != 0 || org.y != 0) {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            for (auto& e : g_Entities) {
                if (!e.onScreen || e.isLocal) continue;
                ImU32 col = 0;
                bool draw = false;
                if (e.isPlayer && g_Tracers && e.distance <= g_MaxPlayerDist) { col = ImGui::ColorConvertFloat4ToU32(ImVec4(g_ColorP[0],g_ColorP[1],g_ColorP[2],1)); draw = true; }
                else if (IsChestType(e.entityType) && g_Chests) { col = ImGui::ColorConvertFloat4ToU32(ImVec4(g_ColorC[0],g_ColorC[1],g_ColorC[2],1)); draw = true; }
                else if (IsSoulStoneType(e.entityType) && g_Stones) { col = ImGui::ColorConvertFloat4ToU32(ImVec4(g_ColorS[0],g_ColorS[1],g_ColorS[2],1)); draw = true; }
                else if (IsCircleType(e.entityType) && g_Circles) { col = ImGui::ColorConvertFloat4ToU32(ImVec4(g_ColorCir[0],g_ColorCir[1],g_ColorCir[2],1)); draw = true; }
                if (draw) dl->AddLine(org, ImVec2(e.screenPos.x, e.screenPos.y), col, 2.0f);
            }
        }

        if (g_ShowMenu) {
            ImGui::SetNextWindowSize(ImVec2(220, 0), ImGuiCond_Always);
            ImGui::Begin("Tracers", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::Checkbox("Players", &g_Tracers); ImGui::SameLine(140);
            ImGui::ColorEdit3("##p", g_ColorP, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::Checkbox("Chests", &g_Chests); ImGui::SameLine(140);
            ImGui::ColorEdit3("##c", g_ColorC, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::Checkbox("Soul Stone", &g_Stones); ImGui::SameLine(140);
            ImGui::ColorEdit3("##s", g_ColorS, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::Checkbox("Circle of Return", &g_Circles); ImGui::SameLine(140);
            ImGui::ColorEdit3("##cir", g_ColorCir, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::Separator();
            ImGui::TextDisabled("RShift - menu");
            ImGui::Separator();
            ImGui::Text("Camera Zoom");
            if (g_ZoomHooked) {
                float zoomVal = g_ZoomOverride;
                if (ImGui::SliderFloat("##zoom", &zoomVal, 1.0f, 100.0f, "Zoom: %.1f"))
                    g_ZoomOverride = zoomVal;
                ImGui::TextDisabled("F1=zoom in  F2=zoom out");
            }
            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        g_pContext->OMSetRenderTargets(1, &g_mainRT, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    return g_oPresent(sc, sync, flags);
}

DWORD WINAPI MainThread(LPVOID) {
    HMODULE ga = GetModuleHandleA("GameAssembly.dll");
    if (!ga) {
        while (!(ga = GetModuleHandleA("GameAssembly.dll"))) { Sleep(500); }
        Sleep(3000);
    } else Sleep(1000);

    if (!il2cpp.init()) return 0;
    il2cpp.thread_attach(il2cpp.domain_get());
    if (!ResolveAll()) return 0;
    InitDirectCalls();
    InstallCameraViewHook();
    DisableFogOfWar();

    do {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(8, (void**)&g_oPresent, hkPresent);
            break;
        }
        Sleep(500);
    } while (true);

    while (g_Running) {
        Sleep(30);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hMod;
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
