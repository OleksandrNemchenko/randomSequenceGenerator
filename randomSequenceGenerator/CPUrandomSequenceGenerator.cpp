
#include <cassert>

#include <randomSequenceGenerator.hpp>
#include "CPUrandomSequenceGenerator.hpp"

CCPURandomSequenceGenerator::CCPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback) :
    CDoubleBuffersRandomSequenceGenerator(memorySizeInBytes, decreaseThreadPriorityCallback)
{
    InitBase();
}

CCPURandomSequenceGenerator::~CCPURandomSequenceGenerator() noexcept
{

}

void CCPURandomSequenceGenerator::AllocBuffers(size_t buffers, size_t bytesInBuffer)
{
    _buffer.resize(2);
    for (size_t i = 0; i < buffers; ++i)
        _buffer[i].resize(bytesInBuffer);
}

bool CCPURandomSequenceGenerator::ImplInit()
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();

    _lceGen.seed(static_cast<unsigned int>(seed));

    static constexpr size_t seedMask = 0xAAAAAAAAAAAAAAAA;
    seed ^= seedMask;

    _mtGen.seed(seed);

    return true;
}

bool CCPURandomSequenceGenerator::FillBuffer(size_t bufferId)
{
    std::uniform_int_distribution<> distribution;

    assert(bufferId < _buffer.size());
    for (TByte& byte : _buffer[bufferId])
        byte = static_cast<TByte>(distribution(_mtGen) ^ distribution(_lceGen));

    return true;
}

CCPURandomSequenceGenerator::TByte* CCPURandomSequenceGenerator::Array(size_t bufferId) noexcept
{
    assert(bufferId < _buffer.size());
    return _buffer[bufferId].data();
}
