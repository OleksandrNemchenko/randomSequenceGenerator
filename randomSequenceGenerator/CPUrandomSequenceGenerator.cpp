
#include <cassert>

#include "include/randomSequenceGenerator.hpp"
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
    _buffer.resize(buffers);

    for (size_t i = 0; i < buffers; ++i)
        _buffer[i].resize(bytesInBuffer);
}

bool CCPURandomSequenceGenerator::ImplInit()
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();

    _lceGen.seed(static_cast<unsigned int>(seed));

    static constexpr size_t seedMask = 0xAAAAAAAA;
    seed ^= seedMask;

    _mtGen.seed(seed);

    return true;
}

bool CCPURandomSequenceGenerator::FillBuffer(size_t bufferId)
{
    std::uniform_int_distribution<> distribution;

    assert(bufferId < _buffer.size());

    const auto startTimePoint = std::chrono::steady_clock::now();
    for (TByte& byte : _buffer[bufferId])
        byte = static_cast<TByte>(distribution(_mtGen) ^ distribution(_lceGen));
    const auto endTimePoint = std::chrono::steady_clock::now();

    SStatistics stat;
    stat._generate = std::chrono::duration_cast<SStatistics::TTimeMeasurement>(endTimePoint - startTimePoint);
    stat._store = SStatistics::TTimeMeasurement{ 0 };
    stat._bufSize = BufferSize();

    SetStatistics(stat);

    return true;
}

CCPURandomSequenceGenerator::TByte* CCPURandomSequenceGenerator::Array(size_t bufferId) noexcept
{
    assert(bufferId < _buffer.size());
    return _buffer[bufferId].data();
}
