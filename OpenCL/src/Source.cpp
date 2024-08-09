#include "Chunks/Chunk.h"
#include <chrono>

#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

int func() {
    cl_uint numPlatforms;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

    for (cl_platform_id platform : platforms) {
        cl_uint numDevices;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
        std::vector<cl_device_id> devices(numDevices);
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr);

        for (cl_device_id device : devices) {
            char deviceName[256];
            clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr);
            std::cout << "Device: " << deviceName << std::endl;

            cl_ulong localMemSize;
            clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMemSize), &localMemSize, nullptr);
            std::cout << "Local Memory Size: " << localMemSize << " bytes" << std::endl;
        }
    }

    return 0;
}


std::string loadKernelSource(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        exit(EXIT_FAILURE);
    }
    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

int main() {
    func();

    Chunk chunk {};

    int face_planes_size = Chunk::FACES_COUNT_PER_CUBE * Chunk::DIRECTION_SIZE * Chunk::DIRECTION_SIZE;
    int X_rows_size = Chunk::HEIGHT * Chunk::DEPTH;
    int Y_rows_size = Chunk::DEPTH * Chunk::WIDTH;
    int Z_rows_size = Chunk::WIDTH * Chunk::HEIGHT;

    cl_int err;
    cl_uint numPlatforms;
    cl_platform_id platform = nullptr;
    clGetPlatformIDs(1, &platform, &numPlatforms);

    cl_uint numDevices;
    cl_device_id device = nullptr;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices);

    size_t maxWorkGroupSize;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, nullptr);
    std::cout << "Maximum Work-Group Size: " << maxWorkGroupSize << '\n';


    size_t maxWorkItemSizes[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, nullptr);
    std::cout << "Max Work-Item Sizes: X=" << maxWorkItemSizes[0]
        << ", Y=" << maxWorkItemSizes[1]
        << ", Z=" << maxWorkItemSizes[2] << '\n';
    std::cout << '\n';


    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);


    std::string kernelSource = loadKernelSource("kernel.cl");
    const char* kernelSourceCStr = kernelSource.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSourceCStr, nullptr, &err);


    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "Error in kernel: " << std::endl << log.data() << std::endl;
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return EXIT_FAILURE;
    }

    cl_mem voxelsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Voxel) * Chunk::VOLUME, chunk.voxels_, &err);
    cl_mem facePlanesBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t) * face_planes_size / 2, nullptr, &err);
    cl_mem XRowsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t) * X_rows_size / 2, nullptr, &err);
    cl_mem YRowsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t) * Y_rows_size / 2, nullptr, &err);
    cl_mem ZRowsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t) * Z_rows_size / 2, nullptr, &err);
    cl_mem vertexBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint64_t) * 4 * 6 * 16 * 16 * 16, nullptr, &err);
    cl_mem vertexDataSizeBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t) * 1, nullptr, &err);


    cl_event kernel_event;

    cl_kernel kernel1 = clCreateKernel(program, "PreCulling", &err);
    size_t globalWorkSize1[] = {Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH};

    clSetKernelArg(kernel1, 0, sizeof(cl_mem), &voxelsBuffer);
    clSetKernelArg(kernel1, 1, sizeof(cl_mem), &XRowsBuffer);
    clSetKernelArg(kernel1, 2, sizeof(cl_mem), &YRowsBuffer);
    clSetKernelArg(kernel1, 3, sizeof(cl_mem), &ZRowsBuffer);

    clEnqueueNDRangeKernel(queue, kernel1, 3, nullptr, globalWorkSize1, nullptr, 0, nullptr, &kernel_event);

    auto start = std::chrono::high_resolution_clock::now();
    clFinish(queue);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time taken by 'PreCulling': " << duration.count() << " us\n";
    cl_ulong time_start, time_end;
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    std::cout << "Kernel execution time: " << (time_end - time_start) << " ns\n\n";


    cl_kernel kernel2 = clCreateKernel(program, "Culling", &err);
    size_t globalWorkSize2[] = {Chunk::FACES_COUNT_PER_CUBE, Chunk::DIRECTION_SIZE, Chunk::DIRECTION_SIZE};
    //size_t localWorkSize2[] = {Chunk::FACES_COUNT_PER_CUBE, Chunk::DIRECTION_SIZE, Chunk::DIRECTION_SIZE};

    clSetKernelArg(kernel2, 0, sizeof(cl_mem), &XRowsBuffer);
    clSetKernelArg(kernel2, 1, sizeof(cl_mem), &YRowsBuffer);
    clSetKernelArg(kernel2, 2, sizeof(cl_mem), &ZRowsBuffer);
    clSetKernelArg(kernel2, 3, sizeof(cl_mem), &facePlanesBuffer);

    //clEnqueueNDRangeKernel(queue, kernel2, 3, nullptr, globalWorkSize2, localWorkSize2, 0, nullptr, &kernel_event);
    clEnqueueNDRangeKernel(queue, kernel2, 3, nullptr, globalWorkSize2, nullptr, 0, nullptr, &kernel_event);

    start = std::chrono::high_resolution_clock::now();
    clFinish(queue);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time taken by 'Culling': " << duration.count() << " us\n";
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    std::cout << "Kernel execution time: " << (time_end - time_start) << " ns\n\n";


    cl_kernel kernel3 = clCreateKernel(program, "GreedyMesh8bit", &err);
    size_t globalWorkSize3[] = {6, 16, 4};
    size_t localWorkSize3[] = {6, 16, 4};

    cl_ulong privateMemSize1;
    clGetKernelWorkGroupInfo(kernel3, device, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(privateMemSize1), &privateMemSize1, nullptr);
    std::cout << "Private Memory Size: " << privateMemSize1 << " bytes" << std::endl;

    clSetKernelArg(kernel3, 0, sizeof(cl_mem), &voxelsBuffer);
    clSetKernelArg(kernel3, 1, sizeof(cl_mem), &facePlanesBuffer);
    clSetKernelArg(kernel3, 2, sizeof(cl_mem), &vertexBuffer);
    clSetKernelArg(kernel3, 3, sizeof(cl_mem), &vertexDataSizeBuffer);

    clEnqueueNDRangeKernel(queue, kernel3, 3, nullptr, globalWorkSize3, localWorkSize3, 0, nullptr, &kernel_event);

    start = std::chrono::high_resolution_clock::now();
    clFinish(queue);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time taken by 'GreedyMesh8bit': " << duration.count() << " us\n";
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    std::cout << "Kernel execution time: " << (time_end - time_start) << " ns\n\n";

    unsigned int vertex_data_size1 = -1;
    clEnqueueReadBuffer(queue, vertexDataSizeBuffer, CL_TRUE, 0, sizeof(uint32_t) * 1, &vertex_data_size1, 0, nullptr, nullptr);


    cl_kernel kernel4 = clCreateKernel(program, "GreedyMesh16bit", &err);
    size_t globalWorkSize4[] = {Chunk::FACES_COUNT_PER_CUBE, Chunk::DIRECTION_SIZE};
    size_t localWorkSize4[] = {Chunk::FACES_COUNT_PER_CUBE, Chunk::DIRECTION_SIZE};

    cl_ulong privateMemSize2;
    clGetKernelWorkGroupInfo(kernel4, device, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(privateMemSize2), &privateMemSize2, nullptr);
    std::cout << "Private Memory Size: " << privateMemSize2 << " bytes" << std::endl;

    clSetKernelArg(kernel4, 0, sizeof(cl_mem), &voxelsBuffer);
    clSetKernelArg(kernel4, 1, sizeof(cl_mem), &facePlanesBuffer);
    clSetKernelArg(kernel4, 2, sizeof(cl_mem), &vertexBuffer);
    clSetKernelArg(kernel4, 3, sizeof(cl_mem), &vertexDataSizeBuffer);

    clEnqueueNDRangeKernel(queue, kernel4, 2, nullptr, globalWorkSize4, localWorkSize4, 0, nullptr, &kernel_event);

    start = std::chrono::high_resolution_clock::now();
    clFinish(queue);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time taken by 'GreedyMesh16bit': " << duration.count() << " us\n";
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    std::cout << "Kernel execution time: " << (time_end - time_start) << " ns\n\n";

    unsigned int vertex_data_size2 = -1;
    clEnqueueReadBuffer(queue, vertexDataSizeBuffer, CL_TRUE, 0, sizeof(uint32_t) * 1, &vertex_data_size2, 0, nullptr, nullptr);

    std::cout << vertex_data_size1 << ' ' << vertex_data_size2 << '\n';

    /*uint16_t* face_planes = new uint16_t[6 * 256];
    clEnqueueReadBuffer(queue, facePlanesBuffer, CL_TRUE, 0, sizeof(uint16_t) * 6 * 256, face_planes, 0, nullptr, nullptr);
    for (int k = 0; k < 6; ++k) {
        std::cout << k << '\n';
        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 16; ++j) {
                std::cout << std::bitset<16>(face_planes[k * 256 + i * 16 + j]) << '\n';
            }
            std::cout << '\n';
        }
        std::cout << "\n\n";
    }
    delete[] face_planes;*/

    /*uint64_t* vertex_data = new uint64_t[4 * 10];
    clEnqueueReadBuffer(queue, vertexBuffer, CL_TRUE, 0, sizeof(uint64_t) * 4 * 10, vertex_data, 0, nullptr, nullptr);
    for (int i = 0; i < 4 * 10; i += 4) {
        for (int j = i; j < i + 4; ++j) {
            std::cout << ((vertex_data[j] >> 52) & 0xF) << ' ' << ((vertex_data[j] >> 48) & 0xF) << ' ' << ((vertex_data[j] >> 44) & 0xF) << ' ' << ((vertex_data[j] >> 40) & 0xF) << ' ' << ((vertex_data[j] >> 32) & 0xFF) << ' ' << ((vertex_data[j] >> 30) & 0x3) << ' ' << ((vertex_data[j] >> 24) & 0x3F) << ' ' << ((vertex_data[j] >> 18) & 0x3F) << ' ' << ((vertex_data[j] >> 12) & 0x3F) << ' ' << ((vertex_data[j] >> 6) & 0x3F) << ' ' << (vertex_data[j] & 0x3F) << '\n';
        }
        std::cout << '\n';
    }
    delete[] vertex_data;*/



    clReleaseMemObject(voxelsBuffer);
    clReleaseMemObject(facePlanesBuffer);
    clReleaseMemObject(XRowsBuffer);
    clReleaseMemObject(YRowsBuffer);
    clReleaseMemObject(ZRowsBuffer);
    clReleaseMemObject(vertexBuffer);
    clReleaseMemObject(vertexDataSizeBuffer);
    clReleaseEvent(kernel_event);
    clReleaseKernel(kernel1);
    clReleaseKernel(kernel2);
    clReleaseKernel(kernel3);
    clReleaseKernel(kernel4);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);


    std::cout << std::endl;

    return 0;
}
