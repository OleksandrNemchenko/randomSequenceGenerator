
#ifndef RANDOM_SEQUENCE_GENERATOR_GPU_IMPLEMENTATION_
#define RANDOM_SEQUENCE_GENERATOR_GPU_IMPLEMENTATION_

#include <memory>
#include <random>

#include "EasyCL.hpp"

#include "doubleBuffersRandomSequenceGenerator.hpp"

class CGPURandomSequenceGenerator : public CDoubleBuffersRandomSequenceGenerator
{
public:
    CGPURandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback);
    ~CGPURandomSequenceGenerator() noexcept override;

    static bool CheckOpenCLdevicesAvailability();

private:
    static const std::string _clFunction;

    struct SComputer
    {
        std::unique_ptr<ecl::Program> _program;
        std::unique_ptr<ecl::Kernel> _kernel;
        std::unique_ptr<ecl::Computer> _gpu;
        std::unique_ptr<ecl::Frame> _frame;
        std::unique_ptr<ecl::array<unsigned int>> _lce1;
        std::unique_ptr<ecl::array<unsigned int>> _lce2;
        std::unique_ptr<ecl::array<TByte>> _res;
    };
    std::vector<SComputer> _computers;

    void AllocBuffers(size_t buffers, size_t bytesInBuffer) override;
    bool ImplInit() override;
    bool FillBuffer(size_t bufferNum) override;
    void FinishThread() override;
    TByte* Array(size_t bufferNum) noexcept override;

};

#endif // RANDOM_SEQUENCE_GENERATOR_CPU_IMPLEMENTATION_
