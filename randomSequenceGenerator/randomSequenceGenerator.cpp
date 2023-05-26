
#include <cassert>
#include <stdexcept>

#include "include/randomSequenceGenerator.hpp"

#include "CPUrandomSequenceGenerator.hpp"
#include "GPUrandomSequenceGenerator.hpp"

/* static */ std::unique_ptr<CRandomSequenceGenerator> CRandomSequenceGenerator::Make(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback, EGeneratorType generatorType)
{
    switch (generatorType)
    {
    case GPU_GENERATOR:
        if (CGPURandomSequenceGenerator::CheckOpenCLdevicesAvailability())
            return std::make_unique<CGPURandomSequenceGenerator>(memorySizeInBytes, decreaseThreadPriorityCallback);
        else
            throw std::runtime_error("No OpenCL device has been found");

    default:
        assert(false);
        [[fallthrough]];

    case GPU_IF_POSSIBLE_GENERATOR:
        if (CGPURandomSequenceGenerator::CheckOpenCLdevicesAvailability())
            return std::make_unique<CGPURandomSequenceGenerator>(memorySizeInBytes, decreaseThreadPriorityCallback);
        else
            return std::make_unique<CCPURandomSequenceGenerator>(memorySizeInBytes, decreaseThreadPriorityCallback);

    case CPU_GENERATOR:
        return std::make_unique<CCPURandomSequenceGenerator>(memorySizeInBytes, decreaseThreadPriorityCallback);
    }
}

CRandomSequenceGenerator::CRandomSequenceGenerator(size_t memorySizeInBytes) :
    _bufferSize(memorySizeInBytes)
{
    if (_bufferSize == 0)
        throw std::length_error("Zero size buffer asked while non-zero size one is required");
}

/* static */ CRandomSequenceGenerator::TBuffer CRandomSequenceGenerator::GetBytesOnce(size_t bytesAmount)
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::minstd_rand lceGen;
    lceGen.seed(static_cast<unsigned int>(seed));

    static constexpr size_t seedMask = 0xAAAAAAAA;
    seed ^= seedMask;
    std::mt19937_64 mtGen;
    mtGen.seed(seed);

    std::uniform_int_distribution<> distribution;

    TBuffer buffer(bytesAmount);
    for (TByte& byte : buffer)
        byte = static_cast<TByte>(distribution(mtGen) ^ distribution(lceGen));

    return buffer;

}
