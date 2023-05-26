
#ifndef RANDOM_SEQUENCE_GENERATOR_GPU_IMPLEMENTATION_
#define RANDOM_SEQUENCE_GENERATOR_GPU_IMPLEMENTATION_

#include <memory>
#include <random>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "doubleBuffersRandomSequenceGenerator.hpp"

class CGPURandomSequenceGenerator : public CDoubleBuffersRandomSequenceGenerator
{
public:
    CGPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback);
    ~CGPURandomSequenceGenerator() noexcept override;

    static bool CheckOpenCLdevicesAvailability();

private:
    static const std::string _clProgram;

    std::vector<TBuffer> _buf;

    cl_context _context;
    cl_program _program;
    cl_command_queue _commandQueue;
    cl_kernel _clKernel;
    cl_mem _lce1;
    cl_mem _lce2;
    cl_mem _res;

    void AllocBuffers(size_t buffers, size_t bytesInBuffer) override;
    bool ImplInit() override;
    bool FillBuffer(size_t bufferNum) override;
    void FinishThread() override;
    TByte* Array(size_t bufferNum) noexcept override;

    static bool CheckClStatus(cl_int status, bool throwException = true);
};

#endif // RANDOM_SEQUENCE_GENERATOR_CPU_IMPLEMENTATION_
