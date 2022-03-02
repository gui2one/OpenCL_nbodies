cmake_minimum_required(VERSION 3.2)

project(OpenCL_test)

link_directories(
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.0/lib/x64/"
)
include_directories(
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.0/include"
    vendor/glad/include
)
add_executable(app
    main.cpp
)

# GLAD
add_subdirectory(vendor/glad)

include_directories(vendor/glad/include)
target_link_libraries(app glad ${GLAD_LIBRARIES})

target_link_libraries(app
    -lOpenCL
    -lglfw3
)
