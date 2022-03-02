#include <iostream>
#include <vector>
#include <fstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
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

int main()
{

    std::ifstream sim_source_file(sim_kernel_path);
    std::string sim_kernel_source;
    std::string line;
    while (std::getline(sim_source_file, line))
    {
        sim_kernel_source += line + "\n";
    }

    std::cout << "SIM KERNEL SOURCE" << std::endl;
    std::cout << sim_kernel_source << std::endl;

    // return 0;
    std::vector<cl::Platform>
        platforms;
    cl::Platform::get(&platforms);
    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.size() == 0)
    {
        std::cout << "No OpenCL device available" << std::endl;
        return -1;
    }
    cl::Device &device = devices[0];
    // int platform_id = 0;
    // int device_id = 0;

    // std::cout << "Number of Platforms: " << platforms.size() << std::endl;
    // for (std::vector<cl::Platform>::iterator it = platforms.begin(); it != platforms.end(); ++it)
    // {
    //     cl::Platform platform(*it);

    //     std::cout << "Platform ID: " << platform_id++ << std::endl;
    //     std::cout << "Platform Name: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    //     std::cout << "Platform Vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;

    //     platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    //     for (std::vector<cl::Device>::iterator it2 = devices.begin(); it2 != devices.end(); ++it2)
    //     {
    //         cl::Device device(*it2);

    //         std::cout << "\tDevice " << device_id++ << ": " << std::endl;
    //         std::cout << "\t\tDevice Name: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    //         std::cout << "\t\tDevice Type: " << device.getInfo<CL_DEVICE_TYPE>();
    //         std::cout << " (GPU: " << CL_DEVICE_TYPE_GPU << ", CPU: " << CL_DEVICE_TYPE_CPU << ")" << std::endl;
    //         std::cout << "\t\tDevice Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;
    //         std::cout << "\t\tDevice Max Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
    //         std::cout << "\t\tDevice Global Memory: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << std::endl;
    //         std::cout << "\t\tDevice Max Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << std::endl;
    //         std::cout << "\t\tDevice Max Allocateable Memory: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << std::endl;
    //         std::cout << "\t\tDevice Local Memory: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << std::endl;
    //         std::cout << "\t\tDevice Available: " << device.getInfo<CL_DEVICE_AVAILABLE>() << std::endl;
    //     }
    //     std::cout << std::endl;
    // }

    cl::Context context;
    context = cl::Context(device);

    // Create command queue.
    cl::CommandQueue queue(context, device);

    // Compile OpenCL program for found device.
    cl::Program sim_program(context, cl::Program::Sources(1, std::make_pair(sim_kernel_source.c_str(), strlen(sim_kernel_source.c_str()))));

    try
    {
        sim_program.build(devices);
    }
    catch (const cl::Error &)
    {
        std::cerr
            << "OpenCL compilation error" << std::endl
            << sim_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
            << std::endl;
        return 1;
    }

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

    glfwMakeContextCurrent(win);
    gladLoadGL();

    while (!glfwWindowShouldClose(win))
    {

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    return 0;
}