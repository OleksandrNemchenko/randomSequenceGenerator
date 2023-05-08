
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "doubleBuffersRandomSequenceGenerator.hpp"

CDoubleBuffersRandomSequenceGenerator::CDoubleBuffersRandomSequenceGenerator(size_t memorySizeInBytes, FDecreaseThreadPriority decreaseThreadPriorityCallback):
    CRandomSequenceGenerator(memorySizeInBytes), _decreaseThreadPriorityCallback(decreaseThreadPriorityCallback)
{
}

void CDoubleBuffersRandomSequenceGenerator::InitBase()
{
    AllocBuffers(BuffersAmount(), BufferSize());

    for (size_t i = 0; i < BuffersAmount(); ++i)
        _buffer[i]._buffer = Array(i);

    _calcThread = std::make_unique<std::thread>([this]()
    {
        _decreaseThreadPriorityCallback();
        try
        {
            Init();
            ProcessEvents();
        }
        catch (std::runtime_error err)
        {
            std::string str = err.what();
            assert(false);
        }
        catch (...)
        {
        }
        _finishThreadEventHappened = true;
        _finishThreadCondVar.notify_one();

    });

    _calcThread->detach();
}

void CDoubleBuffersRandomSequenceGenerator::StartThreadFinish()
{
    _finishThreadEventHappened = false;
    DoAction(TERMINATE_THREAD);

    if (!_finishThreadEventHappened)
    {
        std::unique_lock lock(_doActionMutex);
        constexpr std::chrono::milliseconds waitForFinishing{ 1000 };
        _finishThreadCondVar.wait_for(lock, waitForFinishing, [this] {return _finishThreadEventHappened; });
        if (!_finishThreadEventHappened)
            _calcThread.reset();
    }

}

void CDoubleBuffersRandomSequenceGenerator::Init()
{
    ImplInit();

    for (size_t i = 0; i < _buffer.size(); ++i)
    {
        SBuffer& buffer = _buffer[i];
        bool bufferFilled = FillBuffer(i);

        if (!bufferFilled)
            return;

        buffer._ready = true;
    }
}

void CDoubleBuffersRandomSequenceGenerator::ProcessEvents()
{
    while (true)
    {
        {
            std::unique_lock lock(_doActionMutex);
            _doActionEventHappened = false;
            _doActionCondVar.wait(lock, [this] {return _doActionEventHappened; });
        }

        switch (_actionToDo)
        {
        case TERMINATE_THREAD :
            FinishThread();
            return;

        case FILL_BUFFER :
            for (size_t i = 0; i < _buffer.size(); ++i)
            {
                SBuffer& buffer = _buffer[i];
                if (!buffer._ready)
                {
                    std::chrono::steady_clock::time_point startTimePoint = std::chrono::steady_clock::now();

                    if (FillBuffer(i))
                    {
                        std::chrono::steady_clock::time_point endTimePoint = std::chrono::steady_clock::now();

                        _filledGeneralTime += endTimePoint - startTimePoint;
                        _filledGeneralBytes += BufferSize();
                        _filledGeneralStatistics = std::make_pair(_filledGeneralTime, _filledGeneralBytes);

                        buffer._consumed = 0;
                        buffer._ready = true;
                    }
                }
            }
            break;

        default:
            assert(false);
        }
    }
}

bool CDoubleBuffersRandomSequenceGenerator::ReadyToWork() const noexcept
{
    for (const SBuffer& buffer : _buffer)
        if (buffer._ready)
            return true;

    return false;
}

void CDoubleBuffersRandomSequenceGenerator::DoAction(EActionToDo actionToDo) noexcept
{
    _actionToDo = actionToDo;
    _doActionEventHappened = true;
    _doActionCondVar.notify_one();
}

CRandomSequenceGenerator::TSpan CDoubleBuffersRandomSequenceGenerator::GetRandomBytes(size_t size)
{
    if (size > BufferSize())
    {
        using namespace std::string_literals;
        throw std::length_error("Requested size "s + std::to_string(size) + " is bigger that the buffer size "s + std::to_string(BufferSize()));
    }

    if (size == 0)
    {
        assert(false);
        return TSpan();
    }

    std::lock_guard<std::recursive_mutex> lockRndNumberGetter(_getRandomNumberMutex);

    size_t prevConsumed = _buffer[_activeBuffer]._consumed;
    size_t newConsumed = prevConsumed + size;

    TByte* data = nullptr;

    if (newConsumed < BufferSize() && _buffer[_activeBuffer]._ready)
    {
        data = _buffer[_activeBuffer]._buffer + prevConsumed;
        _buffer[_activeBuffer]._consumed += size;
    }
    else
    {
        _buffer[_activeBuffer]._ready = false;

        DoAction(FILL_BUFFER);
     
        if (++_activeBuffer >= _buffersAmount)
            _activeBuffer = 0;

        SBuffer& currentBuffer = _buffer[_activeBuffer];
        while (!currentBuffer._ready)
        {
            constexpr std::chrono::milliseconds waitForFilling{ 10 };
            std::this_thread::sleep_for(waitForFilling);
        }
        data = currentBuffer._buffer;
        currentBuffer._consumed = size;
    }

    return TSpan(data, size);
}

size_t CDoubleBuffersRandomSequenceGenerator::BuffersAmount() const noexcept
{
    return _buffersAmount;
}

std::pair<CDoubleBuffersRandomSequenceGenerator::TFilledTimeout, size_t> CDoubleBuffersRandomSequenceGenerator::GetGenStatistic() const noexcept
{
    return _filledGeneralStatistics;
}
