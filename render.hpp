#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <Windows.h>
#include <dwmapi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <TlHelp32.h>

#include "settings.hpp"
#include "game.hpp"
#include "particles.hpp"
#include "xorstr.h"
#include "myfont.h"

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static float menuAlpha = 0.0f;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Function Definitions (moved before render() to resolve "identifier not found" errors)
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

float Clamp(const float& value, const float& min, const float& max) {
    return (value < min) ? min : (value > max) ? max : value;
}

bool Keybind(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(keybind->get_name().c_str());
    const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        style.FramePadding.y < window->DC.CurrLineTextBaseOffset)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(
        size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    bool value_changed = false;
    int key = keybind->key;

    auto io = ImGui::GetIO();

    std::string name = keybind->get_key_name();

    if (keybind->waiting_for_input)
        name = "waiting";

    if (ImGui::GetIO().MouseClicked[0] && hovered)
    {
        if (g.ActiveId == id)
        {
            keybind->waiting_for_input = true;
        }
    }
    else if (ImGui::GetIO().MouseClicked[1] && hovered)
    {
        ImGui::OpenPopup(keybind->get_name().c_str());
    }
    else if (ImGui::GetIO().MouseClicked[0] && !hovered)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    if (keybind->waiting_for_input)
    {
        if (ImGui::GetIO().MouseClicked[0] && !hovered)
        {
            keybind->key = VK_LBUTTON;
            ImGui::ClearActiveID();
            keybind->waiting_for_input = false;
        }
        else
        {
            if (keybind->set_key())
            {
                ImGui::ClearActiveID();
                keybind->waiting_for_input = false;
            }
        }
    }

    float glowAlpha = hovered ? 1.0f : 0.3f;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(50, 50, 50, static_cast<int>(255 * glowAlpha)), 5.0f);
    window->DrawList->AddRect(bb.Min, bb.Max, ImColor(100, 100, 255, static_cast<int>(255 * glowAlpha)), 5.0f);

    window->DrawList->AddText(
        bb.Min + ImVec2(
            size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
            size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2),
        ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 1.0f, 1.0f)),
        name.c_str());

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar;

    if (ImGui::BeginPopup(keybind->get_name().c_str(), 0))
    {
        ImGui::BeginGroup();
        if (ImGui::Selectable("Hold", keybind->type == CKeybind::HOLD))
            keybind->type = CKeybind::HOLD;
        if (ImGui::Selectable("Toggle", keybind->type == CKeybind::TOGGLE))
            keybind->type = CKeybind::TOGGLE;
        ImGui::EndGroup();
        ImGui::EndPopup();
    }

    return pressed;
}

// Main code
int render()
{
    WNDCLASSEX wc;
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = TEXT("D3D11 Overlay ImGui");
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    ::RegisterClassEx(&wc);
    const HWND hwnd = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, "D3D11 Overlay ImGui", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = { -1, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(L"D3D11 Overlay ImGui", wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(oldfont, sizeof oldfont, 16.0f, &font_cfg);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.Alpha = 1.0f;
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 3);
    style.ItemSpacing = ImVec2(8, 6);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.98f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.95f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.3f, 0.5f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 0.9f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.35f, 0.9f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.40f, 0.9f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.18f, 0.9f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.30f, 0.80f, 0.9f);
    colors[ImGuiCol_TabActive] = ImVec4(0.50f, 0.40f, 0.90f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.15f, 0.9f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 0.9f);
    colors[ImGuiCol_Button] = ImVec4(0.40f, 0.30f, 0.80f, 0.9f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.40f, 0.90f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.50f, 1.0f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 1.0f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.60f, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.80f, 0.8f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.30f, 0.80f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.40f, 0.90f, 1.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImVec4 clear_color = ImVec4(0, 0, 0, 0);
    bool done = false;
    static bool tab_states[4] = { false, false, false, false }; // 0:Aimbot, 1:Visuals, 2:Colors, 3:Misc

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (GetForegroundWindow() == FindWindowA(0, "Fortnite  ") || GetForegroundWindow() == hwnd)
        {
            if (settings::aimbot::enable)
            {
                settings::aimbot::aimbotkey.update();
                if (settings::aimbot::aimbotkey.enabled)
                {
                    aimbot(cache::closest_pawn);
                }
            }
        }

        game_loop();

        if (settings::aimbot::show_fov) {
            auto* drawList = ImGui::GetBackgroundDrawList();
            ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);

            drawList->AddCircle(screenCenter, settings::aimbot::fov + 1.0f, ImColor(0, 0, 0, 200), 100, 0.65f);
            drawList->AddCircle(screenCenter, settings::aimbot::fov - 1.0f, ImColor(0, 0, 0, 200), 100, 0.65f);

            if (settings::aimbot::fill_fov) {
                drawList->AddCircleFilled(screenCenter, settings::aimbot::fov, ImColor(0, 0, 0, settings::aimbot::fill_opacity), 100);
            }

            drawList->AddCircle(screenCenter, settings::aimbot::fov, ImColor(200, 100, 255, 250), 100, 1.0f);
        }

        float alphaSpeed = 4.0f * ImGui::GetIO().DeltaTime;

        if (settings::overlay::show_menu)
            menuAlpha = Clamp(menuAlpha + alphaSpeed, 0.0f, 1.0f);
        else
            menuAlpha = Clamp(menuAlpha - alphaSpeed, 0.0f, 1.0f);

        // Floating Tab Selector
        static ImVec2 tab_selector_pos = ImVec2(20, 20);
        if (settings::overlay::show_menu)
        {
            ImGui::SetNextWindowPos(tab_selector_pos, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowBgAlpha(menuAlpha);

            ImGui::Begin("Tab Selector", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoScrollbar);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.15f, 0.95f));

            // Stack buttons vertically with visual indication of tab state
            // Aimbot Button
            if (tab_states[0]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.50f, 1.0f, 1.0f)); // Brighter purple when open
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.60f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.70f, 1.0f, 1.0f));
            }
            if (ImGui::Button("Aimbot", ImVec2(100, 30)))
                tab_states[0] = !tab_states[0];
            if (tab_states[0]) {
                ImGui::PopStyleColor(3); // Pop the custom colors if applied
            }

            // Visuals Button
            if (tab_states[1]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.50f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.60f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.70f, 1.0f, 1.0f));
            }
            if (ImGui::Button("Visuals", ImVec2(100, 30)))
                tab_states[1] = !tab_states[1];
            if (tab_states[1]) {
                ImGui::PopStyleColor(3);
            }

            // Colors Button
            if (tab_states[2]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.50f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.60f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.70f, 1.0f, 1.0f));
            }
            if (ImGui::Button("Colors", ImVec2(100, 30)))
                tab_states[2] = !tab_states[2];
            if (tab_states[2]) {
                ImGui::PopStyleColor(3);
            }

            // Misc Button
            if (tab_states[3]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.50f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.60f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.70f, 1.0f, 1.0f));
            }
            if (ImGui::Button("Misc", ImVec2(100, 30)))
                tab_states[3] = !tab_states[3];
            if (tab_states[3]) {
                ImGui::PopStyleColor(3);
            }

            ImGui::PopStyleColor(); // Pop WindowBg color
            ImGui::PopStyleVar(2);
            ImGui::End();
        }

        if (menuAlpha > 0.0f)
        {
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, ImColor(0, 0, 0, static_cast<int>(200 * menuAlpha)));

            switch (settings::overlay::particle_type)
            {
            case 0:
                M::Particle.render(M_Particle_System::prt_type::rain);
                break;
            case 1:
                M::Particle.render(M_Particle_System::prt_type::snow);
                break;
            }

            int combinedSmoothness = static_cast<int>((settings::aimbot::smoothness_x + settings::aimbot::smoothness_y) / 2.0f);

            // Aimbot Window
            if (tab_states[0])
            {
                static ImVec2 aimbot_pos = ImVec2(100, 100);
                ImGui::SetNextWindowPos(aimbot_pos, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Always); // Set fixed size
                ImGui::SetNextWindowBgAlpha(menuAlpha);

                ImGui::Begin("Aimbot - 1337ud", &tab_states[0], ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Custom dragging logic
                    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0))
                    {
                        aimbot_pos += ImGui::GetIO().MouseDelta;
                        ImGui::SetWindowPos(aimbot_pos);
                    }

                    // Begin child window
                    ImGui::BeginChild("AimbotContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                    ImGui::Checkbox("Enable Aimbot", &settings::aimbot::enable);
                    ImGui::SameLine(150);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
                    Keybind(&settings::aimbot::aimbotkey, ImVec2(50, 18));
                    ImGui::Checkbox("Enable Smoothness", &settings::aimbot::enable_smoothness);
                    ImGui::Checkbox("Visible Only", &settings::aimbot::visible_check);
                    if (ImGui::SliderInt("Smoothness", &combinedSmoothness, 1, 20))
                    {
                        settings::aimbot::smoothness_x = static_cast<float>(combinedSmoothness);
                        settings::aimbot::smoothness_y = static_cast<float>(combinedSmoothness);
                    }
                    ImGui::Checkbox("Fill FOV Circle", &settings::aimbot::fill_fov);
                    ImGui::Checkbox("FOV Circle", &settings::aimbot::show_fov);
                    ImGui::SliderFloat("FOV Size", &settings::aimbot::fov, 50, 500);
                    ImGui::Text("Type");
                    ImGui::Combo("##type", &settings::aimbot::aimbot_type, settings::aimbot::aimbot_types, IM_ARRAYSIZE(settings::aimbot::aimbot_types));
                    ImGui::Text("Aim Part");
                    ImGui::Combo("##part", &settings::aimbot::aimbot_part, settings::aimbot::aimbot_parts, IM_ARRAYSIZE(settings::aimbot::aimbot_parts));
                    ImGui::EndChild();
                }
                ImGui::End();
            }

            // Visuals Window
            if (tab_states[1])
            {
                static ImVec2 visuals_pos = ImVec2(150, 150);
                ImGui::SetNextWindowPos(visuals_pos, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Always); // Set fixed size
                ImGui::SetNextWindowBgAlpha(menuAlpha);

                ImGui::Begin("Visuals - 1337ud", &tab_states[1], ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Custom dragging logic
                    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0))
                    {
                        visuals_pos += ImGui::GetIO().MouseDelta;
                        ImGui::SetWindowPos(visuals_pos);
                    }

                    // Begin child window
                    ImGui::BeginChild("ESPContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                    ImGui::Checkbox("Enable", &settings::visuals::enable);
                    ImGui::Checkbox("Box", &settings::visuals::box);
                    ImGui::SameLine();
                    ImGui::Combo("##boxtype", &settings::visuals::box_type, settings::visuals::box_types, IM_ARRAYSIZE(settings::visuals::box_types));
                    ImGui::Checkbox("Skeleton", &settings::visuals::skeleton);
                    ImGui::Checkbox("Snaplines", &settings::visuals::line);
                    ImGui::Checkbox("Username", &settings::visuals::username);
                    ImGui::Checkbox("FOV Radar", &settings::visuals::FOVRadar);
                    ImGui::Checkbox("Platform", &settings::visuals::platform);
                    ImGui::Checkbox("Distance", &settings::visuals::distance);
                    ImGui::EndChild();
                }
                ImGui::End();
            }

            // Colors Window
            if (tab_states[2])
            {
                static ImVec2 colors_pos = ImVec2(200, 200);
                ImGui::SetNextWindowPos(colors_pos, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Always); // Set fixed size
                ImGui::SetNextWindowBgAlpha(menuAlpha);

                ImGui::Begin("Colors - 1337ud", &tab_states[2], ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Custom dragging logic
                    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0))
                    {
                        colors_pos += ImGui::GetIO().MouseDelta;
                        ImGui::SetWindowPos(colors_pos);
                    }

                    // Begin child window
                    ImGui::BeginChild("ColorsContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                    ImGui::Text("Visible Box");
                    ImGui::SameLine();
                    if (ImGui::ColorButton("##VisibleBoxColor", visible_color, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
                        ImGui::OpenPopup("VisibleBoxColorPicker");
                    if (ImGui::BeginPopup("VisibleBoxColorPicker"))
                    {
                        ImGui::ColorPicker3("##VisibleBoxPicker", (float*)&visible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(120);
                    ImGui::Text("Visible Skeleton");
                    ImGui::SameLine();
                    if (ImGui::ColorButton("##VisibleSkeletonColor", visible_skeleton_color, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
                        ImGui::OpenPopup("VisibleSkeletonColorPicker");
                    if (ImGui::BeginPopup("VisibleSkeletonColorPicker"))
                    {
                        ImGui::ColorPicker3("##VisibleSkeletonPicker", (float*)&visible_skeleton_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        ImGui::EndPopup();
                    }
                    ImGui::Text("Invisible Box");
                    ImGui::SameLine();
                    if (ImGui::ColorButton("##NotVisibleBoxColor", not_visible_color, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
                        ImGui::OpenPopup("NotVisibleBoxColorPicker");
                    if (ImGui::BeginPopup("NotVisibleBoxColorPicker"))
                    {
                        ImGui::ColorPicker3("##NotVisibleBoxPicker", (float*)&not_visible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        ImGui::EndPopup();
                    }
                    ImGui::SameLine(120);
                    ImGui::Text("Invisible Skeleton");
                    ImGui::SameLine();
                    if (ImGui::ColorButton("##NotVisibleSkeletonColor", not_visible_skeleton_color, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
                        ImGui::OpenPopup("NotVisibleSkeletonColorPicker");
                    if (ImGui::BeginPopup("NotVisibleSkeletonColorPicker"))
                    {
                        ImGui::ColorPicker3("##NotVisibleSkeletonPicker", (float*)&not_visible_skeleton_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        ImGui::EndPopup();
                    }
                    ImGui::Separator();
                    ImGui::Text("Box Thickness");
                    ImGui::SliderFloat("##BoxThicknessSlider", &regular_box_thickness, 1.5f, 3.0f, "%.1f");
                    ImGui::Text("Skeleton Thickness");
                    ImGui::SliderFloat("##SkeletonThicknessSlider", &skeleton_thickness, 1.5f, 5.5f, "%.1f");
                    ImGui::EndChild();
                }
                ImGui::End();
            }

            // Misc Window
            if (tab_states[3])
            {
                static ImVec2 misc_pos = ImVec2(250, 250);
                ImGui::SetNextWindowPos(misc_pos, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Always); // Set fixed size
                ImGui::SetNextWindowBgAlpha(menuAlpha);

                ImGui::Begin("Misc - 1337ud", &tab_states[3], ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    // Custom dragging logic
                    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0))
                    {
                        misc_pos += ImGui::GetIO().MouseDelta;
                        ImGui::SetWindowPos(misc_pos);
                    }

                    // Begin child window
                    ImGui::BeginChild("SettingsContent", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                    ImGui::Checkbox("Crosshair", &settings::visuals::show_crosshair);
                    ImGui::Checkbox("Watermark Clock", &settings::features::showtime);
                    ImGui::Checkbox("Watermark FPS", &settings::features::showfps);
                    ImGui::EndChild();
                }
                ImGui::End();
            }
        }

        if (GetAsyncKeyState(VK_F2) & 1)
            settings::overlay::show_menu = !settings::overlay::show_menu;

        if (settings::overlay::show_menu)
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        else
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

#define xorstr_(str) (str)

        std::string text1_part1 = xorstr_("1337");
        std::string text1_part2 = xorstr_("UD");

        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time_t);

        std::ostringstream time_stream;
        time_stream << std::put_time(&local_tm, "%I:%M:%S %p");

        std::string text3;
        if (settings::features::showtime && settings::features::showfps) {
            text3 = xorstr_(" | ") + time_stream.str() + xorstr_(" | FPS: ") + std::to_string(static_cast<int>(ImGui::GetIO().Framerate));
        }
        else if (settings::features::showtime) {
            text3 = xorstr_(" | ") + time_stream.str();
        }
        else if (settings::features::showfps) {
            text3 = xorstr_(" | FPS: ") + std::to_string(static_cast<int>(ImGui::GetIO().Framerate));
        }
        else {
            text3 = "";
        }

        ImVec2 text1_part1_size = ImGui::CalcTextSize(text1_part1.c_str());
        ImVec2 text1_part2_size = ImGui::CalcTextSize(text1_part2.c_str());
        ImVec2 text3_size = ImGui::CalcTextSize(text3.c_str());

        float max_height = text1_part1_size.y;
        float total_width = text1_part1_size.x + text1_part2_size.x + text3_size.x + 25.0f;
        float total_height = max_height + 10.0f;

        ImVec2 rect_pos(10, 10);
        ImVec2 rect_size(total_width, total_height);

        ImGui::GetForegroundDrawList()->AddRectFilledMultiColor(
            rect_pos, rect_pos + rect_size,
            ImColor(50, 30, 100, 200), ImColor(100, 50, 200, 200),
            ImColor(100, 50, 200, 200), ImColor(50, 30, 100, 200)
        );

        ImGui::GetForegroundDrawList()->AddText(
            rect_pos + ImVec2(10, (rect_size.y - text1_part1_size.y) / 2),
            ImColor(200, 100, 255, 255),
            text1_part1.c_str()
        );

        ImGui::GetForegroundDrawList()->AddText(
            rect_pos + ImVec2(10 + text1_part1_size.x, (rect_size.y - text1_part2_size.y) / 2),
            ImColor(255, 50, 150, 255),
            text1_part2.c_str()
        );

        if (!text3.empty()) {
            ImGui::GetForegroundDrawList()->AddText(
                rect_pos + ImVec2(text1_part1_size.x + text1_part2_size.x + 10, (rect_size.y - text3_size.y) / 2),
                ImColor(150, 150, 255, 255),
                text3.c_str()
            );
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(L"D3D11 Overlay ImGui", wc.hInstance);

    return 0;
}