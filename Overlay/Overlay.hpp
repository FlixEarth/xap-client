// Externals
#pragma once
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#define GLFW_EXPOSE_NATIVE_X11
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

// Internals
#include "../Utils/InputManager.hpp"
#include "Font.hpp"

class Overlay {
private:
    GLFWwindow* OverlayWindow;
    int ScreenWidth;
    int ScreenHeight;

    void GrabScreenSize() {
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(primaryMonitor);
        ScreenWidth = vidMode->width;
        ScreenHeight = vidMode->height;
    }

    std::string RandomString(int ch) {
        char alpha[35] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                            'h', 'i', 'j', 'k', 'l', 'm', 'n',
                            'o', 'p', 'q', 'r', 's', 't', 'u',
                            'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        std::string result = "";
        for (int i = 0; i<ch; i++)
            result = result + alpha[rand() % 35];
        return result;
    }

    static void GLFWErrorCallback(int error, const char *description) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    long long CurrentEpochMilliseconds() {
        auto currentTime = std::chrono::system_clock::now();
        auto duration = currentTime.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    ImWchar *GetFontGlyphRanges() noexcept
    {
        static ImVector<ImWchar> ranges;
        if (ranges.empty())
        {
            ImFontGlyphRangesBuilder builder;
            constexpr ImWchar baseRanges[] = {
                0x0100, 0x024F, // Latin Extended-A + Latin Extended-B
                0x0300, 0x03FF, // Combining Diacritical Marks + Greek/Coptic
                0x0600, 0x06FF, // Arabic
                0x0E00, 0x0E7F, // Thai
                0
            };
            builder.AddRanges(baseRanges);
            builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
            builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
            builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
            builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
            // ★龍王™
            builder.AddChar(u'\u2605'); // ★
            builder.AddChar(u'\u9F8D'); // 龍
            builder.AddChar(u'\u738B'); // 王
            builder.AddChar(u'\u2122'); // ™
            builder.BuildRanges(&ranges);
        }
        return ranges.Data;
    }

public:
    int ProcessingTime;
    long long StartTime;
    int SleepTime;
    int TimeLeftToSleep;

    bool InitializeOverlay() {
        glfwSetErrorCallback(GLFWErrorCallback);
        if (!glfwInit()) {
            return false;
        }

        GrabScreenSize();
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 240);

        OverlayWindow = glfwCreateWindow(ScreenWidth, ScreenHeight, RandomString(12).c_str(), NULL, NULL);

        CaptureInput(true);
        glfwMakeContextCurrent(OverlayWindow);
        
        // Centering //
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(primaryMonitor);
        glfwSetWindowPos(OverlayWindow, (vidMode->width - ScreenWidth) / 2, (vidMode->height - ScreenHeight) / 2);
        // End of Centering //

        InitializeUI();

        glfwShowWindow(OverlayWindow);
        glfwSwapInterval(1);

        return true;
    }

    void InitializeUI() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImFontConfig cfg;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        cfg.SizePixels = 13.0f;
        cfg.GlyphOffset = {1.0f, -1.0f};
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromMemoryCompressedTTF(_compressedFontData, _compressedFontSize, cfg.SizePixels, &cfg, GetFontGlyphRanges());

        ImGui::StyleColorsDark();
        SetUIStyle();

        ImGui_ImplGlfw_InitForOpenGL(OverlayWindow, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    bool AlignedButton(const char* label, float alignment = 0.5f) {
        ImGuiStyle& style = ImGui::GetStyle();

        float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
        float avail = ImGui::GetContentRegionAvail().x;

        float off = (avail - size) * alignment;
        if (off > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        return ImGui::Button(label);
    }

    void SetUIStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
            style.Alpha = 0.5f;
            style.DisabledAlpha = 0.3f;
            style.WindowPadding = ImVec2(20.0f, 20.0f);
            style.WindowRounding = 20.f;
            style.WindowBorderSize = 2.0f;
            style.WindowMinSize = ImVec2(50.0f, 50.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Right;
            style.ChildRounding = 20.f;
            style.ChildBorderSize = 3.0f;
            style.PopupRounding = 20.f;
            style.PopupBorderSize = 3.0f;
            style.FramePadding = ImVec2(10.0f, 10.0f);
            style.FrameRounding = 20.f;
            style.FrameBorderSize = 2.0f;
            style.ItemSpacing = ImVec2(15.0f, 10.0f);
            style.ItemInnerSpacing = ImVec2(10.0f, 8.0f);
            style.CellPadding = ImVec2(15.0f, 10.0f);
            style.IndentSpacing = 20.0f;
            style.ColumnsMinSpacing = 10.0f;
            style.ScrollbarSize = 15.0f;
            style.ScrollbarRounding = 20.f;
            style.GrabMinSize = 15.0f;
            style.GrabRounding = 20.f;
            style.TabRounding = 20.f;
            style.TabBorderSize = 2.0f;
            style.TabMinWidthForCloseButton = 15.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.2f, 0.8f);
            style.SelectableTextAlign = ImVec2(0.8f, 0.2f);
        
    style.Colors[ImGuiCol_Text]                   = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled]           = ImColor(0.35f, 0.35f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]               = ImColor(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ChildBg]                = ImColor(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]                = ImColor(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border]                 = ImColor(0.90f, 0.40f, 0.00f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow]           = ImColor(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]                = ImColor(0.00f, 0.00f, 0.00f, 0.54f);
    style.Colors[ImGuiCol_FrameBgHovered]         = ImColor(0.37f, 0.14f, 0.14f, 0.67f);
    style.Colors[ImGuiCol_FrameBgActive]          = ImColor(0.39f, 0.20f, 0.20f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]                = ImColor(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]          = ImColor(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed]       = ImColor(0.48f, 0.36f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]              = ImColor(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]            = ImColor(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab]          = ImColor(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImColor(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImColor(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]              = ImColor(0.96f, 0.50f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]             = ImColor(1.00f, 0.50f, 0.00f, 0.40f);
    style.Colors[ImGuiCol_SliderGrabActive]       = ImColor(0.89f, 0.50f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Button]                 = ImColor(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]          = ImColor(0.80f, 0.50f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]           = ImColor(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Header]                 = ImColor(0.33f, 0.35f, 0.36f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered]          = ImColor(0.6f, 0.6f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive]           = ImColor(0.47f, 0.47f, 0.47f, 0.67f);
    style.Colors[ImGuiCol_Separator]              = ImColor(0.32f, 0.32f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]       = ImColor(0.32f, 0.32f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive]        = ImColor(0.32f, 0.32f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]             = ImColor(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered]      = ImColor(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]       = ImColor(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_Tab]                    = ImColor(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_TabHovered]             = ImColor(0.86f, 0.43f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TabActive]              = ImColor(0.19f, 0.19f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused]           = ImColor(0.05f, 0.05f, 0.05f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive]     = ImColor(0.13f, 0.13f, 0.13f, 0.74f);
    style.Colors[ImGuiCol_PlotLines]              = ImColor(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]       = ImColor(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]          = ImColor(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]   = ImColor(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg]          = ImColor(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong]      = ImColor(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight]       = ImColor(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg]             = ImColor(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt]          = ImColor(1.00f, 1.00f, 1.00f, 0.07f);
    style.Colors[ImGuiCol_TextSelectedBg]         = ImColor(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget]         = ImColor(1.00f, 1.00f, 0.00f, 0.90f);
    style.Colors[ImGuiCol_NavHighlight]           = ImColor(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight]  = ImColor(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg]      = ImColor(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg]       = ImColor(0.80f, 0.80f, 0.80f, 0.35f);
    style.Colors[ImGuiCol_SliderGrab] = ImColor(100, 150, 200, 255);
    style.Colors[ImGuiCol_SliderGrabActive] = ImColor(200, 150, 100, 255);
    }

    void DestroyOverlay() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if(OverlayWindow != nullptr) {
            glfwDestroyWindow(OverlayWindow);
        }

        glfwTerminate();
    }

    void CaptureInput(bool capture) {
        glfwSetWindowAttrib(OverlayWindow, GLFW_MOUSE_PASSTHROUGH, capture ? GLFW_FALSE : GLFW_TRUE);
    }

    void FocusOverlay() {
        glfwFocusWindow(OverlayWindow);
    }

    void Start(bool(*Update)(), void(*RenderUI)()) {
        while(!glfwWindowShouldClose(OverlayWindow)) {
            StartTime = CurrentEpochMilliseconds();
            glfwPollEvents();
            glViewport(0, 0, ScreenWidth, ScreenHeight);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, ScreenWidth, ScreenHeight, 0, -1, 1);

            if (Update != nullptr) {
                Update();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            // Render
            if (RenderUI != nullptr){
                RenderUI();
            }
            
            // Render ImGui and swap buffers
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(OverlayWindow);

            ProcessingTime = static_cast<int>(CurrentEpochMilliseconds() - StartTime);
            SleepTime = 4.16; // 16.67 > 60hz | 6.97 > 144hz
            TimeLeftToSleep = std::max(0, SleepTime - ProcessingTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(TimeLeftToSleep));
        }

        DestroyOverlay();
    }

    void GetScreenResolution(int& Width, int& Height) const {
        Width = ScreenWidth;
        Height = ScreenHeight;
    }
};
