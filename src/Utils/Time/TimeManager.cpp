/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** TimeManager implementation
*/

#include "TimeManager.hpp"
#include <thread>

namespace GameEngine
{
    std::chrono::steady_clock::time_point TimeManager::_lastTime =
        std::chrono::steady_clock::now();

    std::uint64_t TimeManager::getTimeMs()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch());
        return static_cast<std::uint64_t>(duration.count());
    }

    std::uint64_t TimeManager::getTimeNs()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch());
        return static_cast<std::uint64_t>(duration.count());
    }

    void TimeManager::delay(std::uint32_t milliseconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    float TimeManager::getDeltaTime()
    {
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            currentTime - _lastTime);
        _lastTime = currentTime;
        return static_cast<float>(duration.count()) / 1000000.0F; // Convert to seconds
    }
} // namespace GameEngine
