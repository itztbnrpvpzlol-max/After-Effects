cmake_minimum_required(VERSION 3.22.1)
project(aftereffects LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_BACKENDS ${imgui_SOURCE_DIR}/backends)
add_library(imgui_android STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS}/imgui_impl_android.cpp
    ${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_android PUBLIC ${imgui_SOURCE_DIR} ${IMGUI_BACKENDS})
target_link_libraries(imgui_android PUBLIC android EGL GLESv3)

add_library(aftereffects SHARED main.cpp)
target_include_directories(aftereffects PRIVATE ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)
target_sources(aftereffects PRIVATE ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_link_options(aftereffects PRIVATE "-u" "ANativeActivity_onCreate")
target_link_libraries(aftereffects PRIVATE imgui_android android log EGL GLESv3)
