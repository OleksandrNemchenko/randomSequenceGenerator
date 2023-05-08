
#ifndef RANDOM_SEQUENCE_GENERATOR_CPU_IMPLEMENTATION_
#define RANDOM_SEQUENCE_GENERATOR_CPU_IMPLEMENTATION_

#include <memory>
#include <random>

#include "doubleBuffersRandomSequenceGenerator.hpp"

class CCPURandomSequenceGenerator : public CDoubleBuffersRandomSequenceGenerator
{
public:
    CCPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback);
    ~CCPURandomSequenceGenerator() noexcept override;

private:
    void AllocBuffers(size_t buffers, size_t bytesInBuffer) override;
    bool ImplInit() override;
    bool FillBuffer(size_t bufferNum) override;
    TByte* Array(size_t bufferNum) noexcept override;

    std::mt19937_64 _mtGen;
    std::minstd_rand _lceGen;
    std::vector<std::vector<std::uint8_t>> _buffer;
};

#endif // RANDOM_SEQUENCE_GENERATOR_CPU_IMPLEMENTATION_
