
#include <cassert>

#include "include/randomSequenceGenerator.hpp"
#include "GPUrandomSequenceGenerator.hpp"

/* static */ const std::string CGPURandomSequenceGenerator::_clProgram = R"(
uchar LCG1(global uint* state)
{
    *state = (*state * 214013U + 2531011U) & 0xffffffff;
    return (*state >> 16) & 0xff;
}

uchar LCG2(global uint* state)
{
    *state = (*state * 1103515245 + 12345) & 0xffffffff;
    return (*state >> 16) & 0xff;
}

kernel void GenerateRandomNumber (
    global uint* lcg1State,
    global uint* lcg2State,
    global uchar* result
)
{
    ulong N = get_global_id(0);
    result[N] = LCG1(lcg1State + N) ^ LCG2(lcg2State + N);
}
)";

/* static */ bool CGPURandomSequenceGenerator::CheckClStatus(cl_int status, bool throwException)
{
    if (!status)
        return true;
    else if (!throwException)
        return false;
    else
    {
        switch (status)
        {
            // run-time and JIT compiler errors
//        case 0: return "CL_SUCCESS");
        case  -1: throw std::runtime_error("CL_DEVICE_NOT_FOUND");
        case  -2: throw std::runtime_error("CL_DEVICE_NOT_AVAILABLE");
        case  -3: throw std::runtime_error("CL_COMPILER_NOT_AVAILABLE");
        case  -4: throw std::runtime_error("CL_MEM_OBJECT_ALLOCATION_FAILURE");
        case  -5: throw std::runtime_error("CL_OUT_OF_RESOURCES");
        case  -6: throw std::runtime_error("CL_OUT_OF_HOST_MEMORY");
        case  -7: throw std::runtime_error("CL_PROFILING_INFO_NOT_AVAILABLE");
        case  -8: throw std::runtime_error("CL_MEM_COPY_OVERLAP");
        case  -9: throw std::runtime_error("CL_IMAGE_FORMAT_MISMATCH");
        case -10: throw std::runtime_error("CL_IMAGE_FORMAT_NOT_SUPPORTED");
        case -11: throw std::runtime_error("CL_BUILD_PROGRAM_FAILURE");
        case -12: throw std::runtime_error("CL_MAP_FAILURE");
        case -13: throw std::runtime_error("CL_MISALIGNED_SUB_BUFFER_OFFSET");
        case -14: throw std::runtime_error("CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST");
        case -15: throw std::runtime_error("CL_COMPILE_PROGRAM_FAILURE");
        case -16: throw std::runtime_error("CL_LINKER_NOT_AVAILABLE");
        case -17: throw std::runtime_error("CL_LINK_PROGRAM_FAILURE");
        case -18: throw std::runtime_error("CL_DEVICE_PARTITION_FAILED");
        case -19: throw std::runtime_error("CL_KERNEL_ARG_INFO_NOT_AVAILABLE");

            // compile-time errors
        case -30: throw std::runtime_error("CL_INVALID_VALUE");
        case -31: throw std::runtime_error("CL_INVALID_DEVICE_TYPE");
        case -32: throw std::runtime_error("CL_INVALID_PLATFORM");
        case -33: throw std::runtime_error("CL_INVALID_DEVICE");
        case -34: throw std::runtime_error("CL_INVALID_CONTEXT");
        case -35: throw std::runtime_error("CL_INVALID_QUEUE_PROPERTIES");
        case -36: throw std::runtime_error("CL_INVALID_COMMAND_QUEUE");
        case -37: throw std::runtime_error("CL_INVALID_HOST_PTR");
        case -38: throw std::runtime_error("CL_INVALID_MEM_OBJECT");
        case -39: throw std::runtime_error("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR");
        case -40: throw std::runtime_error("CL_INVALID_IMAGE_SIZE");
        case -41: throw std::runtime_error("CL_INVALID_SAMPLER");
        case -42: throw std::runtime_error("CL_INVALID_BINARY");
        case -43: throw std::runtime_error("CL_INVALID_BUILD_OPTIONS");
        case -44: throw std::runtime_error("CL_INVALID_PROGRAM");
        case -45: throw std::runtime_error("CL_INVALID_PROGRAM_EXECUTABLE");
        case -46: throw std::runtime_error("CL_INVALID_KERNEL_NAME");
        case -47: throw std::runtime_error("CL_INVALID_KERNEL_DEFINITION");
        case -48: throw std::runtime_error("CL_INVALID_KERNEL");
        case -49: throw std::runtime_error("CL_INVALID_ARG_INDEX");
        case -50: throw std::runtime_error("CL_INVALID_ARG_VALUE");
        case -51: throw std::runtime_error("CL_INVALID_ARG_SIZE");
        case -52: throw std::runtime_error("CL_INVALID_KERNEL_ARGS");
        case -53: throw std::runtime_error("CL_INVALID_WORK_DIMENSION");
        case -54: throw std::runtime_error("CL_INVALID_WORK_GROUP_SIZE");
        case -55: throw std::runtime_error("CL_INVALID_WORK_ITEM_SIZE");
        case -56: throw std::runtime_error("CL_INVALID_GLOBAL_OFFSET");
        case -57: throw std::runtime_error("CL_INVALID_EVENT_WAIT_LIST");
        case -58: throw std::runtime_error("CL_INVALID_EVENT");
        case -59: throw std::runtime_error("CL_INVALID_OPERATION");
        case -60: throw std::runtime_error("CL_INVALID_GL_OBJECT");
        case -61: throw std::runtime_error("CL_INVALID_BUFFER_SIZE");
        case -62: throw std::runtime_error("CL_INVALID_MIP_LEVEL");
        case -63: throw std::runtime_error("CL_INVALID_GLOBAL_WORK_SIZE");
        case -64: throw std::runtime_error("CL_INVALID_PROPERTY");
        case -65: throw std::runtime_error("CL_INVALID_IMAGE_DESCRIPTOR");
        case -66: throw std::runtime_error("CL_INVALID_COMPILER_OPTIONS");
        case -67: throw std::runtime_error("CL_INVALID_LINKER_OPTIONS");
        case -68: throw std::runtime_error("CL_INVALID_DEVICE_PARTITION_COUNT");

            // extension errors
        case -1000: throw std::runtime_error("CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR");
        case -1001: throw std::runtime_error("CL_PLATFORM_NOT_FOUND_KHR");
        case -1002: throw std::runtime_error("CL_INVALID_D3D10_DEVICE_KHR");
        case -1003: throw std::runtime_error("CL_INVALID_D3D10_RESOURCE_KHR");
        case -1004: throw std::runtime_error("CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR");
        case -1005: throw std::runtime_error("CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR");
        default: throw std::runtime_error("Unknown OpenCL error");
        }
    }
}

/* static */ bool CGPURandomSequenceGenerator::CheckOpenCLdevicesAvailability(void)
{
    cl_uint num_platforms;
    cl_int clStatus = clGetPlatformIDs(0, nullptr, &num_platforms); CheckClStatus(clStatus, false);
    if (!num_platforms)
        return false;

    std::vector<cl_platform_id> platforms(num_platforms);
    clStatus = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);  CheckClStatus(clStatus, false);

    return clStatus == 0;
}

CGPURandomSequenceGenerator::CGPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback) :
    CDoubleBuffersRandomSequenceGenerator(memorySizeInBytes, decreaseThreadPriorityCallback)
{
    InitBase();
}

CGPURandomSequenceGenerator::~CGPURandomSequenceGenerator() noexcept
{
    StartThreadFinish();
}

void CGPURandomSequenceGenerator::FinishThread()
{
    cl_int clStatus;

    clStatus = clReleaseKernel(_clKernel);              CheckClStatus(clStatus);
    clStatus = clReleaseMemObject(_lce1);               CheckClStatus(clStatus);
    clStatus = clReleaseMemObject(_lce2);               CheckClStatus(clStatus);
    clStatus = clReleaseProgram(_program);              CheckClStatus(clStatus);
    clStatus = clReleaseCommandQueue(_commandQueue);    CheckClStatus(clStatus);
    clStatus = clReleaseContext(_context);              CheckClStatus(clStatus);
}

void CGPURandomSequenceGenerator::AllocBuffers(size_t buffers, size_t bytesInBuffer)
{
    _buf.resize(buffers);

    for (auto& buf : _buf)
        buf.resize(bytesInBuffer);
}

bool CGPURandomSequenceGenerator::ImplInit()
{
    if (!CheckOpenCLdevicesAvailability())
        return false;

    cl_uint num_platforms;
    cl_int clStatus = clGetPlatformIDs(0, nullptr, &num_platforms); CheckClStatus(clStatus);
    if (!num_platforms)
        return false;

    std::vector<cl_platform_id> platforms(num_platforms);
    clStatus = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);  CheckClStatus(clStatus);

    if (clStatus)
        return false;

    cl_uint num_devices;

    clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);  CheckClStatus(clStatus);
    std::vector<cl_device_id> device_list(num_devices);
    clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices, device_list.data(), nullptr);  CheckClStatus(clStatus);

    _context = clCreateContext(NULL, num_devices, device_list.data(), nullptr, nullptr, &clStatus); CheckClStatus(clStatus);
    if (clStatus)
        return false;

    cl_command_queue command_queue = clCreateCommandQueueWithProperties(_context, device_list[0], 0, &clStatus); CheckClStatus(clStatus);
    if (!command_queue)
        return false;

    const char* clProgramPtr = _clProgram.c_str();
    _program = clCreateProgramWithSource(_context, 1, static_cast<const char**>(&clProgramPtr), nullptr, &clStatus); CheckClStatus(clStatus);
    if (!_program)
        return false;

    clStatus = clBuildProgram(_program, 1, device_list.data(), nullptr, nullptr, nullptr);  CheckClStatus(clStatus);
    if (clStatus != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        printf("Error:Failed to build program executable!");
        clGetProgramBuildInfo(_program, device_list[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s \n", buffer);
    }
    _commandQueue = clCreateCommandQueueWithProperties(_context, device_list[0], 0, &clStatus); CheckClStatus(clStatus);

    std::mt19937_64 mtGen;
    mtGen.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> distribution;

    const size_t bufferSize = BufferSize();
    std::vector<uint32_t> lce1(bufferSize);
    std::vector<uint32_t> lce2(bufferSize);
    for (size_t i = 0; i < bufferSize; ++i)
    {
        lce1[i] = distribution(mtGen);
        lce2[i] = distribution(mtGen);
    }

    _lce1 = clCreateBuffer(_context, CL_MEM_READ_WRITE, bufferSize * sizeof(decltype(lce1)::value_type), nullptr, &clStatus);  CheckClStatus(clStatus);
    clStatus = clEnqueueWriteBuffer(_commandQueue, _lce1, CL_TRUE, 0, bufferSize * sizeof(decltype(lce1)::value_type), lce1.data(), 0, nullptr, nullptr); CheckClStatus(clStatus);

    _lce2 = clCreateBuffer(_context, CL_MEM_READ_WRITE, bufferSize * sizeof(decltype(lce2)::value_type), nullptr, &clStatus);  CheckClStatus(clStatus);
    clStatus = clEnqueueWriteBuffer(_commandQueue, _lce2, CL_TRUE, 0, bufferSize * sizeof(decltype(lce2)::value_type), lce2.data(), 0, nullptr, nullptr); CheckClStatus(clStatus);

    _res = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, bufferSize * sizeof(TByte), nullptr, &clStatus);  CheckClStatus(clStatus);

    _clKernel = clCreateKernel(_program, "GenerateRandomNumber", &clStatus);      CheckClStatus(clStatus);

    enum class EArgPos : cl_uint { lcg1State = 0, lcg2State, result };

    auto setKernelArg = [this](EArgPos argPos, auto& arg)
    {
        using TArg = std::remove_reference_t<decltype(arg)>;
        return clSetKernelArg(_clKernel, static_cast<cl_uint>(argPos), sizeof(TArg), static_cast<void*>(&arg));
    };

    clStatus = setKernelArg(EArgPos::lcg1State, _lce1); CheckClStatus(clStatus);
    clStatus = setKernelArg(EArgPos::lcg2State, _lce2); CheckClStatus(clStatus);
    clStatus = setKernelArg(EArgPos::result, _res);     CheckClStatus(clStatus);

    return true;
}

bool CGPURandomSequenceGenerator::FillBuffer(size_t bufferId)
{
    using namespace std::chrono;

    assert(bufferId < BuffersAmount());

    size_t bufferSize = BufferSize();

    auto startCalc = steady_clock::now();
    cl_int clStatus = clEnqueueNDRangeKernel(_commandQueue, _clKernel, 1, nullptr, &bufferSize, nullptr, 0, nullptr, nullptr);     CheckClStatus(clStatus);
    auto endCalc = steady_clock::now();
    auto calcDuration = endCalc - startCalc;

    auto startRead = steady_clock::now();

    TBuffer& buf = _buf[bufferId];
    clStatus = clEnqueueReadBuffer(_commandQueue, _res, CL_TRUE, 0, bufferSize, buf.data(), 0, nullptr, nullptr);
    CheckClStatus(clStatus);

    clStatus = clFlush(_commandQueue);      CheckClStatus(clStatus);
    clStatus = clFinish(_commandQueue);     CheckClStatus(clStatus);
    auto endRead = steady_clock::now();
    auto readDuration = endRead - startRead;

    SStatistics stat;
    stat._generate = std::chrono::duration_cast<SStatistics::TTimeMeasurement>(calcDuration);
    stat._store = std::chrono::duration_cast<SStatistics::TTimeMeasurement>(readDuration);
    stat._bufSize = BufferSize();

    SetStatistics(stat);

    return true;
}

CGPURandomSequenceGenerator::TByte* CGPURandomSequenceGenerator::Array(size_t bufferId) noexcept
{
    assert(bufferId < BuffersAmount());
    return _buf[bufferId].data();
}
