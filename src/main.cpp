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

#include <glm/glm.hpp>
#include "RNDGenerator.h"
#include "SimPoint.h"
const char *sim_kernel_path = "C:/gui2one/CODE/OpenCL_nbodies/cl_kernels/simulation.ocl";

void BeginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode /*|ImGuiDockNodeFlags_NoResize*/);
}

void EndImGuiFrame()
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

std::vector<SimPoint> InitSimPoints(uint32_t num_pts)
{
    std::vector<SimPoint> pts;
    for (size_t i = 0; i < num_pts; i++)
    {
        auto rng = RNDGenerator::GetInstance();
        SimPoint pt;
        pt.position[0] = rng->Float(-1.f, 1.f);
        pt.position[1] = rng->Float(-1.f, 1.f);
        pt.position[2] = rng->Float(-1.f, 1.f);

        pt.velocity[0] = rng->Float(-1.f, 1.f) * 0.01f;
        pt.velocity[1] = 0.0f;
        pt.velocity[2] = 0.0f;

        pt.mass = 1.0f;
        pt.radius = 1.0f;

        pts.push_back(pt);
    }

    return pts;
}

int main()
{

    std::vector<std::vector<cl::Device>> devices_list = ListDevices();
    PrintDevicesList(devices_list);

    std::string sim_kernel_source;
    LoadKernelSource(sim_kernel_path, sim_kernel_source);
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
    size_t NUM_POINTS = 100000;
    // std::vector<SimPoint> sim_points(NUM_POINTS, {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false});
    std::vector<SimPoint> sim_points = InitSimPoints(NUM_POINTS);
    std::vector<SimPoint> sim_points_output(NUM_POINTS);

    // window stuff
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
    static bool sim_started = false;
    while (!glfwWindowShouldClose(win))
    {
        if (sim_started)
            sim_points = sim_points_output;
        sim_started = true;
        cl::Buffer SIM_POINTS(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              sim_points.size() * sizeof(SimPoint), sim_points.data());
        cl::Buffer SIM_POINTS_OUTPUT(context, CL_MEM_READ_WRITE,
                                     sim_points_output.size() * sizeof(SimPoint));
        simulation.setArg(0, static_cast<cl_ulong>(NUM_POINTS));
        simulation.setArg(1, SIM_POINTS);
        simulation.setArg(2, SIM_POINTS_OUTPUT);

        // Launch kernel on the compute device.
        int offset = 0;
        queue.enqueueNDRangeKernel(simulation, offset, NUM_POINTS);

        // Get result back to host.
        queue.enqueueReadBuffer(SIM_POINTS_OUTPUT, CL_TRUE, 0, sim_points_output.size() * sizeof(SimPoint), sim_points_output.data());

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

        glPointSize(3.0f);
        glBegin(GL_POINTS);

        for (size_t i = 0; i < sim_points_output.size(); i++)
        {
            auto position = sim_points_output[i].position;
            glVertex3f(position[0], position[1], position[2]);
        }

        glEnd();
        BeginImGuiFrame();
        ImGui::Begin("hello");

        ImGui::Text("%d particles", sim_points_output.size());
        ImGui::End();

        EndImGuiFrame();
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    return 0;
}