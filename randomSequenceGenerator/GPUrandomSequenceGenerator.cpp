
#include "EasyCL.hpp"

#include <cassert>
#include <randomSequenceGenerator.hpp>
#include "GPUrandomSequenceGenerator.hpp"

using namespace ecl;

/* static */ const std::string CGPURandomSequenceGenerator::_clFunction = R"(
unsigned char LCG1(global unsigned int* state)
{
    *state = (*state * 214013U + 2531011U) & 0xffffffff;
    return (*state >> 16) & 0xff;
}

unsigned char LCG2(global unsigned int* state)
{
    *state = (*state * 1103515245 + 12345) & 0xffffffff;
    return (*state >> 16) & 0xff;
}

kernel void GenerateRandomNumber (
    global unsigned int* lcg1State,
    global unsigned int* lcg2State,
    global unsigned char* result
)
{
    unsigned long N = get_global_id(0);
    result[N] = LCG1(lcg1State + N) ^ LCG1(lcg2State + N);
}
)";

/* static */ bool CGPURandomSequenceGenerator::CheckOpenCLdevicesAvailability(void)
{
    static std::once_flag initCL;
    std::call_once(initCL, []()
    {
        System::init();
    });

    static const bool platformsAvailable = !System::getPlatforms().empty();
    return platformsAvailable;
}

CGPURandomSequenceGenerator::CGPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback) :
    CDoubleBuffersRandomSequenceGenerator(memorySizeInBytes, decreaseThreadPriorityCallback)
{
    InitBase();

    for (auto& computer : _computers)
    {
        auto init = [this](auto& elem)
        {
            using T = std::remove_reference_t<decltype(elem)>::element_type;
            elem = std::make_unique<T>(BufferSize());
        };

        init(computer._lce1);
        init(computer._lce2);
    }
}

CGPURandomSequenceGenerator::~CGPURandomSequenceGenerator() noexcept
{
    StartThreadFinish();

    std::atexit([]()
    {
        static std::once_flag releaseOnceFlag;
        std::call_once(releaseOnceFlag, []()
        {
            System::release();
        });
    });
}

void CGPURandomSequenceGenerator::FinishThread()
{
    for (auto& computer : _computers)
    {
        computer._lce1.reset();
        computer._lce2.reset();
        computer._res.reset();
        computer._program.reset();
        computer._kernel.reset();
        computer._frame.reset();
    }
}

void CGPURandomSequenceGenerator::AllocBuffers(size_t buffers, size_t bytesInBuffer)
{
    _computers.resize(BuffersAmount());

    for (auto& computer : _computers)
    {
        auto& buffer = computer._res;
        using T = std::remove_reference_t<decltype(computer._res)>::element_type;
        buffer = std::make_unique<T>(BufferSize());
    }
}

bool CGPURandomSequenceGenerator::ImplInit()
{
    if (!CheckOpenCLdevicesAvailability())
        return false;

    std::mt19937_64 mtGen;
    mtGen.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> distribution;

    for (SComputer& comp : _computers)
    {
        for (size_t i = 0; i < BufferSize(); ++i)
        {
            comp._lce1->getArray()[i] = distribution(mtGen);
            comp._lce2->getArray()[i] = distribution(mtGen);
        }

        comp._program = std::make_unique<Program>(_clFunction);
        comp._kernel = std::make_unique<Kernel>("GenerateRandomNumber");
        std::vector<const Buffer*> args = { comp._lce1.get(), comp._lce2.get(), comp._res.get() };

        comp._frame = std::make_unique<Frame>(comp._program.get(), comp._kernel.get(), std::move(args));

        const ecl::Platform& platform = System::getPlatform(0);
        comp._gpu = std::make_unique<Computer>(0, platform, DEVICE::GPU);

        *comp._gpu << *comp._lce1.get() << *comp._lce2.get() << *comp._res.get();
    }

    return true;
}

bool CGPURandomSequenceGenerator::FillBuffer(size_t bufferId)
{
    assert(bufferId < BuffersAmount());

    SComputer& comp = _computers[bufferId];
    comp._gpu->grid(*comp._frame, { BufferSize() }, EXEC::SYNC);
    *comp._gpu >> *comp._res.get();

    return true;
}

CGPURandomSequenceGenerator::TByte* CGPURandomSequenceGenerator::Array(size_t bufferId) noexcept
{
    assert(bufferId < BuffersAmount());
    return _computers[bufferId]._res->getArray();
}
