#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <cstdio>
#include <string>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "AfterEffectsMobile", __VA_ARGS__)

static EGLDisplay g_Display = EGL_NO_DISPLAY;
static EGLSurface g_Surface = EGL_NO_SURFACE;
static EGLContext g_Context = EGL_NO_CONTEXT;
static bool g_Ready = false;
static bool g_Workspace = false;
static bool g_Playing = false;
static float g_Playhead = 0.32f;
static int g_SelectedTool = 0;
static int g_SelectedLayer = 0;
static char g_ProjectName[96] = "Untitled Project";
static char g_CompositionName[96] = "Main Composition";

static bool InitEGL(ANativeWindow* window) {
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 0, EGL_NONE
    };
    g_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_Display == EGL_NO_DISPLAY || !eglInitialize(g_Display, nullptr, nullptr)) return false;
    EGLConfig config;
    EGLint count = 0;
    eglChooseConfig(g_Display, attribs, &config, 1, &count);
    if (count == 0) return false;
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_Context = eglCreateContext(g_Display, config, EGL_NO_CONTEXT, contextAttribs);
    g_Surface = eglCreateWindowSurface(g_Display, config, window, nullptr);
    if (g_Context == EGL_NO_CONTEXT || g_Surface == EGL_NO_SURFACE) return false;
    return eglMakeCurrent(g_Display, g_Surface, g_Surface, g_Context) == EGL_TRUE;
}

static void ShutdownEGL() {
    if (g_Display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_Context != EGL_NO_CONTEXT) eglDestroyContext(g_Display, g_Context);
        if (g_Surface != EGL_NO_SURFACE) eglDestroySurface(g_Display, g_Surface);
        eglTerminate(g_Display);
    }
    g_Display = EGL_NO_DISPLAY;
    g_Context = EGL_NO_CONTEXT;
    g_Surface = EGL_NO_SURFACE;
}

static void OnAppCmd(android_app* app, int32_t cmd) {
    if (cmd == APP_CMD_INIT_WINDOW && app->window) {
        if (InitEGL(app->window)) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.FontGlobalScale = 1.05f;
            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 7.0f;
            style.ChildRounding = 6.0f;
            style.FrameRounding = 5.0f;
            style.PopupRounding = 6.0f;
            style.ScrollbarRounding = 6.0f;
            style.WindowPadding = ImVec2(12, 10);
            style.FramePadding = ImVec2(10, 7);
            style.ItemSpacing = ImVec2(8, 7);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.075f, 0.082f, 0.10f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.105f, 0.115f, 0.14f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.10f, 0.13f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.30f, 0.46f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.39f, 0.58f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.23f, 0.32f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.35f, 0.49f, 1.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.19f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.56f, 0.88f, 1.0f);
            ImGui_ImplAndroid_Init(app->window);
            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_Ready = true;
        }
    } else if (cmd == APP_CMD_TERM_WINDOW && g_Ready) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();
        g_Ready = false;
        ShutdownEGL();
    }
}

static int32_t OnInput(android_app*, AInputEvent* event) {
    return ImGui_ImplAndroid_HandleInputEvent(event);
}

static void SectionTitle(const char* title) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.72f, 0.82f, 1));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
}

static void DrawHome() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextColored(ImVec4(0.42f, 0.62f, 0.96f, 1), "Ae");
    ImGui::SameLine();
    ImGui::Text("AFTER EFFECTS");
    ImGui::SameLine();
    ImGui::TextDisabled("MOBILE");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Good to see you.");
    ImGui::TextDisabled("Create a composition or continue a recent project.");
    ImGui::Spacing();

    ImGui::BeginChild("CreatePanel", ImVec2(0, 112), true);
    SectionTitle("START A NEW PROJECT");
    ImGui::InputText("Project name", g_ProjectName, IM_ARRAYSIZE(g_ProjectName));
    ImGui::SameLine();
    if (ImGui::Button("Create Project", ImVec2(145, 0))) g_Workspace = true;
    ImGui::EndChild();

    ImGui::Spacing();
    SectionTitle("NEW COMPOSITION");
    ImGui::BeginChild("Templates", ImVec2(0, 125), false);
    const char* names[] = { "Mobile 1080 x 1920", "HD 1920 x 1080", "Square 1080 x 1080" };
    for (int i = 0; i < 3; ++i) {
        ImGui::PushID(i);
        ImGui::BeginChild("TemplateCard", ImVec2(190, 100), true);
        ImGui::TextColored(ImVec4(0.45f, 0.66f, 0.98f, 1), "COMPOSITION");
        ImGui::TextWrapped("%s", names[i]);
        if (ImGui::SmallButton("Create")) {
            if (i == 0) snprintf(g_CompositionName, sizeof(g_CompositionName), "Mobile Composition");
            if (i == 1) snprintf(g_CompositionName, sizeof(g_CompositionName), "HD Composition");
            if (i == 2) snprintf(g_CompositionName, sizeof(g_CompositionName), "Square Composition");
            g_Workspace = true;
        }
        ImGui::EndChild();
        ImGui::PopID();
        if (i != 2) ImGui::SameLine();
    }
    ImGui::EndChild();

    ImGui::Spacing();
    SectionTitle("RECENT PROJECTS");
    ImGui::BeginChild("Recent", ImVec2(0, 0), true);
    if (ImGui::Selectable("  Untitled Project     •     Just now", false, 0, ImVec2(0, 34))) g_Workspace = true;
    ImGui::EndChild();
    ImGui::End();
}

static void DrawToolbar() {
    ImGui::BeginChild("Toolbar", ImVec2(0, 46), true);
    const char* tools[] = { "Select", "Hand", "Text", "Shape", "Pen", "Zoom" };
    for (int i = 0; i < 6; ++i) {
        ImGui::PushID(i);
        if (i == g_SelectedTool) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.39f, 0.62f, 1));
        if (ImGui::Button(tools[i], ImVec2(72, 28))) g_SelectedTool = i;
        if (i == g_SelectedTool) ImGui::PopStyleColor();
        ImGui::PopID();
        if (i != 5) ImGui::SameLine();
    }
    ImGui::EndChild();
}

static void DrawWorkspace() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    if (ImGui::Button("< Projects", ImVec2(95, 30))) g_Workspace = false;
    ImGui::SameLine();
    ImGui::Text("%s", g_ProjectName);
    ImGui::SameLine();
    ImGui::TextDisabled("/ %s", g_CompositionName);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120);
    if (ImGui::Button("Export", ImVec2(90, 30))) ImGui::OpenPopup("ExportInfo");
    if (ImGui::BeginPopup("ExportInfo")) {
        ImGui::TextWrapped("Export rendering will be added in a later milestone.");
        ImGui::EndPopup();
    }
    ImGui::Separator();
    DrawToolbar();

    const float available = ImGui::GetContentRegionAvail().y;
    const float timelineHeight = std::max(150.0f, available * 0.37f);
    ImGui::BeginChild("PreviewArea", ImVec2(0, available - timelineHeight - 8), false);
    const float leftWidth = ImGui::GetContentRegionAvail().x * 0.20f;
    ImGui::BeginChild("ProjectPanel", ImVec2(leftWidth, 0), true);
    SectionTitle("PROJECT");
    if (ImGui::Selectable("▸ " "Compositions", true)) {}
    ImGui::Indent(12);
    ImGui::Selectable("  Main Composition", true);
    ImGui::Unindent(12);
    ImGui::Separator();
    SectionTitle("EFFECTS & PRESETS");
    ImGui::TextDisabled("Search effects...");
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("CompositionPanel", ImVec2(0, 0), true);
    SectionTitle("COMPOSITION");
    ImGui::SameLine();
    ImGui::TextDisabled("  Active Camera  |  50%  |  Full");
    ImGui::Spacing();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    float aspect = 16.0f / 9.0f;
    float w = std::min(canvasSize.x * 0.84f, canvasSize.y * aspect);
    float h = w / aspect;
    if (h > canvasSize.y * 0.88f) { h = canvasSize.y * 0.88f; w = h * aspect; }
    ImVec2 p = ImGui::GetCursorScreenPos();
    p.x += (canvasSize.x - w) * 0.5f;
    p.y += (canvasSize.y - h) * 0.5f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(25, 29, 39, 255), 2.0f);
    draw->AddRect(p, ImVec2(p.x + w, p.y + h), IM_COL32(75, 91, 117, 255));
    draw->AddText(ImVec2(p.x + 18, p.y + 18), IM_COL32(210, 218, 232, 255), "Composition Preview");
    draw->AddText(ImVec2(p.x + 18, p.y + 42), IM_COL32(115, 132, 158, 255), "Your visual work appears here");
    ImGui::Dummy(ImVec2(canvasSize.x, canvasSize.y));
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::BeginChild("Timeline", ImVec2(0, timelineHeight), true);
    ImGui::Text("TIMELINE");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", g_CompositionName);
    ImGui::SameLine();
    if (ImGui::SmallButton(g_Playing ? "Pause" : "Play")) g_Playing = !g_Playing;
    ImGui::SameLine();
    if (ImGui::SmallButton("Add Layer")) ImGui::OpenPopup("AddLayer");
    if (ImGui::BeginPopup("AddLayer")) {
        if (ImGui::MenuItem("Text")) { g_SelectedLayer = 0; }
        if (ImGui::MenuItem("Shape")) { g_SelectedLayer = 1; }
        if (ImGui::MenuItem("Solid")) { g_SelectedLayer = 2; }
        ImGui::EndPopup();
    }
    ImGui::Separator();
    ImGui::BeginChild("LayerNames", ImVec2(170, 0), true);
    SectionTitle("LAYER");
    const char* layers[] = { "Text Layer 1", "Shape Layer 1", "Background" };
    for (int i = 0; i < 3; ++i) {
        if (ImGui::Selectable(layers[i], g_SelectedLayer == i)) g_SelectedLayer = i;
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("Tracks", ImVec2(0, 0), true);
    ImDrawList* timelineDraw = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float trackWidth = ImGui::GetContentRegionAvail().x;
    for (int i = 0; i < 3; ++i) {
        float y = origin.y + 8 + i * 29;
        timelineDraw->AddLine(ImVec2(origin.x, y + 23), ImVec2(origin.x + trackWidth, y + 23), IM_COL32(55, 63, 78, 255));
        timelineDraw->AddRectFilled(ImVec2(origin.x + 8 + i * 9, y + 4), ImVec2(origin.x + trackWidth * (0.72f - i * 0.08f), y + 20), i == g_SelectedLayer ? IM_COL32(72, 119, 190, 255) : IM_COL32(63, 83, 113, 255), 4);
        for (int k = 0; k < 3; ++k) {
            float kx = origin.x + trackWidth * (0.28f + k * 0.22f);
            timelineDraw->AddCircleFilled(ImVec2(kx, y + 12), 3.5f, IM_COL32(225, 190, 92, 255));
        }
    }
    ImGui::InvisibleButton("PlayheadTrack", ImVec2(trackWidth, 100));
    if (ImGui::IsItemActive()) g_Playhead = std::clamp((ImGui::GetIO().MousePos.x - origin.x) / std::max(1.0f, trackWidth), 0.0f, 1.0f);
    float px = origin.x + trackWidth * g_Playhead;
    timelineDraw->AddLine(ImVec2(px, origin.y - 4), ImVec2(px, origin.y + 100), IM_COL32(245, 91, 91, 255), 2.0f);
    timelineDraw->AddTriangleFilled(ImVec2(px - 6, origin.y - 5), ImVec2(px + 6, origin.y - 5), ImVec2(px, origin.y + 3), IM_COL32(245, 91, 91, 255));
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::End();
}

void android_main(android_app* app) {
    app->onAppCmd = OnAppCmd;
    app->onInputEvent = OnInput;
    while (true) {
        int events;
        android_poll_source* source;
        while (ALooper_pollOnce(g_Ready ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested) {
                if (g_Ready) {
                    ImGui_ImplOpenGL3_Shutdown();
                    ImGui_ImplAndroid_Shutdown();
                    ImGui::DestroyContext();
                    g_Ready = false;
                    ShutdownEGL();
                }
                return;
            }
        }
        if (!g_Ready) continue;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();
        if (g_Workspace) DrawWorkspace(); else DrawHome();
        ImGui::Render();
        int width, height;
        eglQuerySurface(g_Display, g_Surface, EGL_WIDTH, &width);
        eglQuerySurface(g_Display, g_Surface, EGL_HEIGHT, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.055f, 0.061f, 0.075f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(g_Display, g_Surface);
    }
}
