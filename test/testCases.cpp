
#include <cassert>
#include <array>
#include <chrono>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <source_location>
#include <stack> 
#include <string>
#include <thread>
#include <vector>

#include <randomSequenceGenerator.hpp>

#ifdef _WIN32
    #include <Windows.h>
#endif // _WIN32

#undef min
#undef max

void OutputError(std::source_location location = std::source_location::current())
{
    std::cout << "* Error at " << location.file_name() << ":" << location.line() << ", function " << location.function_name() << std::endl;
}

void DecreaseThreadPriority()
{
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif // _WIN32
};

void WaitForInit(CRandomSequenceGenerator* gen)
{
    while (!gen->ReadyToWork())
        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
}

void TestGeneralAbilities()
{
    std::cout << "- Test the ability to be compiled: ";

    auto gen = CRandomSequenceGenerator::Make(1024, DecreaseThreadPriority);
    WaitForInit(gen.get());

    { auto r = gen->GetValue<int>(); }
    { auto r = gen->GetValue<size_t>(); }
    { auto r = gen->GetValue<float>(); }
    { struct Simple { int i; }; auto r = gen->GetValue<Simple>(); }
    { auto r = gen->GetValue<int*>(); }

    { auto r = gen->GetDataSpan<int>(5); }
    { auto r = gen->GetDataSpan<size_t>(5); }
    { auto r = gen->GetDataSpan<float>(5); }
    { struct Simple { int i; }; auto r = gen->GetDataSpan<Simple>(5); }
    { auto r = gen->GetDataSpan<int*>(5); }

    { auto r = gen->GetValues<std::vector<int>>(5); }
    { auto r = gen->GetValues<std::list<size_t>>(5); }
    { auto r = gen->GetValues<std::queue<float>>(5); }
    { struct Simple { int i; }; auto r = gen->GetValues<std::deque<Simple>>(5); }
    { auto r = gen->GetValues<std::set<int*>>(5); }
    { auto r = gen->GetValues<std::stack<double>>(5); }
    { auto r = gen->GetValues<std::string>(5); }

// UNABLE to be compiled
//    { auto r = gen->GetValue<std::string>(); }
//    { auto r = gen->GetValue<std::vector<int>>(); }
//    { auto r = gen->GetValue<thread>(5); }
//    { auto r = gen->GetDataSpan<thread>(5); }
//    { auto r = gen->GetValues<std::array<int, 145>> > (5); }
//    { auto r = gen->GetValues<thread>(5); }
//    { auto r = gen->GetValues<std::map<int, char>>(5); }
//    { auto r = gen->GetValues<std::unordered_map<int, char>>(5); }
//    { auto r2 = gen->GetValues<std::vector<uint8_t>>(0); }    // assertion

    std::cout << "OK" << std::endl;

    std::cout << "- Test exceptions: ";

    try
    {
        auto gen = CRandomSequenceGenerator::Make(0, DecreaseThreadPriority);
        WaitForInit(gen.get());

        OutputError();
    }
    catch (std::length_error)
    {
    }

    try
    {
        auto gen = CRandomSequenceGenerator::Make(std::numeric_limits<size_t>::max(), DecreaseThreadPriority);
        OutputError();
    }
    catch (std::length_error)
    {
    }
    catch (std::bad_array_new_length)
    {
    }

    try
    {
        auto gen = CRandomSequenceGenerator::Make(100, DecreaseThreadPriority);
        WaitForInit(gen.get());

        auto r = gen->GetValues<std::vector<uint8_t>>(101);
        OutputError();
    }
    catch (std::length_error)
    {
    }

    try
    {
        auto gen = CRandomSequenceGenerator::Make(100, DecreaseThreadPriority);
        WaitForInit(gen.get());

        auto r1 = gen->GetValues<std::vector<uint8_t>>(100);
    }
    catch (std::length_error)
    {
        OutputError();
    }

    std::cout << "OK" << std::endl;

}

void TestSequence(CRandomSequenceGenerator::EGeneratorType genType)
{
    std::cout << "- Test sequences: ";

    auto gen = CRandomSequenceGenerator::Make(1'000'000, DecreaseThreadPriority, genType);
    WaitForInit(gen.get());

    const size_t allocs = 1000;
    const size_t singleAlloc = gen->BufferSize() / 2;
    std::vector<std::vector<uint8_t>> buffers;
    std::array<size_t, 256> probability;
    std::array<std::array<size_t, 256>, 256> double_probability;

    probability.fill(0);
    for (auto& buf : double_probability)
        buf.fill(0);

    buffers.reserve(allocs * gen->BufferSize() / singleAlloc);

    std::chrono::steady_clock::time_point lastCheckTimePoint = std::chrono::steady_clock::now();
    for (size_t i = 0; i < allocs; ++i)
    {
        auto buf = gen->GetValues<std::vector<uint8_t>>(singleAlloc);
        buffers.emplace_back(std::move(buf));

        if (std::chrono::steady_clock::now() - lastCheckTimePoint > std::chrono::seconds{ 1 })
        {
            std::cout << i << " / " << allocs << "... ";
            lastCheckTimePoint = std::chrono::steady_clock::now();
        }
    }
    std::chrono::nanoseconds byteTimeDelayNs = gen->GetGenStatistic().first / gen->GetGenStatistic().second;
    std::chrono::microseconds byteTimeDelayUs = std::chrono::duration_cast<std::chrono::microseconds>(byteTimeDelayNs);

    if (byteTimeDelayUs.count() == 0)
        std::cout << " (" << byteTimeDelayNs.count() << "ns for byte generation) ";
    else
        std::cout << " (" << byteTimeDelayUs.count() << "us for byte generation) ";
    bool firstBuf = true;
    for (auto& buf : buffers)
    {
        for (uint8_t& byte : buf)
            ++probability[byte];
        for (size_t j = 1; j < buf.size(); ++j)
            ++double_probability[buf[j - 1]][buf[j]];

        if (firstBuf)
            firstBuf = false;
        else
        {
            std::vector<uint8_t>& prevBuf = buffers[buffers.size() - 1];
            const size_t bufSize = prevBuf.size();
            ++double_probability[prevBuf[bufSize - 1]][buf[0]];
        }
    }

    bool allAreDiff = true;
    for (size_t i = 0; i < buffers.size() - 1; ++i)
        for (size_t j = i + 1; j < buffers.size(); ++j)
            allAreDiff &= buffers[i] != buffers[j];

    if (!allAreDiff)
        OutputError();

    {
        double avg = 0;
        size_t max, min;
        size_t cntr = 0;
        for (size_t& value : probability)
        {
            if (!avg)
                avg = static_cast<double>(max = min = value);
            else
            {
                avg = (avg * cntr + value) / (cntr + 1);
                min = std::min(min, value);
                max = std::max(max, value);
            }
            ++cntr;
        }

        size_t range = max - min;
        const size_t allowedRange = static_cast<size_t>(avg * 0.1);
        std::cout << " value min/avg/max = " << min << "/" << static_cast<size_t>(avg) << "/" << max;
    }

    {
        double avg = 0;
        size_t max, min;
        size_t cntr = 0;
        for (auto& probability : double_probability)
            for (size_t& value : probability)
            {
                if (!avg)
                    avg = static_cast<double>(max = min = value);
                else
                {
                    avg = (avg * cntr + value) / (cntr + 1);
                    min = std::min(min, value);
                    max = std::max(max, value);
                }
                ++cntr;
            }

        size_t range = max - min;
        const size_t allowedRange = static_cast<size_t>(avg * 0.1);
        std::cout << ", value change min/avg/max = " << min << "/" << static_cast<size_t>(avg) << "/" << max << std::endl;

    }
}