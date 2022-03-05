cmake_minimum_required(VERSION 3.2)

project(OpenCL_test)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
link_directories(
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.0/lib/x64/"
    vendor/
    # build/Release
    C:/msys64/mingw64/lib
)
include_directories(
    src/
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.0/include"
    vendor/glad/include
    vendor/glm/
    vendor/imgui/
    C:/msys64/mingw64/include
)
add_executable(app
    
    src/main.cpp
    src/RNDGenerator.cpp
)

# GLAD
add_subdirectory(vendor/glad)

include_directories(vendor/glad/include)
target_link_libraries(app glad ${GLAD_LIBRARIES})

#ImGUI
add_library(imgui STATIC
    vendor/imgui/backends/imgui_impl_opengl3.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp    
)

target_link_libraries(imgui
    -lglfw3
     # link with imm32.lib because I am on windows and not using visual studio !!
    imm32.lib
)
target_link_libraries(app
    -lOpenCL
    -lglfw3

)

target_include_directories(app PRIVATE vendor/imgui)
target_link_libraries(app imgui ${IMGUI_LIBRARIES})
