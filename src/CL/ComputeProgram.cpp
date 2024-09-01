#include "ComputeProgram.h"

#include <fstream>
#include <sstream>

std::string CL::Program::LoadShader(const std::string& file_name) {
    std::ifstream file("res/cl/" + file_name + ".cl");
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << file_name << std::endl;
        exit(EXIT_FAILURE);
    }
    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

CL::Program::Program(const std::string& name) {
    cl_int err = 0;
    cl_uint numPlatforms;
    cl_platform_id platform = nullptr;
    clGetPlatformIDs(1, &platform, &numPlatforms);

    cl_uint numDevices;
    cl_device_id device = nullptr;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices);

    context_ = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err) {
        std::cout << "Failed to create context: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }

    command_queue_ = clCreateCommandQueue(context_, device, 0, &err);
    if (err) {
        std::cout << "Failed to create command queue: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string sources = LoadShader(name);
    const char* shader_sources = sources.c_str();
    program_ = clCreateProgramWithSource(context_, 1, &shader_sources, nullptr, &err);
    if (err) {
        std::cout << "Failed to create program: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }


    err = clBuildProgram(program_, 1, &device, nullptr, nullptr, nullptr);
    if (err) {
        size_t log_size;
        clGetProgramBuildInfo(program_, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program_, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        std::cerr << "Error in kernel: " << '\n' << log.data() << std::endl;
        std::exit(EXIT_FAILURE);
    }

}

CL::Program::~Program() {
    for (const cl_mem& buffer : buffers_) {
        clReleaseMemObject(buffer);
    }

    for (const cl_kernel& kernel : kernels_) {
        clReleaseKernel(kernel);
    }

    clReleaseProgram(program_);
    clReleaseCommandQueue(command_queue_);
    clReleaseContext(context_);
}


void CL::Program::CreateBuffer(cl_mem_flags flags, size_t data_size) {
    cl_int err = 0;

    buffers_.emplace_back(clCreateBuffer(context_, flags, data_size, nullptr, &err));
    if (err) {
        std::cout << "Failed to create buffer: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void CL::Program::RecreateBuffer(cl_mem_flags flags, size_t data_size, size_t buffer_number) {
    cl_int err = 0;

    clReleaseMemObject(buffers_[buffer_number]);

    buffers_[buffer_number] = clCreateBuffer(context_, flags, data_size, nullptr, &err);
    if (err) {
        std::cout << "Failed to recreate buffer: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void CL::Program::CreateKernel(const std::string& name, const std::vector<size_t>& global_work_size, const std::vector<size_t>& buffer_numbers) {
    cl_int err = 0;
    
    kernels_.emplace_back(clCreateKernel(program_, name.c_str(), &err));
    if (err) {
        std::cout << "Failed to create kernel: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0; i < buffer_numbers.size(); ++i) {
        err = clSetKernelArg(kernels_.back(), i, sizeof(cl_mem), &buffers_[buffer_numbers[i]]);
        if (err) {
            std::cout << "Failed to create kernel: " << err << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    global_work_sizes.push_back(global_work_size);
}

void CL::Program::EnqueueKernel(size_t kernel_number) const {
    clEnqueueNDRangeKernel(command_queue_, kernels_[kernel_number], global_work_sizes[kernel_number].size(), nullptr,
                           global_work_sizes[kernel_number].data(), nullptr, 0, nullptr, nullptr);
}

void CL::Program::WaitQueue() const {
    clFinish(command_queue_);
}