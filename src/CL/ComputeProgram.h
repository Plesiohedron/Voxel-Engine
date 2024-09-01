#pragma once

#include "../Exceptions/Exceptions.h"

#include <CL/cl.h>
#include <vector>
#include <string>

namespace CL {
class Program {
private:
    cl_context context_;
    cl_command_queue command_queue_;
    cl_program program_;

    std::vector<cl_mem> buffers_;
    std::vector<cl_kernel> kernels_;

public:
    std::vector<std::vector<size_t>> global_work_sizes;

private:
    std::string LoadShader(const std::string& file_name);

public:
    Program(const std::string& name);
    ~Program();

    void CreateBuffer(cl_mem_flags flags, size_t data_size);
    void RecreateBuffer(cl_mem_flags flags, size_t data_size, size_t buffer_number);

    template<typename T>
    void WriteToBuffer(const std::vector<T>& input_data, size_t buffer_number) {
        clEnqueueWriteBuffer(command_queue_, buffers_[buffer_number], CL_TRUE, 0, sizeof(T) * input_data.size(), input_data.data(), 0, nullptr, nullptr);
    };

    template<typename U>
    void ReadFromBuffer(std::vector<U>& output_data, size_t buffer_number) {
        clEnqueueReadBuffer(command_queue_, buffers_[buffer_number], CL_TRUE, 0, sizeof(U) * output_data.size(), output_data.data(), 0, nullptr, nullptr);
    };


    void CreateKernel(const std::string& name, const std::vector<size_t>& global_work_size, const std::vector<size_t>& buffer_numbers);
    void EnqueueKernel(size_t kernel_number) const;
    void WaitQueue() const;
};
} // namespace CL