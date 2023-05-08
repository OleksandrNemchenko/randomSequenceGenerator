
#include "EasyCL.hpp"

///////////////////////////////////////////////////////////////////////////////
// Error Class Definition
///////////////////////////////////////////////////////////////////////////////
int ecl::Error::error = 0;

void ecl::Error::checkError(const std::string& where) {
    if (error != 0)
        throw std::runtime_error(where + ": " + getErrorString());
}

std::string ecl::Error::getErrorString() {
    switch (error) {
    case -1:
        return "device not found";
        break;
    case -2:
        return "device not available";
        break;
    case -3:
        return "compiler not available";
        break;
    case -4:
        return "mem object allocation failure";
        break;
    case -5:
        return "out of resources";
        break;
    case -6:
        return "out of host memory";
        break;
    case -7:
        return "profiling info not available";
        break;
    case -8:
        return "mem copy overlap";
        break;
    case -9:
        return "image format mismatch";
        break;
    case -10:
        return "image format not supported";
        break;
    case -11:
        return "build program failure";
        break;
    case -12:
        return "map failure";
        break;
    case -13:
        return "misaligned sub buffer offset";
        break;
    case -14:
        return "exec status error for events in wait list";
        break;
    case -15:
        return "compile program failure";
        break;
    case -16:
        return "linker not available";
        break;
    case -17:
        return "link program failure";
        break;
    case -18:
        return "device partition failed";
        break;
    case -19:
        return "kernel argument info not available";
        break;
    case -30:
        return "invalid value";
        break;
    case -31:
        return "invalid device type";
        break;
    case -32:
        return "invalid platform";
        break;
    case -33:
        return "invalid device";
        break;
    case -34:
        return "invalid context";
        break;
    case -35:
        return "invalid queue properties";
        break;
    case -36:
        return "invalid command queue";
        break;
    case -37:
        return "invalid host ptr";
        break;
    case -38:
        return "invalid mem object";
        break;
    case -39:
        return "invalid image format descriptor";
        break;
    case -40:
        return "invalid image size";
        break;
    case -41:
        return "invalid sampler";
        break;
    case -42:
        return "invalid binary";
        break;
    case -43:
        return "invalid build options";
        break;
    case -44:
        return "invalid program";
        break;
    case -45:
        return "invalid program executable";
        break;
    case -46:
        return "invalid kernel name";
        break;
    case -47:
        return "invalid kernel definition";
        break;
    case -48:
        return "invalid kernel";
        break;
    case -49:
        return "invalid argument index";
        break;
    case -50:
        return "invalid argument value";
        break;
    case -51:
        return "invalid argument size";
        break;
    case -52:
        return "invalid kernel arguments";
        break;
    case -53:
        return "invalid work dimension";
        break;
    case -54:
        return "invalid work group size";
        break;
    case -55:
        return "invalid work item size";
        break;
    case -56:
        return "invalid global offset";
        break;
    case -57:
        return "invalid event wait list";
        break;
    case -58:
        return "invalid event";
        break;
    case -59:
        return "invalid operation";
        break;
    case -60:
        return "invalid gl object";
        break;
    case -61:
        return "invalid buffer size";
        break;
    case -62:
        return "invalid mip level";
        break;
    case -63:
        return "invalid global work size";
        break;
    case -64:
        return "invalid property";
        break;
    case -65:
        return "invalid image descriptor";
        break;
    case -66:
        return "invalid compiler options";
        break;
    case -67:
        return "invalid linker options";
        break;
    case -68:
        return "invalid device partition count";
        break;
    case -1000:
        return "invalid gl sharegroup reference khr";
        break;
    case -1001:
        return "platform not found khr";
        break;
    case -1002:
        return "invalid d3d10 device khr";
        break;
    case -1003:
        return "invalid d3d10 resource khr";
        break;
    case -1004:
        return "d3d10 resource already acquired khr";
        break;
    case -1005:
        return "d3d10 resource not acquired khr";
        break;
    default:
        return "unknown opencl error";
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Platform Class Definition
///////////////////////////////////////////////////////////////////////////////
void ecl::Platform::initDevices(std::vector<cl_device_id>& devs, cl_device_type type) {
    cl_uint count;
    error = clGetDeviceIDs(platform, type, 0, nullptr, &count);

    if (error != CL_DEVICE_NOT_FOUND) {
        cl_device_id temp[MAX_DEVICES_COUNT];

        error = clGetDeviceIDs(platform, type, count, temp, nullptr);
        checkError("Platform [init]");

        for (std::size_t i = 0; i < count; i++) devs.push_back(temp[i]);
    }
}

void ecl::Platform::releaseDevices(std::vector<cl_device_id>& devs) {
    for (auto& d : devs) {
        error = clReleaseDevice(d);
        checkError("Platform [free]");
    }
}

ecl::Platform::Platform(cl_platform_id platform) {
    this->platform = platform;
    initDevices(cpus, CL_DEVICE_TYPE_CPU);
    initDevices(gpus, CL_DEVICE_TYPE_GPU);
    initDevices(accs, CL_DEVICE_TYPE_ACCELERATOR);
    name = getPlatformInfo(CL_PLATFORM_NAME);
}

cl_device_id ecl::Platform::getDevice(std::size_t i, DEVICE type) const {
    cl_uint count = 0;
    const cl_device_id* dev;

    if (type == CPU) {
        count = static_cast<cl_uint>(cpus.size());
        dev = cpus.data();
    }
    else if (type == GPU) {
        count = static_cast<cl_uint>(gpus.size());
        dev = gpus.data();
    }
    else if (type == ACCEL) {
        count = static_cast<cl_uint>(accs.size());
        dev = accs.data();
    }
    else throw std::runtime_error("invalid device type");

    if (i >= count) throw std::runtime_error("invalid device");

    return dev[i];
}
const std::string& ecl::Platform::getName() const {
    return name;
}

std::string ecl::Platform::getPlatformInfo(cl_platform_info info) const {
    std::size_t info_size;

    error = clGetPlatformInfo(platform, info, 0, nullptr, &info_size);
    checkError("Platform [get info]");

    char info_src[MAX_INFO_SIZE];
    error = clGetPlatformInfo(platform, info, info_size, info_src, nullptr);
    checkError("Platform [get info]");

    return info_src;
}

std::string ecl::Platform::getDeviceInfo(std::size_t i, DEVICE type, cl_device_info info) const {
    std::size_t info_size;
    cl_device_id device = getDevice(i, type);

    error = clGetDeviceInfo(device, info, 0, nullptr, &info_size);
    checkError("Platform [get device info]");

    bool is_char = (info == CL_DEVICE_EXTENSIONS) |
        (info == CL_DEVICE_NAME) |
        (info == CL_DEVICE_PROFILE) |
        (info == CL_DEVICE_VENDOR) |
        (info == CL_DEVICE_VERSION) |
        (info == CL_DRIVER_VERSION);

    if (is_char) {
        char info_src[MAX_INFO_SIZE];
        error = clGetDeviceInfo(device, info, info_size, info_src, nullptr);
        checkError("Platform [get device info]");

        return info_src;
    }

    if (info == CL_DEVICE_MAX_WORK_ITEM_SIZES) {
        std::size_t info_src[3];
        error = clGetDeviceInfo(device, info, info_size, info_src, nullptr);
        checkError("Platform [get device info]");

        return std::to_string(info_src[0]) + ", " +
            std::to_string(info_src[1]) + ", " + std::to_string(info_src[2]);
    }

    cl_uint info_src;
    error = clGetDeviceInfo(device, info, info_size, &info_src, nullptr);
    checkError("Platform [get device info]");

    return std::to_string(info_src);
}

namespace ecl {
    std::ostream& operator<<(std::ostream& s, const Platform& p) {
        s << p.getName();
        return s;
    }
}

ecl::Platform::~Platform() {
    releaseDevices(cpus);
    releaseDevices(gpus);
    releaseDevices(accs);
}

///////////////////////////////////////////////////////////////////////////////
// System Class Definition
///////////////////////////////////////////////////////////////////////////////
std::vector<const ecl::Platform*> ecl::System::platforms;
bool ecl::System::initialized = false;

void ecl::System::init() {
    cl_uint count;

    error = clGetPlatformIDs(0, nullptr, &count);
    checkError("System [init]");

    if (count > 0) {
        cl_platform_id temp[MAX_PLATFORMS_COUNT];
        error = clGetPlatformIDs(count, temp, nullptr);
        checkError("System [init]");

        for (std::size_t i = 0; i < count; i++) platforms.push_back(new Platform(temp[i]));
    }
    initialized = true;
}

const ecl::Platform& ecl::System::getPlatform(std::size_t i) {
    if (!initialized) init();
    return *platforms.at(i);
}

void ecl::System::release() {
    for (auto* p : platforms) delete p;
}

///////////////////////////////////////////////////////////////////////////////
// Program Class Definition
///////////////////////////////////////////////////////////////////////////////
std::string ecl::Program::getBuildError(cl_context context, cl_device_id device) {
    std::size_t info_size;
    clGetProgramBuildInfo(program.at(context), device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &info_size);

    char* info = new char[info_size];
    clGetProgramBuildInfo(program.at(context), device, CL_PROGRAM_BUILD_LOG, info_size, info, nullptr);

    return info;
}

void ecl::Program::copy(const Program& other) {
    clear();
    source = other.source;
}
void ecl::Program::move(Program& other) {
    clear();
    source = other.source;
    program = std::move(other.program);

    other.clear();
}

void ecl::Program::clear() {
    for (const auto& p : program) clReleaseProgram(p.second);
    source.clear();
    program.clear();
}

ecl::Program::Program(const char* src) {
    source = src;
}
ecl::Program::Program(const std::string& src) {
    source = src;
}

ecl::Program::Program(const Program& other) {
    copy(other);
}
ecl::Program& ecl::Program::operator=(const Program& other) {
    copy(other);

    return *this;
}

ecl::Program::Program(Program&& other) {
    move(other);
}
ecl::Program& ecl::Program::operator=(Program&& other) {
    move(other);

    return *this;
}

ecl::Program ecl::Program::load(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    if (!f.is_open()) throw std::runtime_error("wrong program filename");

    std::string result;
    std::getline(f, result, (char)f.eof());
    f.close();

    return result;
}

cl_program ecl::Program::getProgram(cl_context context) const {
    return program.at(context);
}
const std::string& ecl::Program::getSource() const {
    return source;
}

namespace ecl {
    std::ostream& operator<<(std::ostream& s, const Program& other) {
        s << other.getSource();
        return s;
    }
}

void ecl::Program::setSource(const std::string& src) {
    if (program.size() == 0) {
        source = src;
    }
    else throw std::runtime_error("unable to change program until it's using");
}


ecl::Program& ecl::Program::operator=(const std::string& src) {
    setSource(src);
    return *this;
}
ecl::Program& ecl::Program::operator=(const char* src) {
    setSource(src);
    return *this;
}

ecl::Program& ecl::Program::operator+=(const std::string& src) {
    setSource(source + src);
    return *this;
}
ecl::Program& ecl::Program::operator+=(const char* src) {
    setSource(source + src);
    return *this;
}
ecl::Program& ecl::Program::operator+=(const Program& other) {
    setSource(source + other.source);
    return *this;
}

ecl::Program ecl::Program::operator+(const std::string& src) {
    Program result(*this);
    result += src;
    return result;
}
ecl::Program ecl::Program::operator+(const char* src) {
    Program result(*this);
    result += src;
    return result;
}
ecl::Program ecl::Program::operator+(const Program& other) {
    Program result(*this);
    result += other;
    return result;
}


bool ecl::Program::checkProgram(cl_context context, cl_device_id device) {
    if (program.find(context) == program.end()) {
        const char* src = source.c_str();
        std::size_t len = source.size();

        program.emplace(context, clCreateProgramWithSource(context, 1, (const char**)&src, (const std::size_t*)&len, &error));
        checkError("Program [check]");

        error = clBuildProgram(program.at(context), 0, nullptr, nullptr, nullptr, nullptr);
        if (error != 0) throw std::runtime_error(getBuildError(context, device));

        return false;
    }
    return true;
}

ecl::Program::~Program() {
    clear();
}

///////////////////////////////////////////////////////////////////////////////
// Kernel Class Definition
///////////////////////////////////////////////////////////////////////////////
void ecl::Kernel::copy(const Kernel& other) {
    clear();
    name = other.name;
}
void ecl::Kernel::move(Kernel& other) {
    clear();

    name = other.name;
    kernel = std::move(other.kernel);

    other.clear();
}

void ecl::Kernel::clear() {
    for (const auto& p : kernel) clReleaseKernel(p.second);
    name.clear();
}
ecl::Kernel::Kernel(const char* name) {
    this->name = name;
}
ecl::Kernel::Kernel(const std::string& name) {
    this->name = name;
}

ecl::Kernel::Kernel(const Kernel& other) {
    copy(other);
}
ecl::Kernel& ecl::Kernel::operator=(const Kernel& other) {
    copy(other);

    return *this;
}

ecl::Kernel::Kernel(Kernel&& other) {
    move(other);
}
ecl::Kernel& ecl::Kernel::operator=(Kernel&& other) {
    move(other);

    return *this;
}

namespace ecl {
    std::ostream& operator<<(std::ostream& s, const Kernel& other) {
        s << other.getName();
        return s;
    }
}

void ecl::Kernel::setName(const std::string& name) {
    if (kernel.size() == 0) this->name = name;
    else throw std::runtime_error("unable to change kernel name until it's using");
}
const std::string& ecl::Kernel::getName() const {
    return name;
}

ecl::Kernel& ecl::Kernel::operator=(const std::string& src) {
    setName(src);
    return *this;
}
ecl::Kernel& ecl::Kernel::operator=(const char* src) {
    setName(src);
    return *this;
}

ecl::Kernel& ecl::Kernel::operator+=(const std::string& src) {
    setName(name + src);
    return *this;
}
ecl::Kernel& ecl::Kernel::operator+=(const char* src) {
    setName(name + src);
    return *this;
}
ecl::Kernel& ecl::Kernel::operator+=(const Kernel& other) {
    setName(name + other.name);
    return *this;
}

ecl::Kernel ecl::Kernel::operator+(const std::string& src) {
    Kernel result(*this);
    result += src;
    return result;
}
ecl::Kernel ecl::Kernel::operator+(const char* src) {
    Kernel result(*this);
    result += src;
    return result;
}
ecl::Kernel ecl::Kernel::operator+(const Kernel& other) {
    Kernel result(*this);
    result += other;
    return result;
}


cl_kernel ecl::Kernel::getKernel(cl_program program) const {
    return kernel.at(program);
}
bool ecl::Kernel::checkKernel(cl_program program) {
    if (kernel.find(program) == kernel.end()) {
        kernel.emplace(program, clCreateKernel(program, name.c_str(), &error));
        checkError("Kernel [check]");
        return false;
    }
    return true;
}

ecl::Kernel::~Kernel() {
    clear();
}

///////////////////////////////////////////////////////////////////////////////
// Buffer Class Definition
///////////////////////////////////////////////////////////////////////////////
void ecl::Buffer::copy(const Buffer& other) {
    clear();

    ptr = other.ptr;
    size = other.size;
    access = other.access;

    for (auto& p : other.buffer) createBuffer(p.first);
}
void ecl::Buffer::move(Buffer& other) {
    clear();

    ptr = other.ptr;
    size = other.size;
    access = other.access;
    buffer = std::move(other.buffer);

    other.clear();
}

ecl::Buffer::Buffer(void* ptr, std::size_t size, ACCESS access) {
    this->ptr = ptr;
    this->size = size;
    this->access = access;
}

ecl::Buffer::Buffer(const Buffer& other) {
    copy(other);
}
ecl::Buffer& ecl::Buffer::operator=(const Buffer& other) {
    copy(other);

    return *this;
}

ecl::Buffer::Buffer(Buffer&& other) {
    move(other);
}
ecl::Buffer& ecl::Buffer::operator=(Buffer&& other) {
    move(other);

    return *this;
}

cl_mem ecl::Buffer::getBuffer(cl_context context) const {
    return buffer.at(context);
}
void* ecl::Buffer::getPtr() {
    return ptr;
}
std::size_t ecl::Buffer::getSize() const {
    return size;
}
ecl::ACCESS ecl::Buffer::getAccess() const {
    return access;
}

void ecl::Buffer::setPtr(void* ptr) {
    this->ptr = ptr;
}

bool ecl::Buffer::checkBuffer(cl_context context) const {
    if (buffer.find(context) != buffer.end()) return true;
    return false;
}
void ecl::Buffer::createBuffer(cl_context context) {
    if (!checkBuffer(context)) {
        buffer.emplace(context, clCreateBuffer(context, access, size, nullptr, &error));
        checkError("Buffer [check]");
    }
}
void ecl::Buffer::releaseBuffer(cl_context context) {
    auto it = buffer.find(context);
    if (it != buffer.end()) {
        error = clReleaseMemObject(buffer.at(context));
        buffer.erase(it);

        checkError("Buffer [clear]");
    }
}

void ecl::Buffer::clear() {
    while (buffer.size() > 0) releaseBuffer(buffer.begin()->first);
    ptr = nullptr;
    size = 0;
    access = READ;
}

ecl::Buffer::~Buffer() {
    clear();
}

///////////////////////////////////////////////////////////////////////////////
// Computer Class Definition
///////////////////////////////////////////////////////////////////////////////
ecl::Computer::Computer(std::size_t i, const Platform& platform, DEVICE dev) {
    device = platform.getDevice(i, dev);
    name = platform.getDeviceInfo(i, dev, CL_DEVICE_NAME);

    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &error);
    checkError("Computer [init]");

    queue = clCreateCommandQueue(context, device, 0, &error);
    checkError("Computer [init]");
}


void ecl::Computer::grid(const Frame& frame, const std::vector<std::size_t>& global_work_size, const std::vector<std::size_t>& local_work_size, EXEC sync) {
    auto& prog = frame.prog;
    auto& kern = frame.kern;
    const auto& args = frame.args;

    prog.checkProgram(context, device);
    cl_program prog_program = prog.getProgram(context);

    kern.checkKernel(prog_program);
    cl_kernel kern_kernel = kern.getKernel(prog_program);

    std::size_t count = args.size();
    for (std::size_t i(0); i < count; i++) {
        const Buffer* curr = args.at(i);
        bool sended = curr->checkBuffer(context);
        if (!sended) throw std::runtime_error("Computer [grid]: buffer wasn't sent to computer");

        cl_mem buf = curr->getBuffer(context);
        error = clSetKernelArg(kern_kernel, static_cast<cl_uint>(i), sizeof(cl_mem), &buf);
        checkError("Computer [grid]");
    }

    error = clEnqueueNDRangeKernel(queue, kern_kernel, static_cast<cl_uint>(global_work_size.size()), nullptr, global_work_size.data(), local_work_size.data(), 0, nullptr, nullptr);
    checkError("Computer [grid]");

    if (sync == SYNC) await();
}
void ecl::Computer::grid(const Frame& frame, const std::vector<std::size_t>& global_work_size, EXEC sync) {
    auto& prog = frame.prog;
    auto& kern = frame.kern;
    const auto& args = frame.args;

    prog.checkProgram(context, device);
    cl_program prog_program = prog.getProgram(context);

    kern.checkKernel(prog_program);
    cl_kernel kern_kernel = kern.getKernel(prog_program);

    std::size_t count = args.size();
    for (std::size_t i(0); i < count; i++) {
        const Buffer* curr = args.at(i);
        bool sended = curr->checkBuffer(context);
        if (!sended) throw std::runtime_error("Computer [grid]: buffer wasn't sent to computer");

        cl_mem buf = curr->getBuffer(context);
        error = clSetKernelArg(kern_kernel, static_cast<cl_uint>(i), sizeof(cl_mem), &buf);
        checkError("Computer [grid]");
    }

    error = clEnqueueNDRangeKernel(queue, kern_kernel, static_cast<cl_uint>(global_work_size.size()), nullptr, global_work_size.data(), nullptr, 0, nullptr, nullptr);
    checkError("Computer [grid]");

    if (sync == SYNC) await();
}
void ecl::Computer::task(const Frame& frame, EXEC sync) {
    auto& prog = frame.prog;
    auto& kern = frame.kern;
    const auto& args = frame.args;

    prog.checkProgram(context, device);
    cl_program prog_program = prog.getProgram(context);

    kern.checkKernel(prog_program);
    cl_kernel kern_kernel = kern.getKernel(prog_program);

    std::size_t count = args.size();
    for (std::size_t i(0); i < count; i++) {
        const Buffer* curr = args.at(i);
        bool sended = curr->checkBuffer(context);
        if (!sended) throw std::runtime_error("Computer [task]: buffer wasn't sent to computer");

        cl_mem buf = curr->getBuffer(context);
        error = clSetKernelArg(kern_kernel, static_cast<cl_uint>(i), sizeof(cl_mem), &buf);
        checkError("Computer [task]");
    }

    error = clEnqueueTask(queue, kern_kernel, 0, nullptr, nullptr);
    checkError("Computer [task]");

    if (sync == SYNC) await();
}

void ecl::Computer::await() {
    error = clFinish(queue);
    checkError("Computer [await]");
}

cl_device_id ecl::Computer::getDevice() const {
    return device;
}
cl_context ecl::Computer::getContext() const {
    return context;
}
cl_command_queue ecl::Computer::getQueue() const {
    return queue;
}
const std::string& ecl::Computer::getName() const {
    return name;
}

void ecl::Computer::send(ecl::Buffer& arg, EXEC sync) {
    arg.createBuffer(context);

    error = clEnqueueWriteBuffer(queue, arg.getBuffer(context), CL_FALSE, 0, arg.getSize(), arg.getPtr(), 0, nullptr, nullptr);
    checkError("Computer [send data]");

    if (sync == SYNC) await();
}
void ecl::Computer::send(const std::vector<Buffer*>& args, EXEC sync) {
    std::size_t count = args.size();
    for (std::size_t i(0); i < count; i++) send(*args[i], ASYNC);

    if (sync == SYNC) await();
}
void ecl::Computer::receive(Buffer& arg, EXEC sync) {
    bool sended = arg.checkBuffer(context);
    if (!sended) throw std::runtime_error("Computer [receive]: buffer wasn't sent to computer");
    if (arg.getAccess() == READ) throw std::runtime_error("Computer [receive]: trying to receive read-only data");

    error = clEnqueueReadBuffer(queue, arg.getBuffer(context), CL_FALSE, 0, arg.getSize(), arg.getPtr(), 0, nullptr, nullptr);
    checkError("Computer [receive data]");

    if (sync == SYNC) await();
}
void ecl::Computer::receive(const std::vector<Buffer*>& args, EXEC sync) {
    std::size_t count = args.size();
    for (std::size_t i(0); i < count; i++) receive(*args[i], ASYNC);

    if (sync == SYNC) await();
}
void ecl::Computer::release(Buffer& arg, EXEC sync) {
    arg.releaseBuffer(context);

    if (sync == SYNC) await();
}
void ecl::Computer::release(const std::vector<Buffer*>& args, EXEC sync) {
    for (auto* arg : args) release(*arg, ASYNC);

    if (sync == SYNC) await();
}
void ecl::Computer::grab(Buffer& arg, EXEC sync) {
    receive(arg, sync);
    release(arg, sync);
}
void ecl::Computer::grab(const std::vector<Buffer*>& args, EXEC sync) {
    receive(args);
    release(args, sync);
}

namespace ecl {
    std::ostream& operator<<(std::ostream& s, const Computer& video) {
        s << video.getName();
        return s;
    }

    Computer& operator<<(Computer& video, Buffer& arg) {
        video.send(arg);
        return video;
    }

    Computer& operator>>(Computer& video, Buffer& arg) {
        video.receive(arg);
        return video;
    }
}

ecl::Computer::~Computer() {
    error = clReleaseCommandQueue(queue);
    checkError("Computer [free]");
    error = clReleaseContext(context);
    checkError("Computer [free]");
}