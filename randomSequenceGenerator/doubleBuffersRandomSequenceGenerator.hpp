
#ifndef RANDOM_SEQUENCE_GENERATOR_DOUBLE_BUFFERS_IMPLEMENTATION_
#define RANDOM_SEQUENCE_GENERATOR_DOUBLE_BUFFERS_IMPLEMENTATION_

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <randomSequenceGenerator.hpp>

class CDoubleBuffersRandomSequenceGenerator : public CRandomSequenceGenerator
{
public:
    CDoubleBuffersRandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback);

    bool ReadyToWork() const noexcept override;
    std::pair<TFilledTimeout, size_t> GetGenStatistic() const noexcept override;

protected:
    enum EActionToDo {FILL_BUFFER, TERMINATE_THREAD};
    virtual void AllocBuffers(size_t buffers, size_t bytesInBuffer) = 0;
    virtual bool ImplInit() = 0;
    virtual void FinishThread() {}
    virtual bool FillBuffer(size_t bufferNum) = 0;
    virtual size_t BuffersAmount() const noexcept;
    virtual void StartThreadFinish();
    virtual TByte* Array(size_t bufferNum) noexcept = 0;
    void InitBase();

private:
    struct SBuffer
    {
        std::atomic<bool> _ready = false;
        TByte* _buffer = nullptr;
        size_t _consumed = 0;
    };

    static constexpr size_t _buffersAmount = 2;
    std::unique_ptr<std::thread> _calcThread;
    std::array<SBuffer, _buffersAmount> _buffer;
    std::atomic<size_t> _activeBuffer = 0;
    std::atomic<EActionToDo> _actionToDo;
    std::recursive_mutex _getRandomNumberMutex;
    std::mutex _doActionMutex;
    std::condition_variable _doActionCondVar;
    bool _doActionEventHappened;
    std::mutex _finishThreadMutex;
    std::condition_variable _finishThreadCondVar;
    bool _finishThreadEventHappened;
    std::chrono::nanoseconds _filledGeneralTime = std::chrono::nanoseconds{ 0 };
    size_t _filledGeneralBytes = 0;
    std::atomic<std::pair<TFilledTimeout, size_t>> _filledGeneralStatistics;
    FDecreaseThreadPriority _decreaseThreadPriorityCallback;

    void Init();
    void ProcessEvents();
    TSpan GetRandomBytes(size_t size) override;
    void DoAction(EActionToDo actionToDo) noexcept;
};

#endif // RANDOM_SEQUENCE_GENERATOR_DOUBLE_BUFFERS_IMPLEMENTATION_
