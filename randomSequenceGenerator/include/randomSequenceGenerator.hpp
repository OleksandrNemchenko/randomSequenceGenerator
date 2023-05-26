
#ifndef RANDOM_SEQUENCE_GENERATOR_INTERFACE_
#define RANDOM_SEQUENCE_GENERATOR_INTERFACE_

#include <chrono>
#include <functional>
#include <memory>
#include <span>
#include <type_traits>

template<typename TContainer>
concept CoRandomSequenceGeneratorContainer = requires(typename TContainer::value_type* pdata)
{
    TContainer(pdata, pdata);
};

class CRandomSequenceGenerator
{
public:
    using FDecreaseThreadPriority = std::function<void()>;
    using TByte = uint8_t;
    using TBuffer = std::vector<TByte>;
    enum EGeneratorType { CPU_GENERATOR, GPU_GENERATOR, GPU_IF_POSSIBLE_GENERATOR };

    struct SStatistics
    {
        using TTimeMeasurement = std::chrono::nanoseconds;
        TTimeMeasurement _generate;
        TTimeMeasurement _store;
        size_t _bufSize;
    };

    static std::unique_ptr<CRandomSequenceGenerator> Make(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback, EGeneratorType generatorType = GPU_IF_POSSIBLE_GENERATOR);

    static TBuffer GetBytesOnce(size_t bytesAmount);

    template<typename TData>
    requires std::is_pod_v<TData>
    static std::vector<TData> GetDataOnce(size_t bytesAmount)
    {
        TBuffer bytes = GetBytesOnce(sizeof(TData) * bytesAmount);
        TData* dataPtr = reinterpret_cast<TData*>(bytes.data());
        std::vector<TData> datas(dataPtr, dataPtr + bytesAmount);
        return datas;
    }

    CRandomSequenceGenerator(size_t memorySizeInBytes);
    virtual ~CRandomSequenceGenerator() noexcept = default;

    virtual bool ReadyToWork() const noexcept = 0;
    size_t BufferSize() const noexcept { return _bufferSize; }

    template<typename TData>
    requires std::is_pod_v<TData>
    TData GetValue(void) noexcept
    {
        TSpan buffer = GetRandomBytes(sizeof(TData));
        TData data = *reinterpret_cast<TData*>(buffer.data());
        return data;
    }

    template<typename TData>
    requires std::is_pod_v<TData>
    std::span<TData> GetDataSpan(size_t arraySize)
    {
        TSpan buffer = GetRandomBytes(sizeof(TData) * arraySize);
        TData* dataPtr = reinterpret_cast<TData*>(buffer.data());
        std::span<TData> span(dataPtr, arraySize);
        return span;
    }

    template<CoRandomSequenceGeneratorContainer TContainer>
    TContainer GetValues(size_t arraySize)
    {
        using TData = TContainer::value_type;
        std::span<TData> span = GetDataSpan<TData>(arraySize);
        TContainer container(span.data(), span.data() + arraySize);
        return container;
    }

    virtual SStatistics Statistics() const noexcept = 0;

protected:
    using TSpan = std::span<TByte>;

    virtual TSpan GetRandomBytes(size_t size) = 0;

private:
    const size_t _bufferSize;
};

#endif // RANDOM_SEQUENCE_GENERATOR_INTERFACE_
