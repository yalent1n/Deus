#include <Engine.Render/Module.h>

#include <Windows.h>
#include <dwmapi.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma comment(lib, "Dwmapi.lib")

FB_IMPLEMENT_MODULE(Render, "Render package")

GLFWwindow* sm_Window = nullptr;

namespace rw
{
void EnableDarkTitleBar(HWND hwnd)
{
    BOOL dark = TRUE;

    // Windows 10 1809+
    constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

    DwmSetWindowAttribute(
        hwnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &dark,
        sizeof(dark));
}

inline ImVec4 ImLerp2(const ImVec4& a, const ImVec4& b, float t)         { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }

void styleColorsRw()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);

    colors[ImGuiCol_Border]                 = ImVec4(0.23f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg]                = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.31f, 0.35f, 1.00f);
  
    colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
  
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
  
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.32f, 0.33f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.41f, 0.44f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.48f, 0.49f, 0.52f, 1.00f);

    // Frostbite orange
    ImVec4 accent = ImVec4(0.95f, 0.42f, 0.08f, 1.00f);
    ImVec4 accentHover = ImVec4(1.00f, 0.52f, 0.18f, 1.00f);
    ImVec4 accentActive = ImVec4(1.00f, 0.62f, 0.28f, 1.00f);

    colors[ImGuiCol_CheckMark]              = accent;
    colors[ImGuiCol_CheckboxSelectedBg]     = ImLerp2(colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBgHovered], 0.65f);
    colors[ImGuiCol_SliderGrab]             = accent;
    colors[ImGuiCol_SliderGrabActive]       = accentActive;

    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = accentHover;
    colors[ImGuiCol_ButtonActive]           = accentActive;
  
    colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.23f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = accentHover;
    colors[ImGuiCol_HeaderActive]           = accent;
  
    colors[ImGuiCol_Separator]              = ImVec4(0.26f, 0.27f, 0.30f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = accentHover;
    colors[ImGuiCol_SeparatorActive]        = accent;
  
    colors[ImGuiCol_Tab]                    = ImVec4(0.16f, 0.17f, 0.19f, 1.00f);
    colors[ImGuiCol_TabHovered]             = accentHover;
    colors[ImGuiCol_TabSelected]            = accent;
    colors[ImGuiCol_TabDimmed]              = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.22f, 0.23f, 0.26f, 1.00f);
  
    colors[ImGuiCol_ResizeGrip]             = accent;
    colors[ImGuiCol_ResizeGripHovered]      = accentHover;
    colors[ImGuiCol_ResizeGripActive]       = accentActive;

    colors[ImGuiCol_InputTextCursor]        = colors[ImGuiCol_Text];
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                    = ImLerp2(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]            = ImLerp2(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp2(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp2(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
    //colors[ImGuiCol_DockingPreview]         = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_TreeLines]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_DragDropTargetBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_UnsavedMarker]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavCursor]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style->WindowRounding    = 2.0f;
    style->ChildRounding     = 2.0f;
    style->FrameRounding     = 2.0f;
    style->PopupRounding     = 2.0f;
    style->ScrollbarRounding = 2.0f;
    style->GrabRounding      = 2.0f;
    style->TabRounding       = 2.0f;

    style->WindowBorderSize  = 1.0f;
    style->FrameBorderSize   = 1.0f;

    style->WindowPadding     = ImVec2(8, 8);
    style->FramePadding      = ImVec2(6, 4);
    style->ItemSpacing       = ImVec2(8, 6);
}

void coreRenderInit(int w, int h, const char* title)
{
//#ifndef ENGINE_DEBUG_BUILD
    ShowWindow(GetConsoleWindow(), SW_HIDE);
//#endif

    int err = glfwInit();
    EASSERT(!!(err), "Render: failed to initialize GLFW");

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    sm_Window = glfwCreateWindow((int)(w * main_scale), (int)(h * main_scale), title, nullptr, nullptr);
    EASSERT(!!(sm_Window), "Render: failed to create window");

    EnableDarkTitleBar(glfwGetWin32Window(sm_Window));

    glfwMakeContextCurrent(sm_Window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    styleColorsRw();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
#endif

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    style.FontSizeBase = 18.0f;
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(sm_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);
}

void coreRenderShutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(sm_Window);
    sm_Window = nullptr;
    glfwTerminate();
}

void coreRenderGetFramebufferSize(int* w, int* h)
{
    int _w, _h;
    glfwGetFramebufferSize(sm_Window, &_w, &_h);
    if(w) *w = _w;
    if(h) *h = _h;
}

void coreRenderSwapBuffers()
{
    glfwSwapBuffers(sm_Window);
}

bool coreRenderShouldClose()
{
    return glfwWindowShouldClose(sm_Window);
}
}