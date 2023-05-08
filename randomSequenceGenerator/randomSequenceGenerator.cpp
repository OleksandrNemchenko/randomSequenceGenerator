
#include <cassert>
#include <stdexcept>

#include <randomSequenceGenerator.hpp>

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

