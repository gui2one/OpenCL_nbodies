#include <iostream>
#include <vector>
#include <fstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
// #define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

const char *sim_kernel_path = "C:/gui2one/CODE/OpenCL_nbodies/cl_kernels/simulation.ocl";

typedef struct SimPoint_struct
{
    float position[3];
    float mass = 1.f;
    float radius = 1.0f;
    bool collided = false;
} SimPoint;

void BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode /*|ImGuiDockNodeFlags_NoResize*/);
}

void EndFrame()
{

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    ImGuiIO &io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        glfwMakeContextCurrent(backup_current_context);
    }
}

std::vector<cl::Platform> ListPlatforms()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    return platforms;
}

std::vector<std::vector<cl::Device>> ListDevices()
{

    std::vector<cl::Platform> platforms = ListPlatforms();
    std::cout << "Num Platforms :" << platforms.size() << std::endl;
    std::vector<std::vector<cl::Device>> devices_list;
    for (auto it = platforms.begin(); it != platforms.end(); ++it)
    {
        std::vector<cl::Device> devices;
        // devices.clear();
        auto cur_platform = *it;

        cur_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        devices_list.push_back(devices);
    }

    return devices_list;
}

void PrintDevicesList(std::vector<std::vector<cl::Device>> list)
{
    for (auto it = list.begin(); it != list.end(); ++it)
    {

        auto cur_list = *it;

        for (auto it2 = cur_list.begin(); it2 != cur_list.end(); ++it2)
        {

            auto cur_device = *it2;
            std::cout << "Device Properties" << std::endl;
            std::cout << "\tName : " << cur_device.getInfo<CL_DEVICE_NAME>() << std::endl;
        }
    }

    std::cout << "--------------------------------" << std::endl;
}

void LoadKernelSource(const char *path, std::string &src)
{
    //// load kernel source
    std::ifstream sim_source_file(path);

    std::string line;
    while (std::getline(sim_source_file, line))
    {
        src += line + "\n";
    }
}

void PrintKernalSource(std::string source_string)
{

    std::cout << "KERNEL SOURCE" << std::endl;
    std::cout << source_string << std::endl;
}

cl::Program BuildProgram(cl::Context context, std::string prog_source, std::vector<std::vector<cl::Device>> devices_list)
{
    // Compile OpenCL program for found device.
    cl::Program program(context, cl::Program::Sources(1, std::make_pair(prog_source.c_str(), strlen(prog_source.c_str()))));

    try
    {
        program.build(devices_list[0]);
    }
    catch (const cl::Error &)
    {
        std::cerr
            << "OpenCL compilation error" << std::endl
            << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices_list[0][0])
            << std::endl;
    }

    return program;
}

int main()
{

    std::vector<std::vector<cl::Device>> devices_list = ListDevices();
    PrintDevicesList(devices_list);

    std::string sim_kernel_source;
    LoadKernelSource("C:/gui2one/CODE/OpenCL_nbodies/cl_kernels/simulation.ocl", sim_kernel_source);
    PrintKernalSource(sim_kernel_source);

    // choose a device
    cl::Device &device = devices_list[0][0];

    cl::Context context;
    context = cl::Context(device);

    // Create command queue.
    cl::CommandQueue queue(context, device);

    cl::Program sim_program = BuildProgram(context, sim_kernel_source, devices_list);

    cl::Kernel simulation(sim_program, "simulation");
    // Prepare input data.
    size_t NUM_POINTS = 10;
    std::vector<SimPoint> sim_points(NUM_POINTS, {1.0f, 2.0f, 3.14f, 1.0f, 1.0f, false});
    std::vector<SimPoint> sim_points_output(NUM_POINTS);

    cl::Buffer SIM_POINTS(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                          sim_points.size() * sizeof(cl_float3), sim_points.data());
    cl::Buffer SIM_POINTS_OUTPUT(context, CL_MEM_READ_WRITE,
                                 sim_points_output.size() * sizeof(cl_float3));
    simulation.setArg(0, static_cast<cl_ulong>(NUM_POINTS));
    simulation.setArg(1, SIM_POINTS);
    simulation.setArg(2, SIM_POINTS_OUTPUT);
    int offset = 0;
    // Launch kernel on the compute device.
    queue.enqueueNDRangeKernel(simulation, offset, NUM_POINTS);

    // Get result back to host.
    queue.enqueueReadBuffer(SIM_POINTS_OUTPUT, CL_TRUE, 0, sim_points_output.size() * sizeof(double), sim_points_output.data());

    // Should get '3' here.
    std::cout << sim_points_output.size() << std::endl;
    std::cout << sim_points_output[0].position[0] << " -- " << sim_points_output[0].position[1] << " -- " << sim_points_output[0].position[2] << std::endl;

    glfwInit();
    GLFWwindow *win = glfwCreateWindow(500, 500, "OpenCL Baby !!!", NULL, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    glfwMakeContextCurrent(win);
    gladLoadGL();

    ImGui_ImplGlfw_InitForOpenGL(win, true);
    const char *glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
    while (!glfwWindowShouldClose(win))
    {

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

        BeginFrame();
        ImGui::Begin("hello");
        ImGui::End();

        EndFrame();
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    return 0;
}