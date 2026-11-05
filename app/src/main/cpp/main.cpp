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


static void PanelHeading(const char* name) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.75f, 0.80f, 1));
    ImGui::TextUnformatted(name);
    ImGui::PopStyleColor();
    ImGui::Separator();
}

static void MenuBarLike() {
    ImGui::BeginChild("MainMenu", ImVec2(0, 28), false);
    const char* menus[] = {"File", "Edit", "Composition", "Layer", "Effect", "Animation", "View", "Window", "Help"};
    for (int i = 0; i < 9; ++i) {
        ImGui::TextDisabled("%s", menus[i]);
        if (i != 8) ImGui::SameLine(0, 15);
    }
    ImGui::EndChild();
}

static void DrawHome() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("After Effects Start Workspace", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    MenuBarLike();
    ImGui::BeginChild("HomeWorkspace", ImVec2(0, ImGui::GetContentRegionAvail().y - 150), false);
    const float fullW = ImGui::GetContentRegionAvail().x;
    const float fullH = ImGui::GetContentRegionAvail().y;
    const float leftW = fullW * 0.235f;
    const float rightW = fullW * 0.16f;
    const float centerW = fullW - leftW - rightW - 16.0f;

    ImGui::BeginChild("HomeProjectPanel", ImVec2(leftW, fullH), true);
    PanelHeading("Project");
    ImGui::Spacing();
    ImGui::TextDisabled("Search");
    ImGui::SetNextItemWidth(-1);
    static char searchProject[80] = "";
    ImGui::InputText("##projectsearch", searchProject, IM_ARRAYSIZE(searchProject));
    ImGui::Spacing();
    ImGui::TextDisabled("Name                         Type");
    ImGui::Separator();
    ImGui::Selectable("  Project", false);
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("HomeCenterPanel", ImVec2(centerW, fullH), true);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + fullH * 0.24f);
    const float cardW = std::min(250.0f, (centerW - 55.0f) * 0.5f);
    const float cardH = std::min(270.0f, fullH * 0.60f);
    const float cardsW = cardW * 2 + 24.0f;
    ImGui::SetCursorPosX(std::max(12.0f, (centerW - cardsW) * 0.5f));
    ImGui::BeginChild("NewCompositionCard", ImVec2(cardW, cardH), true);
    ImGui::SetCursorPosY(cardH * 0.25f);
    ImGui::SetCursorPosX(cardW * 0.5f - 33);
    ImGui::TextColored(ImVec4(0.68f, 0.70f, 0.75f, 1), "[ + ]");
    ImGui::Spacing();
    ImGui::SetCursorPosX(12);
    ImGui::TextWrapped("New Composition");
    ImGui::SetCursorPosY(cardH - 42);
    if (ImGui::Button("New Composition", ImVec2(-1, 30))) {
        snprintf(g_CompositionName, sizeof(g_CompositionName), "Comp 1");
        g_Workspace = true;
    }
    ImGui::EndChild();
    ImGui::SameLine(0, 24);
    ImGui::BeginChild("NewFootageCard", ImVec2(cardW, cardH), true);
    ImGui::SetCursorPosY(cardH * 0.25f);
    ImGui::SetCursorPosX(cardW * 0.5f - 33);
    ImGui::TextColored(ImVec4(0.68f, 0.70f, 0.75f, 1), "[ + ]");
    ImGui::Spacing();
    ImGui::SetCursorPosX(12);
    ImGui::TextWrapped("New Composition From Footage");
    ImGui::SetCursorPosY(cardH - 42);
    if (ImGui::Button("From Footage", ImVec2(-1, 30))) ImGui::OpenPopup("FootageInfo");
    if (ImGui::BeginPopup("FootageInfo")) {
        ImGui::TextWrapped("Footage import will be connected in the media-import milestone.");
        ImGui::EndPopup();
    }
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("HomeRightPanels", ImVec2(0, fullH), false);
    const char* rightPanels[] = {"Info", "Audio", "Preview", "Effects & Presets", "Align", "Libraries", "Character", "Paragraph"};
    for (int i = 0; i < 8; ++i) {
        ImGui::BeginChild(rightPanels[i], ImVec2(0, i == 2 ? 105 : 48), true);
        ImGui::TextUnformatted(rightPanels[i]);
        if (i == 2) ImGui::TextDisabled("Play   Stop   Next Frame");
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::BeginChild("RenderQueuePanel", ImVec2(0, 0), true);
    ImGui::Text("Render Queue");
    ImGui::Separator();
    ImGui::TextDisabled("Render Settings                         Status                         Output To");
    ImGui::EndChild();
    ImGui::End();
}

static void DrawToolbar() {
    ImGui::BeginChild("ToolOptionsBar", ImVec2(0, 38), true);
    const char* tools[] = {"V", "H", "Z", "Rotate", "Camera", "Pan Behind", "Shape", "Pen", "Text", "Brush", "Clone", "Eraser", "Roto Brush"};
    for (int i = 0; i < 13; ++i) {
        ImGui::PushID(i);
        if (i == g_SelectedTool) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.34f, 0.57f, 1));
        if (ImGui::Button(tools[i], ImVec2(i < 3 ? 32 : 68, 25))) g_SelectedTool = i;
        if (i == g_SelectedTool) ImGui::PopStyleColor();
        ImGui::PopID();
        if (i != 12) ImGui::SameLine(0, 3);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Fill  Stroke  2 px     Essentials");
    ImGui::EndChild();
}

static void DrawWorkspace() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("After Effects Editor", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    MenuBarLike();
    DrawToolbar();

    const float availH = ImGui::GetContentRegionAvail().y;
    const float timelineH = std::max(235.0f, availH * 0.43f);
    const float upperH = availH - timelineH - 6.0f;
    const float totalW = ImGui::GetContentRegionAvail().x;
    const float leftW = totalW * 0.235f;
    const float rightW = totalW * 0.16f;
    const float centerW = totalW - leftW - rightW - 12.0f;

    ImGui::BeginChild("UpperWorkspace", ImVec2(0, upperH), false);
    ImGui::BeginChild("LeftDock", ImVec2(leftW, 0), false);
    ImGui::BeginChild("ProjectDock", ImVec2(0, upperH * 0.62f), true);
    ImGui::Text("Project");
    ImGui::SameLine();
    ImGui::TextDisabled("  Effect Controls");
    ImGui::Separator();
    ImGui::TextDisabled("Search");
    static char projectSearch[64] = "";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##editorsearch", projectSearch, IM_ARRAYSIZE(projectSearch));
    ImGui::Separator();
    ImGui::TextDisabled("Name                    Type");
    ImGui::Selectable("Comp 1        Composition", true);
    ImGui::EndChild();
    ImGui::BeginChild("EffectControlsDock", ImVec2(0, 0), true);
    ImGui::Text("Effect Controls");
    ImGui::Separator();
    ImGui::TextDisabled("No effect selected");
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("CompositionViewer", ImVec2(centerW, 0), true);
    ImGui::Text("Composition");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.34f, 0.58f, 0.91f, 1), "%s", g_CompositionName);
    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), centerW - 250));
    ImGui::TextDisabled("50%    Full    Active Camera    1 View");
    ImGui::Separator();
    ImVec2 region = ImGui::GetContentRegionAvail();
    ImVec2 canvas = ImVec2(region.x * 0.96f, region.y * 0.88f);
    ImVec2 cp = ImGui::GetCursorScreenPos();
    cp.x += (region.x - canvas.x) * 0.5f;
    cp.y += (region.y - canvas.y) * 0.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(cp, ImVec2(cp.x + canvas.x, cp.y + canvas.y), IM_COL32(0, 0, 0, 255));
    const float unit = std::min(canvas.x / 6.0f, canvas.y / 3.3f);
    ImVec2 circleCenter(cp.x + canvas.x * 0.20f, cp.y + canvas.y * 0.32f);
    dl->AddCircleFilled(circleCenter, unit * 0.48f, IM_COL32(205, 238, 24, 255), 48);
    dl->AddCircle(circleCenter, unit * 0.48f, IM_COL32(238, 238, 238, 255), 48, 2.0f);
    dl->AddRectFilled(ImVec2(cp.x + canvas.x * 0.78f, cp.y + canvas.y * 0.46f),
        ImVec2(cp.x + canvas.x * 0.90f, cp.y + canvas.y * 0.72f), IM_COL32(205, 238, 24, 255));
    dl->AddRect(ImVec2(cp.x + canvas.x * 0.78f, cp.y + canvas.y * 0.46f),
        ImVec2(cp.x + canvas.x * 0.90f, cp.y + canvas.y * 0.72f), IM_COL32(240, 240, 240, 255), 0, 0, 2);
    ImGui::Dummy(region);
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("RightDock", ImVec2(0, 0), false);
    const char* panels[] = {"Info", "Audio", "Preview", "Effects & Presets", "Align", "Libraries", "Character", "Paragraph"};
    for (int i = 0; i < 8; ++i) {
        ImGui::BeginChild(panels[i], ImVec2(0, i == 2 ? 116 : 42), true);
        ImGui::TextUnformatted(panels[i]);
        if (i == 2) {
            ImGui::TextDisabled("  |<   <<   Play   >>   >|");
            ImGui::TextDisabled("Shortcut: Spacebar");
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::BeginChild("TimelineDock", ImVec2(0, timelineH), true);
    ImGui::Text("%s", g_CompositionName);
    ImGui::SameLine();
    ImGui::TextDisabled("  Render Queue");
    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), totalW - 245));
    if (ImGui::SmallButton(g_Playing ? "Pause" : "Play")) g_Playing = !g_Playing;
    ImGui::SameLine();
    if (ImGui::SmallButton("Add Layer")) ImGui::OpenPopup("AddLayer");
    if (ImGui::BeginPopup("AddLayer")) {
        if (ImGui::MenuItem("Text")) g_SelectedLayer = 0;
        if (ImGui::MenuItem("Shape")) g_SelectedLayer = 1;
        if (ImGui::MenuItem("Solid")) g_SelectedLayer = 2;
        ImGui::EndPopup();
    }
    ImGui::Separator();

    const float namesW = std::min(310.0f, totalW * 0.25f);
    ImGui::BeginChild("TimelineLayerList", ImVec2(namesW, 0), true);
    ImGui::TextDisabled("Source Name                         Mode");
    ImGui::Separator();
    const char* layers[] = {"Shape Layer 2", "Shape Layer 1", "Background"};
    for (int i = 0; i < 3; ++i) {
        ImGui::PushID(i);
        if (ImGui::Selectable(layers[i], g_SelectedLayer == i, 0, ImVec2(0, 38))) g_SelectedLayer = i;
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("TimelineTracks", ImVec2(0, 0), true);
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float trackW = ImGui::GetContentRegionAvail().x;
    ImDrawList* td = ImGui::GetWindowDrawList();
    const float rulerH = 32.0f;
    td->AddRectFilled(origin, ImVec2(origin.x + trackW, origin.y + rulerH), IM_COL32(37, 40, 47, 255));
    for (int i = 0; i <= 12; ++i) {
        float x = origin.x + trackW * i / 12.0f;
        td->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + rulerH), IM_COL32(88, 94, 105, 255));
        char label[16]; snprintf(label, sizeof(label), "%02ds", i);
        td->AddText(ImVec2(x + 3, origin.y + 5), IM_COL32(190, 195, 204, 255), label);
    }
    for (int i = 0; i < 3; ++i) {
        float y = origin.y + rulerH + 8 + i * 45.0f;
        td->AddLine(ImVec2(origin.x, y + 38), ImVec2(origin.x + trackW, y + 38), IM_COL32(57, 63, 73, 255));
        td->AddRectFilled(ImVec2(origin.x + 4, y + 5), ImVec2(origin.x + trackW * (0.96f - i * 0.06f), y + 31),
            i == g_SelectedLayer ? IM_COL32(71, 111, 173, 255) : IM_COL32(67, 83, 108, 255), 3);
        for (int k = 0; k < 2 + (i == 0); ++k) {
            float kx = origin.x + trackW * (0.02f + k * 0.13f);
            td->AddQuadFilled(ImVec2(kx, y + 12), ImVec2(kx + 5, y + 18), ImVec2(kx, y + 24), ImVec2(kx - 5, y + 18), IM_COL32(230, 193, 94, 255));
        }
    }
    ImGui::InvisibleButton("TimelineScrub", ImVec2(trackW, rulerH + 145));
    if (ImGui::IsItemActive()) g_Playhead = std::clamp((ImGui::GetIO().MousePos.x - origin.x) / std::max(1.0f, trackW), 0.0f, 1.0f);
    float playX = origin.x + trackW * g_Playhead;
    td->AddLine(ImVec2(playX, origin.y), ImVec2(playX, origin.y + rulerH + 145), IM_COL32(82, 157, 246, 255), 2.0f);
    td->AddTriangleFilled(ImVec2(playX - 6, origin.y), ImVec2(playX + 6, origin.y), ImVec2(playX, origin.y + 9), IM_COL32(82, 157, 246, 255));
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
